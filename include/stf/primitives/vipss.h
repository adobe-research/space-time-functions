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
     * @param center The target center of the VIPSS surface
     * @param scale The target scale of the VIPSS surface
     * @throws std::runtime_error if the number of points and RBF coefficients don't match
     */
    Vipss(
        std::vector<std::array<Scalar, 3>> points,
        std::vector<std::array<Scalar, 4>> rbf_coeffs,
        std::array<Scalar, 4> affine_coeffs,
        std::array<Scalar, 3> center = {0, 0, 0},
        Scalar scale = 1.0)
        : m_points(std::move(points))
        , m_rbf_coeffs(std::move(rbf_coeffs))
        , m_affine_coeffs(std::move(affine_coeffs))
    {
        if (m_points.size() != m_rbf_coeffs.size()) {
            throw std::runtime_error("Number of points and RBF coefficients must match.");
        }
        if (m_points.empty()) {
            throw std::runtime_error("At least one control point is required.");
        }
        if (std::abs(scale) < 1e-6) {
            throw std::runtime_error("Scale factor must be non-zero.");
        }

        std::array<Scalar, 3> bbox_min = m_points.front();
        std::array<Scalar, 3> bbox_max = m_points.front();
        for (const auto& p : m_points) {
            for (size_t i = 0; i < 3; ++i) {
                bbox_min[i] = std::min(bbox_min[i], p[i]);
                bbox_max[i] = std::max(bbox_max[i], p[i]);
            }
        }

        std::array<Scalar, 3> bbox_center{
            (bbox_min[0] + bbox_max[0]) / 2,
            (bbox_min[1] + bbox_max[1]) / 2,
            (bbox_min[2] + bbox_max[2]) / 2,
        };

        m_translation[0] = -center[0] / scale + bbox_center[0];
        m_translation[1] = -center[1] / scale + bbox_center[1];
        m_translation[2] = -center[2] / scale + bbox_center[2];
        m_scale = 1 / scale;
    }

    /**
     * @brief Evaluates the implicit function at a given point.
     *
     * @param pos The 3D point at which to evaluate the function
     * @return The value of the implicit function at the given point
     */
    Scalar value(std::array<Scalar, 3> pos) const override
    {
        pos = add(scale(pos, m_scale), m_translation);
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

        // Negate because the default vipss has positive values inside.
        return -result;
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
        pos = add(scale(pos, m_scale), m_translation);
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
        // Negate because the default vipss has positive values inside.
        result = scale(result, -m_scale);
        return result;
    }

private:
    std::vector<std::array<Scalar, 3>> m_points; ///< Control points defining the surface
    std::vector<std::array<Scalar, 4>> m_rbf_coeffs; ///< RBF coefficients for each control point
    std::array<Scalar, 4> m_affine_coeffs; ///< Affine coefficients for global shape

    std::array<Scalar, 3> m_translation; ///< Pre-translation vector of the space
    Scalar m_scale; ///< Pre-scale factor for the space
};

} // namespace stf
