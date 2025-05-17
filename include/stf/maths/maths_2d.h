#pragma once

#include <stf/common.h>

#include <array>
#include <cmath>
#include <span>
#include <stdexcept>

namespace stf {

using Vec2 = std::array<Scalar, 2>;
using Mat2 = std::array<std::array<Scalar, 2>, 2>;


// Vector utilities
inline Scalar dot(const Vec2& a, const Vec2& b)
{
    return a[0] * b[0] + a[1] * b[1];
}

inline Scalar norm(const Vec2& v)
{
    return std::sqrt(dot(v, v));
}

inline Vec2 normalize(const Vec2& v)
{
    Scalar n = norm(v);
    if (n < 1e-8) throw std::runtime_error("Zero-length vector");
    return {v[0] / n, v[1] / n};
}

inline Mat2 multiply(const Mat2& A, const Mat2& B)
{
    Mat2 result;
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
            result[i][j] = A[i][0] * B[0][j] + A[i][1] * B[1][j];
        }
    }
    return result;
}

inline Vec2 add(const Vec2& A, const Vec2& B)
{
    return {A[0] + B[0], A[1] + B[1]};
}

inline Vec2 subtract(const Vec2& A, const Vec2& B)
{
    return {A[0] - B[0], A[1] - B[1]};
}

inline Vec2 scale(const Vec2& A, Scalar s)
{
    return {A[0] * s, A[1] * s};
}

// Rotation matrix from one vector to another
inline Mat2 rotation_matrix(const Vec2& from, const Vec2& to)
{
    Vec2 u = normalize(from);
    Vec2 v = normalize(to);

    Scalar c = dot(u, v); // cos(θ)
    Scalar s = u[0] * v[1] - u[1] * v[0]; // sin(θ) = cross product in 2D

    return {{{c, -s}, {s, c}}};
}

// Apply 2D matrix to a vector
inline Vec2 apply_matrix(const Mat2& M, const Vec2& v)
{
    return {M[0][0] * v[0] + M[0][1] * v[1], M[1][0] * v[0] + M[1][1] * v[1]};
}

inline Mat2 transpose(const Mat2& M)
{
    return {{{M[0][0], M[1][0]}, {M[0][1], M[1][1]}}};
}

inline Vec2 bezier(std::span<const Vec2, 4> control_points, Scalar t)
{
    Scalar u = 1 - t;
    return {
        u * u * u * control_points[0][0] + 3 * u * u * t * control_points[1][0] +
            3 * u * t * t * control_points[2][0] + t * t * t * control_points[3][0],
        u * u * u * control_points[0][1] + 3 * u * u * t * control_points[1][1] +
            3 * u * t * t * control_points[2][1] + t * t * t * control_points[3][1]};
}

inline Vec2 bezier_derivative(std::span<const Vec2, 4> control_points, Scalar t)
{
    Scalar u = 1 - t;
    Scalar uu = u * u;
    Scalar tt = t * t;

    return {
        3 * uu * (control_points[1][0] - control_points[0][0]) +
            6 * u * t * (control_points[2][0] - control_points[1][0]) +
            3 * tt * (control_points[3][0] - control_points[2][0]),

        3 * uu * (control_points[1][1] - control_points[0][1]) +
            6 * u * t * (control_points[2][1] - control_points[1][1]) +
            3 * tt * (control_points[3][1] - control_points[2][1])};
}

inline Vec2 bezier_second_derivative(std::span<const Vec2, 4> control_points, Scalar t)
{
    Scalar u = 1 - t;

    return {
        6 * u * (control_points[2][0] - 2 * control_points[1][0] + control_points[0][0]) +
            6 * t * (control_points[3][0] - 2 * control_points[2][0] + control_points[1][0]),
        6 * u * (control_points[2][1] - 2 * control_points[1][1] + control_points[0][1]) +
            6 * t * (control_points[3][1] - 2 * control_points[2][1] + control_points[1][1])};
}

} // namespace stf
