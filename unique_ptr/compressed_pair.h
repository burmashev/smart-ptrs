#pragma once

#include <utility>
#include <type_traits>

template <typename T, std::size_t I, bool EnableEBO = std::is_empty_v<T> && !std::is_final_v<T>>
struct CompressedPairElement {
    T value_;

    template <typename R>
    explicit CompressedPairElement(R&& v) : value_(std::forward<R>(v)){};

    CompressedPairElement() {
    }
    T& Get() {
        return value_;
    }
    const T& Get() const {
        return value_;
    }
};

template <typename T, std::size_t I>
struct CompressedPairElement<T, I, true> : public T {
    template <typename R>
    explicit CompressedPairElement(R&& v) : T(std::forward<R>(v)){};

    CompressedPairElement(){};
    T& Get() {
        return *this;
    }
    const T& Get() const {
        return *this;
    }
};

template <typename F, typename S>
class CompressedPair : private CompressedPairElement<F, 0>, private CompressedPairElement<S, 1> {
public:
    template <typename R, typename U>
    CompressedPair(R&& one, U&& two) : First(std::forward<R>(one)), Second(std::forward<U>(two)) {
    }
    CompressedPair() {
    }
    F& GetFirst() {
        return First::Get();
    }
    S& GetSecond() {
        return Second::Get();
    }
    const F& GetFirst() const {
        return First::Get();
    }
    const S& GetSecond() const {
        return Second::Get();
    }

private:
    using First = CompressedPairElement<F, 0>;
    using Second = CompressedPairElement<S, 1>;
};
