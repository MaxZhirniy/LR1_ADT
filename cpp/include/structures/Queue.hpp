#pragma once
#include <vector>

namespace adt {
class Queue {
    struct Node {
        int value;
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

    void push(int value);
    int pop();
    int front() const;
    std::vector<int> values() const;
};
}
