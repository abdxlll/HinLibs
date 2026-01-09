#include "sysadmin.h"
#include "database.h"

namespace hinlibs {
std::string SysAdmin::getUsername() const {
    return db_->GetUserById(id_)->username;
}
// Nothing to do yet; anchor the vtable with an out-of-line dtor (optional).
}
