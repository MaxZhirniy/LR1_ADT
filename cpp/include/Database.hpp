#pragma once
#include "structures/Array.hpp"
#include "structures/ForwardList.hpp"
#include "structures/DoublyList.hpp"
#include "structures/Stack.hpp"
#include "structures/Queue.hpp"
#include "structures/AvlTree.hpp"
#include <map>
#include <string>
#include <variant>
#include <vector>

using adt::Array;
using adt::ForwardList;
using adt::DoublyList;
using adt::Stack;
using adt::Queue;
using adt::AvlTree;

using Container = std::variant<Array, ForwardList, DoublyList, Stack, Queue, AvlTree>;
using Database = std::map<std::string, Container>;

char kindOf(const Container& data);
Container makeContainer(char type);
std::vector<int> getValues(const Container& data);
void loadData(const std::string& path, Database& db);
void saveData(const std::string& path, const Database& db);
