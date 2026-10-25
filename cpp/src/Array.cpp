#include "structures/Array.hpp"
#include <algorithm>
#include <stdexcept>

namespace adt {
Array::Array() : data(nullptr), len(0), cap(0) {}
Array::~Array() { delete[] data; }

Array::Array(Array&& other) noexcept : data(other.data), len(other.len), cap(other.cap) {
    other.data = nullptr;
    other.len = 0;
    other.cap = 0;
}

Array& Array::operator=(Array&& other) noexcept {
    if (this == &other) return *this;
    delete[] data;
    data = other.data;
    len = other.len;
    cap = other.cap;
    other.data = nullptr;
    other.len = 0;
    other.cap = 0;
    return *this;
}

void Array::reserve(std::size_t capacity) {
    if (capacity <= cap) return;
    std::size_t newCapacity = std::max(capacity, cap == 0 ? std::size_t(2) : cap * 2);
    int* newData = new int[newCapacity];
    for (std::size_t i = 0; i < len; i++) newData[i] = data[i];
    delete[] data;
    data = newData;
    cap = newCapacity;
}

std::size_t Array::size() const { return len; }

void Array::insert(std::size_t index, int value) {
    if (index > len) throw std::out_of_range("index out of range");
    reserve(len + 1);
    for (std::size_t i = len; i > index; i--) data[i] = data[i - 1];
    data[index] = value;
    len++;
}

void Array::push(int value) { insert(len, value); }

int Array::get(std::size_t index) const {
    if (index >= len) throw std::out_of_range("index out of range");
    return data[index];
}

void Array::set(std::size_t index, int value) {
    if (index >= len) throw std::out_of_range("index out of range");
    data[index] = value;
}

int Array::erase(std::size_t index) {
    int value = get(index);
    for (std::size_t i = index + 1; i < len; i++) data[i - 1] = data[i];
    len--;
    return value;
}

std::vector<int> Array::values() const {
    std::vector<int> result;
    for (std::size_t i = 0; i < len; i++) result.push_back(data[i]);
    return result;
}
}
