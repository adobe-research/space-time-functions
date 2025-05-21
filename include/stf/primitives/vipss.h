#pragma once

#include <stf/common.h>
#include <stf/maths/all.h>
#include <stf/primitives/implicit_function.h>

#include <array>
#include <stdexcept>
#include <vector>

namespace stf {

/**
 * @brief A class implementing a Vector Implicit Polynomial Spline Surface (VIPSS).
 *
 * VIPSS is an implicit surface representation that combines radial basis functions (RBF)
 * with an affine term. The surface is defined by a set of control points and their
 * associated RBF coefficients, along with affine coefficients for the global shape.
 *
 * The implicit function is defined as:
 * f(x) = Σᵢ (dᵢ³ * aᵢ + gᵢ·bᵢ) + c₀ + c₁x + c₂y + c₃z
 * where:
 * - dᵢ is the distance from point x to control point pᵢ
 * - gᵢ is the gradient of dᵢ³
 * - aᵢ and bᵢ are the RBF coefficients
 * - c₀, c₁, c₂, c₃ are the affine coefficients
 */
class Vipss : public ImplicitFunction<3>
{
public:
    /**
     * @brief Constructs a VIPSS from control points and coefficients.
     *
     * @param points Vector of control points in 3D space
     * @param rbf_coeffs Vector of RBF coefficients for each control point. Each coefficient
     *                   array contains [a, bₓ, bᵧ, bz] where a is the cubic term coefficient
     *                   and bₓ, bᵧ, bz are the gradient term coefficients
     * @param affine_coeffs Array of affine coefficients [c₀, c₁, c₂, c₃] for the global shape
     * @throws std::runtime_error if the number of points and RBF coefficients don't match
     */
    Vipss(
        std::vector<std::array<Scalar, 3>> points,
        std::vector<std::array<Scalar, 4>> rbf_coeffs,
        std::array<Scalar, 4> affine_coeffs)
        : m_points(std::move(points))
        , m_rbf_coeffs(std::move(rbf_coeffs))
        , m_affine_coeffs(std::move(affine_coeffs))
    {
        if (m_points.size() != m_rbf_coeffs.size()) {
            throw std::runtime_error("Number of points and RBF coefficients must match.");
        }
    }

    /**
     * @brief Evaluates the implicit function at a given point.
     *
     * @param pos The 3D point at which to evaluate the function
     * @return The value of the implicit function at the given point
     */
    Scalar value(std::array<Scalar, 3> pos) const override
    {
        const size_t num_pts = m_points.size();
        Scalar result = 0;

        for (size_t i = 0; i < num_pts; i++) {
            const auto& pi = m_points[i];
            const auto& coeffs = m_rbf_coeffs[i];

            Vec3 diff = subtract(pos, pi);
            Scalar d = norm(diff);
            Vec3 g = scale(diff, 3 * d);

            result +=
                d * d * d * coeffs[0] + g[0] * coeffs[1] + g[1] * coeffs[2] + g[2] * coeffs[3];
        }

        result += m_affine_coeffs[0] + m_affine_coeffs[1] * pos[0] + m_affine_coeffs[2] * pos[1] +
                  m_affine_coeffs[3] * pos[2];
        return result;
    }

    /**
     * @brief Computes the gradient of the implicit function at a given point.
     *
     * The gradient is computed as the sum of:
     * 1. The gradient of each RBF term (cubic term + gradient term)
     * 2. The gradient of the affine term
     *
     * @param pos The 3D point at which to compute the gradient
     * @return The gradient vector at the given point
     */
    std::array<Scalar, 3> gradient(std::array<Scalar, 3> pos) const override
    {
        const size_t num_pts = m_points.size();
        std::array<Scalar, 3> result{0, 0, 0};
        const Mat3 I = identityMatrix();

        for (size_t i = 0; i < num_pts; i++) {
            const auto& pi = m_points[i];
            const auto& coeffs = m_rbf_coeffs[i];

            Vec3 diff = subtract(pos, pi);
            Scalar d = norm(diff);
            Vec3 g = scale(diff, 3 * d);

            Mat3 O{
                {{diff[0] * diff[0], diff[0] * diff[1], diff[0] * diff[2]},
                 {diff[1] * diff[0], diff[1] * diff[1], diff[1] * diff[2]},
                 {diff[2] * diff[0], diff[2] * diff[1], diff[2] * diff[2]}}};
            Mat3 H{{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}};
            if (d > 1e-8) {
                H = scale(add(scale(I, d), scale(O, 1 / d)), 3);
            }

            result =
                add(add(result, scale(g, coeffs[0])),
                    apply_matrix(H, {coeffs[1], coeffs[2], coeffs[3]}));
        }

        result =
            add(result,
                {
                    m_affine_coeffs[1],
                    m_affine_coeffs[2],
                    m_affine_coeffs[3],
                });
        return result;
    }

private:
    std::vector<std::array<Scalar, 3>> m_points; ///< Control points defining the surface
    std::vector<std::array<Scalar, 4>> m_rbf_coeffs; ///< RBF coefficients for each control point
    std::array<Scalar, 4> m_affine_coeffs; ///< Affine coefficients for global shape
};

} // namespace stf
