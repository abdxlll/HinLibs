#include "item.h"
#include "database.h"
#include "types.h"

#include <utility>

namespace hinlibs {

// ---------- Item ----------

std::optional<ItemDetails> Item::details() const {
    if (!db_) return std::nullopt;
    return db_->GetItemDetails(id_);
}

std::optional<ItemSummary> Item::summary() const {
    if (!db_) return std::nullopt;
    return db_->GetItemSummary(id_);
}

// ---------- Book (no extra behavior yet) ----------
// using Item::Item;

// ---------- FictionBook (no extra behavior yet) ----------
// using Book::Book;

// ---------- NonFictionBook ----------

std::optional<std::string> NonFictionBook::dewey() const {
    auto det = details();
    if (!det) return std::nullopt;
    return det->deweyDecimal; // already an std::optional<std::string>
}

// ---------- Magazine / Movie / VideoGame (no extras yet) ----------
// using Item::Item;

} // namespace hinlibs
