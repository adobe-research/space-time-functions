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
        return (x[0] - t) * (x[0] - t) + x[1] * x[1] + x[2] * x[2] - r * r;
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

Another way of defining a space-time function is to provide a spatial function and a sweeping
trajectory. The resulting space-time function is the swept volume of the shape moving through space.
The same ball translation example above can be equivalently defined as:

```c++
#include <stf/stf.h>

using Scalar = stf::Scalar;

stf::ImplicitBall<3> ball(0.5, {0, 0, 0});
stf::Translation<3> translate({-1, 0, 0});
stf::SweepFunction<3> f(ball, translate);
```

Note that the function `f` is a sweep function, which is a special case of space-time function. One
can evaluate its value, spatial gradient and time derivative just as before.

#### Spatial functions

Here is a list of supported spatial functions:

```c++
// Implicit Ball
// The `degree` argument specifies the distance degree (1 for L1, 2 for L2, etc.)
stf::ImplicitBall<Dim> f(radius, center, degree);

// Implicit Torus
// The torus is aligned with the XY plane.
stf::ImplicitTorus<Dim> f(major_radius, minor_radius, center);

// Implicit capsule
// It is defined as the offset surface of the line segment from p1 to p2.
stf::ImplicitCapsule<Dim> f(radius, p1, p2);

// VIPSS surface
// The VIPSS surface is defined by the Duchon interpolant of a set of points. See [1] for details.
//
// [1] Huang, Zhiyang, Nathan Carr, and Tao Ju. "Variational implicit point set surfaces." ACM
// Transactions on Graphics (TOG) 38.4 (2019): 1-13.
stf::Duchon f(points, rbf_coeffs, affine_coeffs);
```

It is also possible to explicitly define a spatial function:

```c++
stf::GenericFunction<Dim> f(value_fn, grad_fn);
```

In addition, it is often useful to union two implicit shapes together to form more complex ones:

```c++
stf::ImplicitUnion<Dim> f(shape1, shape2);
```

#### Trajectories

### Composite space-time functions


## Building

This repo is designed to have a minimal amount of dependencies:

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

