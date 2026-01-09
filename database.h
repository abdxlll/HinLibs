#pragma once

#include "types.h"
#include "item.h"
#include <memory>
#include <vector>
#include <optional>
#include <QSqlDatabase>

namespace hinlibs {

class Database {
public:
    // ----- construction -----
    Database() = default;
    explicit Database(QSqlDatabase db);

    // ----- Session / Identification -----
    std::optional<UserRecord> FindUserByName(const std::string& username) const;
    std::optional<UserRecord> GetUserById(const UserId& id) const;

    // ----- Catalogue -----
    std::vector<Item> GetCatalogueItems() const;
    std::vector<ItemSummary> GetCatalogueSummaries() const;
    std::vector<ItemSummary> GetAvailableCatalogue() const;
    std::optional<ItemDetails> GetItemDetails(const ItemId& itemId) const;
    std::optional<ItemSummary> GetItemSummary(const ItemId& itemId) const;

    // 🔹 NEW: Add / Remove item for librarian (SQL-backed)
    ValueResult<ItemId> AddItem(const ItemDetails& detailsWithoutId);
    OperationResult RemoveItem(const ItemId& itemId);

    // ----- Borrowing pre-checks (read-only) -----
    std::size_t GetActiveLoanCount(const PatronId& patronId) const;
    bool IsItemAvailable(const ItemId& itemId) const;

    // ----- Borrow Item (atomic) -----
    ValueResult<LoanSnapshot> CheckoutItem(const PatronId& patronId,
                                           const ItemId& itemId);

    // ----- Return Item (atomic) -----
    OperationResult ReturnItem(const PatronId& patronId,
                               const ItemId& itemId);

    // ----- Holds (atomic) -----
    ValueResult<std::size_t> PlaceHold(const PatronId& patronId,
                                       const ItemId& itemId);
    OperationResult CancelHold(const PatronId& patronId, const ItemId& itemId);

    // ----- Account Status & queries -----
    AccountStatusView GetPatronAccountStatus(const PatronId& patronId) const;

    std::vector<LoanSnapshot> GetPatronActiveLoans(const PatronId& patronId) const;
    std::vector<HoldSnapshot> GetPatronActiveHolds(const PatronId& patronId) const;
    std::vector<HoldSnapshot> GetHoldQueueForItem(const ItemId& itemId) const;

    std::optional<LoanSnapshot> GetLoanById(const LoanId& loanId) const;
    std::optional<HoldSnapshot> GetHoldById(const HoldId& holdId) const;

    // ----- Policy -----
    std::size_t MaxActiveLoansPerPatron() const;
    int LoanPeriodDays() const;

private:
    QSqlDatabase db_;
};

} // namespace hinlibs
