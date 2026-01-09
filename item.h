#pragma once

#include "types.h"
#include <memory>

namespace hinlibs {

class Database;


class Item {
public:
    // Constructor
    Item(std::shared_ptr<Database> db, ItemId id) : db_(std::move(db)), id_(std::move(id)) {}
    virtual ~Item() = default;

    // Functions
    const ItemId& id() const noexcept { return id_; }
    std::optional<ItemDetails> details() const;
    std::optional<ItemSummary> summary() const; // equivalent to details() projected

protected:
    std::shared_ptr<Database> db_;
    ItemId id_;
};

class Book : public Item {
public:
    using Item::Item;
};

class FictionBook : public Book {
public:
    using Book::Book;
};

class NonFictionBook : public Book {
public:
    using Book::Book;
    std::optional<std::string> dewey() const;
};

class Magazine : public Item {
public:
    using Item::Item;
};

class Movie : public Item {
public:
    using Item::Item;
};

class VideoGame : public Item {
public:
    using Item::Item;
};


}
