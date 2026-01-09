#include "patron.h"
#include "database.h"
#include "item.h"
#include "loan.h"

#include <algorithm>
#include <optional>
#include <utility>

namespace hinlibs {

namespace {

// Validate that this user exists and is a Patron.
inline std::optional<std::string> ValidatePatron(std::shared_ptr<Database> db,
                                                 const PatronId& id) {
    if (!db) return std::optional<std::string>{"No database handle"};
    auto rec = db->GetUserById(id);
    if (!rec) return std::optional<std::string>{"User not found"};
    if (rec->role != Role::Patron) return std::optional<std::string>{"User is not a patron"};
    return std::nullopt;
}

// Find an active loan (owned by patron) for a given item, if any.
inline std::optional<LoanSnapshot>
FindOwnedLoanForItem(std::shared_ptr<Database> db,
                     const PatronId& patronId,
                     const ItemId& itemId) {
    auto loans = db->GetPatronActiveLoans(patronId);
    for (const auto& l : loans) {
        if (l.itemId == itemId) return l;
    }
    return std::nullopt;
}

// Find an active hold (owned by patron) for a given item, if any.
inline std::optional<HoldSnapshot>
FindOwnedHoldForItem(std::shared_ptr<Database> db,
                     const PatronId& patronId,
                     const ItemId& itemId) {
    auto holds = db->GetPatronActiveHolds(patronId);
    for (const auto& h : holds) {
        if (h.itemId == itemId) return h;
    }
    return std::nullopt;
}

} // namespace

// -------------------------------
// Patron interface implementation
// -------------------------------
std::string Patron::getUsername() const {
    return db_->GetUserById(id_)->username;
}

std::vector<ItemDetails> Patron::browseCatalogue() const {
    std::vector<ItemDetails> out;

    // Verify patron exists
    if (auto err = ValidatePatron(db_, id_)) {
        // On identity failure, return empty; UI can show an error elsewhere if desired.
        return out;
    }

    // Per your comment: “Calls db_.GetAllItems(), then .details() on each, include only available”.
    // Using your current DB API: GetCatalogueItems() -> vector<Item>
    auto items = db_->GetCatalogueItems();
    out.reserve(items.size());

    for (auto& it : items) {
        auto det = it.details(); // optional<ItemDetails>
        if (det && det->status == ItemStatus::Available) {
            out.push_back(*det);  // copy snapshot
        }
    }
    return out;
}

std::vector<ItemDetails> Patron::browseCatalogueAll() const {
    std::vector<ItemDetails> out;

    // Reuse the same identity check helper we already have
    if (auto err = ValidatePatron(db_, id_)) {
        return out; // invalid user -> empty list
    }

    // Use existing DB: GetCatalogueItems() -> vector<Item>
    auto items = db_->GetCatalogueItems();
    out.reserve(items.size());

    // Pull full details for each item; push if present
    for (auto& it : items) {
        auto det = it.details(); // std::optional<ItemDetails>
        if (det) out.push_back(*det);
    }

    return out;
}


ValueResult<std::shared_ptr<Loan>> Patron::borrowItem(const ItemId& itemId) {
    ValueResult<std::shared_ptr<Loan>> res;

    if (auto err = ValidatePatron(db_, id_)) {
        res.ok = false; res.message = *err; return res;
    }

    auto current = db_->GetActiveLoanCount(id_);
    if (current >= db_->MaxActiveLoansPerPatron()) {
        res.ok = false; res.message = "Loan limit reached"; return res;
    }

    auto summary = db_->GetItemSummary(itemId);
    if (!summary) { res.ok = false; res.message = "Item not found"; return res; }
    if (summary->status != ItemStatus::Available) {
        res.ok = false; res.message = "Item is not available"; return res;
    }

    auto loanRes = db_->CheckoutItem(id_, itemId);
    if (!loanRes.ok || !loanRes.value) {
        res.ok = false; res.message = loanRes.message.empty() ? "Checkout failed" : loanRes.message;
        return res;
    }

    // Wrap snapshot id into a Loan interface
    res.ok = true;
    res.message.clear();
    res.value = std::make_shared<Loan>(db_, loanRes.value->id);
    return res;
}

OperationResult Patron::returnItem(const ItemId& itemId) {
    OperationResult r;

    // Verify patron exists
    if (auto err = ValidatePatron(db_, id_)) {
        r.ok = false; r.message = *err; return r;
    }

    // Must have an active loan for this item
    auto owned = FindOwnedLoanForItem(db_, id_, itemId);
    if (!owned) {
        r.ok = false; r.message = "No active loan for this item by this patron";
        return r;
    }

    // Atomic return
    auto dbres = db_->ReturnItem(id_, itemId);
    if (!dbres.ok) return dbres;

    r.ok = true; r.message.clear();
    return r;
}

ValueResult<std::size_t> Patron::placeHold(const ItemId& itemId) {
    ValueResult<std::size_t> out;

    // Verify patron exists
    if (auto err = ValidatePatron(db_, id_)) {
        out.ok = false; out.message = *err; return out;
    }

    // Item existence
    auto summary = db_->GetItemSummary(itemId);
    if (!summary) {
        out.ok = false; out.message = "Item not found";
        return out;
    }

    // Allowed only when item is NOT available
    if (summary->status == ItemStatus::Available) {
        out.ok = false; out.message = "Cannot place a hold on an available item";
        return out;
    }

    // Optional: avoid duplicate holds (DB.PlaceHold is idempotent per our impl, but we can fast-path)
    if (FindOwnedHoldForItem(db_, id_, itemId)) {
        // Return current position via DB to keep source of truth
        auto queue = db_->GetHoldQueueForItem(itemId);
        for (const auto& h : queue) {
            if (h.patronId == id_) {
                out.ok = true;
                out.value = h.queuePosition;
                out.message = "Hold already exists";
                return out;
            }
        }
        // Fallback if not found in queue map (should not happen):
        out.ok = false; out.message = "Hold state inconsistent";
        return out;
    }

    // Place hold
    auto posRes = db_->PlaceHold(id_, itemId);
    if (!posRes.ok || !posRes.value) {
        out.ok = false; out.message = posRes.message.empty() ? "Failed to place hold" : posRes.message;
        return out;
    }

    out.ok = true;
    out.value = *posRes.value;
    out.message.clear();
    return out;
}

OperationResult Patron::cancelHold(const ItemId& itemId) {
    OperationResult r;

    // Verify patron exists
    if (auto err = ValidatePatron(db_, id_)) {
        r.ok = false; r.message = *err; return r;
    }

    // Ensure there is a hold owned by this patron for this item
    if (!FindOwnedHoldForItem(db_, id_, itemId)) {
        r.ok = false; r.message = "No hold for this item by this patron";
        return r;
    }

    // Cancel via DB (also re-numbers queue)
    auto dbres = db_->CancelHold(id_, itemId);
    if (!dbres.ok) return dbres;

    r.ok = true; r.message.clear();
    return r;
}

AccountStatusView Patron::viewAccountStatus() const {
    // If patron invalid, return empty view (UI can choose to show an error elsewhere)
    if (auto err = ValidatePatron(db_, id_)) {
        return AccountStatusView{};
    }
    return db_->GetPatronAccountStatus(id_);
}

// ---- Non-state-altering helpers ----

std::size_t Patron::activeLoanCount() const {
    if (auto err = ValidatePatron(db_, id_)) return 0;
    return db_->GetActiveLoanCount(id_);
}

std::vector<LoanSnapshot> Patron::activeLoans() const {
    if (auto err = ValidatePatron(db_, id_)) return {};
    return db_->GetPatronActiveLoans(id_);
}

std::vector<HoldSnapshot> Patron::activeHolds() const {
    if (auto err = ValidatePatron(db_, id_)) return {};
    return db_->GetPatronActiveHolds(id_);
}

std::optional<hinlibs::ItemDetails> Patron::getItemDetails(ItemId itemId)
{
    return db_->GetItemDetails(itemId);
}

} // namespace hinlibs
