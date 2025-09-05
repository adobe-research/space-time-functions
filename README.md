# Space-Time Functions (STF)

This repo provides a comprehensive framework for composing implicit space-time functions.

Space-time functions are mathematical functions `f(x, t)` that map R^4 to R, where `x` represents
spatial coordinates and `t` represents time.

## Features

### Explicit space-time functions

The most general way of defining a space-time function is to provide an explicit formula.

```c++
#include <stf/stf.h>

using Scalar = stf::Scalar;

stf::ExplicitForm<3> f(
    [](std::array<Scalar, 3> x, Scalar t) {
        constexpr Scalar r = 0.5;
        return (x[0] - t) * (x[1] - t) + x[1] * x[1] + x[2] * x[2] - r * r;
    }
);
```

This space-time function defines an implicit ball of radius 0.5 translating from (0, 0, 0) at time
`t=0` to (1, 0, 0) at time `t=1`. Once a space-time function is defined, we can query its value,
spatial gradient and time derivative at space-time points.

```c++
std::array<Scalar, 3> x = {0.5, 0.0, 0.0};
Scalar t = 0.5;

auto value = f.value(x, t);
auto grad = f.gradient(x, t);
auto dt = f.time_derivative(x, t);
```

Note that spatial gradient and time derivative are both approximated using finite difference. If an
analytical expression for them is available, it is better to specify them at the time of
construction.

```c++
constexpr Scalar radius = 0.5;

auto value_fn = [r=radius](std::array<Scalar, 3> x, Scalar t) {
    return (x[0] - t) * (x[0] - t) + x[1] * x[1] + x[2] * x[2] - r * r;
};

auto grad_fn = [r=radius](std::array<Scalar, 3> x, Scalar t) {
    return {2 * x[0] - 2 * t, 2 * x[1], 2 * x[2]};
};

auto dt_fn = [r=radius](std::array<Scalar, 3> x, Scalar t) {
    return -2 * (x[0] - t);
};

stf::ExplicitForm<3> f(value_fn, grad_fn, dt_fn);
```

### Swept volume functions

### Composite space-time functions


## Building

This repo is defined to have minimal amount of dependencies:

- C++20 compatible compiler
- CMake 3.28 or higher
- (Optional) Catch2 for unit tests only

To build the library:

```sh
mkdir build
cd build
cmake ..
cmake --build .
```

