#pragma once
#include "session.h"

namespace tests {

// Runs all session login/logout tests.
// Returns number of failures (0 means success).
int run_login_tests(hinlibs::Session& session);
int run_patron_catalogue_flow(hinlibs::Session& session);
int run_multi_patron_holds_flow(hinlibs::Session& session);


} // namespace tests
