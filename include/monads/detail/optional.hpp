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

#ifndef MONADS_DETAIL_OPTIONAL_HPP
#define MONADS_DETAIL_OPTIONAL_HPP

#include <monads/detail/common.hpp>

#include <initializer_list>
#include <utility>

namespace monads {
namespace detail {

template <typename T, typename = void>
struct OptionalStorage {
	union {
		Monostate monostate;
		T value;
	};

	bool has_value = false;

	constexpr OptionalStorage() noexcept : monostate{ } { }

    template <
    	typename ...Ts,
    	std::enable_if_t<std::is_constructible<T, Ts&&...>::value, int
    > = 0>
    constexpr OptionalStorage(ValueTag, Ts &&...args)
    noexcept(std::is_nothrow_constructible<T, Ts&&...>::value)
    : value(std::forward<Ts>(args)...), has_value{ true } { }

    template <
    	typename U,
    	typename ...Ts,
    	std::enable_if_t<std::is_constructible<
	    	T,
	    	std::initializer_list<U>&,
	    	Ts&&...
	    >::value, int> = 0
	>
    constexpr OptionalStorage(ValueTag, std::initializer_list<U> list,
    						  Ts &&...args)
    noexcept(std::is_nothrow_constructible<
    	T,
    	std::initializer_list<U>&,
    	Ts&&...
    >::value)
    : value(list, std::forward<Ts>(args)...), has_value{ true } { }

    constexpr void reset() noexcept {
        has_value = false;
    }
};

template <typename T>
struct OptionalStorage<T, void_t<std::enable_if_t<
    !std::is_trivially_destructible<T>::value
>>> {
	union {
		Monostate monostate;
		T value;
	};

	bool has_value = false;

	constexpr OptionalStorage() noexcept : monostate{ } { }

    template <
    	typename ...Ts,
    	std::enable_if_t<std::is_constructible<T, Ts&&...>::value, int
    > = 0>
    constexpr OptionalStorage(ValueTag, Ts &&...args)
    noexcept(std::is_nothrow_constructible<T, Ts&&...>::value)
    : value(std::forward<Ts>(args)...), has_value{ true } { }

    template <
    	typename U,
    	typename ...Ts,
    	std::enable_if_t<std::is_constructible<
	    	T,
	    	std::initializer_list<U>&,
	    	Ts&&...
	    >::value, int> = 0
	>
    constexpr OptionalStorage(ValueTag, std::initializer_list<U> list,
    						  Ts &&...args)
    noexcept(std::is_nothrow_constructible<
    	T,
    	std::initializer_list<U>&,
    	Ts&&...
    >::value)
    : value(list, std::forward<Ts>(args)...), has_value{ true } { }

    ~OptionalStorage() {
    	reset();
    }

    void reset() noexcept {
    	if (!has_value) {
    		return;
    	}

    	value.T::~T();
        has_value = false;
    }
};

} // namespace detail
} // namespace monads

#endif
