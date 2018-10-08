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

#include <monads/detail/expected_impl.hpp>
#include <monads/detail/invoke.hpp>

#include <exception>
#include <initializer_list>
#include <type_traits>
#include <utility>

namespace monads {
namespace detail {

template <typename E = std::exception_ptr>
struct TryInvoker {
	template <
		typename C,
		typename ...Ts,
		std::enable_if_t<is_invocable<C&&, Ts&&...>::value, int> = 0
	>
	Expected<invoke_result_t<C&&, Ts&&...>, E> operator()(
		C &&callable,
		Ts &&...ts
	) noexcept {
		using Result = invoke_result_t<C&&, Ts&&...>;
		using Expected = Expected<Result, E>;

		try {
			auto &&result = invoke(
				std::forward<C>(callable),
				std::forward<Ts>(ts)...
			);

			return Expected{
				InPlaceValueType{ },
				std::forward<decltype(result)>(result)
			};
		} catch (const E &err) {
			return Expected{ InPlaceErrorType{ }, err };
		}
	}

	template <
		typename C,
		typename T,
		typename ...Ts,
		std::enable_if_t<is_invocable<
			C&&,
			std::initializer_list<T>&,
			Ts&&...
		>::value, int> = 0
	>
	Expected<invoke_result_t<C&&, Ts&&...>, E> operator()(
		C &&callable,
		std::initializer_list<T> list,
		Ts &&...ts
	) noexcept {
		return (*this)(
			std::forward<C>(callable),
			list,
			std::forward<Ts>(ts)...
		);
	}
};

template <>
struct TryInvoker<std::exception_ptr> {
	template <
		typename C,
		typename ...Ts,
		std::enable_if_t<is_invocable<C&&, Ts&&...>::value, int> = 0
	>
	Expected<invoke_result_t<C&&, Ts&&...>, std::exception_ptr>
	operator()(C &&callable, Ts &&...ts) noexcept {
		using Result = invoke_result_t<C&&, Ts&&...>;
		using Expected = Expected<Result, std::exception_ptr>;

		try {
			auto &&result = invoke(
				std::forward<C>(callable),
				std::forward<Ts>(ts)...
			);

			return Expected{
				InPlaceValueType{ },
				std::forward<decltype(result)>(result)
			};
		} catch (...) {
			return Expected{ InPlaceErrorType{ }, std::current_exception() };
		}
	}

	template <
		typename C,
		typename T,
		typename ...Ts,
		std::enable_if_t<is_invocable<
			C&&,
			std::initializer_list<T>&,
			Ts&&...
		>::value, int> = 0
	>
	Expected<invoke_result_t<C&&, Ts&&...>, std::exception_ptr> operator()(
		C &&callable,
		std::initializer_list<T> list,
		Ts &&...ts
	) noexcept {
		return (*this)(
			std::forward<C>(callable),
			list,
			std::forward<Ts>(ts)...
		);
	}
};

} // namespace detail
} // namespace monads
