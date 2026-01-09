#include "librarian.h"
#include "database.h"

namespace hinlibs {
std::string Librarian::getUsername() const {
    return db_->GetUserById(id_)->username;
}
// Nothing to do yet; anchor the vtable with an out-of-line dtor (optional).

}
