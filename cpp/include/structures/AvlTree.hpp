#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace adt {
class AvlTree {
    struct Node {
        int value;
        int height;
        Node* left;
        Node* right;
    };
    Node* root;
    static void clear(Node* node);
    static int height(const Node* node);
    static int balance(const Node* node);
    static void update(Node* node);
    static Node* rotateRight(Node* node);
    static Node* rotateLeft(Node* node);
    static Node* rebalance(Node* node);
    static Node* add(Node* node, int value, bool& changed);
    static int minimum(const Node* node);
    static Node* remove(Node* node, int value, bool& changed);
    static void walk(const Node* node, const std::string& mode, std::vector<int>& result);
    static bool verify(const Node* node, std::int64_t low, std::int64_t high, int& resultHeight);
public:
    AvlTree();
    ~AvlTree();
    AvlTree(const AvlTree&) = delete;
    AvlTree& operator=(const AvlTree&) = delete;
    AvlTree(AvlTree&& other) noexcept;
    AvlTree& operator=(AvlTree&& other) noexcept;

    bool insert(int value);
    bool erase(int value);
    bool contains(int value) const;
    std::vector<int> values(std::string mode = "INORDER") const;
    int treeHeight() const;
    bool valid() const;
};
}
