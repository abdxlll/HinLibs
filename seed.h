#pragma once

#include "database.h"
#include "mockdb.h"
#include <memory>

namespace hinlibs::seed {

std::shared_ptr<MockDb> CreateSeededMockDb();
Database MakeDatabaseWithSeed();
void SeedMockData(MockDb& db);

}
