#pragma once
#include <vector>
#include <string>

namespace adt {
class Queue {
    struct Node {
        std::string value;
        Node* next;
    };
    Node* head;
    Node* tail;
    void clear();
public:
    Queue();
    ~Queue();
    Queue(const Queue&) = delete;
    Queue& operator=(const Queue&) = delete;
    Queue(Queue&& other) noexcept;
    Queue& operator=(Queue&& other) noexcept;

    void push(const std::string& value);
    std::string pop();
    std::string front() const;
    std::vector<std::string> values() const;
};
}
