#pragma once

#include <stf/common.h>
#include <stf/transforms/transform.h>

#include <array>
#include <span>

namespace stf {

/**
 * @brief Composes two transformations by applying them in sequence.
 *
 * This class combines two transformations by applying them one after another.
 * The first transformation is applied to the input position, and then the
 * second transformation is applied to the result.
 *
 * @tparam dim The dimensionality of the space (2D or 3D)
 */
template <int dim>
class Compose : public Transform<dim>
{
public:
    /**
     * @brief Constructs a composition of two transformations.
     *
     * @param transform1 The first transformation to apply
     * @param transform2 The second transformation to apply
     */
    Compose(Transform<dim>& transform1, Transform<dim>& transform2)
        : m_transform1(transform1)
        , m_transform2(transform2)
    {}

    std::array<Scalar, dim> transform(std::array<Scalar, dim> pos, Scalar t) const override
    {
        auto intermediate = m_transform1.transform(pos, t);
        return m_transform2.transform(intermediate, t);
    }

    std::array<Scalar, dim> velocity(std::array<Scalar, dim> pos, Scalar t) const override
    {
        auto intermediate = m_transform1.transform(pos, t);
        const auto v1 = m_transform1.velocity(pos, t);
        const auto v2 = m_transform2.velocity(intermediate, t);
        const auto J2 = m_transform2.position_Jacobian(intermediate, t);

        // result = v2 + J2 * v1
        std::array<Scalar, dim> result;

        if constexpr (dim == 3) {
            result = {
                v2[0] + J2[0][0] * v1[0] + J2[0][1] * v1[1] + J2[0][2] * v1[2],
                v2[1] + J2[1][0] * v1[0] + J2[1][1] * v1[1] + J2[1][2] * v1[2],
                v2[2] + J2[2][0] * v1[0] + J2[2][1] * v1[1] + J2[2][2] * v1[2],
            };
        } else {
            static_assert(dim == 2, "Composite transform is only implemented for 2D and 3D");
            result = {
                v2[0] + J2[0][0] * v1[0] + J2[0][1] * v1[1],
                v2[1] + J2[1][0] * v1[0] + J2[1][1] * v1[1],
            };
        }

        return result;
    }

    std::array<std::array<Scalar, dim>, dim> position_Jacobian(
        std::array<Scalar, dim> pos,
        Scalar t) const override
    {
        // 1) evaluate first transform and both Jacobians
        const auto intermediate = m_transform1.transform(pos, t);
        const auto J1 = m_transform1.position_Jacobian(pos, t);
        const auto J2 = m_transform2.position_Jacobian(intermediate, t);

        // 2) matrix product  J = J2 * J1
        std::array<std::array<Scalar, dim>, dim> J{};
        for (int i = 0; i < dim; ++i)
            for (int j = 0; j < dim; ++j) {
                Scalar sum = 0;
                for (int k = 0; k < dim; ++k) sum += J2[i][k] * J1[k][j];
                J[i][j] = sum;
            }
        return J;
    }

private:
    Transform<dim>& m_transform1; ///< First transformation
    Transform<dim>& m_transform2; ///< Second transformation
};

} // namespace stf
