#pragma once
#include "Commands.hpp"
#include <string>

// Фасад: команда -> изменение хранилища -> запись -> результат.
int execute(Database& db, const std::string& path, const std::string& command);
