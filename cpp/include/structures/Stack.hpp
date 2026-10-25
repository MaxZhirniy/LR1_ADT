#pragma once
#include <vector>

namespace adt {
class Stack {
    struct Node {
        int value;
        Node* next;
    };
    Node* topNode;
    void clear();
public:
    Stack();
    ~Stack();
    Stack(const Stack&) = delete;
    Stack& operator=(const Stack&) = delete;
    Stack(Stack&& other) noexcept;
    Stack& operator=(Stack&& other) noexcept;

    void push(int value);
    int pop();
    int top() const;
    std::vector<int> values() const;
};
}
