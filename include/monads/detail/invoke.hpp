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

#ifndef MONADS_DETAIL_INVOKE_HPP
#define MONADS_DETAIL_INVOKE_HPP

#include <monads/detail/common.hpp>
#include <monads/detail/invoke_detail.hpp>

#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

namespace monads {
namespace detail {

template <typename T, typename ...As>
struct invoke_result : monads_detail_invoke_detail::invoke_result<T, void, As...> { };

template <typename T, typename ...As>
using invoke_result_t = typename invoke_result<T, As...>::type;

template <typename T, typename ...As>
struct is_invocable : monads_detail_invoke_detail::is_invocable<T, As...> { };

template <typename T, typename ...As>
struct is_nothrow_invocable : monads_detail_invoke_detail::is_nothrow_invocable<T, void, As...> { };

template <typename C, typename ...As, std::enable_if_t<is_invocable<C&&, As&&...>::value, int> = 0>
constexpr invoke_result_t<C&&, As&&...> invoke(C &&c, As &&...args)
noexcept(is_nothrow_invocable<C&&, As&&...>::value) {
    return monads_detail_invoke_detail::do_invoke(
        monads_detail_invoke_detail::invoke_tag<C, As...>{ },
        std::forward<C>(c),
        std::forward<As>(args)...
    );
}

} // namespace detail
} // namespace monads

#endif
