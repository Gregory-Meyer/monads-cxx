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

#ifndef MONADS_DETAIL_INVOKE_DETAIL_HPP
#define MONADS_DETAIL_INVOKE_DETAIL_HPP

#include <monads/detail/common.hpp>

#include <type_traits>
#include <utility>

namespace monads_detail_invoke_detail {

template <typename ...Ts>
using void_t = void;

struct dot_star_paren_tag { };

struct get_dot_star_paren_tag { };

struct star_dot_star_paren_tag { };

struct dot_star_tag { };

struct get_dot_star_tag { };

struct star_dot_star_tag { };

struct paren_tag { };

struct not_invocable_tag { };

template <typename C, typename = void, typename ...Ts>
struct is_dot_star_paren_invocable : std::false_type { };

template <typename C, typename T, typename ...Ts>
struct is_dot_star_paren_invocable<C, void_t<decltype(
    (std::declval<T>().*std::declval<C>())(std::declval<Ts>()...)
)>, T, Ts...> : std::true_type { };

template <typename C, typename = void, typename ...Ts>
struct is_get_dot_star_paren_invocable : std::false_type { };

template <typename C, typename T, typename ...Ts>
struct is_get_dot_star_paren_invocable<C, void_t<decltype(
    (std::declval<T>().get().*std::declval<C>())(std::declval<Ts>()...)
)>, T, Ts...> : std::true_type { };

template <typename C, typename = void, typename ...Ts>
struct is_star_dot_star_paren_invocable : std::false_type { };

template <typename C, typename T, typename ...Ts>
struct is_star_dot_star_paren_invocable<C, void_t<decltype(
    ((*std::declval<T>()).*std::declval<C>())(std::declval<Ts>()...)
)>, T, Ts...> : std::true_type { };

template <typename C, typename = void, typename ...Ts>
struct is_dot_star_invocable : std::false_type { };

template <typename C, typename T>
struct is_dot_star_invocable<C, void_t<decltype(
    std::declval<T>().*std::declval<C>()
)>, T> : std::true_type { };

template <typename C, typename = void, typename ...Ts>
struct is_get_dot_star_invocable : std::false_type { };

template <typename C, typename T>
struct is_get_dot_star_invocable<C, void_t<decltype(
    std::declval<T>().get().*std::declval<C>()
)>, T> : std::true_type { };

template <typename C, typename = void, typename ...Ts>
struct is_star_dot_star_invocable : std::false_type { };

template <typename C, typename T>
struct is_star_dot_star_invocable<C, void_t<decltype(
    (*std::declval<T>()).*std::declval<C>()
)>, T> : std::true_type { };

template <typename C, typename = void, typename ...Ts>
struct is_paren_invocable : std::false_type { };

template <typename C, typename ...Ts>
struct is_paren_invocable<C, void_t<decltype(
    std::declval<C>()(std::declval<Ts>()...)
)>, Ts...> : std::true_type { };

template <typename C, typename ...Ts>
using invoke_tag = std::conditional_t<
    is_dot_star_paren_invocable<C, void, Ts...>::value,
    dot_star_paren_tag,
    std::conditional_t<
        is_get_dot_star_paren_invocable<C, void, Ts...>::value,
        get_dot_star_paren_tag,
        std::conditional_t<
            is_star_dot_star_paren_invocable<C, void, Ts...>::value,
            star_dot_star_paren_tag,
            std::conditional_t<
                is_dot_star_invocable<C, void, Ts...>::value,
                dot_star_tag,
                std::conditional_t<
                    is_get_dot_star_invocable<C, void, Ts...>::value,
                    get_dot_star_tag,
                    std::conditional_t<
                        is_star_dot_star_invocable<C, void, Ts...>::value,
                        star_dot_star_tag,
                        std::conditional_t<
                            is_paren_invocable<C, void, Ts...>::value,
                            paren_tag,
                            not_invocable_tag
                        >
                    >
                >
            >
        >
    >
>;

template <typename C, typename ...Ts>
struct is_invocable : std::integral_constant<bool, !std::is_same<
    not_invocable_tag,
    invoke_tag<C, Ts...>
>::value> { };

template <typename T, typename C, typename ...Ts>
struct invoke_traits { };

template <typename C, typename T, typename ...Ts>
struct invoke_traits<dot_star_paren_tag, C, T, Ts...> {
    using tag = dot_star_paren_tag;
    using is_nothrow = std::integral_constant<bool, noexcept(
        (std::declval<T>().*std::declval<C>())(std::declval<Ts>()...)
    )>;
    using result = decltype((
        (std::declval<T>().*std::declval<C>())(std::declval<Ts>()...)
    ));
};

template <typename C, typename T, typename ...Ts>
struct invoke_traits<get_dot_star_paren_tag, C, T, Ts...> {
    using tag = dot_star_paren_tag;
    using is_nothrow = std::integral_constant<bool, noexcept(
        (std::declval<T>().get().*std::declval<C>())(std::declval<Ts>()...)
    )>;
    using result = decltype((
        (std::declval<T>().get().*std::declval<C>())(std::declval<Ts>()...)
    ));
};

template <typename C, typename T, typename ...Ts>
struct invoke_traits<star_dot_star_paren_tag, C, T, Ts...> {
    using tag = star_dot_star_paren_tag;
    using is_nothrow = std::integral_constant<bool, noexcept(
        ((*std::declval<T>()).*std::declval<C>())(std::declval<Ts>()...)
    )>;
    using result = decltype((
        ((*std::declval<T>()).*std::declval<C>())(std::declval<Ts>()...)
    ));
};

template <typename C, typename T>
struct invoke_traits<dot_star_tag, C, T> {
    using tag = dot_star_tag;
    using is_nothrow = std::integral_constant<bool, noexcept(
        std::declval<T>().*std::declval<C>()
    )>;
    using result = decltype((
        std::declval<T>().*std::declval<C>()
    ));
};

template <typename C, typename T>
struct invoke_traits<get_dot_star_tag, C, T> {
    using tag = get_dot_star_tag;
    using is_nothrow = std::integral_constant<bool, noexcept(
        std::declval<T>().get().*std::declval<C>()
    )>;
    using result = decltype((
        std::declval<T>().get().*std::declval<C>()
    ));
};

template <typename C, typename T>
struct invoke_traits<star_dot_star_tag, C, T> {
    using tag = star_dot_star_tag;
    using is_nothrow = std::integral_constant<bool, noexcept(
        (*std::declval<T>()).*std::declval<C>()
    )>;
    using result = decltype((
        (*std::declval<T>()).*std::declval<C>()
    ));
};

template <typename C, typename ...Ts>
struct invoke_traits<paren_tag, C, Ts...> {
    using tag = paren_tag;
    using is_nothrow = std::integral_constant<bool, noexcept(
        std::declval<C>()(std::declval<Ts>()...)
    )>;
    using result = decltype((
        std::declval<C>()(std::declval<Ts>()...)
    ));
};

template <typename C, typename = void, typename ...Ts>
struct invoke_result { };

template <typename C, typename ...Ts>
struct invoke_result<C, void_t<std::enable_if_t<
    is_invocable<C, Ts...>::value
>>, Ts...> {
    using type = typename invoke_traits<invoke_tag<C, Ts...>, C, Ts...>::result;
};

template <typename C, typename = void, typename ...Ts>
struct is_nothrow_invocable : std::false_type { };

template <typename C, typename ...Ts>
struct is_nothrow_invocable<C, void_t<
    typename invoke_traits<invoke_tag<C, Ts...>, C, Ts...>::is_nothrow,
    std::enable_if_t<invoke_traits<invoke_tag<C, Ts...>, C, Ts...>::is_nothrow::value>
>, Ts...> : std::true_type { };

template <typename C, typename T, typename ...Ts>
constexpr decltype(auto)
do_invoke(dot_star_paren_tag, C &&c, T &&t, Ts &&...ts)
noexcept(noexcept(
    (std::forward<T>(t).*std::forward<C>(c))(std::forward<Ts>(ts)...)
)) {
    return (std::forward<T>(t).*std::forward<C>(c))(std::forward<Ts>(ts)...);
}

template <typename C, typename T, typename ...Ts>
constexpr decltype(auto)
do_invoke(get_dot_star_paren_tag, C &&c, T &&t, Ts &&...ts)
noexcept(noexcept(
    (std::forward<T>(t).get().*std::forward<C>(c))(std::forward<Ts>(ts)...)
)) {
    return (std::forward<T>(t).get().*std::forward<C>(c))(
        std::forward<Ts>(ts)...
    );
}

template <typename C, typename T, typename ...Ts>
constexpr decltype(auto)
do_invoke(star_dot_star_paren_tag, C &&c, T &&t, Ts &&...ts)
noexcept(noexcept(
    ((*std::forward<T>(t)).*std::forward<C>(c))(std::forward<Ts>(ts)...)
)) {
    return ((*std::forward<T>(t)).*std::forward<C>(c))(std::forward<Ts>(ts)...);
}

template <typename C, typename T>
constexpr decltype(auto) do_invoke(dot_star_tag, C &&c, T &&t)
noexcept(noexcept(std::forward<T>(t).*std::forward<C>(c))) {
    return std::forward<T>(t).*std::forward<C>(c);
}

template <typename C, typename T>
constexpr decltype(auto) do_invoke(get_dot_star_tag, C &&c, T &&t)
noexcept(noexcept(std::forward<T>(t).get().*std::forward<C>(c))) {
    return std::forward<T>(t).get().*std::forward<C>(c);
}

template <typename C, typename T>
constexpr decltype(auto) do_invoke(star_dot_star_tag, C &&c, T &&t)
noexcept(noexcept((*std::forward<T>(t)).*std::forward<C>(c))) {
    return (*std::forward<T>(t)).*std::forward<C>(c);
}

template <typename C, typename ...Ts>
constexpr decltype(auto) do_invoke(paren_tag, C &&c, Ts &&...ts)
noexcept(noexcept(std::forward<C>(c)(std::forward<Ts>(ts)...))) {
    return std::forward<C>(c)(std::forward<Ts>(ts)...);
}

template <typename C, typename ...Ts>
constexpr void do_invoke(not_invocable_tag, C&&, Ts &&...) {
    static_assert(sizeof(C) != sizeof(C), "not invocable");
}

} // namespace monads_detail_invoke_detail

#endif
