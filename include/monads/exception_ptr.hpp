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

#ifndef MONADS_EXCEPTION_PTR
#define MONADS_EXCEPTION_PTR

#include <exception>
#include <memory>
#include <type_traits>

namespace monads {

template <typename E>
class ExceptionPtr : public std::exception {
public:
	static_assert(
		std::is_base_of<std::exception, E>::value,
		"E must have std::exception as a base class"
	);

	ExceptionPtr() = delete;

	virtual ~ExceptionPtr() = default;

	const E& get() const noexcept {
		return thrown_.get();
	}

	const char* what() const noexcept override {
		return get().what();
	}

	template <typename F>
	friend ExceptionPtr<F> current_exception();

private:
	explicit ExceptionPtr(std::exception_ptr ptr, const E &e) noexcept
	: owner_{ ptr }, thrown_{ e } { }

	std::exception_ptr owner_;
	std::reference_wrapper<const E> thrown_;
};

template <typename E>
ExceptionPtr<E> current_exception() {
	const auto err = std::current_exception();

	try {
		std::rethrow_exception(err);
	} catch (const E &e) {
		return ExceptionPtr<E>{ err, e };
	}
}

} // namespace monads

#endif
