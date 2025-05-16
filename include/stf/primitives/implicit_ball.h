#pragma once

#include <stf/common.h>
#include <stf/primitives/implicit_function.h>

namespace stf {

/**
 * @brief Implicit function representing a ball (circle in 2D, sphere in 3D).
 *
 * This class implements an implicit function for a ball in N-dimensional space.
 * The function returns the signed distance to the surface of the ball, with
 * positive values outside the ball and negative values inside.
 *
 * @tparam dim The dimension of the space (2 for 2D, 3 for 3D)
 */
template <int dim>
class ImplicitBall : public ImplicitFunction<dim>
{
public:
    /**
     * @brief Constructs a new implicit ball.
     *
     * @param radius The radius of the ball
     * @param center The center point of the ball
     */
    ImplicitBall(Scalar radius, std::array<Scalar, dim> center)
        : m_radius(radius)
        , m_center(center)
    {}

    /**
     * @brief Evaluates the implicit function at a given position.
     *
     * Computes the signed distance to the surface of the ball. The function
     * returns positive values outside the ball, negative values inside, and
     * zero on the surface.
     *
     * @param pos The position to evaluate at
     * @return Scalar The signed distance to the surface
     */
    Scalar value(std::array<Scalar, dim> pos) const override
    {
        if constexpr (dim == 2) {
            return std::sqrt(
                       (pos[0] - m_center[0]) * (pos[0] - m_center[0]) +
                       (pos[1] - m_center[1]) * (pos[1] - m_center[1])) -
                   m_radius;
        } else if constexpr (dim == 3) {
            return std::sqrt(
                       (pos[0] - m_center[0]) * (pos[0] - m_center[0]) +
                       (pos[1] - m_center[1]) * (pos[1] - m_center[1]) +
                       (pos[2] - m_center[2]) * (pos[2] - m_center[2])) -
                   m_radius;
        } else {
            throw std::invalid_argument("ImplicitBall is only defined for 2D and 3D.");
        }
    }

    /**
     * @brief Computes the gradient of the implicit function at a given position.
     *
     * The gradient is a unit vector pointing outward from the surface of the
     * ball. For points on the surface, it is the normal vector. For points
     * inside or outside, it points toward or away from the center.
     *
     * @param pos The position to evaluate at
     * @return std::array<Scalar, dim> The normalized gradient vector
     */
    std::array<Scalar, dim> gradient(std::array<Scalar, dim> pos) const override
    {
        if constexpr (dim == 2) {
            Scalar r = std::sqrt(
                (pos[0] - m_center[0]) * (pos[0] - m_center[0]) +
                (pos[1] - m_center[1]) * (pos[1] - m_center[1]));
            if (r == 0) return {0, 0};

            return {(pos[0] - m_center[0]) / r, (pos[1] - m_center[1]) / r};
        } else if constexpr (dim == 3) {
            Scalar r = std::sqrt(
                (pos[0] - m_center[0]) * (pos[0] - m_center[0]) +
                (pos[1] - m_center[1]) * (pos[1] - m_center[1]) +
                (pos[2] - m_center[2]) * (pos[2] - m_center[2]));
            if (r == 0) return {0, 0, 0};

            return {
                (pos[0] - m_center[0]) / r,
                (pos[1] - m_center[1]) / r,
                (pos[2] - m_center[2]) / r};
        } else {
            throw std::invalid_argument("ImplicitBall is only defined for 2D and 3D.");
        }
    }

private:
    Scalar m_radius; ///< The radius of the ball
    std::array<Scalar, dim> m_center; ///< The center point of the ball
};

using ImplicitCircle = ImplicitBall<2>; ///< 2D implicit ball (circle)
using ImplicitSphere = ImplicitBall<3>; ///< 3D implicit ball (sphere)

} // namespace stf
