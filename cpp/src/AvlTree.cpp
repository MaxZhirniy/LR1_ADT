#include "structures/AvlTree.hpp"
#include <algorithm>
#include <limits>
#include <queue>
#include <stdexcept>
#include <cstdlib>

namespace adt {
AvlTree::AvlTree() : root(nullptr) {}
AvlTree::~AvlTree() { clear(root); }

void AvlTree::clear(Node* node) {
    if (node == nullptr) return;
    clear(node->left);
    clear(node->right);
    delete node;
}

AvlTree::AvlTree(AvlTree&& other) noexcept : root(other.root) { other.root = nullptr; }

AvlTree& AvlTree::operator=(AvlTree&& other) noexcept {
    if (this == &other) return *this;
    clear(root);
    root = other.root;
    other.root = nullptr;
    return *this;
}

int AvlTree::height(const Node* node) { return node == nullptr ? 0 : node->height; }

int AvlTree::balance(const Node* node) {
    return node == nullptr ? 0 : height(node->left) - height(node->right);
}

void AvlTree::update(Node* node) {
    node->height = 1 + std::max(height(node->left), height(node->right));
}

AvlTree::Node* AvlTree::rotateRight(Node* y) {
    Node* x = y->left;
    Node* subtree = x->right;
    x->right = y;
    y->left = subtree;
    update(y);
    update(x);
    return x;
}

AvlTree::Node* AvlTree::rotateLeft(Node* x) {
    Node* y = x->right;
    Node* subtree = y->left;
    y->left = x;
    x->right = subtree;
    update(x);
    update(y);
    return y;
}

AvlTree::Node* AvlTree::rebalance(Node* node) {
    if (node == nullptr) return nullptr;
    update(node);
    if (balance(node) > 1) {
        if (balance(node->left) < 0) node->left = rotateLeft(node->left);
        return rotateRight(node);
    }
    if (balance(node) < -1) {
        if (balance(node->right) > 0) node->right = rotateRight(node->right);
        return rotateLeft(node);
    }
    return node;
}

AvlTree::Node* AvlTree::add(Node* node, int value, bool& changed) {
    if (node == nullptr) {
        Node* created = new Node{value, 1, nullptr, nullptr};
        changed = true;
        return created;
    }
    if (value < node->value) node->left = add(node->left, value, changed);
    else if (value > node->value) node->right = add(node->right, value, changed);
    return rebalance(node);
}

int AvlTree::minimum(const Node* node) {
    while (node->left != nullptr) node = node->left;
    return node->value;
}

AvlTree::Node* AvlTree::remove(Node* node, int value, bool& changed) {
    if (node == nullptr) return nullptr;
    if (value < node->value) node->left = remove(node->left, value, changed);
    else if (value > node->value) node->right = remove(node->right, value, changed);
    else {
        changed = true;
        if (node->left == nullptr || node->right == nullptr) {
            Node* next = node->left != nullptr ? node->left : node->right;
            delete node;
            return next;
        }
        int successor = minimum(node->right);
        node->value = successor;
        bool removed = false;
        node->right = remove(node->right, successor, removed);
    }
    return rebalance(node);
}

void AvlTree::walk(const Node* node, const std::string& mode, std::vector<int>& result) {
    if (node == nullptr) return;
    if (mode == "PREORDER") result.push_back(node->value);
    walk(node->left, mode, result);
    if (mode == "INORDER") result.push_back(node->value);
    walk(node->right, mode, result);
    if (mode == "POSTORDER") result.push_back(node->value);
}

bool AvlTree::verify(const Node* node, std::int64_t low, std::int64_t high, int& resultHeight) {
    if (node == nullptr) { resultHeight = 0; return true; }
    if (node->value <= low || node->value >= high) return false;
    int lh = 0, rh = 0;
    if (!verify(node->left, low, node->value, lh)) return false;
    if (!verify(node->right, node->value, high, rh)) return false;
    resultHeight = 1 + std::max(lh, rh);
    return std::abs(lh - rh) <= 1 && node->height == resultHeight;
}

bool AvlTree::insert(int value) {
    bool changed = false;
    root = add(root, value, changed);
    return changed;
}

bool AvlTree::erase(int value) {
    bool changed = false;
    root = remove(root, value, changed);
    return changed;
}

bool AvlTree::contains(int value) const {
    const Node* node = root;
    while (node != nullptr) {
        if (node->value == value) return true;
        node = value < node->value ? node->left : node->right;
    }
    return false;
}

std::vector<int> AvlTree::values(std::string mode) const {
    if (mode != "INORDER" && mode != "PREORDER" && mode != "POSTORDER" && mode != "BFS") {
        throw std::invalid_argument("unknown traversal");
    }
    std::vector<int> result;
    if (mode == "BFS") {
        std::queue<const Node*> nodes;
        if (root != nullptr) nodes.push(root);
        while (!nodes.empty()) {
            const Node* node = nodes.front();
            nodes.pop();
            result.push_back(node->value);
            if (node->left != nullptr) nodes.push(node->left);
            if (node->right != nullptr) nodes.push(node->right);
        }
    } else {
        walk(root, mode, result);
    }
    return result;
}

int AvlTree::treeHeight() const { return height(root); }

bool AvlTree::valid() const {
    int actualHeight = 0;
    return verify(root, std::numeric_limits<std::int64_t>::min(), std::numeric_limits<std::int64_t>::max(), actualHeight);
}
}
