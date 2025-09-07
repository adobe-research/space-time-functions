#pragma once

#include <type_traits>

namespace stf {

using Scalar = double;

template <class...> constexpr std::false_type always_false{};

}  // namespace stf

