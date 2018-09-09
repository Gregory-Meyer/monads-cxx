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

#ifndef MONADS_DETAIL_EXPECTED_HPP
#define MONADS_DETAIL_EXPECTED_HPP

#include <monads/detail/common.hpp>

#include <tuple>
#include <type_traits>
#include <utility>

namespace monads {
namespace detail {

struct Monostate { };

enum class ExpectedState {
    Monostate,
    Value,
    Error
};

struct ValueTag { };

struct ErrorTag { };

template <typename T, typename ...Ts>
struct ValueArgs {
    static_assert(std::is_constructible<T, Ts&&...>::value, "T must be constructible from Ts&&...");

    std::tuple<Ts...> args;
};

template <typename E, typename ...Ts>
struct ErrorArgs {
    static_assert(std::is_constructible<E, Ts&&...>::value, "E must be constructible from Ts&&...");

    std::tuple<Ts...> args;
};

template <typename T, typename E, typename = void>
struct ExpectedStorage {
    union {
        Monostate monostate;
        T value;
        E error;
    };

    constexpr ExpectedStorage() noexcept : monostate{ } { }

    template <typename ...Ts, std::enable_if_t<std::is_constructible<T, Ts&&...>::value, int> = 0>
    constexpr ExpectedStorage(ValueTag, Ts &&...args)
    noexcept(std::is_nothrow_constructible<T, Ts&&...>::value)
    : value(std::forward<Ts>(args)...), state{ ExpectedState::Value } { }

    template <typename ...Ts, std::enable_if_t<std::is_constructible<E, Ts&&...>::value, int> = 0>
    constexpr ExpectedStorage(ErrorTag, Ts &&...args)
    noexcept(std::is_nothrow_constructible<E, Ts&&...>::value)
    : error(std::forward<Ts>(args)...), state{ ExpectedState::Value } { }

    constexpr void reset() noexcept {
        state = ExpectedState::Monostate;
    }

    ExpectedState state = ExpectedState::Monostate;
};

template <typename T, typename E>
struct ExpectedStorage<T, E, void_t<std::enable_if_t<
    !std::is_trivially_destructible<T>::value
    || !std::is_trivially_destructible<E>::value
>>> {
    union {
        Monostate monostate;
        T value;
        E error;
    };

    ExpectedState state = ExpectedState::Monostate;

    constexpr ExpectedStorage() noexcept : monostate{ } { }

    template <typename ...Ts, std::enable_if_t<std::is_constructible<T, Ts&&...>::value, int> = 0>
    constexpr ExpectedStorage(ValueTag, Ts &&...args)
    noexcept(std::is_nothrow_constructible<T, Ts&&...>::value)
    : value(std::forward<Ts>(args)...), state{ ExpectedState::Value } { }

    template <typename ...Ts, std::enable_if_t<std::is_constructible<E, Ts&&...>::value, int> = 0>
    constexpr ExpectedStorage(ErrorTag, Ts &&...args)
    noexcept(std::is_nothrow_constructible<E, Ts&&...>::value)
    : error(std::forward<Ts>(args)...), state{ ExpectedState::Error } { }

    ~ExpectedStorage() {
        reset();
    }

    void reset() noexcept {
        if (state == ExpectedState::Value) {
            value.T::~T();
        } else if (state == ExpectedState::Error) {
            error.E::~E();
        }

        state = ExpectedState::Monostate;
    }
};

} // namespace detail
} // namespace monads

#endif
