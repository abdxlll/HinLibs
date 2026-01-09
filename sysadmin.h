#pragma once

#include "user.h"

namespace hinlibs {

class SysAdmin : public User {
public:
    // Constructor
    SysAdmin(std::shared_ptr<Database> db, SysAdminId id) : User(std::move(db), std::move(id)) {}
    Role role() const override { return Role::SysAdmin; }
    virtual ~SysAdmin() = default;
    std::string getUsername() const;
private:

};

}
