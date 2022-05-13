#pragma once

#include "compressed_pair.h"

#include <algorithm>
#include <cstddef>  // std::nullptr_t
#include <type_traits>
#include <utility>

template <typename T>
struct [[maybe_unused]] DefaultDeleter {
    DefaultDeleter() = default;
    template <typename U>
    DefaultDeleter([[maybe_unused]] U deleter){};

    void operator()(T* ptr) noexcept {
        if (ptr != nullptr) {
            delete ptr;
        }
    }
};

template <typename T>
struct [[maybe_unused]] DefaultDeleter<T[]> {
    DefaultDeleter() = default;
    template <typename U>
    DefaultDeleter([[maybe_unused]] U deleter){};

    void operator()(T* ptr) noexcept {
        if (ptr != nullptr) {
            delete[] ptr;
        }
    }
};
// Primary template
template <typename T, typename Deleter = DefaultDeleter<T>>
class UniquePtr {
private:
    CompressedPair<T*, Deleter> ptr_;

public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) noexcept : ptr_(ptr, Deleter()) {
    }

    UniquePtr(T* ptr, Deleter deleter) noexcept : ptr_(ptr, std::forward<Deleter>(deleter)) {
    }

    UniquePtr(UniquePtr&& other) noexcept
        : ptr_(other.Release(), std::forward<Deleter>(other.GetDeleter())) {
    }

    UniquePtr(UniquePtr& other) = delete;

    template <class U, class E>
    UniquePtr(UniquePtr<U, E>&& other) noexcept
        : ptr_(other.Release(), std::forward<E>(other.GetDeleter())) {
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // operator=-s

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        Reset(other.Release());
        ptr_.GetSecond() = std::forward<Deleter>(other.GetDeleter());
        return *this;
    }

    template <class U, class E>
    UniquePtr& operator=(UniquePtr<U, E>&& other) noexcept {
        Reset(other.Release());
        ptr_.GetSecond() = std::forward<E>(other.GetDeleter());
        return *this;
    }

    UniquePtr& operator=(std::nullptr_t) noexcept {
        Reset();
        return *this;
    }

    UniquePtr& operator=(UniquePtr& other) = delete;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        Reset();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() noexcept {
        T* prev = ptr_.GetFirst();
        ptr_.GetFirst() = nullptr;
        return prev;
    }

    void Reset(T* ptr = nullptr) noexcept {
        T* prev = ptr_.GetFirst();
        ptr_.GetFirst() = ptr;
        if (prev != nullptr) {
            ptr_.GetSecond()(prev);
        }
    }

    void Swap(UniquePtr& other) {
        std::swap(*this, other);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return ptr_.GetFirst();
    }
    Deleter& GetDeleter() {
        return ptr_.GetSecond();
    }
    const Deleter& GetDeleter() const {
        return ptr_.GetSecond();
    }

    explicit operator bool() const {
        if (ptr_.GetFirst() == nullptr) {
            return false;
        }
        return true;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    std::add_lvalue_reference_t<T> operator*() const {
        return *ptr_.GetFirst();
    }
    T* operator->() const {
        return ptr_.GetFirst();
    }
};

// Specialization for arrays

template <typename T, typename Deleter>
class UniquePtr<T[], Deleter> {
private:
    CompressedPair<T*, Deleter> ptr_;

public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) noexcept : ptr_(ptr, Deleter()) {
    }

    UniquePtr(T* ptr, Deleter deleter) noexcept : ptr_(ptr, std::forward<Deleter>(deleter)) {
    }

    UniquePtr(UniquePtr&& other) noexcept
        : ptr_(other.Release(), std::forward<Deleter>(other.GetDeleter())) {
    }

    UniquePtr(UniquePtr& other) = delete;

    template <class U, class E>
    UniquePtr(UniquePtr<U, E>&& other) noexcept
        : ptr_(other.Release(), std::forward<E>(other.GetDeleter())) {
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // operator=-s

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        Reset(other.Release());
        ptr_.GetSecond() = std::forward<Deleter>(other.GetDeleter());
        return *this;
    }

    template <class U, class E>
    UniquePtr& operator=(UniquePtr<U, E>&& other) noexcept {
        Reset(other.Release());
        ptr_.GetSecond() = std::forward<E>(other.GetDeleter());
        return *this;
    }

    UniquePtr& operator=(std::nullptr_t) noexcept {
        Reset();
        return *this;
    }

    UniquePtr& operator=(UniquePtr& other) = delete;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        Reset();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() noexcept {
        T* prev = ptr_.GetFirst();
        ptr_.GetFirst() = nullptr;
        return prev;
    }

    void Reset(T* ptr = nullptr) noexcept {
        T* prev = ptr_.GetFirst();
        ptr_.GetFirst() = ptr;
        if (prev) {
            ptr_.GetSecond()(prev);
        }
    }

    void Swap(UniquePtr& other) {
        std::swap(*this, other);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return ptr_.GetFirst();
    }
    Deleter& GetDeleter() {
        return ptr_.GetSecond();
    }
    const Deleter& GetDeleter() const {
        return ptr_.GetSecond();
    }

    explicit operator bool() const {
        if (ptr_.GetFirst() == nullptr) {
            return false;
        }
        return true;
    }

    T& operator[](std::size_t i) {
        return ptr_.GetFirst()[i];
    };

    T& operator[](std::size_t i) const {
        return ptr_.GetFirst()[i];
    };
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    std::add_lvalue_reference_t<T> operator*() const {
        return *ptr_.GetFirst();
    }
    T* operator->() const {
        return ptr_.GetFirst();
    }
};
