#pragma once

#include "types.h"
#include <memory>

namespace hinlibs {

class Database;
class Patron;
class Item;

class Hold {
public:
    // Constructor
    Hold(std::shared_ptr<Database> db, HoldId id) : db_(std::move(db)), id_(std::move(id)) {}
    ~Hold() = default;

    // Functions
    const HoldId& id() const noexcept { return id_; }
    std::optional<HoldSnapshot> snapshot() const;
    std::optional<HoldOverview> overview() const;

private:
    std::shared_ptr<Database> db_;
    HoldId id_;
};

}
