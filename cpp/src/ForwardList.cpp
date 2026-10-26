#include "structures/ForwardList.hpp"
#include <cstddef>
#include <new>
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

void ForwardList::insert(const std::string& where, int target, int value) {
    if (where != "BEFORE" && where != "AFTER") throw std::invalid_argument("invalid position");
    if (head == nullptr) throw std::invalid_argument("target not found");
    if (where == "BEFORE" && head->value == target){
        pushFront(value);
        return;
    }

    Node* current = head;
    while (current != nullptr){
        if (where == "BEFORE" && current->next != nullptr && current->next->value == target){
            Node* insertion = new Node{value, current->next};
            current->next = insertion;
            len++;
            return;
        } else if (where == "AFTER" && current->value == target){
            Node* insertion = new Node{value, current->next};
            current->next = insertion;
            if (tail == current) tail = insertion;
            len++;
            return;
        }
        current = current->next;
    }
    throw std::invalid_argument("target not found");
}

int ForwardList::unlink(Node* previous, Node* node){
    if (previous == nullptr) head = node->next;
    else previous->next = node->next;
    if (tail == node) tail = previous;
    int value = node->value;
    delete node;
    len--;
    return value;
}

int ForwardList::erase(const std::string& where, int target){
    if (where != "HEAD" && where != "TAIL" && where != "BEFORE" && where != "AFTER")
        throw std::invalid_argument("invalid position");
    if (head == nullptr) throw std::out_of_range("list is empty");
    if (where == "HEAD") return unlink(nullptr, head);
    if (where == "TAIL"){
        Node* previous = nullptr;
        Node* current = head;
        while (current->next != nullptr){
            previous = current;
            current = current->next;
        }
        return unlink(previous, current);
    }
    Node* beforePrevious = nullptr;
    Node* previous = nullptr;
    Node* current = head;
    while (current != nullptr && current->value != target){
        beforePrevious = previous;
        previous = current;
        current = current->next;
    }
    if (current == nullptr) throw std::invalid_argument("target not found");
    if (where == "BEFORE"){
        if (previous == nullptr) throw std::out_of_range("no element before target");
        return unlink(beforePrevious, previous);
    }
    if (current->next == nullptr) throw std::out_of_range("no element after target");
    return unlink(current, current->next);
}

int ForwardList::get(std::size_t index) const { return locate(index)->value; }
std::ptrdiff_t ForwardList::find(int value) const {
    std::ptrdiff_t index = 0;
    for (Node* node = head; node != nullptr; node = node->next, index++) {
        if (node->value == value) return index;
    }
    return -1;
}

bool ForwardList::remove(int value){
    Node* previous = nullptr;
    Node* current = head;
    while (current != nullptr && current->value != value){
        previous = current;
        current = current->next;
    }
    if (current == nullptr) return false;
    unlink(previous, current);
    return true;
}

std::vector<int> ForwardList::values() const {
    std::vector<int> result;
    for (Node* node = head; node != nullptr; node = node->next) result.push_back(node->value);
    return result;
}
}
