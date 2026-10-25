#include "structures/DoublyList.hpp"
#include <stdexcept>

namespace adt {
DoublyList::DoublyList() : head(nullptr), tail(nullptr), len(0) {}
DoublyList::~DoublyList() { clear(); }
void DoublyList::clear() {
    while (head != nullptr) {
        Node* node = head;
        head = head->next;
        delete node;
    }
    tail = nullptr;
    len = 0;
}
DoublyList::DoublyList(DoublyList&& other) noexcept : head(other.head), tail(other.tail), len(other.len) {
    other.head = nullptr;
    other.tail = nullptr;
    other.len = 0;
}
DoublyList& DoublyList::operator=(DoublyList&& other) noexcept {
    if (this == &other) return *this;
    clear();
    head = other.head;
    tail = other.tail;
    len = other.len;
    other.head = nullptr;
    other.tail = nullptr;
    other.len = 0;
    return *this;
}
DoublyList::Node* DoublyList::locate(std::size_t index) const {
    if (index >= len) throw std::out_of_range("index out of range");
    if (index < len / 2) {
        Node* node = head;
        for (std::size_t i = 0; i < index; i++) node = node->next;
        return node;
    }
    Node* node = tail;
    for (std::size_t i = len - 1; i > index; i--) node = node->prev;
    return node;
}
std::size_t DoublyList::size() const { return len; }
void DoublyList::pushFront(int value) {
    Node* node = new Node{value, nullptr, head};
    if (head != nullptr) head->prev = node;
    else tail = node;
    head = node;
    len++;
}
void DoublyList::pushBack(int value) {
    if (tail == nullptr) { pushFront(value); return; }
    Node* node = new Node{value, tail, nullptr};
    tail->next = node;
    tail = node;
    len++;
}
void DoublyList::insert(std::size_t index, int value) {
    if (index > len) throw std::out_of_range("index out of range");
    if (index == 0) { pushFront(value); return; }
    if (index == len) { pushBack(value); return; }
    Node* curr = locate(index);
    Node* prev = curr->prev;
    Node* node = new Node{value, prev, curr};
    prev->next = node;
    curr->prev = node;
    len++;
}
int DoublyList::erase(std::size_t index) {
    Node* node = locate(index);
    if (node->prev != nullptr) node->prev->next = node->next;
    else head = node->next;
    if (node->next != nullptr) node->next->prev = node->prev;
    else tail = node->prev;
    int value = node->value;
    delete node;
    len--;
    return value;
}
int DoublyList::get(std::size_t index) const { return locate(index)->value; }
std::ptrdiff_t DoublyList::find(int value) const {
    std::ptrdiff_t index = 0;
    for (Node* node = head; node != nullptr; node = node->next, index++) {
        if (node->value == value) return index;
    }
    return -1;
}
bool DoublyList::remove(int value) {
    std::ptrdiff_t index = find(value);
    if (index == -1) return false;
    erase(static_cast<std::size_t>(index));
    return true;
}
std::vector<int> DoublyList::values(bool reverse) const {
    std::vector<int> result;
    if (reverse) {
        for (Node* node = tail; node != nullptr; node = node->prev) result.push_back(node->value);
    } else {
        for (Node* node = head; node != nullptr; node = node->next) result.push_back(node->value);
    }
    return result;
}
}
