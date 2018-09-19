### Status

[![Build Status](https://travis-ci.com/Gregory-Meyer/monads-cxx.svg?branch=master)](https://travis-ci.com/Gregory-Meyer/monads-cxx)

### Examples

```c++
const auto maybe_vec = monads::make_expected([] -> std::vector<int> { return { 0, 1, 2, 3 }; });
const auto maybe_size = maybe_vec.map([](const std::vector<int> &v) { return v.size() });

if (maybe_vec.has_value()) {
    assert(maybe_size && maybe_size.has_value()); // explicit operator bool == has_value()

    // value() and error() throw if !has_value() or !has_error() respectively
    // unwrap makes no such checks
    assert(maybe_vec.value() == maybe_vec.unwrap());
    assert(maybe_size.value() == maybe_size.value());

    assert(maybe_vec.unwrap() == std::vector<int>{ 0, 1, 2, 3});
    assert(maybe_size.unwrap() == 4);
} else if (maybe_vec.has_error()) {
    assert(!maybe_size && maybe_size.has_error()); // operator! == has_error()

    assert(maybe_vec.error() == maybe_vec.unwrap_error());
    assert(maybe_size.error() == maybe_size.unwrap_error());
    assert(maybe_vec.unwrap_error() == maybe_size.unwrap_error());
}
```
