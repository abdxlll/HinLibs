#include "session.h"
#include "database.h"
#include "user.h"
#include "patron.h"
#include "librarian.h"
#include "sysadmin.h"

#include <utility>

namespace hinlibs {

Session::Session(std::shared_ptr<Database> db)
    : db_(std::move(db)) {}

Session::~Session() = default;

bool Session::signIn(const std::string& username) {
    current_.reset();
    if (!db_) return false;

    auto rec = db_->FindUserByName(username);
    if (!rec) return false;

    switch (rec->role) {
        case Role::Patron:
            current_ = std::make_shared<Patron>(db_, rec->id);
            break;
        case Role::Librarian:
            current_ = std::make_shared<Librarian>(db_, rec->id);
            break;
        case Role::SysAdmin:
            current_ = std::make_shared<SysAdmin>(db_, rec->id);
            break;
        default:
            return false;
    }
    return static_cast<bool>(current_);
}

void Session::signOut() {
    current_.reset();
}

std::shared_ptr<User> Session::currentUser() const {
    return current_;
}

} // namespace hinlibs
