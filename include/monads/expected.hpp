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

template <typename T, typename ...Ts>
constexpr detail::ValueArgs<T, Ts&&...> make_expected(Ts &&...ts) noexcept {
    return detail::ValueArgs<T, Ts&&...>{ { std::forward<Ts>(ts)... } };
}

template <typename T, typename U, typename ...Ts>
constexpr detail::ValueArgs<T, std::initializer_list<U>&, Ts&&...>
make_expected(std::initializer_list<U> list, Ts &&...ts) noexcept {
    return detail::ValueArgs<T, std::initializer_list<U>&, Ts&&...>{ { list, std::forward<Ts>(ts)... } };
}

template <typename E, typename ...Ts>
constexpr detail::ErrorArgs<E, Ts&&...> make_unexpected(Ts &&...ts) noexcept {
    return detail::ErrorArgs<E, Ts&&...>{ { std::forward<Ts>(ts)... } };
}

template <typename E, typename T, typename ...Ts>
constexpr detail::ErrorArgs<E, std::initializer_list<T>&, Ts&&...>
make_unexpected(std::initializer_list<T> list, Ts &&...ts) noexcept {
    return detail::ErrorArgs<E, std::initializer_list<T>&, Ts&&...>{ { list, std::forward<Ts>(ts)... } };
}

template <typename T, typename E>
class Expected {
public:
    template <std::enable_if_t<std::is_default_constructible<T>::value, int> = 0>
    constexpr Expected() noexcept(std::is_nothrow_default_constructible<T>::value)
    : storage_{ detail::ValueTag{ } } { }

    template <typename ...Ts>
    constexpr Expected(detail::ValueArgs<T, Ts...> value_args)
    noexcept(std::is_nothrow_constructible<T, Ts&&...>::value)
    : Expected{ detail::ValueTag{ }, value_args.args, std::index_sequence_for<Ts...>{ } } { }

    template <typename ...Ts>
    constexpr Expected(detail::ErrorArgs<E, Ts...> error_args)
    noexcept(std::is_nothrow_constructible<E, Ts&&...>::value)
    : Expected{ detail::ErrorTag{ }, error_args.args, std::index_sequence_for<Ts...>{ } } { }

    Expected(const Expected &other) noexcept(
        std::is_nothrow_copy_constructible<T>::value
        && std::is_nothrow_copy_constructible<E>::value
    ) {
        if (other.has_value()) {
            construct_value(other.storage_.value);
        } else if (other.has_error()) {
            construct_error(other.storage_.value);
        }
    }

    Expected(Expected &&other) noexcept(
        std::is_nothrow_move_constructible<T>::value
        && std::is_nothrow_move_constructible<E>::value
    ) {
        if (other.has_value()) {
            construct_value(std::move(other.storage_.value));
        } else if (other.has_error()) {
            construct_error(std::move(other.storage_.error));
        }
    }

    Expected& operator=(const Expected &other) noexcept(
        std::is_nothrow_copy_constructible<T>::value
        && std::is_nothrow_copy_constructible<E>::value
        && std::is_nothrow_copy_assignable<T>::value
        && std::is_nothrow_copy_assignable<E>::value
    ) {
        if (other.has_value()) {
            if (has_value()) {
                storage_.value = other.storage_.value;
            } else if (other.has_error()) {
                emplace_value(other.storage_.value);
            }
        } else if (other.has_error()) {
            if (has_error()) {
                storage_.error = other.storage_.error;
            } else if (other.has_value()) {
                emplace_error(other.storage_.error);
            }
        }
    }

    Expected& operator=(Expected &&other) noexcept(
        std::is_nothrow_move_constructible<T>::value
        && std::is_nothrow_move_constructible<E>::value
        && std::is_nothrow_move_assignable<E>::value
        && std::is_nothrow_move_assignable<T>::value
    ) {
        if (other.has_value()) {
            if (has_value()) {
                storage_.value = std::move(other.storage_.value);
            } else {
                emplace_value(std::move(other.storage_.value));
            }
        } else if (other.has_error()) {
            if (has_error()) {
                storage_.error = std::move(other.storage_.error);
            } else {
                emplace_error(std::move(other.storage_.error));
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

    constexpr T& value() & {
        if (!has_value()) {
            throw BadExpectedAccess{ };
        }

        return storage_.value;
    }

    constexpr const T& value() const & {
        if (!has_value()) {
            throw BadExpectedAccess{ };
        }

        return storage_.value;
    }

    constexpr T&& value() && {
        if (!has_value()) {
            throw BadExpectedAccess{ };
        }

        return std::move(storage_.value);
    }

    constexpr const T&& value() const && {
        if (!has_value()) {
            throw BadExpectedAccess{ };
        }

        return std::move(storage_.value);
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

        return storage_.error;
    }

    constexpr E&& error() && {
        if (!has_error()) {
            throw BadExpectedAccess{ };
        }

        return std::move(storage_.error);
    }

    constexpr const E&& error() const && {
        if (!has_error()) {
            throw BadExpectedAccess{ };
        }

        return std::move(storage_.error);
    }

    template <typename ...Ts, std::enable_if_t<std::is_constructible<T, Ts&&...>::value, int> = 0>
    T& emplace_value(Ts &&...ts) noexcept(std::is_nothrow_constructible<T, Ts&&...>::value) {
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
    T& emplace_value(std::initializer_list<U> list, Ts &&...ts)
    noexcept(std::is_nothrow_constructible<T, std::initializer_list<U>&, Ts&&...>::value) {
        return emplace_value(list, std::forward<Ts>(ts)...);
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

private:
    template <typename U, std::size_t ...Is>
    constexpr Expected(detail::ValueTag, U &&u, std::index_sequence<Is...>)
    : storage_{ detail::ValueTag{ }, std::get<Is>(std::forward<U>(u))... } { }

    template <typename U, std::size_t ...Is>
    constexpr Expected(detail::ErrorTag, U &&u, std::index_sequence<Is...>)
    : storage_{ detail::ErrorTag{ }, std::get<Is>(std::forward<U>(u))... } { }

    template <typename ...Ts, std::enable_if_t<std::is_constructible<T, Ts&&...>::value, int> = 0>
    T& construct_value(Ts &&...args) noexcept(std::is_nothrow_constructible<T, Ts&&...>::value) {
        ::new(std::addressof(storage_.value)) T(std::forward<Ts>(args)...);
        storage_.state = detail::ExpectedState::Value;

        return storage_.value;
    }

    template <typename ...Ts, std::enable_if_t<std::is_constructible<E, Ts&&...>::value, int> = 0>
    E& construct_error(Ts &&...args) noexcept(std::is_nothrow_constructible<E, Ts&&...>::value) {
        ::new(std::addressof(storage_.value)) E(std::forward<Ts>(args)...);
        storage_.state = detail::ExpectedState::Error;

        return storage_.error;
    }


    detail::ExpectedStorage<T, E> storage_;
};

template <
    typename C,
    typename ...As,
    std::enable_if_t<detail::is_invocable<C&&, As&&...>::value, int> = 0
>
Expected<detail::invoke_result_t<C&&, As&&...>, std::exception_ptr> try_invoke(
    C &&callable,
    As &&...args
) noexcept {
    using ResultT = detail::invoke_result_t<C&&, As&&...>;

    try {
        auto &&result = detail::invoke(std::forward<C>(callable), std::forward<As>(args)...);

        return make_expected<ResultT>(std::forward<decltype(result)>(result));
    } catch (...) {
        return make_unexpected<std::exception_ptr>(std::current_exception());
    }
}

} // namespace monads

#endif
