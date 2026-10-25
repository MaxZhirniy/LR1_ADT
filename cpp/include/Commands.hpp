#pragma once
#include "Database.hpp"
#include <string>

// Выполнение одной команды без вывода и без обращения к файлу.
// changed=true только при фактическом изменении контейнера.
std::string runCommand(Database& db, const std::string& command, bool& changed);
std::string upperText(std::string text);
