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

#include <monads/optional.hpp>

#include "catch.hpp"

SCENARIO(
	"monads::Optional",
	"[monads][monads/optional.hpp][monads::Optional]"
) {
	WHEN("Optional is used in a constexpr context") {
		constexpr const auto maybe_int = monads::make_optional<int>(15);
		constexpr const monads::Optional<int> not_int;

		THEN("it works") {
			static_assert(maybe_int, "maybe_int must contain a value");
			static_assert(maybe_int.has_value(),
						  "maybe_int must contain a value");
			static_assert(!(!maybe_int), "maybe_int must contain a value");
			static_assert(*maybe_int == 15, "maybe_int must contain 15");
			static_assert(maybe_int.value() == 15,
						  "maybe_int must contain 15");
			static_assert(maybe_int.unwrap() == 15,\
						  "maybe_int must contain 15");

			static_assert(!not_int, "not_int must not contain a value");
			static_assert(!(not_int), "not_int must not contain a value");
			static_assert(!not_int.has_value(),
						  "not_int must not contain a value");

			REQUIRE_THROWS(not_int.value());
		}
	}

	WHEN("Optional::map is used in a constexpr context") {
		struct Doubler {
			constexpr int operator()(int i) noexcept {
				return i * 2;
			}
		};

		constexpr const auto maybe_int = monads::make_optional<int>(10);
		constexpr const auto maybe_doubled = maybe_int.map(Doubler{ });

		THEN("it works") {
			static_assert(maybe_doubled, "maybe_doubled must contain a value");
			static_assert(!(!maybe_doubled),
						  "maybe_doubled must contain a value");
			static_assert(maybe_doubled.has_value(),
						  "maybe_doubled must contain a value");

			static_assert(*maybe_doubled == 20,
						  "maybe_doubled must contain 20");
			static_assert(maybe_doubled.value() == 20,
						  "maybe_doubled must contain 20");
			static_assert(maybe_doubled.unwrap() == 20,
						  "maybe_doubled must contain 20");
		}
	}

	WHEN("Optional is used") {
		monads::Optional<int> opt;

		monads::Optional<int> &ref = opt;
		const monads::Optional<int> &cref = opt;
		monads::Optional<int> &&rref = std::move(opt);
		const monads::Optional<int> &&crref = std::move(opt);

		THEN("it has the correct return types") {
			static_assert(
				std::is_same<decltype((ref.value())), int&>::value,
				""
			);

			static_assert(
				std::is_same<decltype((*ref)), int&>::value,
				""
			);

			static_assert(
				std::is_same<decltype((ref.unwrap())), int&>::value,
				""
			);


			static_assert(
				std::is_same<decltype((cref.value())), const int&>::value,
				""
			);

			static_assert(
				std::is_same<decltype((*cref)), const int&>::value,
				""
			);

			static_assert(
				std::is_same<decltype((cref.unwrap())), const int&>::value,
				""
			);


			static_assert(
				std::is_same<decltype((std::move(rref).value())), int&&>::value,
				""
			);

			static_assert(
				std::is_same<decltype((*std::move(rref))), int&&>::value,
				""
			);

			static_assert(
				std::is_same<decltype((std::move(rref).unwrap())), int&&>::value,
				""
			);


			static_assert(
				std::is_same<decltype((std::move(crref).value())), const int&&>::value,
				""
			);

			static_assert(
				std::is_same<decltype((*std::move(crref))), const int&&>::value,
				""
			);

			static_assert(
				std::is_same<decltype((std::move(crref).unwrap())), const int&&>::value,
				""
			);
		}
	}

	WHEN("maybe_invoke is used") {
		const auto none = monads::maybe_invoke([]() -> int {
			throw "nope";
		});

		const auto some = monads::maybe_invoke([] {
			return 0;
		});

		REQUIRE_FALSE(none);
		REQUIRE(some);
		REQUIRE(*some == 0);
	}
}
