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

#include <monads/detail/expected_impl.hpp>
#include <monads/detail/try_invoke.hpp>

#include <exception>
#include <functional>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>

namespace monads {

template <
    typename T,
    typename E,
    typename ...Ts,
    std::enable_if_t<std::is_constructible<T, Ts&&...>::value, int> = 0
>
constexpr Expected<T, E> make_expected(Ts &&...ts)
noexcept(std::is_nothrow_constructible<T, Ts&&...>::value) {
    return Expected<T, E>{ InPlaceValueType{ }, std::forward<Ts>(ts)... };
}

template <
    typename T,
    typename E,
    typename U,
    typename ...Ts,
    std::enable_if_t<std::is_constructible<
        T,
        std::initializer_list<U>&,
        Ts&&...
    >::value, int> = 0
>
constexpr Expected<T, E> make_expected(
    std::initializer_list<U> list,
    Ts &&...ts
) noexcept(std::is_nothrow_constructible<
    T,
    std::initializer_list<U>&,
    Ts&&...
>::value) {
    return make_expected<T, E>(list, std::forward<Ts>(ts)...);
}

template <
    typename T,
    typename E,
    typename ...Ts,
    std::enable_if_t<std::is_constructible<E, Ts&&...>::value, int> = 0
>
constexpr Expected<T, E> make_unexpected(Ts &&...ts)
noexcept(std::is_nothrow_constructible<E, Ts&&...>::value) {
    return Expected<T, E>{ InPlaceErrorType{ }, std::forward<Ts>(ts)... };
}

template <
    typename T,
    typename E,
    typename U,
    typename ...Ts,
    std::enable_if_t<std::is_constructible<
        E,
        std::initializer_list<U>&,
        Ts&&...
    >::value, int> = 0
>
constexpr Expected<T, E> make_unexpected(
    std::initializer_list<U> list,
    Ts &&...ts
)
noexcept(std::is_nothrow_constructible<
    E,
    std::initializer_list<U>&,
    Ts&&...
>::value) {
    return make_unexpected<T, E>(list, std::forward<Ts>(ts)...);
}

template <
    typename E = std::exception_ptr,
    typename C,
    typename ...As,
    std::enable_if_t<detail::is_invocable<C&&, As&&...>::value, int> = 0
>
Expected<detail::invoke_result_t<C&&, As&&...>, E> try_invoke(
    C &&callable,
    As &&...args
) noexcept(std::is_nothrow_constructible<
    detail::invoke_result_t<C&&, As&&...>,
    detail::invoke_result_t<C&&, As&&...>
>::value) {
    return detail::TryInvoker<E>{ }(
        std::forward<C>(callable),
        std::forward<As>(args)...
    );
}

} // namespace monads

#endif
