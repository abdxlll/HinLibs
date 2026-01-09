#include "database.h"
#include "types.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QDebug>

namespace hinlibs {

// Helpers to convert enums and time
static hinlibs::Role roleFromString(const QString& s) {
    if (s == "Patron") return hinlibs::Role::Patron;
    if (s == "Librarian") return hinlibs::Role::Librarian;
    return hinlibs::Role::SysAdmin;
}
static QString roleToString(hinlibs::Role r) {
    switch (r) {
        case hinlibs::Role::Patron: return "Patron";
        case hinlibs::Role::Librarian: return "Librarian";
        case hinlibs::Role::SysAdmin: return "SysAdmin";
    }
    return "Patron";
}
static hinlibs::ItemFormat formatFromString(const QString& s) {
    if (s == "Book") return hinlibs::ItemFormat::Book;
    if (s == "Magazine") return hinlibs::ItemFormat::Magazine;
    if (s == "Movie") return hinlibs::ItemFormat::Movie;
    return hinlibs::ItemFormat::VideoGame;
}
static QString formatToString(hinlibs::ItemFormat f) {
    switch (f) {
        case hinlibs::ItemFormat::Book: return "Book";
        case hinlibs::ItemFormat::Magazine: return "Magazine";
        case hinlibs::ItemFormat::Movie: return "Movie";
        case hinlibs::ItemFormat::VideoGame: return "VideoGame";
    }
    return "Book";
}
static hinlibs::ItemStatus statusFromString(const QString& s) {
    return (s == "Available") ? hinlibs::ItemStatus::Available : hinlibs::ItemStatus::CheckedOut;
}
static QString statusToString(hinlibs::ItemStatus st) {
    return st == hinlibs::ItemStatus::Available ? "Available" : "CheckedOut";
}
static QString toIso(const std::chrono::system_clock::time_point& tp) {
    auto secs = std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count();
    return QDateTime::fromSecsSinceEpoch(secs, Qt::UTC).toString(Qt::ISODate);
}
static std::chrono::system_clock::time_point fromIso(const QString& iso) {
    auto dt = QDateTime::fromString(iso, Qt::ISODate);
    return std::chrono::system_clock::time_point{std::chrono::seconds(dt.toSecsSinceEpoch())};
}

// ----- construction -----
Database::Database(QSqlDatabase db) : db_(db) {}

// ----- Session / Identification -----
std::optional<UserRecord> Database::FindUserByName(const std::string& username) const {
    QSqlQuery q(db_);
    q.prepare("SELECT id, username, role FROM users WHERE LOWER(username) = LOWER(?)");
    q.addBindValue(QString::fromStdString(username));
    if (!q.exec() || !q.next()) {
        qDebug() << "1";
        return std::nullopt;
    }
    UserRecord rec;
    rec.id = q.value(0).toString().toStdString();
    rec.username = q.value(1).toString().toStdString();
    rec.role = roleFromString(q.value(2).toString());
    return rec;
}

std::optional<UserRecord> Database::GetUserById(const UserId& id) const {
    QSqlQuery q(db_);
    q.prepare("SELECT id, username, role FROM users WHERE id=?");
    q.addBindValue(QString::fromStdString(id));
    if (!q.exec() || !q.next()) return std::nullopt;
    UserRecord rec;
    rec.id = q.value(0).toString().toStdString();
    rec.username = q.value(1).toString().toStdString();
    rec.role = roleFromString(q.value(2).toString());
    return rec;
}

// ----- Catalogue -----
std::vector<Item> Database::GetCatalogueItems() const {
    std::vector<Item> out;
    QSqlQuery q("SELECT id FROM items ORDER BY title ASC", db_);
    while (q.next()) {
        out.emplace_back(std::shared_ptr<Database>(const_cast<Database*>(this), [](Database*){}),
                         q.value(0).toString().toStdString());
    }
    return out;
}

std::vector<ItemSummary> Database::GetCatalogueSummaries() const {
    std::vector<ItemSummary> out;
    QSqlQuery q("SELECT id, title, authorOrCreator, format, status FROM items ORDER BY title ASC", db_);
    while (q.next()) {
        ItemSummary s;
        s.id = q.value(0).toString().toStdString();
        s.title = q.value(1).toString().toStdString();
        s.authorOrCreator = q.value(2).toString().toStdString();
        s.format = formatFromString(q.value(3).toString());
        s.status = statusFromString(q.value(4).toString());
        out.push_back(std::move(s));
    }
    return out;
}

std::vector<ItemSummary> Database::GetAvailableCatalogue() const {
    std::vector<ItemSummary> out;
    QSqlQuery q("SELECT id, title, authorOrCreator, format, status FROM items WHERE status='Available' ORDER BY title ASC", db_);
    while (q.next()) {
        ItemSummary s;
        s.id = q.value(0).toString().toStdString();
        s.title = q.value(1).toString().toStdString();
        s.authorOrCreator = q.value(2).toString().toStdString();
        s.format = formatFromString(q.value(3).toString());
        s.status = statusFromString(q.value(4).toString());
        out.push_back(std::move(s));
    }
    return out;
}

std::optional<ItemDetails> Database::GetItemDetails(const ItemId& itemId) const {
    QSqlQuery q(db_);
    q.prepare("SELECT id, title, authorOrCreator, format, status, publicationYear, isbn, deweyDecimal, genre, rating, issueNumber, publicationDate FROM items WHERE id=?");
    q.addBindValue(QString::fromStdString(itemId));
    if (!q.exec() || !q.next()) return std::nullopt;
    ItemDetails d;
    d.id = q.value(0).toString().toStdString();
    d.title = q.value(1).toString().toStdString();
    d.authorOrCreator = q.value(2).toString().toStdString();
    d.format = formatFromString(q.value(3).toString());
    d.status = statusFromString(q.value(4).toString());
    if (!q.value(5).isNull()) d.publicationYear = q.value(5).toInt();
    if (!q.value(6).isNull()) d.isbn = q.value(6).toString().toStdString();
    if (!q.value(7).isNull()) d.deweyDecimal = q.value(7).toString().toStdString();
    if (!q.value(8).isNull()) d.genre = q.value(8).toString().toStdString();
    if (!q.value(9).isNull()) d.rating = q.value(9).toString().toStdString();
    if (!q.value(10).isNull()) d.issueNumber = q.value(10).toString().toStdString();
    if (!q.value(11).isNull()) d.publicationDate = fromIso(q.value(11).toString());
    return d;
}

std::optional<ItemSummary> Database::GetItemSummary(const ItemId& itemId) const {
    QSqlQuery q(db_);
    q.prepare("SELECT id, title, authorOrCreator, format, status FROM items WHERE id=?");
    q.addBindValue(QString::fromStdString(itemId));
    if (!q.exec() || !q.next()) return std::nullopt;
    ItemSummary s;
    s.id = q.value(0).toString().toStdString();
    s.title = q.value(1).toString().toStdString();
    s.authorOrCreator = q.value(2).toString().toStdString();
    s.format = formatFromString(q.value(3).toString());
    s.status = statusFromString(q.value(4).toString());
    return s;
}

// ----- Borrowing pre-checks -----
std::size_t Database::GetActiveLoanCount(const PatronId& patronId) const {
    QSqlQuery q(db_);
    q.prepare("SELECT COUNT(*) FROM loans WHERE patronId=?");
    q.addBindValue(QString::fromStdString(patronId));
    if (!q.exec() || !q.next()) return 0;
    return q.value(0).toInt();
}

bool Database::IsItemAvailable(const ItemId& itemId) const {
    QSqlQuery q(db_);
    q.prepare("SELECT status FROM items WHERE id=?");
    q.addBindValue(QString::fromStdString(itemId));
    if (!q.exec() || !q.next()) return false;
    return q.value(0).toString() == "Available";
}

// ----- Borrow Item -----
ValueResult<LoanSnapshot> Database::CheckoutItem(const PatronId& patronId, const ItemId& itemId) {
    ValueResult<LoanSnapshot> res;

    if (GetActiveLoanCount(patronId) >= MaxActiveLoansPerPatron()) {
        res.ok = false; res.message = "Loan limit reached"; return res;
    }
    if (!IsItemAvailable(itemId)) {
        res.ok = false; res.message = "Item not available"; return res;
    }

    db_.transaction();
    auto now = std::chrono::system_clock::now();
    auto due = now + std::chrono::hours(24 * LoanPeriodDays());
    QString loanId = QString("L%1").arg(QDateTime::currentSecsSinceEpoch());

    QSqlQuery ins(db_);
    ins.prepare("INSERT INTO loans (id, patronId, itemId, checkoutDate, dueDate) VALUES (?, ?, ?, ?, ?)");
    ins.addBindValue(loanId);
    ins.addBindValue(QString::fromStdString(patronId));
    ins.addBindValue(QString::fromStdString(itemId));
    ins.addBindValue(toIso(now));
    ins.addBindValue(toIso(due));
    if (!ins.exec()) { db_.rollback(); res.ok=false; res.message="Insert failed"; return res; }

    QSqlQuery upd(db_);
    upd.prepare("UPDATE items SET status='CheckedOut' WHERE id=?");
    upd.addBindValue(QString::fromStdString(itemId));
    if (!upd.exec()) { db_.rollback(); res.ok=false; res.message="Update failed"; return res; }

    db_.commit();
    res.ok = true;
    res.value = LoanSnapshot{ loanId.toStdString(), patronId, itemId, now, due };
    return res;
}

// ----- Return Item -----
OperationResult Database::ReturnItem(const PatronId& patronId, const ItemId& itemId) {
    OperationResult r;
    db_.transaction();

    QSqlQuery del(db_);
    del.prepare("DELETE FROM loans WHERE patronId=? AND itemId=?");
    del.addBindValue(QString::fromStdString(patronId));
    del.addBindValue(QString::fromStdString(itemId));
    if (!del.exec()) { db_.rollback(); r.ok=false; r.message="Delete failed"; return r; }

    QSqlQuery upd(db_);
    upd.prepare("UPDATE items SET status='Available' WHERE id=?");
    upd.addBindValue(QString::fromStdString(itemId));
    if (!upd.exec()) { db_.rollback(); r.ok=false; r.message="Update failed"; return r; }

    db_.commit();
    r.ok = true;
    return r;
}

// NEW: generate next ID like "I021" based on existing item IDs in the DB
static ItemId generateNewItemId(const QSqlDatabase& db) {
    QSqlQuery q(db);

    // Find the maximum numeric part of IDs that look like 'I###'
    if (!q.exec("SELECT MAX(CAST(SUBSTR(id, 2) AS INTEGER)) FROM items WHERE id LIKE 'I%'")) {
        qDebug() << "generateNewItemId: query failed:" << q.lastError().text();
        // Fallback: start at I001
        return "I001";
    }

    int maxNum = 0;
    if (q.next() && !q.value(0).isNull()) {
        maxNum = q.value(0).toInt();
    }

    int next = maxNum + 1;
    QString id = QString("I%1").arg(next, 3, 10, QLatin1Char('0'));  // I001, I002, ...
    return id.toStdString();
}

ValueResult<ItemId> Database::AddItem(const ItemDetails& detailsWithoutId) {
    ValueResult<ItemId> res;

    if (!db_.isOpen()) {
        res.ok = false;
        res.message = "Database not open";
        return res;
    }

    // 1) Generate new ID
    ItemDetails d = detailsWithoutId;
    d.id = generateNewItemId(db_);

    // 2) Normalize status: default to Available unless explicitly Available/CheckedOut
    if (d.status != ItemStatus::Available && d.status != ItemStatus::CheckedOut) {
        d.status = ItemStatus::Available;
    }

    QSqlQuery q(db_);
    q.prepare(R"(
        INSERT INTO items
        (id, title, authorOrCreator, format, status,
         publicationYear, isbn, deweyDecimal, genre,
         rating, issueNumber, publicationDate)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    )");

    q.addBindValue(QString::fromStdString(d.id));
    q.addBindValue(QString::fromStdString(d.title));
    q.addBindValue(QString::fromStdString(d.authorOrCreator));
    q.addBindValue(formatToString(d.format));   // your existing helper
    q.addBindValue(statusToString(d.status));   // your existing helper

    // Optional fields – bind NULL if not set
    if (d.publicationYear.has_value())
        q.addBindValue(*d.publicationYear);
    else
        q.addBindValue(QVariant(QVariant::Int));

    if (d.isbn.has_value())
        q.addBindValue(QString::fromStdString(*d.isbn));
    else
        q.addBindValue(QVariant(QVariant::String));

    if (d.deweyDecimal.has_value())
        q.addBindValue(QString::fromStdString(*d.deweyDecimal));
    else
        q.addBindValue(QVariant(QVariant::String));

    if (d.genre.has_value())
        q.addBindValue(QString::fromStdString(*d.genre));
    else
        q.addBindValue(QVariant(QVariant::String));

    if (d.rating.has_value())
        q.addBindValue(QString::fromStdString(*d.rating));
    else
        q.addBindValue(QVariant(QVariant::String));

    if (d.issueNumber.has_value())
        q.addBindValue(QString::fromStdString(*d.issueNumber));
    else
        q.addBindValue(QVariant(QVariant::String));

    if (d.publicationDate.has_value())
        q.addBindValue(toIso(*d.publicationDate));  // you already have toIso(...)
    else
        q.addBindValue(QVariant(QVariant::String));

    if (!q.exec()) {
        qDebug() << "AddItem INSERT failed:" << q.lastError().text();
        res.ok = false;
        res.message = "Insert failed";
        return res;
    }

    res.ok = true;
    res.value = d.id;
    res.message.clear();
    return res;
}

OperationResult Database::RemoveItem(const ItemId& itemId) {
    OperationResult r;

    if (!db_.isOpen()) {
        r.ok = false;
        r.message = "Database not open";
        return r;
    }

    QString qItemId = QString::fromStdString(itemId);

    // 1) Check item exists and status
    QSqlQuery qItem(db_);
    qItem.prepare("SELECT status FROM items WHERE id=?");
    qItem.addBindValue(qItemId);
    if (!qItem.exec() || !qItem.next()) {
        r.ok = false;
        r.message = "Item not found";
        return r;
    }

    const QString status = qItem.value(0).toString();
    if (status == "CheckedOut") {
        r.ok = false;
        r.message = "Cannot remove an item that is currently checked out";
        return r;
    }

    // 2) Disallow removal if there are active holds for this item
    QSqlQuery qHolds(db_);
    qHolds.prepare("SELECT COUNT(*) FROM holds WHERE itemId=?");
    qHolds.addBindValue(qItemId);
    if (!qHolds.exec() || !qHolds.next()) {
        r.ok = false;
        r.message = "Hold check failed";
        return r;
    }
    if (qHolds.value(0).toInt() > 0) {
        r.ok = false;
        r.message = "Cannot remove an item that has active holds";
        return r;
    }

    // Extra safety: ensure no loan rows exist for this item
    QSqlQuery qLoans(db_);
    qLoans.prepare("SELECT COUNT(*) FROM loans WHERE itemId=?");
    qLoans.addBindValue(qItemId);
    if (!qLoans.exec() || !qLoans.next()) {
        r.ok = false;
        r.message = "Loan check failed";
        return r;
    }
    if (qLoans.value(0).toInt() > 0) {
        r.ok = false;
        r.message = "Cannot remove an item that is currently checked out";
        return r;
    }

    // 3) Safe to delete the item
    db_.transaction();
    QSqlQuery del(db_);
    del.prepare("DELETE FROM items WHERE id=?");
    del.addBindValue(qItemId);

    if (!del.exec()) {
        qDebug() << "RemoveItem DELETE failed:" << del.lastError().text();
        db_.rollback();
        r.ok = false;
        r.message = "Delete failed";
        return r;
    }

    db_.commit();
    r.ok = true;
    r.message.clear();
    return r;
}


// ----- Holds -----
ValueResult<std::size_t> Database::PlaceHold(const PatronId& patronId, const ItemId& itemId) {
    ValueResult<std::size_t> res;
    if (IsItemAvailable(itemId)) {
        res.ok=false; res.message="Cannot place hold on available item"; return res;
    }

    QSqlQuery dup(db_);
    dup.prepare("SELECT COUNT(*) FROM holds WHERE patronId=? AND itemId=?");
    dup.addBindValue(QString::fromStdString(patronId));
    dup.addBindValue(QString::fromStdString(itemId));
    if (!dup.exec() || !dup.next()) { res.ok=false; res.message="Hold check failed"; return res; }
    if (dup.value(0).toInt() > 0) { res.ok=false; res.message="Hold already exists"; return res; }

    QSqlQuery posq(db_);
    posq.prepare("SELECT IFNULL(MAX(queuePosition),0)+1 FROM holds WHERE itemId=?");
    posq.addBindValue(QString::fromStdString(itemId));
    if (!posq.exec() || !posq.next()) { res.ok=false; res.message="Queue calc failed"; return res; }
    int pos = posq.value(0).toInt();

    QString holdId = QString("H%1").arg(QDateTime::currentSecsSinceEpoch());
    QSqlQuery ins(db_);
    ins.prepare("INSERT INTO holds (id, patronId, itemId, queuePosition) VALUES (?, ?, ?, ?)");
    ins.addBindValue(holdId);
    ins.addBindValue(QString::fromStdString(patronId));
    ins.addBindValue(QString::fromStdString(itemId));
    ins.addBindValue(pos);
    if (!ins.exec()) { res.ok=false; res.message="Insert hold failed"; return res; }

    res.ok = true;
    res.value = static_cast<std::size_t>(pos);
    return res;
}

OperationResult Database::CancelHold(const PatronId& patronId, const ItemId& itemId) {
    OperationResult r;
    db_.transaction();

    QSqlQuery sel(db_);
    sel.prepare("SELECT id FROM holds WHERE patronId=? AND itemId=?");
    sel.addBindValue(QString::fromStdString(patronId));
    sel.addBindValue(QString::fromStdString(itemId));
    if (!sel.exec() || !sel.next()) { db_.rollback(); r.ok=false; r.message="Hold not found"; return r; }
    QString holdId = sel.value(0).toString();

    QSqlQuery del(db_);
    del.prepare("DELETE FROM holds WHERE id=?");
    del.addBindValue(holdId);
    if (!del.exec()) { db_.rollback(); r.ok=false; r.message="Delete failed"; return r; }

    // Re-number queue
    QSqlQuery q(db_);
    q.prepare("SELECT id FROM holds WHERE itemId=? ORDER BY queuePosition ASC");
    q.addBindValue(QString::fromStdString(itemId));
    if (!q.exec()) { db_.rollback(); r.ok=false; r.message="Queue select failed"; return r; }
    int pos=1;
    while (q.next()) {
        QSqlQuery upd(db_);
        upd.prepare("UPDATE holds SET queuePosition=? WHERE id=?");
        upd.addBindValue(pos++);
        upd.addBindValue(q.value(0));
        if (!upd.exec()) { db_.rollback(); r.ok=false; r.message="Queue update failed"; return r; }
    }

    db_.commit();
    r.ok = true;
    return r;
}

// ----- Policy -----
std::size_t Database::MaxActiveLoansPerPatron() const {
    QSqlQuery q("SELECT maxActiveLoansPerPatron FROM policy", db_);
    if (q.next()) return q.value(0).toInt();
    return 3;
}

int Database::LoanPeriodDays() const {
    QSqlQuery q("SELECT loanPeriodDays FROM policy", db_);
    if (q.next()) return q.value(0).toInt();
    return 14; // default
}

// ----- Account Status -----
AccountStatusView Database::GetPatronAccountStatus(const PatronId& patronId) const {
    AccountStatusView view;
    auto now = std::chrono::system_clock::now();

    // Loans
    QSqlQuery q1(db_);
    q1.prepare("SELECT i.title, l.dueDate FROM loans l JOIN items i ON l.itemId=i.id WHERE l.patronId=?");
    q1.addBindValue(QString::fromStdString(patronId));
    if (q1.exec()) {
        while (q1.next()) {
            LoanStatusView lv;
            lv.itemTitle = q1.value(0).toString().toStdString();
            auto due = fromIso(q1.value(1).toString());
            lv.dueDate = due;
            lv.daysRemaining = static_cast<int>(
                std::chrono::duration_cast<std::chrono::hours>(due - now).count() / 24
            );
            view.loans.push_back(lv);
        }
    }

    // Holds
    QSqlQuery q2(db_);
    q2.prepare("SELECT i.title, h.queuePosition FROM holds h JOIN items i ON h.itemId=i.id WHERE h.patronId=? ORDER BY h.queuePosition ASC");
    q2.addBindValue(QString::fromStdString(patronId));
    if (q2.exec()) {
        while (q2.next()) {
            HoldStatusView hv;
            hv.itemTitle = q2.value(0).toString().toStdString();
            hv.queuePosition = static_cast<std::size_t>(q2.value(1).toInt());
            view.holds.push_back(hv);
        }
    }

    return view;
}

// ----- Fine-grained APIs -----
std::vector<LoanSnapshot> Database::GetPatronActiveLoans(const PatronId& patronId) const {
    std::vector<LoanSnapshot> out;
    QSqlQuery q(db_);
    q.prepare("SELECT id, itemId, checkoutDate, dueDate FROM loans WHERE patronId=?");
    q.addBindValue(QString::fromStdString(patronId));
    if (q.exec()) {
        while (q.next()) {
            LoanSnapshot snap;
            snap.id = q.value(0).toString().toStdString();
            snap.patronId = patronId;
            snap.itemId = q.value(1).toString().toStdString();
            snap.checkoutDate = fromIso(q.value(2).toString());
            snap.dueDate = fromIso(q.value(3).toString());
            out.push_back(snap);
        }
    }
    return out;
}

std::vector<HoldSnapshot> Database::GetPatronActiveHolds(const PatronId& patronId) const {
    std::vector<HoldSnapshot> out;
    QSqlQuery q(db_);
    q.prepare("SELECT id, itemId, queuePosition FROM holds WHERE patronId=? ORDER BY queuePosition ASC");
    q.addBindValue(QString::fromStdString(patronId));
    if (q.exec()) {
        while (q.next()) {
            HoldSnapshot snap;
            snap.id = q.value(0).toString().toStdString();
            snap.patronId = patronId;
            snap.itemId = q.value(1).toString().toStdString();
            snap.queuePosition = static_cast<std::size_t>(q.value(2).toInt());
            out.push_back(snap);
        }
    }
    return out;
}

std::vector<HoldSnapshot> Database::GetHoldQueueForItem(const ItemId& itemId) const {
    std::vector<HoldSnapshot> out;
    QSqlQuery q(db_);
    q.prepare("SELECT id, patronId, queuePosition FROM holds WHERE itemId=? ORDER BY queuePosition ASC");
    q.addBindValue(QString::fromStdString(itemId));
    if (q.exec()) {
        while (q.next()) {
            HoldSnapshot snap;
            snap.id = q.value(0).toString().toStdString();
            snap.patronId = q.value(1).toString().toStdString();
            snap.itemId = itemId;
            snap.queuePosition = static_cast<std::size_t>(q.value(2).toInt());
            out.push_back(snap);
        }
    }
    return out;
}

std::optional<LoanSnapshot> Database::GetLoanById(const LoanId& loanId) const {
    QSqlQuery q(db_);
    q.prepare("SELECT patronId, itemId, checkoutDate, dueDate FROM loans WHERE id=?");
    q.addBindValue(QString::fromStdString(loanId));
    if (!q.exec() || !q.next()) return std::nullopt;
    LoanSnapshot snap;
    snap.id = loanId;
    snap.patronId = q.value(0).toString().toStdString();
    snap.itemId = q.value(1).toString().toStdString();
    snap.checkoutDate = fromIso(q.value(2).toString());
    snap.dueDate = fromIso(q.value(3).toString());
    return snap;
}

std::optional<HoldSnapshot> Database::GetHoldById(const HoldId& holdId) const {
    QSqlQuery q(db_);
    q.prepare("SELECT patronId, itemId, queuePosition FROM holds WHERE id=?");
    q.addBindValue(QString::fromStdString(holdId));
    if (!q.exec() || !q.next()) return std::nullopt;
    HoldSnapshot snap;
    snap.id = holdId;
    snap.patronId = q.value(0).toString().toStdString();
    snap.itemId = q.value(1).toString().toStdString();
    snap.queuePosition = static_cast<std::size_t>(q.value(2).toInt());
    return snap;
}

} // namespace hinlibs
