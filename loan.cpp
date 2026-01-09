#include "loan.h"
#include "database.h"
#include "patron.h"
#include "item.h"

namespace hinlibs {

std::optional<LoanSnapshot> Loan::snapshot() const {
    if (!db_) return std::nullopt;
    return db_->GetLoanById(id_);
}

std::shared_ptr<Patron> Loan::patron() const {
    if (!db_) return nullptr;
    auto snap = db_->GetLoanById(id_);
    if (!snap) return nullptr;
    return std::make_shared<Patron>(db_, snap->patronId);
}

std::shared_ptr<Item> Loan::item() const {
    if (!db_) return nullptr;
    auto snap = db_->GetLoanById(id_);
    if (!snap) return nullptr;
    return std::make_shared<Item>(db_, snap->itemId);
}

} // namespace hinlibs
