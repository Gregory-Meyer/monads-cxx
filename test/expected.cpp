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

#include <monads/expected.hpp>

#include "catch.hpp"

#include <memory>
#include <stdexcept>

using namespace std::literals;

class Identifier {
public:
    constexpr explicit Identifier(int base) noexcept : base_{ base } { }

    constexpr explicit operator int() const noexcept {
        return base_;
    }

    constexpr int get() const noexcept {
        return base_;
    }

private:
    int base_;
};

SCENARIO("monads::Expected", "[monads][monads/expected.hpp][monads::Expected]") {
    WHEN("Expected is used in a basic way") {
        const auto maybe_int = monads::try_invoke([] { return 0; });

        THEN("it works") {
            REQUIRE(maybe_int);
            REQUIRE_FALSE(!maybe_int);
            REQUIRE(maybe_int.has_value());
            REQUIRE_FALSE(maybe_int.has_error());
            REQUIRE(maybe_int.value() == 0);
        }
    }

    WHEN("try_invoke catches an exception") {
        const auto maybe_string = monads::try_invoke([]() -> std::string {
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
                static const std::string EXPECTED = "try_invoke";

                REQUIRE(e.what() == EXPECTED);
            } catch (...) {
                REQUIRE(false);
            }
        }
    }

    WHEN("make_expected is used with std::vector") {
        const auto maybe_vector =
            monads::make_expected<std::vector<int>, std::exception_ptr>();

        THEN("it works") {
            REQUIRE(maybe_vector);
            REQUIRE_FALSE(!maybe_vector);
            REQUIRE(maybe_vector.has_value());
            REQUIRE_FALSE(maybe_vector.has_error());
            REQUIRE(maybe_vector.value() == std::vector<int>{ });
        }
    }

    WHEN("make_expected is used with std::string") {
        static const std::string EXPECTED =
            "this string is so long it won't be collapsed into a small buffer optimization";

        const auto maybe_string =
            monads::make_expected<std::string, std::exception_ptr>(std::string{ EXPECTED });

        THEN("it works") {
            REQUIRE(maybe_string);
            REQUIRE_FALSE(!maybe_string);
            REQUIRE(maybe_string.has_value());
            REQUIRE_FALSE(maybe_string.has_error());
            REQUIRE(maybe_string.value() == EXPECTED);
        }
    }

    WHEN("make_expected is used in constexpr contexts") {
        constexpr auto maybe_int = monads::make_expected<int, int>(0);

        THEN("it works") {
            static_assert(maybe_int, "maybe_int must contain a value");
            static_assert(!(!maybe_int), "maybe_int must contain a value");
            static_assert(maybe_int.has_value(), "maybe_int must contain a value");
            static_assert(!maybe_int.has_error(), "maybe_int must contain a value");
            static_assert(maybe_int.value() == 0, "maybe_int must contain 0");
        }
    }

    WHEN("Expected::map is used with a value in a constexpr context") {
        struct Mapper {
            constexpr int operator()(long c) noexcept {
                return static_cast<int>(c) + 5;
            }
        };

        constexpr auto maybe_long = monads::make_expected<long, double>(0);
        constexpr auto maybe_int = std::move(maybe_long).map(Mapper{ });

        THEN("it works") {
            static_assert(maybe_long, "");
            static_assert(maybe_long, "");
            static_assert(maybe_long.value() == 0, "");
            static_assert(maybe_int.value() == 5, "");
        }
    }

    WHEN("Expected::map is used with a string") {
        const auto maybe_string = monads::try_invoke([] { return std::string{ "Hello, world!" }; });
        const auto maybe_length = maybe_string
            .map([](const std::string &str) noexcept { return str.size(); });

        if (maybe_string.has_value()) {
            REQUIRE(maybe_length);
            REQUIRE(maybe_length.value() == 13);
        } else if (maybe_string.has_error()) {
            REQUIRE(!maybe_length);
            REQUIRE(maybe_length.error() == maybe_string.error());
        }
    }

    WHEN("Expected implicit conversion ctors are used") {
        static_assert(std::is_constructible<
            monads::Expected<double, std::exception_ptr>,
            const monads::Expected<int, std::exception_ptr>&
        >::value, "");

        static_assert(std::is_constructible<
            monads::Expected<double, std::exception_ptr>,
            monads::Expected<int, std::exception_ptr>&&
        >::value, "");

        static_assert(std::is_convertible<
            const monads::Expected<int, std::exception_ptr>&,
            monads::Expected<double, std::exception_ptr>
        >::value, "");

        static_assert(std::is_convertible<
            monads::Expected<int, std::exception_ptr>&&,
            monads::Expected<double, std::exception_ptr>
        >::value, "");

        const auto maybe_int = monads::make_expected<int, std::exception_ptr>(5);
        const monads::Expected<double, std::exception_ptr> maybe_double = maybe_int;

        THEN("they work") {
            REQUIRE(maybe_double);
            REQUIRE(maybe_double.unwrap() == double(maybe_int.unwrap()));
        }
    }

    WHEN("Expected explicit conversion ctors are used") {
        static_assert(std::is_constructible<
            monads::Expected<Identifier, std::exception_ptr>,
            const monads::Expected<int, std::exception_ptr>&
        >::value, "");

        static_assert(std::is_constructible<
            monads::Expected<Identifier, std::exception_ptr>,
            monads::Expected<int, std::exception_ptr>&&
        >::value, "");

        static_assert(!std::is_convertible<
            const monads::Expected<int, std::exception_ptr>&,
            monads::Expected<Identifier, std::exception_ptr>
        >::value, "");

        static_assert(!std::is_convertible<
            monads::Expected<int, std::exception_ptr>&&,
            monads::Expected<Identifier, std::exception_ptr>
        >::value, "");

        const auto maybe_int = monads::make_expected<int, std::exception_ptr>(5);
        const monads::Expected<Identifier, std::exception_ptr> maybe_id{ maybe_int };

        THEN("they work") {
            REQUIRE(maybe_id);
            REQUIRE(maybe_id.unwrap().get() == maybe_int.unwrap());
        }
    }

    WHEN("Expected forwarding ctors are used") {
        static_assert(
            std::is_constructible<monads::Expected<int, double>, const Identifier&>::value,
            ""
        );
        static_assert(
            !std::is_convertible<const Identifier&, monads::Expected<int, double>>::value,
            ""
        );

        const monads::Expected<int, double> maybe_int{ Identifier{ 15 } };

        THEN("they work") {
            REQUIRE(maybe_int);
            REQUIRE(maybe_int.unwrap() == 15);
        }
    }

    WHEN("try_invoke is used to catch an exception by value") {
        using namespace std::literals;

        const auto maybe_int = monads::try_invoke<std::runtime_error>(
            []() -> int {
                throw std::runtime_error{ "oh no" };
            }
        );

        THEN("it works") {
            REQUIRE_FALSE(maybe_int);
            REQUIRE(maybe_int.has_error());

            REQUIRE(maybe_int.unwrap_error().what() == "oh no"s);
        }
    }
}
