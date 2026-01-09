#pragma once

#include "user.h"
#include "types.h"
#include <vector>
#include <memory>

namespace hinlibs {

class Loan;

class Patron : public User {
public:
    // Constructor
    Patron(std::shared_ptr<Database> db, PatronId id) : User(std::move(db), std::move(id)) {}
    Role role() const override { return Role::Patron; }

    // Functions
    std::vector<ItemDetails> browseCatalogue() const;
    std::vector<ItemDetails> browseCatalogueAll() const;
    ValueResult<std::shared_ptr<Loan>> borrowItem(const ItemId& itemId);
    OperationResult returnItem(const ItemId& itemId);
    ValueResult<std::size_t> placeHold(const ItemId& itemId);
    OperationResult cancelHold(const ItemId& itemId);
    AccountStatusView viewAccountStatus() const;
    std::size_t activeLoanCount() const;
    std::vector<LoanSnapshot> activeLoans() const;
    std::vector<HoldSnapshot> activeHolds() const;
    std::string getUsername() const;
    std::optional<hinlibs::ItemDetails> getItemDetails(ItemId itemId);

};

}
