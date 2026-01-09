#pragma once

#include "types.h"
#include <unordered_map>
#include <unordered_set>
#include <deque>
#include <vector>
#include <string>
#include <chrono>
#include <cstdint>
#include <optional>

namespace hinlibs {

// Plain data store for Deliverable 1 (no behavior; just state).
// Database façade reads/writes these structures and returns copies.
struct MockDb {
    // ---- Users ----
    // Minimal records for D1 identification (username -> id/role).
    std::unordered_map<UserId, UserRecord> usersById;
    std::unordered_map<std::string, UserId> usernameToId;

    // ---- Items / Catalogue ----
    // ItemDetails contains title/author/format/status, etc.
    std::unordered_map<ItemId, ItemDetails> itemsById;
    std::vector<ItemId> itemInsertionOrder; // optional: preserve display order

    // ---- Loans ----
    // All active loans (D1). If you later track history, you can add an "inactive" store.
    std::unordered_map<LoanId, LoanSnapshot> loansById;
    std::unordered_map<PatronId, std::vector<LoanId>> loansByPatron; // active only
    std::unordered_map<ItemId, LoanId> activeLoanByItem;             // item -> active loan id

    // ---- Holds (FIFO per item) ----
    std::unordered_map<HoldId, HoldSnapshot> holdsById;
    std::unordered_map<PatronId, std::vector<HoldId>> holdsByPatron;
    std::unordered_map<ItemId, std::deque<HoldId>> holdQueueByItem;  // front() is next to fulfill

    // ---- Policy (D1 defaults) ----
    std::size_t maxActiveLoansPerPatron = 3;
    int loanPeriodDays = 14;

    // ---- ID generation helpers (used by Database to assign new IDs) ----
    uint64_t nextLoanSeq = 1;
    uint64_t nextHoldSeq = 1;

    // Optional: simple prefixes to make IDs readable in UI/debug logs
    std::string loanIdPrefix = "L";
    std::string holdIdPrefix = "H";
};

} // namespace hinlibs
