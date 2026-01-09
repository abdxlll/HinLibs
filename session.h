#pragma once

#include <memory>
#include <string>

namespace hinlibs {

class Database;
class User;

class Session {
public:
    explicit Session(std::shared_ptr<Database> db);
    ~Session();

    bool signIn(const std::string& username);
    void signOut();

    std::shared_ptr<User> currentUser() const;

    // 🔹 NEW: expose the shared Database so UI can use it
    std::shared_ptr<Database> db() const { return db_; }

private:
    std::shared_ptr<Database> db_;     // backing Database (MockDb now, SQLite later)
    std::shared_ptr<User>     current_; // currently signed-in user
};

} // namespace hinlibs
