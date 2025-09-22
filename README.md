# Space-Time Functions (STF)

This repo provides a comprehensive framework for composing implicit space-time functions.

Space-time functions are mathematical functions `f(x, t)` that map ℝ⁴ to ℝ, where `x` represents
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
gradient and time derivative at any space-time points.

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

Another way of defining a space-time function is by sweeping an implicit shape through space.
Let `f(x)` be a spatial function that maps ℝ³ to ℝ. The zero level set of `f(x)` defines a shape
in 3D space. The sign of `f(x)` indicates whether the point `x` is inside (`f(x) < 0`) or outside
(`f(x) > 0`) the shape.
Let `g(x, t)` be a spatial transform that maps ℝ³ to ℝ³. The swept shape is defined implicitly as
`f(g(x, t))`.

For example, the same ball translation example above can be equivalently defined as:

```c++
#include <stf/stf.h>

using Scalar = stf::Scalar;

stf::ImplicitBall<3> shape(0.5, {0, 0, 0});
stf::Translation<3> transform({-1, 0, 0});
stf::SweepFunction<3> f(shape, transform);
```

Note that the function `f` is a sweep function, which is a special case of space-time function. One
can evaluate its value, spatial gradient and time derivative just as before.

#### Spatial functions

Here is a list of supported spatial functions:

```c++
// Implicit Ball
// The `degree` argument specifies the distance degree (1 for L1, 2 for L2, etc.)
stf::ImplicitBall<Dim> b(radius, center, degree);

// Implicit Torus
// The torus is aligned with the XY plane.
stf::ImplicitTorus<Dim> b(major_radius, minor_radius, center);

// Implicit capsule
// It is defined as the offset surface of the line segment from p1 to p2.
stf::ImplicitCapsule<Dim> b(radius, p1, p2);

// VIPSS surface
// The VIPSS surface is defined by the Duchon interpolant of a set of points. See [1] for details.
//
// [1] Huang, Zhiyang, Nathan Carr, and Tao Ju. "Variational implicit point set surfaces." ACM
// Transactions on Graphics (TOG) 38.4 (2019): 1-13.
stf::Duchon b(points, rbf_coeffs, affine_coeffs);
```

It is also possible to explicitly define a spatial function:

```c++
stf::GenericFunction<Dim> b(value_fn, grad_fn);
```

where `value_fn` is the implicit function and `grad_fn` its spatial gradient function.

#### Soft union of spatial functions

In addition, it is often useful to union two implicit shapes together to form a more complex spatial
function:

```c++
// Soft union of two implicit shapes
stf::ImplicitUnion<Dim> b(b1, b2, smooth_distance);
```

Note that the implicit union function implements the [soft
union](https://iquilezles.org/articles/smin/) operation.
The `smooth_distance` parameter controls the amount of smoothness, and `smooth_distance = 0`
corresponds to the regular (non-smooth) union operation.
By default, we use the quadratic blending function for the soft union. Other blending functions are
also supported:

```c++
// We support four blending functions from the clamped difference (CD) family
stf::ImplicitUnion<Dim, stf::BlendingFunction::Quadratic> b(b1, b2, smooth_distance); // default
stf::ImplicitUnion<Dim, stf::BlendingFunction::Cubic> b(b1, b2, smooth_distance);
stf::ImplicitUnion<Dim, stf::BlendingFunction::Quartic> b(b1, b2, smooth_distance);
stf::ImplicitUnion<Dim, stf::BlendingFunction::Circular> b(b1, b2, smooth_distance);
```

#### Transforms

Here is a list of supported spatial transforms:

```c++
// Translation
stf::Translation<Dim> g({tx, ty, tz});

// Rotation
stf::Rotation<Dim> g(center, axis, angle);

// Scaling
stf::Scale<Dim> g(scale_factor, center);

// Polyline
stf::Polyline<Dim> g(points);

// Poly-Bezier curve
// Note that control points consist of 3N + 1 points, where N is the number of Bezier curves.
// Points with index in [3*k,3*(k+1) +1] define a cubic Bezier curve.
stf::PolyBezier<Dim> g(control_points, degree);
```

It is often useful to combine multiple transforms together:

```c++
stf::Compose<Dim> g(g1, g2);
```

For all transforms, the following are support:

```c++
// Map a point `x` to its transformed position at time `t`
auto x2 = g.transform(x, t);

// Velocity at point `x` and time `t`
auto v = g.velocity(x, t);

// Jacobian of the transform at point `x` and time `t`
auto J = g.position_Jacobian(x, t);
```

### Composite space-time functions

While explicit space-time functions and swept volume functions are powerful, sometimes we need to
apply more complex operations that transform one or more space-time functions into another. This can
be done using composite space-time functions.

#### Offset function

The offset function creates a new space-time function by adding a time-dependent offset to an
existing space-time function. Let `f(x, t)` be a space-time function and `o(t)` be a scalar-valued
time-dependent offset function. The offset space-time function is defined as `f(x, t) + o(t)`.


```c++
// Assume `f` is an existing `stf::SpaceTimeFunction<Dim>` object.

stf::OffsetFunction<Dim> f_offset(f,
    [](Scalar t) { return std::sin(t * 2 * M_PI); },
    [](Scalar t) { return 2 * M_PI * std::cos(t * 2 * M_PI); }
);
```

#### Space-time union function

The space-time union function combines two space-time function via a (soft) space union operation.

```c++
// Assume `f1` and `f2` are existing `stf::SpaceTimeFunction<Dim>` objects.

stf::UnionFunction<Dim> f_union(f1, f2, smooth_distance);
```

The `smoothing_distance` parameter controls the amount of smoothness, and `smoothing_distance = 0`
corresponds to the regular (non-smooth) union operation.

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

