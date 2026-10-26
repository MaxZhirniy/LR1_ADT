#include "Database.hpp"
#include <algorithm>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <utility>
using namespace std;

char kindOf(const Container& c) {
    switch (c.index()) { case 0: return 'M'; case 1: return 'F'; case 2: return 'L'; case 3: return 'S'; case 4: return 'Q'; default: return 'T'; }
}
Container makeContainer(char k) {
    switch (k) { case 'M': return Array{}; case 'F': return ForwardList{}; case 'L': return DoublyList{}; case 'S': return Stack{}; case 'Q': return Queue{}; case 'T': return AvlTree{}; default: throw invalid_argument("unknown structure type"); }
}
vector<string> getValues(const Container& c) {
    if (kindOf(c) == 'M') return get<Array>(c).values();
    vector<int> numbers;
    switch (kindOf(c)) {
        case 'F': numbers = get<ForwardList>(c).values(); break;
        case 'L': numbers = get<DoublyList>(c).values(); break;
        case 'S': numbers = get<Stack>(c).values(); break;
        case 'Q': numbers = get<Queue>(c).values(); break;
        case 'T': numbers = get<AvlTree>(c).values(); break;
        default: throw invalid_argument("unknown structure type");
    }
    vector<string> result;
    for (int value : numbers) result.push_back(to_string(value));
    return result;
}
void saveData(const string& path, const Database& db) {
    const string tmp = path + ".tmp";
    ofstream out(tmp, ios::trunc);
    if (!out) throw runtime_error("cannot write: " + tmp);
    for (Database::const_iterator it = db.begin(); it != db.end(); ++it) {
        const string& name = it->first;
        const Container& data = it->second;
        vector<string> values = getValues(data);
        if (kindOf(data) == 'S') reverse(values.begin(), values.end());
        out << kindOf(data) << ' ' << name << ' ' << values.size();
        for (const string& value : values) out << ' ' << value;
        out << '\n';
    }
    out.close();
    if (!out) throw runtime_error("cannot close file: " + tmp);
    if (rename(tmp.c_str(), path.c_str()) != 0) throw runtime_error("cannot replace data file");
}
void loadData(const string& path, Database& db) {
    ifstream in(path);
    if (!in) return;
    string line;
    size_t lineNumber = 0;
    while (getline(in, line)) {
        lineNumber++;
        if (line.empty()) continue;
        istringstream row(line);
        char type;
        string name;
        size_t n;
        if (!(row >> type >> name >> n) || n > 1000000 || (type != 'M' && type != 'F' && type != 'L' && type != 'S' && type != 'Q' && type != 'T') || db.count(name)) throw runtime_error("invalid data file, line " + to_string(lineNumber));
        Container data = makeContainer(type);
        for (size_t i = 0; i < n; i++) {
            if (type == 'M'){
                string value;
                if (!(row >> value)) throw runtime_error("truncated data file, line " + to_string(lineNumber));
                get<Array>(data).push(value);
            } else {
                int value;
                if (!(row >> value)) throw runtime_error("truncated data file, line " + to_string(lineNumber));
                if (type == 'F') get<ForwardList>(data).pushBack(value);
                else if (type == 'L') get<DoublyList>(data).pushBack(value);
                else if (type == 'S') get<Stack>(data).push(value);
                else if (type == 'Q') get<Queue>(data).push(value);
                else if (!get<AvlTree>(data).insert(value)) throw runtime_error("duplicate tree key in file");
            }
        }
        string extra;
        if (row >> extra) throw runtime_error("extra data in file, line " + to_string(lineNumber));
        db.emplace(name, move(data));
    }
}
