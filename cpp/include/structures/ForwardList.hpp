#pragma once
#include <cstddef>
#include <vector>
#include <string>
namespace adt {
class ForwardList {
    struct Node {
        int value;
        Node* next;
    };
    Node* head;
    Node* tail;
    std::size_t len;
    Node* locate(std::size_t index) const;
    void clear();
    int unlink(Node* previous, Node* node);
public:
    ForwardList();
    ~ForwardList();
    ForwardList(const ForwardList&) = delete;
    ForwardList& operator=(const ForwardList&) = delete;
    ForwardList(ForwardList&& other) noexcept;
    ForwardList& operator=(ForwardList&& other) noexcept;

    std::size_t size() const;
    void pushFront(int value);
    void pushBack(int value);
    void insert(const std::string& where, int target, int value);
    int erase(const std::string& where, int target = 0);
    int get(std::size_t index) const;
    std::ptrdiff_t find(int value) const;
    bool remove(int value);
    std::vector<int> values() const;
};
}
