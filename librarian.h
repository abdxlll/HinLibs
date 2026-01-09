#pragma once

#include "user.h"

namespace hinlibs {

class Librarian : public User {
public:
    // Constructor
    Librarian(std::shared_ptr<Database> db, LibrarianId id) : User(std::move(db), std::move(id)) {}
    Role role() const override { return Role::Librarian; }
    virtual ~Librarian() = default;
    std::string getUsername() const;

};

}
