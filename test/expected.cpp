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

#include <expected/expected.hpp>

#include "catch.hpp"

#include <memory>
#include <stdexcept>
#include <string_view>

using namespace std::literals;

SCENARIO("expected::Expected", "[expected][expected/expected.hpp][expected::Expected]") {
    const auto maybe_int = expected::try_invoke([] { return 0; });

    REQUIRE(maybe_int);
    REQUIRE_FALSE(!maybe_int);
    REQUIRE(maybe_int.has_value());
    REQUIRE_FALSE(maybe_int.has_error());
    REQUIRE(maybe_int.value() == 0);

    WHEN("try_invoke catches an exception") {
        const auto maybe_string = expected::try_invoke([]() -> std::string {
            throw std::runtime_error{ "try_invoke" };
        });

        THEN("it works") {
            REQUIRE_FALSE(maybe_string);
            REQUIRE(!maybe_string);
            REQUIRE_FALSE(maybe_string.has_value());
            REQUIRE(maybe_string.has_error());

            try {
                std::rethrow_exception(maybe_string.error());
                REQUIRE(false);
            } catch (const std::runtime_error &e) {
                REQUIRE(e.what() == "try_invoke"sv);
            }
        }
    }

    WHEN("make_expected is used with std::vector") {
        const expected::Expected<std::vector<int>, std::exception_ptr> maybe_vector =
            expected::make_expected<std::vector<int>>({ 0, 1, 2, 3 });

        THEN("it works") {
            REQUIRE(maybe_vector);
            REQUIRE_FALSE(!maybe_vector);
            REQUIRE(maybe_vector.has_value());
            REQUIRE_FALSE(maybe_vector.has_error());
            REQUIRE(maybe_vector.value() == std::vector<int>{ 0, 1, 2, 3 });
        }
    }

    WHEN("make_expected is used with std::string") {
        const expected::Expected<std::string, std::exception_ptr> maybe_string =
            expected::make_expected<std::string>(4, 'f');

        THEN("it works") {
            REQUIRE(maybe_string);
            REQUIRE_FALSE(!maybe_string);
            REQUIRE(maybe_string.has_value());
            REQUIRE_FALSE(maybe_string.has_error());
            REQUIRE(maybe_string.value() == "ffff");
        }
    }
}
