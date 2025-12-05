#include <stf/stf.h>

#include <nanobind/nanobind.h>
#include <nanobind/stl/array.h>
#include <nanobind/stl/function.h>

namespace nb = nanobind;

NB_MODULE(pystf, m)
{
    using namespace nb::literals;
    using Scalar = stf::Scalar;

    // Create submodules
    auto primitive = m.def_submodule("primitive", "Primitive implicit functions");
    auto transform = m.def_submodule("transform", "Geometric transformations");

    // Base classes
    nb::class_<stf::SpaceTimeFunction<2>>(m, "SpaceTimeFunction2D")
        .def("value", &stf::SpaceTimeFunction<2>::value, "pos"_a, "t"_a,
             "Evaluate the function at a given position and time")
        .def("time_derivative", &stf::SpaceTimeFunction<2>::time_derivative, "pos"_a, "t"_a,
             "Compute the time derivative of the function")
        .def("gradient", &stf::SpaceTimeFunction<2>::gradient, "pos"_a, "t"_a,
             "Compute the gradient of the function with respect to both space and time");

    nb::class_<stf::SpaceTimeFunction<3>>(m, "SpaceTimeFunction3D")
        .def("value", &stf::SpaceTimeFunction<3>::value, "pos"_a, "t"_a,
             "Evaluate the function at a given position and time")
        .def("time_derivative", &stf::SpaceTimeFunction<3>::time_derivative, "pos"_a, "t"_a,
             "Compute the time derivative of the function")
        .def("gradient", &stf::SpaceTimeFunction<3>::gradient, "pos"_a, "t"_a,
             "Compute the gradient of the function with respect to both space and time");

    nb::class_<stf::ImplicitFunction<2>>(primitive, "ImplicitFunction2D")
        .def("value", &stf::ImplicitFunction<2>::value, "pos"_a,
             "Evaluate the implicit function at a given position")
        .def("gradient", &stf::ImplicitFunction<2>::gradient, "pos"_a,
             "Compute the gradient of the implicit function")
        .def("finite_difference_gradient", &stf::ImplicitFunction<2>::finite_difference_gradient, 
             "pos"_a, "delta"_a = 1e-6,
             "Compute the finite difference approximation of the gradient");

    nb::class_<stf::ImplicitFunction<3>>(primitive, "ImplicitFunction3D")
        .def("value", &stf::ImplicitFunction<3>::value, "pos"_a,
             "Evaluate the implicit function at a given position")
        .def("gradient", &stf::ImplicitFunction<3>::gradient, "pos"_a,
             "Compute the gradient of the implicit function")
        .def("finite_difference_gradient", &stf::ImplicitFunction<3>::finite_difference_gradient, 
             "pos"_a, "delta"_a = 1e-6,
             "Compute the finite difference approximation of the gradient");

    nb::class_<stf::Transform<2>>(transform, "Transform2D")
        .def("transform", &stf::Transform<2>::transform, "pos"_a, "t"_a,
             "Transform a point in space according to the transformation rules")
        .def("velocity", &stf::Transform<2>::velocity, "pos"_a, "t"_a,
             "Calculate the velocity of a point under the transformation")
        .def("position_Jacobian", &stf::Transform<2>::position_Jacobian, "pos"_a, "t"_a,
             "Calculate the Jacobian matrix of the transformation");

    nb::class_<stf::Transform<3>>(transform, "Transform3D")
        .def("transform", &stf::Transform<3>::transform, "pos"_a, "t"_a,
             "Transform a point in space according to the transformation rules")
        .def("velocity", &stf::Transform<3>::velocity, "pos"_a, "t"_a,
             "Calculate the velocity of a point under the transformation")
        .def("position_Jacobian", &stf::Transform<3>::position_Jacobian, "pos"_a, "t"_a,
             "Calculate the Jacobian matrix of the transformation");

    // ExplicitForm classes
    nb::class_<stf::ExplicitForm<2>, stf::SpaceTimeFunction<2>>(m, "ExplicitForm2D")
        .def(nb::init<
                std::function<Scalar(std::array<Scalar, 2>, Scalar)>,
                std::function<Scalar(std::array<Scalar, 2>, Scalar)>,
                std::function<std::array<Scalar, 3>(std::array<Scalar, 2>, Scalar)>>(),
            "func"_a, "time_deriv_func"_a = nullptr, "grad_func"_a = nullptr,
            R"(Explicit form of a 2D space-time function.

:param func: A function that takes space-time coordinates ([x, y], t) as input and returns the value.
:param time_deriv_func: Optional function for time derivative computation.
:param grad_func: Optional function for gradient computation.)");

    nb::class_<stf::ExplicitForm<3>, stf::SpaceTimeFunction<3>>(m, "ExplicitForm3D")
        .def(nb::init<
                std::function<Scalar(std::array<Scalar, 3>, Scalar)>,
                std::function<Scalar(std::array<Scalar, 3>, Scalar)>,
                std::function<std::array<Scalar, 4>(std::array<Scalar, 3>, Scalar)>>(),
            "func"_a, "time_deriv_func"_a = nullptr, "grad_func"_a = nullptr,
            R"(Explicit form of a 3D space-time function.

:param func: A function that takes space-time coordinates ([x, y, z], t) as input and returns the value.
:param time_deriv_func: Optional function for time derivative computation.
:param grad_func: Optional function for gradient computation.)");

    // SweepFunction classes
    nb::class_<stf::SweepFunction<2>, stf::SpaceTimeFunction<2>>(m, "SweepFunction2D")
        .def(nb::init<stf::ImplicitFunction<2>&, stf::Transform<2>&>(),
            "implicit_function"_a, "transform"_a,
            "Space-time function created by sweeping a 2D implicit function through space");

    nb::class_<stf::SweepFunction<3>, stf::SpaceTimeFunction<3>>(m, "SweepFunction3D")
        .def(nb::init<stf::ImplicitFunction<3>&, stf::Transform<3>&>(),
            "implicit_function"_a, "transform"_a,
            "Space-time function created by sweeping a 3D implicit function through space");

    // UnionFunction classes
    nb::class_<stf::UnionFunction<2>, stf::SpaceTimeFunction<2>>(m, "UnionFunction2D")
        .def(nb::init<stf::SpaceTimeFunction<2>&, stf::SpaceTimeFunction<2>&, Scalar>(),
            "f1"_a, "f2"_a, "smooth_distance"_a = 0,
            R"(Union of two 2D space-time functions.

:param f1: The first space-time function
:param f2: The second space-time function  
:param smooth_distance: Distance for smooth union (0 for sharp union))");

    nb::class_<stf::UnionFunction<3>, stf::SpaceTimeFunction<3>>(m, "UnionFunction3D")
        .def(nb::init<stf::SpaceTimeFunction<3>&, stf::SpaceTimeFunction<3>&, Scalar>(),
            "f1"_a, "f2"_a, "smooth_distance"_a = 0,
            R"(Union of two 3D space-time functions.

:param f1: The first space-time function
:param f2: The second space-time function
:param smooth_distance: Distance for smooth union (0 for sharp union))");

    // InterpolateFunction classes
    nb::class_<stf::InterpolateFunction<2>, stf::SpaceTimeFunction<2>>(m, "InterpolateFunction2D")
        .def(nb::init<stf::SpaceTimeFunction<2>&, stf::SpaceTimeFunction<2>&, 
                std::function<Scalar(Scalar)>, std::function<Scalar(Scalar)>>(),
            "f1"_a, "f2"_a, 
            "interpolation_func"_a = std::function<Scalar(Scalar)>([](Scalar t) { return t; }),
            "interpolation_derivative"_a = std::function<Scalar(Scalar)>([](Scalar t) { return 1; }),
            R"(Linearly interpolates between two 2D space-time functions.

:param f1: The first function (used at t=0)
:param f2: The second function (used at t=1)
:param interpolation_func: The interpolation function (default is linear)
:param interpolation_derivative: The derivative of interpolation function)");

    nb::class_<stf::InterpolateFunction<3>, stf::SpaceTimeFunction<3>>(m, "InterpolateFunction3D")
        .def(nb::init<stf::SpaceTimeFunction<3>&, stf::SpaceTimeFunction<3>&, 
                std::function<Scalar(Scalar)>, std::function<Scalar(Scalar)>>(),
            "f1"_a, "f2"_a, 
            "interpolation_func"_a = std::function<Scalar(Scalar)>([](Scalar t) { return t; }),
            "interpolation_derivative"_a = std::function<Scalar(Scalar)>([](Scalar t) { return 1; }),
            R"(Linearly interpolates between two 3D space-time functions.

:param f1: The first function (used at t=0)
:param f2: The second function (used at t=1)
:param interpolation_func: The interpolation function (default is linear)
:param interpolation_derivative: The derivative of interpolation function)");

    // OffsetFunction classes
    nb::class_<stf::OffsetFunction<2>, stf::SpaceTimeFunction<2>>(m, "OffsetFunction2D")
        .def(nb::init<stf::SpaceTimeFunction<2>&, std::function<Scalar(Scalar)>, std::function<Scalar(Scalar)>>(),
            "f"_a, 
            "offset_func"_a = std::function<Scalar(Scalar)>([](Scalar t) { return 0; }),
            "offset_derivative"_a = std::function<Scalar(Scalar)>([](Scalar t) { return 0; }),
            R"(Adds a time-dependent offset to a 2D space-time function.

:param f: The base space-time function
:param offset_func: Function computing the time-dependent offset
:param offset_derivative: Function computing the offset's time derivative)");

    nb::class_<stf::OffsetFunction<3>, stf::SpaceTimeFunction<3>>(m, "OffsetFunction3D")
        .def(nb::init<stf::SpaceTimeFunction<3>&, std::function<Scalar(Scalar)>, std::function<Scalar(Scalar)>>(),
            "f"_a, 
            "offset_func"_a = std::function<Scalar(Scalar)>([](Scalar t) { return 0; }),
            "offset_derivative"_a = std::function<Scalar(Scalar)>([](Scalar t) { return 0; }),
            R"(Adds a time-dependent offset to a 3D space-time function.

:param f: The base space-time function
:param offset_func: Function computing the time-dependent offset
:param offset_derivative: Function computing the offset's time derivative)");

    // Primitive submodule classes
    // ImplicitBall classes
    nb::class_<stf::ImplicitBall<2>, stf::ImplicitFunction<2>>(primitive, "ImplicitBall2D")
        .def(nb::init<Scalar, std::array<Scalar, 2>, int>(),
            "radius"_a, "center"_a, "degree"_a = 1,
            R"(Implicit function representing a 2D ball (circle).

:param radius: The radius of the ball
:param center: The center point of the ball
:param degree: The degree of the distance function (default is 1))");

    nb::class_<stf::ImplicitBall<3>, stf::ImplicitFunction<3>>(primitive, "ImplicitBall3D")
        .def(nb::init<Scalar, std::array<Scalar, 3>, int>(),
            "radius"_a, "center"_a, "degree"_a = 1,
            R"(Implicit function representing a 3D ball (sphere).

:param radius: The radius of the ball
:param center: The center point of the ball
:param degree: The degree of the distance function (default is 1))");

    // Convenience aliases in primitive submodule
    primitive.attr("ImplicitCircle") = primitive.attr("ImplicitBall2D");
    primitive.attr("ImplicitSphere") = primitive.attr("ImplicitBall3D");

    // ImplicitCapsule classes
    nb::class_<stf::ImplicitCapsule<2>, stf::ImplicitFunction<2>>(primitive, "ImplicitCapsule2D")
        .def(nb::init<Scalar, std::array<Scalar, 2>, std::array<Scalar, 2>>(),
            "radius"_a, "p1"_a, "p2"_a,
            R"(Implicit function representing a 2D capsule.

:param radius: The radius of the capsule
:param p1: The first end point of the capsule
:param p2: The second end point of the capsule)");

    nb::class_<stf::ImplicitCapsule<3>, stf::ImplicitFunction<3>>(primitive, "ImplicitCapsule3D")
        .def(nb::init<Scalar, std::array<Scalar, 3>, std::array<Scalar, 3>>(),
            "radius"_a, "p1"_a, "p2"_a,
            R"(Implicit function representing a 3D capsule.

:param radius: The radius of the capsule
:param p1: The first end point of the capsule
:param p2: The second end point of the capsule)");

    // ImplicitTorus class (3D only)
    nb::class_<stf::ImplicitTorus, stf::ImplicitFunction<3>>(primitive, "ImplicitTorus")
        .def(nb::init<Scalar, Scalar, std::array<Scalar, 3>>(),
            "R"_a, "r"_a, "center"_a,
            R"(Implicit function representing a 3D torus.

:param R: The major radius (distance from center of tube to center of torus)
:param r: The minor radius (radius of the tube)
:param center: The center point of the torus)");

    // ImplicitUnion classes
    nb::class_<stf::ImplicitUnion<2>, stf::ImplicitFunction<2>>(primitive, "ImplicitUnion2D")
        .def(nb::init<stf::ImplicitFunction<2>&, stf::ImplicitFunction<2>&, Scalar>(),
            "f1"_a, "f2"_a, "smooth_distance"_a = 0,
            R"(Union of two 2D implicit functions.

:param f1: The first implicit function
:param f2: The second implicit function
:param smooth_distance: Distance for smooth union (0 for sharp union))");

    nb::class_<stf::ImplicitUnion<3>, stf::ImplicitFunction<3>>(primitive, "ImplicitUnion3D")
        .def(nb::init<stf::ImplicitFunction<3>&, stf::ImplicitFunction<3>&, Scalar>(),
            "f1"_a, "f2"_a, "smooth_distance"_a = 0,
            R"(Union of two 3D implicit functions.

:param f1: The first implicit function
:param f2: The second implicit function
:param smooth_distance: Distance for smooth union (0 for sharp union))");

    // GenericFunction classes
    nb::class_<stf::GenericFunction<2>, stf::ImplicitFunction<2>>(primitive, "GenericFunction2D")
        .def(nb::init<std::function<Scalar(std::array<Scalar, 2>)>, 
                std::function<std::array<Scalar, 2>(std::array<Scalar, 2>)>>(),
            "value_func"_a, "gradient_func"_a,
            R"(Generic 2D implicit function from function pointers.

:param value_func: Function that computes the value at a given position
:param gradient_func: Function that computes the gradient at a given position)");

    nb::class_<stf::GenericFunction<3>, stf::ImplicitFunction<3>>(primitive, "GenericFunction3D")
        .def(nb::init<std::function<Scalar(std::array<Scalar, 3>)>, 
                std::function<std::array<Scalar, 3>(std::array<Scalar, 3>)>>(),
            "value_func"_a, "gradient_func"_a,
            R"(Generic 3D implicit function from function pointers.

:param value_func: Function that computes the value at a given position
:param gradient_func: Function that computes the gradient at a given position)");

    // Duchon class (3D only) - also in primitive submodule
    nb::class_<stf::Duchon, stf::ImplicitFunction<3>>(primitive, "Duchon")
        .def(nb::init<std::vector<std::array<Scalar, 3>>, 
                std::vector<std::array<Scalar, 4>>,
                std::array<Scalar, 4>,
                std::array<Scalar, 3>,
                Scalar,
                bool>(),
            "points"_a, "rbf_coeffs"_a, "affine_coeffs"_a,
            "center"_a = std::array<Scalar, 3>{0, 0, 0},
            "radius"_a = 1.0,
            "positive_inside"_a = false,
            R"(Duchon's interpolant from control points and coefficients.

:param points: Vector of control points in 3D space
:param rbf_coeffs: Vector of RBF coefficients for each control point [a, bx, by, bz]
:param affine_coeffs: Array of affine coefficients [c0, c1, c2, c3]
:param center: The target center of the implicit surface
:param radius: The target bounding sphere radius
:param positive_inside: Flag indicating if inside of surface is positive)")
        .def(nb::init<std::filesystem::path, 
                std::filesystem::path,
                std::array<Scalar, 3>,
                Scalar,
                bool>(),
            "samples_file"_a, "coeffs_file"_a,
            "center"_a = std::array<Scalar, 3>{0, 0, 0},
            "radius"_a = 1.0,
            "positive_inside"_a = false,
            R"(Duchon's interpolant from files.

:param samples_file: Path to .xyz file containing 3D sample points
:param coeffs_file: Path to file containing RBF and affine coefficients
:param center: The target center of the implicit surface
:param radius: The target bounding sphere radius
:param positive_inside: Flag indicating if inside of surface is positive)");

    // Add convenience aliases in primitive submodule
    primitive.attr("ImplicitBall") = primitive.attr("ImplicitBall3D");
    primitive.attr("ImplicitCapsule") = primitive.attr("ImplicitCapsule3D");
    primitive.attr("GenericFunction") = primitive.attr("GenericFunction3D");

    // Transform submodule classes
    // Translation classes
    nb::class_<stf::Translation<2>, stf::Transform<2>>(transform, "Translation2D")
        .def(nb::init<std::array<Scalar, 2>>(),
            "translation"_a,
            R"(Translation transformation in 2D.

:param translation: The translation vector [dx, dy])");

    nb::class_<stf::Translation<3>, stf::Transform<3>>(transform, "Translation3D")
        .def(nb::init<std::array<Scalar, 3>>(),
            "translation"_a,
            R"(Translation transformation in 3D.

:param translation: The translation vector [dx, dy, dz])");

    // Rotation classes
    nb::class_<stf::Rotation<2>, stf::Transform<2>>(transform, "Rotation2D")
        .def(nb::init<std::array<Scalar, 2>, std::array<Scalar, 2>, Scalar>(),
            "center"_a, "axis"_a, "angle"_a = 360,
            R"(Rotation transformation in 2D.

:param center: The center point of rotation
:param axis: The rotation axis (not used in 2D but required for interface)
:param angle: The total angle of rotation in degrees (default: 360))");

    nb::class_<stf::Rotation<3>, stf::Transform<3>>(transform, "Rotation3D")
        .def(nb::init<std::array<Scalar, 3>, std::array<Scalar, 3>, Scalar>(),
            "center"_a, "axis"_a, "angle"_a = 360,
            R"(Rotation transformation in 3D.

:param center: The center point of rotation
:param axis: The rotation axis
:param angle: The total angle of rotation in degrees (default: 360))");

    // Scale classes
    nb::class_<stf::Scale<2>, stf::Transform<2>>(transform, "Scale2D")
        .def(nb::init<std::array<Scalar, 2>, std::array<Scalar, 2>>(),
            "factors"_a, "center"_a = std::array<Scalar, 2>{0, 0},
            R"(Scaling transformation in 2D.

:param factors: The scaling factors for each dimension [sx, sy]
:param center: The pivot point around which scaling occurs (default: origin))");

    nb::class_<stf::Scale<3>, stf::Transform<3>>(transform, "Scale3D")
        .def(nb::init<std::array<Scalar, 3>, std::array<Scalar, 3>>(),
            "factors"_a, "center"_a = std::array<Scalar, 3>{0, 0, 0},
            R"(Scaling transformation in 3D.

:param factors: The scaling factors for each dimension [sx, sy, sz]
:param center: The pivot point around which scaling occurs (default: origin))");

    // Compose classes
    nb::class_<stf::Compose<2>, stf::Transform<2>>(transform, "Compose2D")
        .def(nb::init<stf::Transform<2>&, stf::Transform<2>&>(),
            "transform1"_a, "transform2"_a,
            R"(Composition of two 2D transformations.

:param transform1: The first transformation to apply
:param transform2: The second transformation to apply)");

    nb::class_<stf::Compose<3>, stf::Transform<3>>(transform, "Compose3D")
        .def(nb::init<stf::Transform<3>&, stf::Transform<3>&>(),
            "transform1"_a, "transform2"_a,
            R"(Composition of two 3D transformations.

:param transform1: The first transformation to apply
:param transform2: The second transformation to apply)");

    // Polyline classes
    nb::class_<stf::Polyline<2>, stf::Transform<2>>(transform, "Polyline2D")
        .def(nb::init<std::vector<std::array<Scalar, 2>>>(),
            "points"_a,
            R"(Polyline transformation in 2D.

:param points: The points defining the polyline (must contain at least 2 points))");

    nb::class_<stf::Polyline<3>, stf::Transform<3>>(transform, "Polyline3D")
        .def(nb::init<std::vector<std::array<Scalar, 3>>>(),
            "points"_a,
            R"(Polyline transformation in 3D.

:param points: The points defining the polyline (must contain at least 2 points))");

    // PolyBezier classes
    nb::class_<stf::PolyBezier<2>, stf::Transform<2>>(transform, "PolyBezier2D")
        .def_static("from_samples", &stf::PolyBezier<2>::from_samples,
            "samples"_a, "follow_tangent"_a = true,
            R"(Create a 2D PolyBezier from sample points.

:param samples: A vector of sample points (minimum 3 points required)
:param follow_tangent: If true, adds rotation so Z-axis follows curve tangent)");

    nb::class_<stf::PolyBezier<3>, stf::Transform<3>>(transform, "PolyBezier3D")
        .def_static("from_samples", &stf::PolyBezier<3>::from_samples,
            "samples"_a, "follow_tangent"_a = true,
            R"(Create a 3D PolyBezier from sample points.

:param samples: A vector of sample points (minimum 3 points required)
:param follow_tangent: If true, adds rotation so Z-axis follows curve tangent)");

    // Add convenience aliases in transform submodule
    transform.attr("Translation") = transform.attr("Translation3D");
    transform.attr("Rotation") = transform.attr("Rotation3D");
    transform.attr("Scale") = transform.attr("Scale3D");
    transform.attr("Compose") = transform.attr("Compose3D");
    transform.attr("Polyline") = transform.attr("Polyline3D");
    transform.attr("PolyBezier") = transform.attr("PolyBezier3D");

    // Add convenience aliases for backward compatibility in main module
    // These point to the 3D versions for backward compatibility
    m.attr("ExplicitForm") = m.attr("ExplicitForm3D");
    m.attr("SweepFunction") = m.attr("SweepFunction3D");
    m.attr("UnionFunction") = m.attr("UnionFunction3D");
    m.attr("InterpolateFunction") = m.attr("InterpolateFunction3D");
    m.attr("OffsetFunction") = m.attr("OffsetFunction3D");
    
}
