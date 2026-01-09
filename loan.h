#pragma once

#include "types.h"
#include <memory>

namespace hinlibs {

class Database;
class Patron;
class Item;

class Loan {
public:
    // Constructor
    Loan(std::shared_ptr<Database> db, LoanId id) : db_(std::move(db)), id_(std::move(id)) {}
    ~Loan() = default;

    // Functions
    const LoanId& id() const noexcept { return id_; }
    std::optional<LoanSnapshot> snapshot() const;
    std::shared_ptr<Patron> patron() const;
    std::shared_ptr<Item>   item()   const;

private:
    std::shared_ptr<Database> db_;
    LoanId id_;
};

}
