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
// * Redistributions in binary form must reproducne the above copyright notice,
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

#ifndef EXPECTED_EXPECTED_HPP
#define EXPECTED_EXPECTED_HPP

#include <expected/detail.hpp>

#include <exception>
#include <functional>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>

namespace expected {

class BadExpectedAccess : public std::exception {
public:
    BadExpectedAccess() noexcept : std::exception{ } { }

    virtual ~BadExpectedAccess() = default;

    const char* what() const noexcept override {
        return "expected::BadExpectedAccess";
    }
};

template <typename T, typename ...Ts>
constexpr detail::ValueArgs<T, Ts&&...> make_expected(Ts &&...ts) noexcept {
    return detail::ValueArgs<T, Ts&&...>{ std::forward<Ts>(ts)... };
}

template <typename T, typename U, typename ...Ts>
constexpr detail::ValueArgs<T, std::initializer_list<U>, Ts&&...>
make_expected(std::initializer_list<U> list, Ts &&...ts) noexcept {
    return detail::ValueArgs<T, std::initializer_list<U>, Ts&&...>{ list, std::forward<Ts>(ts)... };
}

template <typename E, typename ...Ts>
constexpr detail::ErrorArgs<E, Ts&&...> make_unexpected(Ts &&...ts) noexcept {
    return detail::ErrorArgs<E, Ts&&...>{ std::forward<Ts>(ts)... };
}

template <typename E, typename T, typename ...Ts>
constexpr detail::ErrorArgs<E, std::initializer_list<T>, Ts&&...>
make_unexpected(std::initializer_list<T> list, Ts &&...ts) noexcept {
    return detail::ErrorArgs<E, std::initializer_list<T>, Ts&&...>{ list, std::forward<Ts>(ts)... };
}

template <typename T, typename E>
class Expected {
public:
    template <std::enable_if_t<std::is_default_constructible_v<T>, int> = 0>
    constexpr Expected() noexcept(std::is_nothrow_default_constructible_v<T>)
    : value_(), state_{ detail::State::Value } { }

    template <typename ...Ts>
    constexpr Expected(detail::ValueArgs<T, Ts...> value_args)
    noexcept(std::is_nothrow_constructible_v<T, Ts&&...>)
    : Expected{ detail::ValueTag{ }, std::index_sequence_for<Ts...>{ }, value_args.args } { }

    template <typename ...Ts>
    constexpr Expected(detail::ErrorArgs<E, Ts...> error_args)
    noexcept(std::is_nothrow_constructible_v<E, Ts&&...>)
    : Expected{ detail::ErrorTag{ }, std::index_sequence_for<Ts...>{ }, error_args.args } { }

    constexpr Expected(const Expected &other) noexcept(
        std::is_nothrow_copy_constructible_v<T>
        && std::is_nothrow_copy_constructible_v<E>
    ) {
        if (other.has_value()) {
            construct_value(other.value_);
        } else if (other.has_error()) {
            construct_error(other.error_);
        }
    }

    constexpr Expected(Expected &&other) noexcept(
        std::is_nothrow_move_constructible_v<T>
        && std::is_nothrow_move_constructible_v<E>
    ) {
        if (other.has_value()) {
            construct_value(std::move(other.value_));
        } else if (other.has_error()) {
            construct_error(std::move(other.error_));
        }
    }

    constexpr Expected& operator=(const Expected &other) noexcept(
        std::is_nothrow_copy_constructible_v<T>
        && std::is_nothrow_copy_constructible_v<E>
        && std::is_nothrow_copy_assignable_v<T>
        && std::is_nothrow_copy_assignable_v<E>
    ) {
        if (other.has_value()) {
            construct_value(other.value_);
        } else if (other.has_error()) {
            construct_error(other.error_);
        }
    }

    constexpr Expected& operator=(Expected &&other) noexcept(
        std::is_nothrow_move_constructible_v<T>
        && std::is_nothrow_move_constructible_v<E>
        && std::is_nothrow_move_assignable_v<E>
        && std::is_nothrow_move_assignable_v<T>
    ) {
        if (other.has_value()) {
            if (has_value()) {
                value_ = std::move(other.value_);
            } else {
                emplace_value(std::move(other.value_));
            }
        } else if (other.has_error()) {
            construct_error(std::move(other.error_));
        }
    }

    ~Expected() {
        destroy();
    }

    constexpr bool has_value() const noexcept {
        return state_ == detail::State::Value;
    }

    constexpr bool has_error() const noexcept {
        return state_ == detail::State::Error;
    }

    constexpr explicit operator bool() const noexcept {
        return has_value();
    }

    constexpr bool operator!() const noexcept {
        return !has_value();
    }

    constexpr T& value() & {
        if (!has_value()) {
            throw BadExpectedAccess{ };
        }

        return value_;
    }

    constexpr const T& value() const & {
        if (!has_value()) {
            throw BadExpectedAccess{ };
        }

        return value_;
    }

    constexpr T&& value() && {
        if (!has_value()) {
            throw BadExpectedAccess{ };
        }

        return std::move(value_);
    }

    constexpr const T&& value() const && {
        if (!has_value()) {
            throw BadExpectedAccess{ };
        }

        return std::move(value_);
    }

    constexpr E& error() & {
        if (!has_error()) {
            throw BadExpectedAccess{ };
        }

        return error_;
    }

    constexpr const E& error() const & {
        if (!has_error()) {
            throw BadExpectedAccess{ };
        }

        return error_;
    }

    constexpr E&& error() && {
        if (!has_error()) {
            throw BadExpectedAccess{ };
        }

        return std::move(error_);
    }

    constexpr const E&& error() const && {
        if (!has_error()) {
            throw BadExpectedAccess{ };
        }

        return std::move(error_);
    }

    template <typename ...Ts, std::enable_if_t<std::is_constructible_v<T, Ts&&...>, int> = 0>
    T& emplace_value(Ts &&...ts) noexcept(std::is_nothrow_constructible_v<T, Ts&&...>) {
        destroy();

        try {
            return construct_value(std::forward<Ts>(ts)...);
        } catch (...) {
            state_ = detail::State::Monostate;

            throw;
        }
    }

    template <
        typename U,
        typename ...Ts,
        std::enable_if_t<
            std::is_constructible_v<T, std::initializer_list<U>&, Ts&&...>,
            int
        > = 0
    >
    T& emplace_value(std::initializer_list<U> list, Ts &&...ts)
    noexcept(std::is_nothrow_constructible_v<T, std::initializer_list<U>&, Ts&&...>) {
        return emplace_value(list, std::forward<Ts>(ts)...);
    }

    template <typename ...Ts, std::enable_if_t<std::is_constructible_v<E, Ts&&...>, int> = 0>
    E& emplace_error(Ts &&...ts) noexcept(std::is_nothrow_constructible_v<E, Ts&&...>) {
        try {
            return construct_error(std::forward<Ts>(ts)...);
        } catch (...) {
            state_ = detail::State::Monostate;

            throw;
        }
    }

    template <
        typename U,
        typename ...Ts,
        std::enable_if_t<
            std::is_constructible_v<E, std::initializer_list<U>&, Ts&&...>,
            int
        > = 0
    >
    E& emplace_error(std::initializer_list<U> list, Ts &&...ts)
    noexcept(std::is_nothrow_constructible_v<E, std::initializer_list<U>&, Ts&&...>) {
        return emplace_error(list, std::forward<Ts>(ts)...);
    }

private:
    template <typename ...Ts, std::enable_if_t<std::is_constructible_v<T, Ts&&...>, int> = 0>
    constexpr T& construct_value(Ts &&...ts) noexcept(std::is_nothrow_constructible_v<T, Ts&&...>) {
        ::new (std::addressof(value_)) T(std::forward<Ts>(ts)...);
        state_ = detail::State::Value;

        return value_;
    }

    template <
        typename U,
        typename ...Ts,
        std::enable_if_t<
            std::is_constructible_v<T, std::initializer_list<U>&, Ts&&...>,
            int
        > = 0
    >
    constexpr T& construct_value(std::initializer_list<U> list, Ts &&...ts)
    noexcept(std::is_nothrow_constructible_v<T, std::initializer_list<U>&, Ts&&...>) {
        return construct_value(list, std::forward<Ts>(ts)...);
    }

    template <typename ...Ts, std::enable_if_t<std::is_constructible_v<E, Ts&&...>, int> = 0>
    constexpr E& construct_error(Ts &&...ts) noexcept(std::is_nothrow_constructible_v<E, Ts&&...>) {
        ::new (std::addressof(error_)) E(std::forward<Ts>(ts)...);
        state_ = detail::State::Error;

        return error_;
    }

    template <
        typename U,
        typename ...Ts,
        std::enable_if_t<
            std::is_constructible_v<E, std::initializer_list<U>&, Ts&&...>,
            int
        > = 0
    >
    constexpr E& construct_error(std::initializer_list<U> list, Ts &&...ts)
    noexcept(std::is_nothrow_constructible_v<E, std::initializer_list<U>&, Ts&&...>) {
        return construct_error(list, std::forward<Ts>(ts)...);
    }

    constexpr void destroy() noexcept {
        if (has_value()) {
            value_.T::~T();
        } else if (has_error()) {
            error_.E::~E();
        }
    }

    template <typename U, std::size_t ...Is, std::enable_if_t<std::is_constructible_v<
        T,
        decltype((std::get<Is>(std::declval<U&&>())))...
    >, int> = 0>
    constexpr Expected(detail::ValueTag, std::index_sequence<Is...>, U &&value_args)
    noexcept(std::is_nothrow_constructible_v<
        T,
        decltype((std::get<Is>(std::forward<U>(value_args))))...
    >) : value_(std::get<Is>(std::forward<U>(value_args))...), state_{ detail::State::Value } { }

    template <typename U, std::size_t ...Is, std::enable_if_t<std::is_constructible_v<
        E,
        decltype((std::get<Is>(std::declval<U&&>())))...
    >, int> = 0>
    constexpr Expected(detail::ErrorTag, std::index_sequence<Is...>, U &&error_args)
    noexcept(std::is_nothrow_constructible_v<
        E,
        decltype((std::get<Is>(std::forward<U>(error_args))))...
    >) : error_(std::get<Is>(std::forward<U>(error_args))...), state_{ detail::State::Error } { }

    union {
        detail::Monostate monostate_{ };
        T value_;
        E error_;
    };

    detail::State state_ = detail::State::Monostate;
};

template <typename C, typename ...As, std::enable_if_t<std::is_invocable_v<C&&, As&&...>, int> = 0>
Expected<std::invoke_result_t<C&&, As&&...>, std::exception_ptr> try_invoke(
    C &&callable,
    As &&...args
) noexcept {
    using ResultT = std::invoke_result_t<C&&, As&&...>;

    try {
        auto &&result = std::invoke(std::forward<C>(callable), std::forward<As>(args)...);

        return make_expected<ResultT>(std::forward<decltype(result)>(result));
    } catch (...) {
        return make_unexpected<std::exception_ptr>(std::current_exception());
    }
}

} // namespace expected

#endif
