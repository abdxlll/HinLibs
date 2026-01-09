#include "hold.h"
#include "database.h"
#include "patron.h"
#include "item.h"

namespace hinlibs {

std::optional<HoldSnapshot> Hold::snapshot() const {
    if (!db_) return std::nullopt;
    return db_->GetHoldById(id_);
}

std::optional<HoldOverview> Hold::overview() const {
    if (!db_) return std::nullopt;

    auto snap = db_->GetHoldById(id_);
    if (!snap) return std::nullopt;

    auto p = std::make_shared<Patron>(db_, snap->patronId);
    auto it = std::make_shared<Item>(db_, snap->itemId);

    return HoldOverview{ std::move(p), std::move(it), snap->queuePosition };
}

} // namespace hinlibs
