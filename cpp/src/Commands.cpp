#include "Commands.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <stdexcept>
#include <utility>
using namespace std;

static string join(const vector<int>& values) {
    string text = "[";
    for (size_t i = 0; i < values.size(); i++) {
        if (i) text += " ";
        text += to_string(values[i]);
    }
    return text + "]";
}

string upperText(string s) { for (char& c : s) c = static_cast<char>(toupper(static_cast<unsigned char>(c))); return s; }

static int integer(const string& s) {
    size_t used = 0;
    int value = stoi(s, &used);
    if (used != s.size()) throw invalid_argument("invalid integer: " + s);
    return value;
}

static size_t indexOf(const string& s) {
    int index = integer(s);
    if (index < 0) throw out_of_range("negative index");
    return static_cast<size_t>(index);
}

static string arg(istringstream& in) {
    string item;
    if (!(in >> item)) throw invalid_argument("missing command argument");
    return item;
}

static void endArgs(istringstream& in) { string extra; if (in >> extra) throw invalid_argument("too many command arguments"); }

static size_t position(const string& action, size_t reference, size_t size, bool insertion) {
    if (action == "HEAD") return 0;
    if (action == "TAIL") return insertion ? size : (size ? size - 1 : 0);
    if (reference >= size) throw out_of_range("reference index out of range");
    if (action == "BEFORE") return insertion ? reference : (reference == 0 ? throw out_of_range("no element before reference") : reference - 1);
    if (action == "AFTER") return insertion ? reference + 1 : (reference + 1 >= size ? throw out_of_range("no element after reference") : reference + 1);
    throw invalid_argument("expected HEAD, TAIL, BEFORE or AFTER");
}

string runCommand(Database& db, const string& command, bool& changed) {
    istringstream in(command);
    string op;
    if (!(in >> op)) return "";
    op = upperText(op);
    if (op == "HELP") {
        return "CREATE type name | DROP name | PRINT name | MPUSH/MINSERT/MGET/MSET/MDEL/MLEN/MREAD | F/L PUSH/INSERT/DEL/REMOVE/FIND/GET/READ | SPUSH/SPOP/SGET/SREAD | QPUSH/QPOP/QGET/QREAD | TINSERT/TDEL/TGET/TREAD/TCHECK/THEIGHT | EXIT";
    } else if (op == "CREATE") {
        string type = upperText(arg(in)), name = arg(in);
        endArgs(in);
        if (type.size() != 1) throw invalid_argument("type must be M/F/L/S/Q/T");
        if (db.count(name)) throw invalid_argument("name already exists");
        db.emplace(name, makeContainer(type[0]));
        changed = true;
        return "OK";
    } else if (op == "DROP") {
        string name = arg(in);
        endArgs(in);
        if (db.erase(name) == 0){
            throw invalid_argument("structure not found: " + name);
        }
        changed = true;
        return "OK";
    }

    string name = arg(in);
    Database::iterator it = db.find(name);
    if (it == db.end()) throw invalid_argument("structure not found: " + name);
    Container& data = it->second;
    char k = kindOf(data);
    if (op == "PRINT") { endArgs(in); return join(getValues(data)); }
    if (op.empty() || op[0] != k) throw invalid_argument("command does not match structure type");
    if (k == 'M') {
        Array& a = get<Array>(data);
        if (op == "MPUSH") { int value = integer(arg(in)); endArgs(in); a.push(value); changed = true; return "OK"; }
        if (op == "MINSERT") { size_t i = indexOf(arg(in)); int value = integer(arg(in)); endArgs(in); a.insert(i, value); changed = true; return "OK"; }
        if (op == "MDEL") { size_t i = indexOf(arg(in)); endArgs(in); int value = a.erase(i); changed = true; return to_string(value); }
        if (op == "MSET") { size_t i = indexOf(arg(in)); int value = integer(arg(in)); endArgs(in); a.set(i, value); changed = true; return "OK"; }
        if (op == "MGET") { size_t i = indexOf(arg(in)); endArgs(in); return to_string(a.get(i)); }
        if (op == "MLEN") { endArgs(in); return to_string(a.size()); }
        if (op == "MREAD") { endArgs(in); return join(a.values()); }
    }
    if (k == 'F' || k == 'L') {
        bool forward = k == 'F';
        size_t n = forward ? get<ForwardList>(data).size() : get<DoublyList>(data).size();
        const string prefix(1, k);
        if (op == prefix + "PUSH") {
            int value = integer(arg(in)); endArgs(in);
            if (forward) get<ForwardList>(data).pushBack(value);
            else get<DoublyList>(data).pushBack(value);
            changed = true; return "OK";
        }
        if (op == prefix + "INSERT"){
            string where = upperText(arg(in));
            int target = 0;
            if (where == "BEFORE" || where == "AFTER") target = integer(arg(in));
            int value = integer(arg(in));
            endArgs(in);
            if (forward) get<ForwardList>(data).insert(where, target, value);
            else get<DoublyList>(data).insert(where, target, value);
            changed = true;
            return "OK";
        }
        if (op == prefix + "DEL"){
            string where = upperText(arg(in));
            int target = 0;
            if (where == "BEFORE" || where == "AFTER") target = integer(arg(in));
            endArgs(in);
            int value = forward ? get<ForwardList>(data).erase(where, target) : get<DoublyList>(data).erase(where, target);
            changed = true;
            return to_string(value);
        }
        if (op == prefix + "REMOVE") {
            int value = integer(arg(in)); endArgs(in);
            bool ok = forward ? get<ForwardList>(data).remove(value) : get<DoublyList>(data).remove(value);
            changed = ok; return ok ? "TRUE" : "FALSE";
        }
        if (op == prefix + "FIND") {
            int value = integer(arg(in)); endArgs(in);
            return to_string(forward ? get<ForwardList>(data).find(value) : get<DoublyList>(data).find(value));
        }
        if (op == prefix + "GET") {
            size_t i = indexOf(arg(in)); endArgs(in);
            return to_string(forward ? get<ForwardList>(data).get(i) : get<DoublyList>(data).get(i));
        }
        if (op == prefix + "READ") {
            string mode = "FORWARD";
            in >> mode;
            mode = upperText(mode);
            endArgs(in);
            if (mode != "FORWARD" && mode != "REVERSE") throw invalid_argument("read mode: FORWARD or REVERSE");
            vector<int> values = forward ? get<ForwardList>(data).values() : get<DoublyList>(data).values(mode == "REVERSE");
            if (forward && mode == "REVERSE") reverse(values.begin(), values.end());
            return join(values);
        }
    }
    if (k == 'S') {
        Stack& s = get<Stack>(data);
        if (op == "SPUSH") { int v = integer(arg(in)); endArgs(in); s.push(v); changed = true; return "OK"; }
        if (op == "SPOP") { endArgs(in); int v = s.pop(); changed = true; return to_string(v); }
        if (op == "SGET") { endArgs(in); return to_string(s.top()); }
        if (op == "SREAD") { endArgs(in); return join(s.values()); }
    }
    if (k == 'Q') {
        Queue& q = get<Queue>(data);
        if (op == "QPUSH") { int v = integer(arg(in)); endArgs(in); q.push(v); changed = true; return "OK"; }
        if (op == "QPOP") { endArgs(in); int v = q.pop(); changed = true; return to_string(v); }
        if (op == "QGET") { endArgs(in); return to_string(q.front()); }
        if (op == "QREAD") { endArgs(in); return join(q.values()); }
    }
    if (k == 'T') {
        AvlTree& t = get<AvlTree>(data);
        if (op == "TINSERT") { int v = integer(arg(in)); endArgs(in); changed = t.insert(v); return changed ? "TRUE" : "FALSE"; }
        if (op == "TDEL") { int v = integer(arg(in)); endArgs(in); changed = t.erase(v); return changed ? "TRUE" : "FALSE"; }
        if (op == "TGET") { int v = integer(arg(in)); endArgs(in); return t.contains(v) ? "TRUE" : "FALSE"; }
        if (op == "TREAD") { string mode = "INORDER"; in >> mode; endArgs(in); return join(t.values(upperText(mode))); }
        if (op == "TCHECK") { endArgs(in); return t.valid() ? "TRUE" : "FALSE"; }
        if (op == "THEIGHT") { endArgs(in); return to_string(t.treeHeight()); }
    }
    throw invalid_argument("unknown command: " + op);
}
