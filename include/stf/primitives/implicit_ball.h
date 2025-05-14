#pragma once

#include <stf/common.h>
#include <stf/primitives/implicit_function.h>

namespace stf {

template <int dim>
class ImplicitBall : ImplicitFunction<dim>
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
    Scalar m_radius; ///< The radius of the circle
    std::array<Scalar, dim> m_center; ///< The center point of the circle
};

using ImplicitCircle = ImplicitBall<2>; ///< 2D implicit ball (circle)
using ImplicitSphere = ImplicitBall<3>; ///< 3D implicit ball (sphere)

} // namespace stf
