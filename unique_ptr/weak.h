#pragma once

#include "../shared_ptr/sw_fwd.h"  // Forward declaration
#include "../shared_ptr/shared.h"

// https://en.cppreference.com/w/cpp/memory/weak_ptr
template <typename T>
class WeakPtr {
private:
    template <typename U>
    friend class WeakPtr;

    template <typename U>
    friend class SharedPtr;

    friend class SharedPtr<T>;

    ControlBlockBase* block_ = nullptr;
    T* ptr_ = nullptr;

    void MinusWeak() {
        if (block_ != nullptr) {
            block_->MinusWeak();
        }
    }

    void PlusWeak() {
        if (block_ != nullptr) {
            block_->PlusWeak();
        }
    }

public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    WeakPtr() = default;

    WeakPtr(const WeakPtr& other) {
        block_ = other.block_;
        ptr_ = other.ptr_;
        PlusWeak();
    }

    template <typename U>
    WeakPtr(const WeakPtr<U>& other) {
        block_ = other.block_;
        ptr_ = other.ptr_;
        PlusWeak();
    }

    WeakPtr(WeakPtr&& other) {
        block_ = other.block_;
        ptr_ = other.ptr_;
        other.ptr_ = nullptr;
        other.block_ = nullptr;
    }

    template <typename U>
    WeakPtr(WeakPtr<U>&& other) {
        block_ = other.block_;
        ptr_ = other.ptr_;
        other.ptr_ = nullptr;
        other.block_ = nullptr;
    }

    // Demote `SharedPtr`
    // #2 from https://en.cppreference.com/w/cpp/memory/weak_ptr/weak_ptr
    WeakPtr(const SharedPtr<T>& other) {
        block_ = other.block_;
        ptr_ = other.ptr_;
        PlusWeak();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    WeakPtr& operator=(const WeakPtr& other) {
        if (ptr_ != other.ptr_) {
            MinusWeak();
            ptr_ = other.ptr_;
            block_ = other.block_;
            PlusWeak();
        }
        return *this;
    }

    WeakPtr& operator=(WeakPtr&& other) {
        MinusWeak();
        ptr_ = other.ptr_;
        block_ = other.block_;
        other.ptr_ = nullptr;
        other.block_ = nullptr;
        return *this;
    }

    template <typename U>
    WeakPtr& operator=(WeakPtr<U>&& other) {
        MinusWeak();
        ptr_ = other.ptr_;
        block_ = other.block_;
        other.ptr_ = nullptr;
        other.block_ = nullptr;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~WeakPtr() {
        MinusWeak();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        MinusWeak();
        ptr_ = nullptr;
        block_ = nullptr;
    }

    void Swap(WeakPtr& other) {
        std::swap(*this, other);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    size_t UseCount() const {
        if (block_ == nullptr) {
            return 0;
        }
        return block_->Strong();
    }
    bool Expired() const {
        if (block_ == nullptr || UseCount() == 0) {
            return true;
        }
        return false;
    }
    SharedPtr<T> Lock() const {
        if (UseCount() == 0) {
            return SharedPtr<T>();
        }
        return SharedPtr(*this);
    }
};
