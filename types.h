#pragma once

#include <string>
#include <QString>
#include <vector>
#include <memory>
#include <optional>
#include <chrono>
#include <cstddef>
#include <cstdint>


namespace hinlibs {

// ---------- ID aliases ----------
using UserId  = std::string;
using PatronId = UserId;
using LibrarianId = UserId;
using SysAdminId = UserId;

using ItemId  = std::string;
using LoanId  = std::string;
using HoldId  = std::string;

class Patron;
class Librarian;
class SysAdmin;
class Item;
class Loan;

// ---------- Roles & formats ----------
enum class Role {
    Patron,
    Librarian,
    SysAdmin
};

enum class ItemFormat {
    Book,
    Magazine,
    Movie,
    VideoGame
};

enum class ItemStatus {
    Available,
    CheckedOut
};

// ---------- Core record “snapshots” returned by Database (always by value) ----------
struct UserRecord {
    UserId   id;
    std::string username;
    Role     role;
};

struct ItemSummary {
    ItemId      id;
    std::string title;
    std::string authorOrCreator;
    ItemFormat  format;
    ItemStatus  status;
};

struct ItemDetails : ItemSummary {
    // Optional, format-specific extras (not all will be populated for all formats)
    std::optional<int> publicationYear;
    std::optional<std::string> isbn;          // when applicable
    std::optional<std::string> deweyDecimal;  // for non-fiction book
    std::optional<std::string> genre;         // movies & video games (or books if desired)
    std::optional<std::string> rating;        // e.g., MPAA/ESRB/etc.
    std::optional<std::string> issueNumber;   // e.g., "2025-03", "Vol. 238 No. 1"
    std::optional<std::chrono::system_clock::time_point> publicationDate;
};

struct LoanSnapshot {
    LoanId      id;
    PatronId    patronId;
    ItemId      itemId;
    std::chrono::system_clock::time_point checkoutDate;
    std::chrono::system_clock::time_point dueDate;
};

struct LoanSnapshotWithItem {
    LoanId      id;
    PatronId    patronId;
    ItemId      itemId;
    std::chrono::system_clock::time_point checkoutDate;
    std::chrono::system_clock::time_point dueDate;
};

struct HoldSnapshot {
    HoldId      id;
    PatronId    patronId;
    ItemId      itemId;
    std::size_t queuePosition;   // position at time of retrieval
};

struct HoldOverview {
    std::shared_ptr<Patron> patron; // interface wrapper
    std::shared_ptr<Item>   item;   // interface wrapper
    std::size_t queuePosition;
};

// ---------- Account-status “views” for UI ----------
struct LoanStatusView {
    std::string itemTitle;
    std::chrono::system_clock::time_point dueDate;
    int daysRemaining;  // signed int so negative can represent overdue if needed later
};

struct HoldStatusView {
    std::string itemTitle;
    std::size_t queuePosition;
};

struct AccountStatusView {
    std::vector<LoanStatusView> loans;
    std::vector<HoldStatusView> holds;
};

// ---------- Generic result carriers (no exceptions required) ----------
struct OperationResult {
    bool ok = false;
    std::string message; // non-empty on failure; optional human-readable info on success
};

template <typename T>
struct ValueResult {
    bool ok = false;
    std::optional<T> value; // engaged when ok==true
    std::string message;    // diagnostic if ok==false
};

} // namespace hinlibs
