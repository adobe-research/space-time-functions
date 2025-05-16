#pragma once

#include <stf/common.h>
#include <stf/primitives/implicit_function.h>

#include <array>
#include <cmath>
#include <numeric>

namespace stf {

/**
 * @brief A class representing an implicit capsule (a cylinder with hemispherical caps).
 * 
 * The capsule is defined by two endpoints and a radius. The implicit function
 * returns the signed distance to the surface of the capsule, with positive values
 * outside, negative values inside, and zero on the surface.
 * 
 * @tparam dim The dimensionality of the space (2D or 3D)
 */
template <int dim>
class ImplicitCapsule : public ImplicitFunction<dim>
{
public:
    /**
     * @brief Constructs a new implicit capsule.
     *
     * @param radius The radius of the capsule
     * @param p1 The first end point of the capsule
     * @param p2 The second end point of the capsule
     */
    ImplicitCapsule(Scalar radius, std::array<Scalar, dim> p1, std::array<Scalar, dim> p2)
        : m_radius(radius)
        , m_p1(p1)
        , m_p2(p2)
    {}

    /**
     * @brief Evaluates the implicit function at a given position.
     *
     * Computes the signed distance to the surface of the capsule. The function
     * returns positive values outside the capsule, negative values inside, and
     * zero on the surface.
     *
     * @param pos The position to evaluate at
     * @return Scalar The signed distance to the surface
     */
    Scalar value(std::array<Scalar, dim> pos) const override
    {
        std::array<Scalar, dim> closest_point = compute_closest_point(pos);

        // Calculate the distance from the point to the closest point on the line segment
        Scalar distance_squared = 0;
        for (int i = 0; i < dim; ++i) {
            distance_squared += (pos[i] - closest_point[i]) * (pos[i] - closest_point[i]);
        }

        return std::sqrt(distance_squared) - m_radius;
    }

    /**
     * @brief Computes the gradient of the implicit function at a given position.
     * 
     * The gradient points outward from the surface of the capsule and is normalized.
     * For points very close to the surface, the gradient is undefined and returns
     * a zero vector.
     * 
     * @param pos The position to evaluate the gradient at
     * @return std::array<Scalar, dim> The normalized gradient vector
     */
    std::array<Scalar, dim> gradient(std::array<Scalar, dim> pos) const override
    {
        std::array<Scalar, dim> closest_point = compute_closest_point(pos);

        // Calculate the gradient
        std::array<Scalar, dim> grad;
        Scalar grad_norm = 0;
        for (int i = 0; i < dim; ++i) {
            grad[i] = pos[i] - closest_point[i];
            grad_norm += grad[i] * grad[i];
        }

        // Normalize the gradient
        grad_norm = std::sqrt(grad_norm);
        if (grad_norm > 1e-6) {
            for (int i = 0; i < dim; ++i) {
                grad[i] /= grad_norm;
            }
        } else {
            grad.fill(0);
        }

        return grad;
    }

private:
    /**
     * @brief Computes the closest point on the line segment to a given position.
     * 
     * @param pos The position to compute the closest point from
     * @return std::array<Scalar, dim> The closest point on the line segment
     */
    std::array<Scalar, dim> compute_closest_point(const std::array<Scalar, dim>& pos) const
    {
        // Calculate the distance from the point to the line segment defined by p1 and p2
        std::array<Scalar, dim> d;
        for (int i = 0; i < dim; ++i) {
            d[i] = m_p2[i] - m_p1[i];
        }

        Scalar t = 0;
        for (int i = 0; i < dim; ++i) {
            t += (pos[i] - m_p1[i]) * d[i];
        }
        t /= std::inner_product(d.begin(), d.end(), d.begin(), Scalar(0));

        // Clamp t to the range [0, 1]
        t = std::max(Scalar(0), std::min(Scalar(1), t));

        // Calculate the closest point on the line segment
        std::array<Scalar, dim> closest_point;
        for (int i = 0; i < dim; ++i) {
            closest_point[i] = m_p1[i] + t * d[i];
        }
        return closest_point;
    }

private:
    Scalar m_radius; ///< The radius of the capsule
    std::array<Scalar, dim> m_p1; ///< The first end point of the capsule
    std::array<Scalar, dim> m_p2; ///< The second end point of the capsule
};

} // namespace stf
