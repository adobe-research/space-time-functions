#pragma once

#include <stf/common.h>
#include <stf/transform/transform.h>

#include <array>
#include <span>

namespace stf {

/**
 * @brief A rotation transformation around an axis in 3D or around a point in
 * 2D.
 *
 * This class implements rotation transformations. In 3D, it rotates points
 * around a specified axis through a center point. In 2D, it rotates points
 * around a center point.
 *
 * @tparam dim The dimensionality of the space (2D or 3D)
 */
template <int dim>
class Rotation : public Transform<dim>
{
public:
    /**
     * @brief Constructs a rotation transformation.
     *
     * @param center The center point of rotation
     * @param axis The rotation axis (only used in 3D)
     * @param angle The total angle of rotation in degrees (default: 360)
     */
    Rotation(std::array<Scalar, dim> center, std::array<Scalar, dim> axis, Scalar angle = 360)
        : m_axis(axis)
        , m_center(center)
        , m_angle(angle)
    {}

    std::array<Scalar, dim> transform(std::array<Scalar, dim> pos, Scalar t) const override
    {
        if constexpr (dim == 3) {
            // Convert angle to radians
            Scalar angle = t * m_angle * M_PI / 180.0;

            // Normalize the axis
            Scalar axis_length = 0;
            for (int i = 0; i < dim; ++i) {
                axis_length += m_axis[i] * m_axis[i];
            }
            axis_length = std::sqrt(axis_length);

            std::array<Scalar, dim> normalized_axis;
            for (int i = 0; i < dim; ++i) {
                normalized_axis[i] = m_axis[i] / axis_length;
            }

            // Rodrigues' rotation formula
            Scalar cos_angle = std::cos(angle);
            Scalar sin_angle = std::sin(angle);

            // Translate point to origin
            for (int i = 0; i < dim; ++i) {
                pos[i] -= m_center[i];
            }

            // Apply rotation
            std::array<Scalar, dim> result;
            result[0] =
                pos[0] * (cos_angle + normalized_axis[0] * normalized_axis[0] * (1 - cos_angle)) +
                pos[1] * (normalized_axis[0] * normalized_axis[1] * (1 - cos_angle) -
                          normalized_axis[2] * sin_angle) +
                pos[2] * (normalized_axis[0] * normalized_axis[2] * (1 - cos_angle) +
                          normalized_axis[1] * sin_angle);

            result[1] =
                pos[0] * (normalized_axis[1] * normalized_axis[0] * (1 - cos_angle) +
                          normalized_axis[2] * sin_angle) +
                pos[1] * (cos_angle + normalized_axis[1] * normalized_axis[1] * (1 - cos_angle)) +
                pos[2] * (normalized_axis[1] * normalized_axis[2] * (1 - cos_angle) -
                          normalized_axis[0] * sin_angle);

            result[2] =
                pos[0] * (normalized_axis[2] * normalized_axis[0] * (1 - cos_angle) -
                          normalized_axis[1] * sin_angle) +
                pos[1] * (normalized_axis[2] * normalized_axis[1] * (1 - cos_angle) +
                          normalized_axis[0] * sin_angle) +
                pos[2] * (cos_angle + normalized_axis[2] * normalized_axis[2] * (1 - cos_angle));

            // Translate back from origin
            for (int i = 0; i < dim; ++i) {
                result[i] += m_center[i];
            }

            return result;
        } else {
            static_assert(dim == 2, "Rotation is only implemented for 2D and 3d");

            // Convert angle to radians
            Scalar angle = t * m_angle * M_PI / 180.0;

            pos[0] -= m_center[0];
            pos[1] -= m_center[1];

            std::array<Scalar, dim> result;
            result[0] = pos[0] * std::cos(angle) - pos[1] * std::sin(angle) + m_center[0];
            result[1] = pos[0] * std::sin(angle) + pos[1] * std::cos(angle) + m_center[1];

            return result;
        }
    }

    std::array<Scalar, dim> velocity(std::array<Scalar, dim> pos, Scalar t) const override
    {
        if constexpr (dim == 3) {
            // Normalize the axis
            Scalar axis_length = 0;
            for (int i = 0; i < dim; ++i) {
                axis_length += m_axis[i] * m_axis[i];
            }
            axis_length = std::sqrt(axis_length);

            std::array<Scalar, dim> normalized_axis;
            for (int i = 0; i < dim; ++i) {
                normalized_axis[i] = m_axis[i] / axis_length;
            }

            pos = transform(pos, t);
            // Translate point to origin
            for (int i = 0; i < dim; ++i) {
                pos[i] -= m_center[i];
            }

            // Cross product of axis and position gives the velocity direction
            std::array<Scalar, dim> velocity;
            velocity[0] = (normalized_axis[1] * pos[2] - normalized_axis[2] * pos[1]) * m_angle *
                          M_PI / 180.0;
            velocity[1] = (normalized_axis[2] * pos[0] - normalized_axis[0] * pos[2]) * m_angle *
                          M_PI / 180.0;
            velocity[2] = (normalized_axis[0] * pos[1] - normalized_axis[1] * pos[0]) * m_angle *
                          M_PI / 180.0;

            return velocity;
        } else {
            static_assert(dim == 2, "Rotation is only implemented for 2D and 3d");

            pos = transform(pos, t);
            pos[0] -= m_center[0];
            pos[1] -= m_center[1];

            return {
                -pos[1] * m_angle * M_PI / 180.0,
                pos[0] * m_angle * M_PI / 180.0,
            };
        }
    }

    std::array<std::array<Scalar, dim>, dim> position_Jacobian(
        std::array<Scalar, dim> /*pos*/,
        Scalar t) const override
    {
        // rotation angle (rad)
        const Scalar theta = t * m_angle * M_PI / 180.0;
        std::array<std::array<Scalar, dim>, dim> J{};

        // since theta and center do not depend on pos, the Jacobian is the
        // rotation matrix
        if constexpr (dim == 2) {
            const Scalar c = std::cos(theta);
            const Scalar s = std::sin(theta);

            J[0][0] = c;
            J[0][1] = -s;
            J[1][0] = s;
            J[1][1] = c;
        } else {
            static_assert(dim == 3, "Rotation is only implemented for 2D and 3d");
            // normalise axis
            const Scalar len =
                std::sqrt(m_axis[0] * m_axis[0] + m_axis[1] * m_axis[1] + m_axis[2] * m_axis[2]);
            const Scalar ux = m_axis[0] / len;
            const Scalar uy = m_axis[1] / len;
            const Scalar uz = m_axis[2] / len;

            const Scalar c = std::cos(theta);
            const Scalar s = std::sin(theta);
            const Scalar oc = 1 - c; // 1 - cosθ

            J[0][0] = c + ux * ux * oc;
            J[0][1] = ux * uy * oc - uz * s;
            J[0][2] = ux * uz * oc + uy * s;

            J[1][0] = uy * ux * oc + uz * s;
            J[1][1] = c + uy * uy * oc;
            J[1][2] = uy * uz * oc - ux * s;

            J[2][0] = uz * ux * oc - uy * s;
            J[2][1] = uz * uy * oc + ux * s;
            J[2][2] = c + uz * uz * oc;
        }
        return J;
    }

private:
    std::array<Scalar, dim> m_center; ///< Center point of rotation
    std::array<Scalar, dim> m_axis; ///< Rotation axis (3D only)
    Scalar m_angle; ///< Total rotation angle in degrees
};

} // namespace stf
