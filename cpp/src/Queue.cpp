#include "structures/Queue.hpp"
#include <stdexcept>

namespace adt {
Queue::Queue() : head(nullptr), tail(nullptr) {}
Queue::~Queue() { clear(); }
void Queue::clear() {
    while (head != nullptr) {
        Node* node = head;
        head = head->next;
        delete node;
    }
    tail = nullptr;
}
Queue::Queue(Queue&& other) noexcept : head(other.head), tail(other.tail) {
    other.head = nullptr;
    other.tail = nullptr;
}
Queue& Queue::operator=(Queue&& other) noexcept {
    if (this == &other) return *this;
    clear();
    head = other.head;
    tail = other.tail;
    other.head = nullptr;
    other.tail = nullptr;
    return *this;
}
void Queue::push(const std::string& value) {
    Node* node = new Node{value, nullptr};
    if (tail != nullptr) tail->next = node;
    else head = node;
    tail = node;
}
std::string Queue::pop() {
    if (head == nullptr) throw std::out_of_range("queue is empty");
    Node* node = head;
    std::string value = node->value;
    head = node->next;
    if (head == nullptr) tail = nullptr;
    delete node;
    return value;
}
std::string Queue::front() const {
    if (head == nullptr) throw std::out_of_range("queue is empty");
    return head->value;
}
std::vector<std::string> Queue::values() const {
    std::vector<std::string> result;
    for (Node* node = head; node != nullptr; node = node->next) result.push_back(node->value);
    return result;
}
}
