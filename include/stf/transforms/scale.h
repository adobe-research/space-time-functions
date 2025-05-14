#pragma once

#include <stf/common.h>
#include <stf/transforms/transform.h>

#include <array>
#include <span>

namespace stf {

/**
 * @brief A scaling transformation that scales points by different factors in
 * each dimension.
 *
 * This class implements a scaling transformation where points are scaled by
 * different factors in each dimension relative to a pivot point. The scaling
 * can be uniform (same factor in all dimensions) or non-uniform (different
 * factors per dimension).
 *
 * @tparam dim The dimensionality of the space (2D or 3D)
 */
template <int dim>
class Scale : public Transform<dim>
{
public:
    /**
     * @brief Constructs a scaling transformation.
     *
     * @param factors The scaling factors for each dimension
     * @param center The pivot point around which scaling occurs (default:
     * origin)
     */
    Scale(
        std::array<Scalar, dim> factors,
        std::array<Scalar, dim> center = std::array<Scalar, dim>{0})
        : m_factors(factors)
        , m_center(center)
    {}

    std::array<Scalar, dim> transform(std::array<Scalar, dim> pos, Scalar t) const override
    {
        // Translate to origin
        for (int i = 0; i < dim; ++i) {
            pos[i] -= m_center[i];
        }

        // Apply scaling
        for (int i = 0; i < dim; ++i) {
            pos[i] *= (1.0 + (m_factors[i] - 1.0) * t);
        }

        // Translate back
        for (int i = 0; i < dim; ++i) {
            pos[i] += m_center[i];
        }

        return pos;
    }

    std::array<Scalar, dim> velocity(std::array<Scalar, dim> pos, Scalar t) const override
    {
        // Translate to origin
        for (int i = 0; i < dim; ++i) {
            pos[i] -= m_center[i];
        }

        std::array<Scalar, dim> velocity;
        for (int i = 0; i < dim; ++i) {
            velocity[i] = pos[i] * (m_factors[i] - 1.0);
        }

        return velocity;
    }

    std::array<std::array<Scalar, dim>, dim> position_Jacobian(
        std::array<Scalar, dim> /*pos*/,
        Scalar t) const override
    {
        std::array<std::array<Scalar, dim>, dim> jacobian{};
        for (int i = 0; i < dim; ++i) {
            jacobian[i][i] = 1.0 + (m_factors[i] - 1.0) * t;
        }
        return jacobian;
    }

private:
    std::array<Scalar, dim> m_factors; ///< Scaling factors for each dimension
    std::array<Scalar, dim> m_center; ///< Center point of scaling
};

} // namespace stf
