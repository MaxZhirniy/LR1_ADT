#pragma once
#include <cstddef>
#include <vector>
#include <string>
namespace adt {
class ForwardList {
    struct Node {
        std::string value;
        Node* next;
    };
    Node* head;
    Node* tail;
    std::size_t len;
    Node* locate(std::size_t index) const;
    void clear();
    std::string unlink(Node* previous, Node* node);
public:
    ForwardList();
    ~ForwardList();
    ForwardList(const ForwardList&) = delete;
    ForwardList& operator=(const ForwardList&) = delete;
    ForwardList(ForwardList&& other) noexcept;
    ForwardList& operator=(ForwardList&& other) noexcept;

    std::size_t size() const;
    void pushFront(const std::string& value);
    void pushBack(const std::string& value);
    void insert(const std::string& where, const std::string& target, const std::string& value);
    std::string erase(const std::string& where, const std::string& target = "");
    std::string get(std::size_t index) const;
    std::ptrdiff_t find(const std::string& value) const;
    bool remove(const std::string& value);
    std::vector<std::string> values() const;
};
}
