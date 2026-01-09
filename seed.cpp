#include "seed.h"
#include <sstream>
#include <iomanip>
#include <utility>

namespace hinlibs::seed {
namespace {

using namespace std::chrono;

inline std::chrono::system_clock::time_point MakeDate(int y, unsigned m, unsigned d) {
    std::tm tm = {};
    tm.tm_year = y - 1900;
    tm.tm_mon = m - 1;
    tm.tm_mday = d;
    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

inline std::string idWithPrefix(const char* prefix, int n, int width = 3) {
    std::ostringstream os;
    os << prefix << std::setw(width) << std::setfill('0') << n;
    return os.str();
}

void AddUser(MockDb& db, const UserId& id, std::string username, Role role) {
    UserRecord rec{ id, std::move(username), role };
    db.usersById[id] = rec;
    db.usernameToId[rec.username] = id;
}

void AddItem(MockDb& db, const ItemDetails& details) {
    db.itemsById[details.id] = details;
    db.itemInsertionOrder.push_back(details.id);
}

} // namespace

// ---- public API ----
std::shared_ptr<MockDb> CreateSeededMockDb() {
    auto store = std::make_shared<MockDb>();
    SeedMockData(*store);
    return store;
}

Database MakeDatabaseWithSeed() {
    Database db;
    db.SetMockStore(CreateSeededMockDb());
    return db;
}

void SeedMockData(MockDb& db) {
    // Ensure clean policy for D1
    db.maxActiveLoansPerPatron = 3;
    db.loanPeriodDays = 14;

    // ---------- Users (7 total: 5 patrons, 1 librarian, 1 sysadmin) ----------
    // IDs U001..U007
    AddUser(db, "U001", "nikolai", Role::Patron);
    AddUser(db, "U002", "mike", Role::Patron);
    AddUser(db, "U003", "oyin", Role::Patron);
    AddUser(db, "U004", "abdulrahman", Role::Patron);
    AddUser(db, "U005", "mohammad", Role::Patron);
    AddUser(db, "U006", "lib", Role::Librarian);
    AddUser(db, "U007", "admin", Role::SysAdmin);

    // ---------- Items (20 total) ----------
    int idx = 1;
    auto nextItemId = [&]() { return idWithPrefix("I", idx++); };

    // ---- Fiction books (5) ---- (id - title - author/creator - format - status - publication year - dewey - rating - publication date)
    AddItem(db, ItemDetails { nextItemId(), "Crime and Punishment", "Fyodor Dostoevsky", ItemFormat::Book, ItemStatus::Available, 1866,
                std::nullopt, std::nullopt, std::string{"Classic"},  std::nullopt, std::nullopt, std::nullopt });

    AddItem(db, ItemDetails { nextItemId(), "White Nights", "Fyodor Dostoevsky", ItemFormat::Book, ItemStatus::Available, 1848,
                std::nullopt, std::nullopt, std::string{"Classic"},  std::nullopt, std::nullopt, std::nullopt });

    AddItem(db, ItemDetails{ nextItemId(), "The Hobbit", "J. R. R. Tolkien",
        ItemFormat::Book, ItemStatus::Available, 1937, std::nullopt,
        std::nullopt, std::string{"Fantasy"}, std::nullopt, std::nullopt, std::nullopt });

    AddItem(db, ItemDetails{ nextItemId(), "Pride and Prejudice", "Jane Austen",
        ItemFormat::Book, ItemStatus::Available, 1813, std::nullopt,
        std::nullopt, std::string{"Romance"}, std::nullopt, std::nullopt, std::nullopt });

    AddItem(db, ItemDetails{ nextItemId(), "To Kill a Mockingbird", "Harper Lee",
        ItemFormat::Book, ItemStatus::Available, 1960, std::nullopt,
        std::nullopt, std::string{"Classic"}, std::nullopt, std::nullopt, std::nullopt });

    // ---- Non-fiction books (5; must include Dewey Decimal) ----
    AddItem(db, ItemDetails{ nextItemId(), "Sapiens", "Yuval Noah Harari",
        ItemFormat::Book, ItemStatus::Available, 2011, std::nullopt,
        std::string{"001.94"}, std::nullopt, std::nullopt, std::nullopt, std::nullopt });

    AddItem(db, ItemDetails{ nextItemId(), "A Brief History of Time", "Stephen Hawking",
        ItemFormat::Book, ItemStatus::Available, 1988, std::nullopt,
        std::string{"523.10"}, std::nullopt, std::nullopt, std::nullopt, std::nullopt });

    AddItem(db, ItemDetails{ nextItemId(), "The Selfish Gene", "Richard Dawkins",
        ItemFormat::Book, ItemStatus::Available, 1976, std::nullopt,
        std::string{"576.50"}, std::nullopt, std::nullopt, std::nullopt, std::nullopt });

    AddItem(db, ItemDetails{ nextItemId(), "Guns, Germs, and Steel", "Jared Diamond",
        ItemFormat::Book, ItemStatus::Available, 1997, std::nullopt,
        std::string{"303.48"}, std::nullopt, std::nullopt, std::nullopt, std::nullopt });

    AddItem(db, ItemDetails{ nextItemId(), "Thinking, Fast and Slow", "Daniel Kahneman",
        ItemFormat::Book, ItemStatus::Available, 2011, std::nullopt,
        std::string{"153.42"}, std::nullopt, std::nullopt, std::nullopt, std::nullopt });

    // ---- Magazines (3; must include issue number + publication date) ----
    AddItem(db, ItemDetails{ nextItemId(), "The worm Runner's Digest", "Dr. James V. McConnell",
        ItemFormat::Magazine, ItemStatus::Available, 1967, std::nullopt,
        std::nullopt, std::nullopt, std::nullopt,
        std::string{"1967-01"}, MakeDate(1967,1,15) });

    AddItem(db, ItemDetails{ nextItemId(), "Weekly World News", "Spy Cat LLC",
        ItemFormat::Magazine, ItemStatus::Available, 2025, std::nullopt,
        std::nullopt, std::nullopt, std::nullopt,
        std::string{"2025-02-07"}, MakeDate(2025,2,7) });

    AddItem(db, ItemDetails{ nextItemId(), "Meatpaper", "Amy Standen & Sasha Wizansky",
        ItemFormat::Magazine, ItemStatus::Available, 2025, std::nullopt,
        std::nullopt, std::nullopt, std::nullopt,
        std::string{"2025-03"}, MakeDate(2025,3,1) });

    // ---- Movies (3; include genre + rating) ---- (includes "Her" and "Dogville")
    AddItem(db, ItemDetails{ nextItemId(), "Her", "Spike Jonze",
        ItemFormat::Movie, ItemStatus::Available, 2013, std::nullopt,
        std::nullopt, std::string{"Drama/Romance"}, std::string{"R"},
        std::nullopt, std::nullopt });

    AddItem(db, ItemDetails{ nextItemId(), "Dogville", "Lars von Trier",
        ItemFormat::Movie, ItemStatus::Available, 2003, std::nullopt,
        std::nullopt, std::string{"Drama"}, std::string{"R"},
        std::nullopt, std::nullopt });

    AddItem(db, ItemDetails{ nextItemId(), "Arrival", "Denis Villeneuve",
        ItemFormat::Movie, ItemStatus::Available, 2016, std::nullopt,
        std::nullopt, std::string{"Sci-Fi"}, std::string{"PG-13"},
        std::nullopt, std::nullopt });

    // ---- Video games (4; include genre + rating) ----
    // includes "Detroit: Become Human", "Angry Birds", and "Plants vs. Zombies" (ZVP)
    AddItem(db, ItemDetails{ nextItemId(), "Detroit: Become Human", "Quantic Dream",
        ItemFormat::VideoGame, ItemStatus::Available, 2018, std::nullopt,
        std::nullopt, std::string{"Adventure"}, std::string{"M"},
        std::nullopt, std::nullopt });

    AddItem(db, ItemDetails{ nextItemId(), "Angry Birds", "Rovio",
        ItemFormat::VideoGame, ItemStatus::Available, 2009, std::nullopt,
        std::nullopt, std::string{"Puzzle"}, std::string{"E"},
        std::nullopt, std::nullopt });

    AddItem(db, ItemDetails{ nextItemId(), "Plants vs. Zombies", "PopCap Games",
        ItemFormat::VideoGame, ItemStatus::Available, 2009, std::nullopt,
        std::nullopt, std::string{"Tower Defense"}, std::string{"E10+"},
        std::nullopt, std::nullopt });

    AddItem(db, ItemDetails{ nextItemId(), "Head Soccer", "D&D Dream Corp",
        ItemFormat::VideoGame, ItemStatus::Available, 2017, std::nullopt,
        std::nullopt, std::string{"Sports"}, std::string{"E10+"},
        std::nullopt, std::nullopt });

    // No loans/holds seeded for D1 start; everything is Available.
    db.loansById.clear();
    db.loansByPatron.clear();
    db.activeLoanByItem.clear();
    db.holdsById.clear();
    db.holdsByPatron.clear();
    db.holdQueueByItem.clear();

    // Reset sequences (useful if you later create loans/holds)
    db.nextLoanSeq = 1;
    db.nextHoldSeq = 1;
}

} // namespace hinlibs::seed
