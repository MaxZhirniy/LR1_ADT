#pragma once
#include <cstddef>
#include <vector>

namespace adt {
class DoublyList {
    struct Node {
        int value;
        Node* prev;
        Node* next;
    };
    Node* head;
    Node* tail;
    std::size_t len;
    Node* locate(std::size_t index) const;
    void clear();
public:
    DoublyList();
    ~DoublyList();
    DoublyList(const DoublyList&) = delete;
    DoublyList& operator=(const DoublyList&) = delete;
    DoublyList(DoublyList&& other) noexcept;
    DoublyList& operator=(DoublyList&& other) noexcept;

    std::size_t size() const;
    void pushFront(int value);
    void pushBack(int value);
    void insert(std::size_t index, int value);
    int erase(std::size_t index);
    int get(std::size_t index) const;
    std::ptrdiff_t find(int value) const;
    bool remove(int value);
    std::vector<int> values(bool reverse = false) const;
};
}
