#pragma once

#include "types.h"
#include <memory>

namespace hinlibs {

class Database;

class User {
public:
    virtual ~User() = default;

    const UserId& id() const noexcept { return id_; }

    virtual Role role() const = 0;

protected:
    User(std::shared_ptr<Database> db, UserId id) : db_(std::move(db)), id_(std::move(id)) {}

    std::shared_ptr<Database> db_;
    UserId id_;
};

} // namespace hinlibs
