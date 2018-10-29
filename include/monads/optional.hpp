// BSD 3-Clause License
//
// Copyright (c) 2018, Gregory Meyer
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// * Neither the name of the copyright holder nor the names of its
//   contributors may be used to endorse or promote products derived from
//   this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS AS IS
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#ifndef MONADS_OPTIONAL_HPP
#define MONADS_OPTIONAL_HPP

#include <monads/detail/common.hpp>
#include <monads/detail/invoke.hpp>
#include <monads/detail/optional.hpp>

#include <stdexcept>
#include <type_traits>
#include <utility>

namespace monads {

class BadOptionalAccess : public std::exception {
public:
    BadOptionalAccess() noexcept = default;

    virtual ~BadOptionalAccess() = default;

    const char* what() const noexcept override {
        return "monads::BadOptionalAccess";
    }
};

struct InPlaceType { };

template <typename T>
class Optional;

template <typename T, typename ...Ts,
          std::enable_if_t<std::is_constructible<T, Ts&&...>::value, int> = 0>
constexpr Optional<T> make_optional(Ts &&...ts)
noexcept(std::is_nothrow_constructible<T, Ts&&...>::value);

template <
    typename T,
    typename U,
    typename ...Ts,
    std::enable_if_t<
        std::is_constructible<T, std::initializer_list<U>&, Ts&&...>::value,
        int
    > = 0
>
constexpr Optional<T> make_optional(std::initializer_list<U> list, Ts &&...ts)
noexcept(std::is_nothrow_constructible<
    T,
    std::initializer_list<U>&,
    Ts&&...
>::value);

template <typename T>
class Optional {
public:
    template <typename U>
    friend class Optional;

    constexpr Optional() noexcept = default;

    template <
        typename ...Ts,
        std::enable_if_t<std::is_constructible<T, Ts&&...>::value, int> = 0
    >
    constexpr explicit Optional(InPlaceType, Ts &&...ts)
    noexcept(std::is_nothrow_constructible<T, Ts&&...>::value)
    : storage_(detail::ValueTag{ }, std::forward<Ts>(ts)...) { }

    template <
        typename U,
        typename ...Ts,
        std::enable_if_t<std::is_constructible<
            T,
            std::initializer_list<U>&,
            Ts&&...
        >::value, int> = 0
    >
    constexpr explicit Optional(InPlaceType, std::initializer_list<U> list,
                                Ts &&...ts)
    noexcept(std::is_nothrow_constructible<
        T,
        std::initializer_list<U>&,
        Ts&&...
    >::value)
    : storage_(detail::ValueTag{ }, list, std::forward<Ts>(ts)...) { }

    template <std::enable_if_t<std::is_copy_constructible<T>::value, int> = 0>
    Optional(const Optional &other)
    noexcept(std::is_nothrow_copy_constructible<T>::value) {
        if (other.has_value()) {
            construct(other.unwrap());
        }
    }

    template <std::enable_if_t<std::is_move_constructible<T>::value, int> = 0>
    Optional(Optional &&other)
    noexcept(std::is_nothrow_move_constructible<T>::value) {
        if (other.has_value()) {
            construct(std::move(other).unwrap());
        }
    }

    template <
        typename U,
        std::enable_if_t<
            std::is_constructible<T, const U&>::value
            && std::is_convertible<const U&, T>::value,
            int
        > = 0
    >
    Optional(const Optional<U> &other)
    noexcept(std::is_nothrow_constructible<T, const U&>::value) {
        if (other.has_value()) {
            construct(other.unwrap());
        }
    }

    template <
        typename U,
        std::enable_if_t<
            std::is_constructible<T, const U&>::value
            && !std::is_convertible<const U&, T>::value,
            int
        > = 0
    >
    explicit Optional(const Optional<U> &other)
    noexcept(std::is_nothrow_constructible<T, const U&>::value) {
        if (other.has_value()) {
            construct(other.unwrap());
        }
    }

    template <
        typename U,
        std::enable_if_t<
            std::is_constructible<T, U&&>::value
            && std::is_convertible<U&&, T>::value,
            int
        > = 0
    >
    Optional(Optional<U> &&other)
    noexcept(std::is_nothrow_constructible<T, U&&>::value) {
        if (other.has_value()) {
            construct(std::move(other).unwrap());
        }
    }

    template <
        typename U,
        std::enable_if_t<
            std::is_constructible<T, U&&>::value
            && !std::is_convertible<U&&, T>::value,
            int
        > = 0
    >
    explicit Optional(Optional<U> &&other)
    noexcept(std::is_nothrow_constructible<T, U&&>::value) {
        if (other.has_value()) {
            construct(std::move(other).unwrap());
        }
    }

    template <typename U, std::enable_if_t<
        std::is_constructible<T, U&&>::value
        && std::is_convertible<U&&, T>::value,
        int
    > = 0>
    constexpr Optional(U &&u)
    noexcept(std::is_nothrow_constructible<T, U&&>::value)
    : storage_{ detail::ValueTag{ }, std::forward<U>(u) } { }

    template <typename U, std::enable_if_t<
        std::is_constructible<T, U&&>::value
        && !std::is_convertible<U&&, T>::value,
        int
    > = 0>
    constexpr explicit Optional(U &&u)
    noexcept(std::is_nothrow_constructible<T, U&&>::value)
    : storage_{ detail::ValueTag{ }, std::forward<U>(u) } { }

    template <std::enable_if_t<
        std::is_copy_constructible<T>::value &&
        std::is_copy_assignable<T>::value,
        int
    > = 0>
    Optional& operator=(const Optional &other) noexcept(
        std::is_nothrow_copy_constructible<T>::value
        && std::is_nothrow_copy_assignable<T>::value
    ) {
        if (other.has_value()) {
            if (has_value()) {
                unwrap() = other.unwrap();
            } else {
                construct(other.unwrap());
            }
        } else {
            reset();
        }
    }

    constexpr bool has_value() const noexcept {
        return storage_.has_value;
    }

    constexpr explicit operator bool() const noexcept {
        return has_value();
    }

    constexpr bool operator!() const noexcept {
        return !has_value();
    }

    constexpr T& operator*() & {
        return unwrap();
    }

    constexpr const T& operator*() const & {
        return unwrap();
    }

    constexpr T&& operator*() && {
        return unwrap();
    }

    constexpr const T&& operator*() const && {
        return unwrap();
    }

    constexpr T* operator->() {
        return std::addressof(unwrap());
    }

    constexpr const T* operator->() const {
        return std::addressof(unwrap());
    }

    constexpr T& value() & {
        if (!has_value()) {
            throw BadOptionalAccess{ };
        }

        return unwrap();
    }

    constexpr const T& value() const & {
        if (!has_value()) {
            throw BadOptionalAccess{ };
        }

        return unwrap();
    }

    constexpr T&& value() && {
        if (!has_value()) {
            throw BadOptionalAccess{ };
        }

        return unwrap();
    }

    constexpr const T&& value() const && {
        if (!has_value()) {
            throw BadOptionalAccess{ };
        }

        return unwrap();
    }

    constexpr T& unwrap() & {
        return storage_.value;
    }

    constexpr const T& unwrap() const & {
        return storage_.value;
    }

    constexpr T&& unwrap() && {
        return std::move(storage_.value);
    }

    constexpr const T&& unwrap() const && {
        return std::move(storage_.value);
    }

    template <
        typename ...Ts,
        std::enable_if_t<std::is_constructible<T, Ts&&...>::value, int> = 0
    >
    T& emplace(Ts &&...ts)
    noexcept(std::is_nothrow_constructible<T, Ts&&...>::value) {
        storage_.reset();

        return construct(std::forward<Ts>(ts)...);
    }

    template <
        typename U,
        typename ...Ts,
        std::enable_if_t<
            std::is_constructible<T, std::initializer_list<U>&, Ts&&...>::value,
            int
        > = 0
    >
    T& emplace(std::initializer_list<U> list, Ts &&...ts)
    noexcept(std::is_nothrow_constructible<
        T,
        std::initializer_list<U>&, Ts&&...
    >::value) {
        return emplace(list, std::forward<Ts>(ts)...);
    }

    void reset() {
        storage_.reset();
    }

    template <
        typename C,
        std::enable_if_t<
            detail::is_invocable<C&&, const T&>::value,
            int
        > = 0
    >
    constexpr Optional<detail::invoke_result_t<C&&, const T&>>
    map(C &&callable) const & noexcept(
        detail::is_nothrow_invocable<C&&, const T&>::value
    ) {
        using U = detail::invoke_result_t<C&&, const T&>;

        if (!has_value()) {
            return Optional<U>{ };
        }

        auto &&result = detail::invoke(std::forward<C>(callable), unwrap());

        return make_optional<U>(std::forward<decltype(result)>(result));
    }

    template <
        typename C,
        std::enable_if_t<
            detail::is_invocable<C&&, T&&>::value,
            int
        > = 0
    >
    constexpr Optional<detail::invoke_result_t<C&&, T&&>>
    map(C &&callable) && noexcept(
        detail::is_nothrow_invocable<C&&, T&&>::value
    ) {
        using U = detail::invoke_result_t<C&&, T&&>;

        if (!has_value()) {
            return Optional<U>{ };
        }

        auto &&result = detail::invoke(std::forward<C>(callable),
                                       std::move(*this).unwrap());

        return make_optional<U>(std::forward<decltype(result)>(result));
    }

private:
    template <
        typename ...Ts,
        std::enable_if_t<std::is_constructible<T, Ts&&...>::value, int> = 0
    >
    T& construct(Ts &&...args)
    noexcept(std::is_nothrow_constructible<T, Ts&&...>::value) {
        try {
            ::new(std::addressof(storage_.value)) T(std::forward<Ts>(args)...);
            storage_.has_value = true;

            return storage_.value;
        } catch (...) {
            storage_.has_value = false;

            throw;
        }
    }

    template <
        typename U,
        typename ...Ts,
        std::enable_if_t<std::is_constructible<
            T,
            std::initializer_list<U>&,
            Ts&&...
        >::value, int> = 0
    >
    T& construct(std::initializer_list<U> list, Ts &&...args)
    noexcept(std::is_nothrow_constructible<
        T,
        std::initializer_list<U>&,
        Ts&&...
    >::value) {
        return construct(list, std::forward<Ts>(args)...);
    }

    detail::OptionalStorage<T> storage_;
};

template <typename T, typename ...Ts,
          std::enable_if_t<std::is_constructible<T, Ts&&...>::value, int>>
constexpr Optional<T> make_optional(Ts &&...ts)
noexcept(std::is_nothrow_constructible<T, Ts&&...>::value) {
    return Optional<T>{ InPlaceType{ }, std::forward<Ts>(ts)... };
}

template <
    typename T,
    typename U,
    typename ...Ts,
    std::enable_if_t<
        std::is_constructible<T, std::initializer_list<U>&, Ts&&...>::value,
        int
    >
>
constexpr Optional<T> make_optional(std::initializer_list<U> list, Ts &&...ts)
noexcept(std::is_nothrow_constructible<
    T,
    std::initializer_list<U>&,
    Ts&&...
>::value) {
    return make_optional<T>(list, std::forward<Ts>(ts)...);
}

template <
    typename C,
    typename ...As,
    std::enable_if_t<detail::is_invocable<C&&, As&&...>::value, int> = 0
>
Optional<detail::invoke_result_t<C&&, As&&...>> maybe_invoke(
    C &&callable,
    As &&...args
) noexcept {
    using Optional = Optional<detail::invoke_result_t<C&&, As&&...>>;

    Optional maybe_result;

    try {
        maybe_result.emplace(detail::invoke(
            std::forward<C>(callable),
            std::forward<As>(args)...
        ));
    } catch (...) { }

    return maybe_result;
}

} // namespace monads

#endif
