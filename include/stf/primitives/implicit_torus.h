#pragma once

#include <stf/common.h>
#include <stf/primitives/implicit_function.h>

namespace stf {

/**
 * @brief Implicit function representing a 3D torus.
 *
 * The torus is defined by two radii (R and r) and a center point.
 * R is the distance from the center of the tube to the center of the torus,
 * and r is the radius of the tube.
 */
class ImplicitTorus : public ImplicitFunction<3>
{
public:
    /**
     * @brief Constructs a new implicit torus.
     *
     * @param R The major radius (distance from center of tube to center of
     * torus)
     * @param r The minor radius (radius of the tube)
     * @param center The center point of the torus
     */
    ImplicitTorus(Scalar R, Scalar r, std::array<Scalar, 3> center)
        : m_R(R)
        , m_r(r)
        , m_center(center)
    {}

    Scalar value(std::array<Scalar, 3> pos) const override
    {
        Scalar x = pos[0] - m_center[0];
        Scalar y = pos[1] - m_center[1];
        Scalar z = pos[2] - m_center[2];
        Scalar len_xy = std::sqrt(x * x + y * y);

        return std::sqrt(z * z + (len_xy - m_R) * (len_xy - m_R)) - m_r;
    }

    std::array<Scalar, 3> gradient(std::array<Scalar, 3> pos) const override
    {
        Scalar x = pos[0] - m_center[0];
        Scalar y = pos[1] - m_center[1];
        Scalar z = pos[2] - m_center[2];

        Scalar len_xy = std::sqrt(x * x + y * y);

        // Avoid division by zero (if point is at z-axis)
        if (len_xy < 1e-6f) {
            return {0, 0, static_cast<Scalar>(z >= 0 ? 1 : -1)};
        }

        Scalar a = len_xy - m_R;
        Scalar q_len = std::sqrt(a * a + z * z);

        // Again avoid division by zero if exactly on torus surface
        if (q_len < 1e-6f) {
            return {0, 0, 0}; // Undefined
        }

        Scalar dx = (a / q_len) * (x / len_xy);
        Scalar dy = (a / q_len) * (y / len_xy);
        Scalar dz = z / q_len;

        return {dx, dy, dz};
    }

private:
    Scalar m_R; ///< The major radius of the torus
    Scalar m_r; ///< The minor radius of the torus
    std::array<Scalar, 3> m_center; ///< The center point of the torus
};

} // namespace stf
