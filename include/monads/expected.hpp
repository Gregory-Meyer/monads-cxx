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

#ifndef MONADS_EXPECTED_HPP
#define MONADS_EXPECTED_HPP

#include <monads/detail/expected.hpp>
#include <monads/detail/invoke.hpp>

#include <exception>
#include <functional>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>

namespace monads {

class BadExpectedAccess : public std::exception {
public:
    BadExpectedAccess() noexcept = default;

    virtual ~BadExpectedAccess() = default;

    const char* what() const noexcept override {
        return "monads::BadExpectedAccess";
    }
};

struct InPlaceValueType { };

struct InPlaceErrorType { };

template <typename T, typename E>
class Expected;

template <typename T, typename E, typename ...Ts,
          std::enable_if_t<std::is_constructible<T, Ts&&...>::value, int> = 0>
constexpr Expected<T, E> make_expected(Ts &&...ts)
noexcept(std::is_nothrow_constructible<T, Ts&&...>::value);

template <
    typename T,
    typename E,
    typename U,
    typename ...Ts,
    std::enable_if_t<std::is_constructible<T, std::initializer_list<U>&, Ts&&...>::value, int> = 0
>
constexpr Expected<T, E> make_expected(std::initializer_list<U> list, Ts &&...ts)
noexcept(std::is_nothrow_constructible<T, std::initializer_list<U>&, Ts&&...>::value);

template <typename T, typename E, typename ...Ts,
          std::enable_if_t<std::is_constructible<E, Ts&&...>::value, int> = 0>
constexpr Expected<T, E> make_unexpected(Ts &&...ts)
noexcept(std::is_nothrow_constructible<E, Ts&&...>::value);

template <
    typename T,
    typename E,
    typename U,
    typename ...Ts,
    std::enable_if_t<std::is_constructible<E, std::initializer_list<U>&, Ts&&...>::value, int> = 0
>
constexpr Expected<T, E> make_unexpected(std::initializer_list<U> list, Ts &&...ts)
noexcept(std::is_nothrow_constructible<E, std::initializer_list<U>&, Ts&&...>::value);

template <typename T, typename E>
class Expected {
public:
    template <typename U, typename F>
    friend class Expected;

    template <typename ...Ts, std::enable_if_t<std::is_constructible<T, Ts&&...>::value, int> = 0>
    constexpr explicit Expected(InPlaceValueType, Ts &&...ts)
    noexcept(std::is_nothrow_constructible<T, Ts&&...>::value)
    : storage_(detail::ValueTag{ }, std::forward<Ts>(ts)...) { }

    template <
        typename U,
        typename ...Ts,
        std::enable_if_t<
            std::is_constructible<T, std::initializer_list<U>&, Ts&&...>::value,
            int
        > = 0
    >
    constexpr explicit Expected(InPlaceValueType, std::initializer_list<U> list, Ts &&...ts)
    noexcept(std::is_nothrow_constructible<T, std::initializer_list<U>&, Ts&&...>::value)
    : Expected{ InPlaceValueType{ }, list, std::forward<Ts>(ts)... }  { }

    template <typename ...Ts, std::enable_if_t<std::is_constructible<E, Ts&&...>::value, int> = 0>
    constexpr explicit Expected(InPlaceErrorType, Ts &&...ts)
    noexcept(std::is_nothrow_constructible<E, Ts&&...>::value)
    : storage_(detail::ErrorTag{ }, std::forward<Ts>(ts)...) { }

    template <
        typename U,
        typename ...Ts,
        std::enable_if_t<
            std::is_constructible<E, std::initializer_list<U>&, Ts&&...>::value,
            int
        > = 0
    >
    constexpr explicit Expected(InPlaceErrorType, std::initializer_list<U> list, Ts &&...ts)
    noexcept(std::is_nothrow_constructible<E, std::initializer_list<U>&, Ts&&...>::value)
    : Expected{ InPlaceErrorType{ }, list, std::forward<Ts>(ts)... }  { }


    template <std::enable_if_t<
        std::is_copy_constructible<T>::value && std::is_copy_constructible<E>::value,
        int
    > = 0>
    Expected(const Expected &other) noexcept(
        std::is_nothrow_copy_constructible<T>::value
        && std::is_nothrow_copy_constructible<E>::value
    ) {
        if (other.has_value()) {
            construct_value(other.unwrap());
        } else if (other.has_error()) {
            construct_error(other.unwrap_error());
        }
    }

    template <std::enable_if_t<
        std::is_move_constructible<T>::value && std::is_move_constructible<E>::value,
        int
    > = 0>
    Expected(Expected &&other) noexcept(
        std::is_nothrow_move_constructible<T>::value
        && std::is_nothrow_move_constructible<E>::value
    ) {
        if (other.has_value()) {
            construct_value(std::move(other).unwrap());
        } else if (other.has_error()) {
            construct_error(std::move(other).unwrap_error());
        }
    }

    template <typename U, typename F, std::enable_if_t<
        std::is_constructible<T, const U&>::value && std::is_constructible<E, const F&>::value
        && std::is_convertible<const U&, T>::value && std::is_convertible<const F&, E>::value,
        int
    > = 0>
    Expected(const Expected<U, F> &other) noexcept(
        std::is_nothrow_constructible<T, const U&>::value
        && std::is_nothrow_constructible<E, const F&>::value
    ) {
        if (other.has_value()) {
            construct_value(other.unwrap());
        } else if (other.has_error()) {
            construct_error(other.unwrap_error());
        }
    }

    template <typename U, typename F, std::enable_if_t<
        std::is_constructible<T, const U&>::value && std::is_constructible<E, const F&>::value
        && (!std::is_convertible<const U&, T>::value || !std::is_convertible<const F&, E>::value),
        int
    > = 0>
    explicit Expected(const Expected<U, F> &other) noexcept(
        std::is_nothrow_constructible<T, const U&>::value
        && std::is_nothrow_constructible<E, const F&>::value
    ) {
        if (other.has_value()) {
            construct_value(other.unwrap());
        } else if (other.has_error()) {
            construct_error(other.unwrap_error());
        }
    }

    template <typename U, typename F, std::enable_if_t<
        std::is_constructible<T, U&&>::value && std::is_constructible<E, F&&>::value
        && std::is_convertible<U&&, T>::value && std::is_convertible<F&&, E>::value,
        int
    > = 0>
    Expected(Expected<U, F> &&other) noexcept(
        std::is_nothrow_constructible<T, U&&>::value
        && std::is_nothrow_constructible<E, F&&>::value
    ) {
        if (other.has_value()) {
            construct_value(std::move(other).unwrap());
        } else if (other.has_error()) {
            construct_error(std::move(other).unwrap_error());
        }
    }

    template <typename U, typename F, std::enable_if_t<
        std::is_convertible<T, U&&>::value && std::is_convertible<E, F&&>::value
        && (!std::is_convertible<U&, T>::value || !std::is_convertible<F&&, E>::value),
        int
    > = 0>
    explicit Expected(Expected<U, F> &&other) noexcept(
        std::is_nothrow_constructible<T, U&&>::value
        && std::is_nothrow_constructible<E, F&&>::value
    ) {
        if (other.has_value()) {
            construct_value(std::move(other).unwrap());
        } else if (other.has_error()) {
            construct_error(std::move(other).unwrap_error());
        }
    }

    template <typename U, std::enable_if_t<
        std::is_constructible<T, U&&>::value && !std::is_constructible<E, U&&>::value
        && std::is_convertible<U&&, T>::value && !std::is_convertible<U&&, E>::value,
        int
    > = 0>
    constexpr Expected(U &&u) noexcept(std::is_nothrow_constructible<T, U&&>::value)
    : storage_{ detail::ValueTag{ }, std::forward<U>(u) } { }

    template <typename U, std::enable_if_t<
        std::is_constructible<T, U&&>::value && !std::is_constructible<E, U&&>::value
        && !std::is_convertible<U&&, T>::value && !std::is_convertible<U&&, E>::value,
        int
    > = 0>
    constexpr explicit Expected(U &&u) noexcept(std::is_nothrow_constructible<T, U&&>::value)
    : storage_{ detail::ValueTag{ }, std::forward<U>(u) } { }

    template <typename F, std::enable_if_t<
        !std::is_constructible<T, F&&>::value && std::is_constructible<E, F&&>::value
        && !std::is_convertible<F&&, T>::value && std::is_convertible<F&&, E>::value,
        int
    > = 0>
    constexpr Expected(F &&f) noexcept(std::is_nothrow_constructible<T, F&&>::value)
    : storage_{ detail::ValueTag{ }, std::forward<F>(f) } { }

    template <typename F, std::enable_if_t<
        !std::is_constructible<T, F&&>::value && std::is_constructible<E, F&&>::value
        && !std::is_convertible<F&&, T>::value && !std::is_convertible<F&&, E>::value,
        int
    > = 0>
    constexpr explicit Expected(F &&f) noexcept(std::is_nothrow_constructible<E, F&&>::value)
    : storage_{ detail::ErrorTag{ }, std::forward<F>(f) } { }

    template <std::enable_if_t<
        std::is_copy_constructible<T>::value && std::is_copy_constructible<E>::value
        && std::is_copy_assignable<T>::value && std::is_copy_assignable<E>::value,
        int
    > = 0>
    Expected& operator=(const Expected &other) noexcept(
        std::is_nothrow_copy_constructible<T>::value
        && std::is_nothrow_copy_constructible<E>::value
        && std::is_nothrow_copy_assignable<T>::value
        && std::is_nothrow_copy_assignable<E>::value
    ) {
        if (other.has_value()) {
            if (has_value()) {
                unwrap() = other.unwrap();
            } else if (other.has_error()) {
                emplace(other.unwrap());
            }
        } else if (other.has_error()) {
            if (has_error()) {
                unwrap_error() = other.unwrap_error();
            } else if (other.has_value()) {
                emplace_error(other.unwrap_error());
            }
        }
    }

    template <std::enable_if_t<
        std::is_move_constructible<T>::value && std::is_move_constructible<E>::value
        && std::is_move_assignable<T>::value && std::is_move_assignable<E>::value,
        int
    > = 0>
    Expected& operator=(Expected &&other) noexcept(
        std::is_nothrow_move_constructible<T>::value
        && std::is_nothrow_move_constructible<E>::value
        && std::is_nothrow_move_assignable<E>::value
        && std::is_nothrow_move_assignable<T>::value
    ) {
        if (other.has_value()) {
            if (has_value()) {
                unwrap() = std::move(other).unwrap();
            } else {
                emplace(std::move(other).unwrap());
            }
        } else if (other.has_error()) {
            if (has_error()) {
                unwrap_error() = std::move(other).unwrap();
            } else {
                emplace_error(std::move(other).unwrap_error());
            }
        }
    }

    constexpr bool has_value() const noexcept {
        return storage_.state == detail::ExpectedState::Value;
    }

    constexpr bool has_error() const noexcept {
        return storage_.state == detail::ExpectedState::Error;
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
            throw BadExpectedAccess{ };
        }

        return unwrap();
    }

    constexpr const T& value() const & {
        if (!has_value()) {
            throw BadExpectedAccess{ };
        }

        return unwrap();
    }

    constexpr T&& value() && {
        if (!has_value()) {
            throw BadExpectedAccess{ };
        }

        return unwrap();
    }

    constexpr const T&& value() const && {
        if (!has_value()) {
            throw BadExpectedAccess{ };
        }

        return unwrap();
    }

    constexpr E& error() & {
        if (!has_error()) {
            throw BadExpectedAccess{ };
        }

        return storage_.error;
    }

    constexpr const E& error() const & {
        if (!has_error()) {
            throw BadExpectedAccess{ };
        }

        return unwrap_error();
    }

    constexpr E&& error() && {
        if (!has_error()) {
            throw BadExpectedAccess{ };
        }

        return unwrap_error();
    }

    constexpr const E&& error() const && {
        if (!has_error()) {
            throw BadExpectedAccess{ };
        }

        return unwrap_error();
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

    constexpr E& unwrap_error() & {
        return storage_.error;
    }

    constexpr const E& unwrap_error() const & {
        return storage_.error;
    }

    constexpr E&& unwrap_error() && {
        return std::move(storage_.error);
    }

    constexpr const E&& unwrap_error() const && {
        return std::move(storage_.error);
    }

    template <typename ...Ts, std::enable_if_t<std::is_constructible<T, Ts&&...>::value, int> = 0>
    T& emplace(Ts &&...ts) noexcept(std::is_nothrow_constructible<T, Ts&&...>::value) {
        storage_.reset();

        return construct_value(std::forward<Ts>(ts)...);
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
    noexcept(std::is_nothrow_constructible<T, std::initializer_list<U>&, Ts&&...>::value) {
        return emplace(list, std::forward<Ts>(ts)...);
    }

    template <typename ...Ts, std::enable_if_t<std::is_constructible<E, Ts&&...>::value, int> = 0>
    E& emplace_error(Ts &&...ts) noexcept(std::is_nothrow_constructible<E, Ts&&...>::value) {
        storage_.reset();

        return construct_error(std::forward<Ts>(ts)...);
    }

    template <
        typename U,
        typename ...Ts,
        std::enable_if_t<
            std::is_constructible<E, std::initializer_list<U>&, Ts&&...>::value,
            int
        > = 0
    >
    E& emplace_error(std::initializer_list<U> list, Ts &&...ts)
    noexcept(std::is_nothrow_constructible<E, std::initializer_list<U>&, Ts&&...>::value) {
        return emplace_error(list, std::forward<Ts>(ts)...);
    }

    template <
        typename C,
        std::enable_if_t<
            detail::is_invocable<C&&, const T&>::value
            && std::is_copy_constructible<E>::value,
            int
        > = 0
    >
    constexpr Expected<detail::invoke_result_t<C&&, const T&>, E> map(C &&callable) const &
    noexcept(
        detail::is_nothrow_invocable<C&&, const T&>::value
        && std::is_nothrow_copy_constructible<E>::value
    ) {
        using U = detail::invoke_result_t<C&&, const T&>;

        if (has_error()) {
            return make_unexpected<U, E>(unwrap_error());
        } else if (!has_value()) {
            return Expected<U, E>{ detail::Monostate{ } };
        }

        auto &&result = detail::invoke(std::forward<C>(callable), unwrap());

        return make_expected<U, E>(std::forward<decltype(result)>(result));
    }

    template <
        typename C,
        std::enable_if_t<
            detail::is_invocable<C&&, T&&>::value
            && std::is_move_constructible<E>::value,
            int
        > = 0
    >
    constexpr Expected<detail::invoke_result_t<C&&, T&&>, E> map(C &&callable) && noexcept(
        detail::is_nothrow_invocable<C&&, T&&>::value
        && std::is_nothrow_move_constructible<E>::value
    ) {
        using U = detail::invoke_result_t<C&&, T&&>;

        if (has_error()) {
            return make_unexpected<U, E>(std::move(*this).unwrap_error());
        } else if (!has_value()) {
            return Expected<U, E>{ detail::Monostate{ } };
        }

        auto &&result = detail::invoke(std::forward<C>(callable),
                                       std::move(*this).unwrap());

        return make_expected<U, E>(std::forward<decltype(result)>(result));
    }

    template <
        typename C,
        std::enable_if_t<
            detail::is_invocable<C&&, const E&>::value
            && std::is_copy_constructible<T>::value,
            int
        > = 0
    >
    constexpr Expected<T, detail::invoke_result_t<C&&, const E&>> map_error(C &&callable) const &
    noexcept(
        detail::is_nothrow_invocable<C&&, const E&>::value
        && std::is_nothrow_copy_constructible<T>::value
    ) {
        using F = detail::invoke_result_t<C&&, const E&>;

        if (has_value()) {
            return make_expected<T, F>(unwrap());
        } else if (!has_error()) {
            return Expected<T, F>{ detail::Monostate{ } };
        }

        auto &&result = detail::invoke(std::forward<C>(callable),
                                       unwrap_error());

        return make_unexpected<T, F>(std::forward<decltype(result)>(result));
    }

    template <
        typename C,
        std::enable_if_t<
            detail::is_invocable<C&&, E&&>::value
            && std::is_move_constructible<T>::value,
            int
        > = 0
    >
    constexpr Expected<T, detail::invoke_result_t<C&&, E&&>> map_error(C &&callable) && noexcept(
        detail::is_nothrow_invocable<C&&, E&&>::value
        && std::is_nothrow_move_constructible<T>::value
    ) {
        using F = detail::invoke_result_t<C&&, E&&>;

        if (has_value()) {
            return make_expected<T, F>(std::move(*this).unwrap());
        } else if (!has_error()) {
            return Expected<T, F>{ detail::Monostate{ } };
        }

        auto &&result = detail::invoke(std::forward<C>(callable),
                                       std::move(*this).unwrap_error());

        return make_unexpected<T, F>(std::forward<decltype(result)>(result));
    }

private:
    constexpr explicit Expected(detail::Monostate) noexcept { }

    template <typename ...Ts, std::enable_if_t<std::is_constructible<T, Ts&&...>::value, int> = 0>
    T& construct_value(Ts &&...args) noexcept(std::is_nothrow_constructible<T, Ts&&...>::value) {
        try {
            ::new(std::addressof(storage_.value)) T(std::forward<Ts>(args)...);
            storage_.state = detail::ExpectedState::Value;

            return storage_.value;
        } catch (...) {
            storage_.state = detail::ExpectedState::Monostate;

            throw;
        }
    }

    template <typename ...Ts, std::enable_if_t<std::is_constructible<E, Ts&&...>::value, int> = 0>
    E& construct_error(Ts &&...args) noexcept(std::is_nothrow_constructible<E, Ts&&...>::value) {
        try {
            ::new(std::addressof(storage_.value)) E(std::forward<Ts>(args)...);
            storage_.state = detail::ExpectedState::Error;

            return storage_.error;
        } catch (...) {
            storage_.state = detail::ExpectedState::Monostate;

            throw;
        }
    }


    detail::ExpectedStorage<T, E> storage_;
};

template <typename T, typename E, typename ...Ts,
          std::enable_if_t<std::is_constructible<T, Ts&&...>::value, int>>
constexpr Expected<T, E> make_expected(Ts &&...ts)
noexcept(std::is_nothrow_constructible<T, Ts&&...>::value) {
    return Expected<T, E>{ InPlaceValueType{ }, std::forward<Ts>(ts)... };
}

template <
    typename T,
    typename E,
    typename U,
    typename ...Ts,
    std::enable_if_t<std::is_constructible<T, std::initializer_list<U>&, Ts&&...>::value, int>
>
constexpr Expected<T, E> make_expected(std::initializer_list<U> list, Ts &&...ts)
noexcept(std::is_nothrow_constructible<T, std::initializer_list<U>&, Ts&&...>::value) {
    return make_expected<T, E>(list, std::forward<Ts>(ts)...);
}

template <typename T, typename E, typename ...Ts,
          std::enable_if_t<std::is_constructible<E, Ts&&...>::value, int>>
constexpr Expected<T, E> make_unexpected(Ts &&...ts)
noexcept(std::is_nothrow_constructible<E, Ts&&...>::value) {
    return Expected<T, E>{ InPlaceErrorType{ }, std::forward<Ts>(ts)... };
}

template <
    typename T,
    typename E,
    typename U,
    typename ...Ts,
    std::enable_if_t<std::is_constructible<E, std::initializer_list<U>&, Ts&&...>::value, int>
>
constexpr Expected<T, E> make_unexpected(std::initializer_list<U> list, Ts &&...ts)
noexcept(std::is_nothrow_constructible<E, std::initializer_list<U>&, Ts&&...>::value) {
    return make_unexpected<T, E>(list, std::forward<Ts>(ts)...);
}

template <
    typename C,
    typename ...As,
    std::enable_if_t<detail::is_invocable<C&&, As&&...>::value, int> = 0
>
Expected<detail::invoke_result_t<C&&, As&&...>, std::exception_ptr> try_invoke(
    C &&callable,
    As &&...args
) noexcept(std::is_nothrow_constructible<
    detail::invoke_result_t<C&&, As&&...>,
    detail::invoke_result_t<C&&, As&&...>
>::value) {
    using Result = detail::invoke_result_t<C&&, As&&...>;

    try {
        auto &&result = detail::invoke(std::forward<C>(callable), std::forward<As>(args)...);

        return make_expected<Result, std::exception_ptr>(std::forward<decltype(result)>(result));
    } catch (...) {
        return make_unexpected<Result, std::exception_ptr>(std::current_exception());
    }
}

} // namespace monads

#endif
