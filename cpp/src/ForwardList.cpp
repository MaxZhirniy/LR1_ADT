#include "structures/ForwardList.hpp"
#include <stdexcept>

namespace adt {
ForwardList::ForwardList() : head(nullptr), tail(nullptr), len(0) {}
ForwardList::~ForwardList() { clear(); }
void ForwardList::clear() {
    while (head != nullptr) {
        Node* node = head;
        head = head->next;
        delete node;
    }
    tail = nullptr;
    len = 0;
}
ForwardList::ForwardList(ForwardList&& other) noexcept : head(other.head), tail(other.tail), len(other.len) {
    other.head = nullptr;
    other.tail = nullptr;
    other.len = 0;
}
ForwardList& ForwardList::operator=(ForwardList&& other) noexcept {
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
ForwardList::Node* ForwardList::locate(std::size_t index) const {
    if (index >= len) throw std::out_of_range("index out of range");
    Node* node = head;
    for (std::size_t i = 0; i < index; i++) node = node->next;
    return node;
}
std::size_t ForwardList::size() const { return len; }
void ForwardList::pushFront(int value) {
    Node* node = new Node{value, head};
    head = node;
    if (tail == nullptr) tail = node;
    len++;
}
void ForwardList::pushBack(int value) {
    if (tail == nullptr) { pushFront(value); return; }
    Node* node = new Node{value, nullptr};
    tail->next = node;
    tail = node;
    len++;
}
void ForwardList::insert(std::size_t index, int value) {
    if (index > len) throw std::out_of_range("index out of range");
    if (index == 0) { pushFront(value); return; }
    if (index == len) { pushBack(value); return; }
    Node* prev = locate(index - 1);
    prev->next = new Node{value, prev->next};
    len++;
}
int ForwardList::erase(std::size_t index) {
    if (index >= len) throw std::out_of_range("index out of range");
    Node* node;
    if (index == 0) {
        node = head;
        head = head->next;
        if (head == nullptr) tail = nullptr;
    } else {
        Node* prev = locate(index - 1);
        node = prev->next;
        prev->next = node->next;
        if (node == tail) tail = prev;
    }
    int value = node->value;
    delete node;
    len--;
    return value;
}
int ForwardList::get(std::size_t index) const { return locate(index)->value; }
std::ptrdiff_t ForwardList::find(int value) const {
    std::ptrdiff_t index = 0;
    for (Node* node = head; node != nullptr; node = node->next, index++) {
        if (node->value == value) return index;
    }
    return -1;
}
bool ForwardList::remove(int value) {
    std::ptrdiff_t index = find(value);
    if (index == -1) return false;
    erase(static_cast<std::size_t>(index));
    return true;
}
std::vector<int> ForwardList::values() const {
    std::vector<int> result;
    for (Node* node = head; node != nullptr; node = node->next) result.push_back(node->value);
    return result;
}
}
