#pragma once
#include <cstddef>
#include <vector>

namespace adt {
class Array {
    int* data;
    std::size_t len;
    std::size_t cap;
    void reserve(std::size_t capacity);
public:
    Array();
    ~Array();
    Array(const Array&) = delete;
    Array& operator=(const Array&) = delete;
    Array(Array&& other) noexcept;
    Array& operator=(Array&& other) noexcept;

    std::size_t size() const;
    void insert(std::size_t index, int value);
    void push(int value);
    int get(std::size_t index) const;
    void set(std::size_t index, int value);
    int erase(std::size_t index);
    std::vector<int> values() const;
};
}
