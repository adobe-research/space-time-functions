#pragma once

#include <stf/common.h>
#include <stf/transforms/transform.h>

#include <array>
#include <span>

namespace stf {

/**
 * @brief A translation transformation that moves points along a constant
 * vector.
 *
 * This class implements a translation transformation where points are moved
 * along a constant vector scaled by time.
 *
 * @tparam dim The dimensionality of the space (2D or 3D)
 */
template <int dim>
class Translation : public Transform<dim>
{
public:
    /**
     * @brief Constructs a translation transformation.
     *
     * @param translation The translation vector that defines the direction and
     * magnitude of the translation
     */
    Translation(std::array<Scalar, dim> translation)
        : m_translation(translation)
    {}

    std::array<Scalar, dim> transform(std::array<Scalar, dim> pos, Scalar t) const override
    {
        for (int i = 0; i < dim; ++i) {
            pos[i] += m_translation[i] * t;
        }
        return pos;
    }

    std::array<Scalar, dim> velocity(std::array<Scalar, dim> pos, Scalar t) const override
    {
        return m_translation;
    }

    std::array<std::array<Scalar, dim>, dim> position_Jacobian(
        std::array<Scalar, dim> pos,
        Scalar t) const override
    {
        std::array<std::array<Scalar, dim>, dim> jacobian{};
        // For translation, the Jacobian is the identity matrix
        for (int i = 0; i < dim; ++i) {
            jacobian[i][i] = 1;
        }
        return jacobian;
    }

private:
    std::array<Scalar, dim> m_translation;
};

} // namespace stf
