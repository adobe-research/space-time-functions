#pragma once

#include <stf/common.h>

#include <array>
#include <cmath>
#include <span>
#include <stdexcept>

namespace stf {

using Vec3 = std::array<Scalar, 3>;
using Mat3 = std::array<std::array<Scalar, 3>, 3>;


// Vector utilities
inline Scalar dot(const Vec3& a, const Vec3& b)
{
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

inline Vec3 cross(const Vec3& a, const Vec3& b)
{
    return {a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]};
}

inline Scalar norm(const Vec3& v)
{
    return std::sqrt(dot(v, v));
}

inline Vec3 normalize(const Vec3& v)
{
    Scalar n = norm(v);
    if (n < 1e-8) throw std::runtime_error("Zero-length vector");
    return {v[0] / n, v[1] / n, v[2] / n};
}

// Identity matrix
inline Mat3 identityMatrix()
{
    return {{{1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}}};
}

// Skew-symmetric matrix from vector
inline Mat3 skew(const Vec3& v)
{
    return {{{0.0, -v[2], v[1]}, {v[2], 0.0, -v[0]}, {-v[1], v[0], 0.0}}};
}

// Matrix addition: A + B
inline Mat3 add(const Mat3& A, const Mat3& B)
{
    Mat3 result;
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j) result[i][j] = A[i][j] + B[i][j];
    return result;
}

// Scalar * matrix
inline Mat3 scale(const Mat3& A, Scalar s)
{
    Mat3 result;
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j) result[i][j] = A[i][j] * s;
    return result;
}

// Matrix multiplication: A * B
inline Mat3 multiply(const Mat3& A, const Mat3& B)
{
    Mat3 result = {};
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            for (int k = 0; k < 3; ++k) result[i][j] += A[i][k] * B[k][j];
    return result;
}

// Rodrigues' rotation formula
inline Mat3 rotation_matrix(const Vec3& from, const Vec3& to)
{
    Vec3 v1 = normalize(from);
    Vec3 v2 = normalize(to);

    Scalar c = dot(v1, v2);

    if (c > 0.999999) {
        return identityMatrix(); // Vectors are nearly identical
    } else if (c < -0.999999) {
        // Vectors are opposite — pick orthogonal vector to rotate 180°
        Vec3 axis = cross(v1, {1.0, 0.0, 0.0});
        if (norm(axis) < 1e-6) axis = cross(v1, {0.0, 1.0, 0.0});
        axis = normalize(axis);

        Mat3 K = skew(axis);
        Mat3 KK = multiply(K, K);
        return add(identityMatrix(), scale(K, 2.0)); // 180° rotation
    } else {
        Vec3 axis = normalize(cross(v1, v2));
        Scalar s = std::sqrt(1.0 - c * c);
        Mat3 K = skew(axis);
        Mat3 KK = multiply(K, K);

        return add(add(identityMatrix(), scale(K, s)), scale(KK, 1 - c));
    }
}

// Apply 3D matrix to a vector
inline Vec3 apply_matrix(const Mat3& M, const Vec3& v)
{
    return {
        M[0][0] * v[0] + M[0][1] * v[1] + M[0][2] * v[2],
        M[1][0] * v[0] + M[1][1] * v[1] + M[1][2] * v[2],
        M[2][0] * v[0] + M[2][1] * v[1] + M[2][2] * v[2]};
}

inline Mat3 transpose(const Mat3& M)
{
    return {
        {{M[0][0], M[1][0], M[2][0]}, {M[0][1], M[1][1], M[2][1]}, {M[0][2], M[1][2], M[2][2]}}};
}

inline Vec3 bezier(std::span<const Vec3, 4> control_points, Scalar t)
{
    Scalar u = 1 - t;
    Scalar uu = u * u;
    Scalar tt = t * t;
    Scalar uuu = uu * u;
    Scalar uut = uu * t;
    Scalar utt = u * tt;
    Scalar ttt = tt * t;

    return {
        uuu * control_points[0][0] + 3 * uut * control_points[1][0] +
            3 * utt * control_points[2][0] + ttt * control_points[3][0],

        uuu * control_points[0][1] + 3 * uut * control_points[1][1] +
            3 * utt * control_points[2][1] + ttt * control_points[3][1],

        uuu * control_points[0][2] + 3 * uut * control_points[1][2] +
            3 * utt * control_points[2][2] + ttt * control_points[3][2]};
}

inline Vec3 bezier_derivative(std::span<const Vec3, 4> control_points, Scalar t)
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
            3 * tt * (control_points[3][1] - control_points[2][1]),

        3 * uu * (control_points[1][2] - control_points[0][2]) +
            6 * u * t * (control_points[2][2] - control_points[1][2]) +
            3 * tt * (control_points[3][2] - control_points[2][2])};
}

inline Vec3 bezier_second_derivative(std::span<const Vec3, 4> control_points, Scalar t)
{
    Scalar u = 1 - t;

    return {
        6 * u * (control_points[2][0] - 2 * control_points[1][0] + control_points[0][0]) +
            6 * t * (control_points[3][0] - 2 * control_points[2][0] + control_points[1][0]),

        6 * u * (control_points[2][1] - 2 * control_points[1][1] + control_points[0][1]) +
            6 * t * (control_points[3][1] - 2 * control_points[2][1] + control_points[1][1]),

        6 * u * (control_points[2][2] - 2 * control_points[1][2] + control_points[0][2]) +
            6 * t * (control_points[3][2] - 2 * control_points[2][2] + control_points[1][2])};
}

} // namespace stf
