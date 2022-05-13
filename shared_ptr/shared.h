#pragma once

#include "sw_fwd.h"  // Forward dec
#include <cstddef>  // std::nullptr_t

struct ControlBlockBase {
    virtual void MinusStrong() = 0;
    virtual void PlusStrong() = 0;
    virtual void MinusWeak() = 0;
    virtual void PlusWeak() = 0;
    virtual void OnZeroStrong() = 0;
    virtual void OnZeroWeak() = 0;
    virtual int& Strong() = 0;
    virtual int& Weak() = 0;
    virtual ~ControlBlockBase() = default;
};

template <typename T>
struct ControlBlockPointer : ControlBlockBase {
    // Variables
    int strong_ = 0;
    int weak_ = 0;
    T* ptr_;
    //
    ControlBlockPointer(T* ptr) : ptr_(ptr){};
    ~ControlBlockPointer() override = default;
    // Functions
    void MinusStrong() override {
        --strong_;
        if (strong_ == 0) {
            OnZeroStrong();
        }
        if (strong_ == 0 && weak_ == 0) {
            OnZeroWeak();
        }
    }
    void PlusStrong() override {
        ++strong_;
    }

    void MinusWeak() override {
        --weak_;
        if (strong_ == 0 && weak_ == 0) {
            OnZeroWeak();
        }
    }

    void PlusWeak() override {
        ++weak_;
    }

    int& Strong() override {
        return strong_;
    }
    int& Weak() override {
        return weak_;
    }
    void OnZeroStrong() override {
        delete ptr_;
        ptr_ = nullptr;
    }
    void OnZeroWeak() override {
        delete this;
    }
};

template <typename T>
struct ControlBlockMakeShared : ControlBlockBase {
    // Variables
    int strong_ = 0;
    int weak_ = 0;
    alignas(T) char holder[sizeof(T)];
    //
    template <typename... Args>
    ControlBlockMakeShared(Args&&... args) {
        new (holder) T(std::forward<decltype(args)>(args)...);
    }
    ~ControlBlockMakeShared() override = default;
    // Functions
    void MinusStrong() override {
        --strong_;
        if (strong_ == 0) {
            OnZeroStrong();
        }
        if (strong_ == 0 && weak_ == 0) {
            OnZeroWeak();
        }
    }
    void PlusStrong() override {
        ++strong_;
    }

    void MinusWeak() override {
        --weak_;
        if (strong_ == 0 && weak_ == 0) {
            OnZeroWeak();
        }
    }

    void PlusWeak() override {
        ++weak_;
    }

    int& Strong() override {
        return strong_;
    }
    int& Weak() override {
        return weak_;
    }
    void OnZeroStrong() override {
        reinterpret_cast<T*>(&holder)->~T();
    }
    void OnZeroWeak() override {
        delete this;
    }
    T* GetHolder() {
        return reinterpret_cast<T*>(holder);
    }
};

template <typename T>
class SharedPtr {
private:
    template <typename U>
    friend class WeakPtr;

    friend class WeakPtr<T>;

    template <typename U>
    friend class SharedPtr;

    ControlBlockBase* block_;
    T* ptr_;

    void PlusShared() {
        if (block_ != nullptr) {
            block_->PlusStrong();
        }
    }

    void MinusShared() {
        if (block_ != nullptr) {
            block_->MinusStrong();
        }
    }

public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors
    SharedPtr() : block_(nullptr), ptr_(nullptr) {
    }
    SharedPtr(std::nullptr_t) : block_(nullptr), ptr_(nullptr) {
    }

    template <typename U>
    explicit SharedPtr(U* ptr) {
        ptr_ = ptr;
        block_ = new ControlBlockPointer<U>(ptr);
        PlusShared();
    }

    SharedPtr(const SharedPtr& other) {
        ptr_ = other.ptr_;
        block_ = other.block_;
        PlusShared();
    }

    template <typename U>
    SharedPtr(const SharedPtr<U>& other) {
        ptr_ = other.ptr_;
        block_ = other.block_;
        PlusShared();
    }

    SharedPtr(SharedPtr&& other) {
        ptr_ = other.ptr_;
        block_ = other.block_;
        other.ptr_ = nullptr;
        other.block_ = nullptr;
    }

    template <typename U>
    SharedPtr(SharedPtr<U>&& other) {
        ptr_ = other.ptr_;
        block_ = other.block_;
        other.ptr_ = nullptr;
        other.block_ = nullptr;
    }

    // for MakeShared
    SharedPtr(ControlBlockBase* block_make_shared, T* ptr) {
        ptr_ = ptr;
        block_ = block_make_shared;
        PlusShared();
    }

    // Aliasing constructor
    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other, T* ptr) {
        ptr_ = ptr;
        block_ = other.block_;
        PlusShared();
    }


    // Promote `WeakPtr`
    // #11 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    explicit SharedPtr(const WeakPtr<T>& other) {
        if (!other.Expired()) {
            ptr_ = other.ptr_;
            block_ = other.block_;
            PlusShared();
        } else {
            throw BadWeakPtr();
        }
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    SharedPtr& operator=(const SharedPtr& other) {
        if (ptr_ != other.ptr_) {
            MinusShared();
            ptr_ = other.ptr_;
            block_ = other.block_;
            PlusShared();
        }
        return *this;
    }

    SharedPtr& operator=(SharedPtr&& other) {
        MinusShared();
        ptr_ = other.ptr_;
        block_ = other.block_;
        other.ptr_ = nullptr;
        other.block_ = nullptr;
        return *this;
    }

    template <typename U>
    SharedPtr<T>& operator=(SharedPtr<U>&& other) {
        MinusShared();
        ptr_ = other.ptr_;
        block_ = other.block_;
        other.ptr_ = nullptr;
        other.block_ = nullptr;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~SharedPtr() {
        MinusShared();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        MinusShared();
        ptr_ = nullptr;
        block_ = nullptr;
    }

    template <typename U>
    void Reset(U* ptr) {
        MinusShared();
        ptr_ = ptr;
        block_ = new ControlBlockPointer<U>(ptr);
        PlusShared();
    }

    void Swap(SharedPtr& other) {
        std::swap(*this, other);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return ptr_;
    }

    T& operator*() const {
        return *ptr_;
    }

    T* operator->() const {
        return ptr_;
    }

    size_t UseCount() const {
        if (block_ == nullptr) {
            return 0;
        }
        return block_->Strong();
    }

    explicit operator bool() const {
        if (ptr_ == nullptr) {
            return false;
        }
        return true;
    }
};

template <typename T, typename U>
inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right) {
    if (left.Get() == right.Get()) {
        return true;
    }
    return false;
}

// Allocate memory only once
template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    ControlBlockMakeShared<T>* temp =
        new ControlBlockMakeShared<T>(std::forward<decltype(args)>(args)...);
    return SharedPtr<T>(temp, temp->GetHolder());
}
