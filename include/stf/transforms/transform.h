#pragma once

#include <stf/common.h>

#include <array>
#include <span>

namespace stf {

/**
 * @brief Base class for geometric transformations in n-dimensional space.
 *
 * This abstract class defines the interface for geometric transformations that
 * can be applied to points in n-dimensional space. It provides methods for both
 * position transformation and velocity calculation.
 *
 * @tparam dim The dimensionality of the space (2D or 3D)
 */
template <int dim>
class Transform
{
public:
    virtual ~Transform() = default;

public:
    /**
     * @brief Transforms a point in space according to the transformation rules.
     *
     * @param pos The input position to transform
     * @param t The time parameter for time-dependent transformations
     * @return std::array<Scalar, dim> The transformed position
     */
    virtual std::array<Scalar, dim> transform(std::array<Scalar, dim> pos, Scalar t) const = 0;

    /**
     * @brief Calculates the velocity of a point under the transformation.
     *
     * @param pos The position at which to calculate the velocity
     * @param t The time parameter for time-dependent transformations
     * @return std::array<Scalar, dim> The velocity vector
     */
    virtual std::array<Scalar, dim> velocity(std::array<Scalar, dim> pos, Scalar t) const = 0;

    /**
     * @brief Calculates the Jacobian matrix of the transformation with respect
     * to position.
     *
     * This method computes the partial derivatives of the transformation with
     * respect to each component of the input position. The Jacobian matrix
     * represents how small changes in the input position affect the transformed
     * position.
     *
     * @param pos The position at which to calculate the Jacobian
     * @param t The time parameter for time-dependent transformations
     * @return std::array<std::array<Scalar, dim>, dim> The Jacobian matrix
     */
    virtual std::array<std::array<Scalar, dim>, dim> position_Jacobian(
        std::array<Scalar, dim> pos,
        Scalar t) const = 0;

    /**
     * @brief Calculates velocity using finite difference approximation.
     *
     * This is a helper method that computes velocity using central difference
     * approximation. It's used for verification purposes.
     *
     * @param pos The position at which to calculate the velocity
     * @param t The time parameter
     * @param delta The small perturbation used for finite difference
     * @return std::array<Scalar, dim> The approximated velocity vector
     */
    std::array<Scalar, dim>
    finite_difference_velocity(std::array<Scalar, dim> pos, Scalar t, Scalar delta = 1e-6) const
    {
        // Using finite difference to calculate the velocity
        auto value_prev = transform(pos, t - delta);
        auto value_next = transform(pos, t + delta);
        std::array<Scalar, dim> velocity;
        for (int i = 0; i < dim; ++i) {
            velocity[i] = (value_next[i] - value_prev[i]) / (2 * delta);
        }
        return velocity;
    }

    /**
     * @brief Calculates the Jacobian matrix using finite difference
     *
     * This method computes the Jacobian matrix using finite difference
     * approximation. It calculates the partial derivatives of the transformation
     * with respect to each component of the input position.
     *
     * @param pos The position at which to calculate the Jacobian
     * @param t The time parameter
     * @return std::array<std::array<Scalar, dim>, dim> The Jacobian matrix
     */
    std::array<std::array<Scalar, dim>, dim> finite_difference_Jacobian(
        std::array<Scalar, dim> pos,
        Scalar t) const
    {
        constexpr Scalar eps = 1e-6;
        std::array<std::array<Scalar, dim>, dim> J{};

        // For each dimension i, compute partial derivative with respect to
        // pos[i]
        for (int i = 0; i < dim; ++i) {
            // Forward point
            auto pos_plus = pos;
            pos_plus[i] += eps;
            auto val_plus = transform(pos_plus, t);

            // Backward point
            auto pos_minus = pos;
            pos_minus[i] -= eps;
            auto val_minus = transform(pos_minus, t);

            // Central difference
            for (int j = 0; j < dim; ++j) {
                J[j][i] = (val_plus[j] - val_minus[j]) / (2 * eps);
            }
        }
        return J;
    }
};

} // namespace stf
