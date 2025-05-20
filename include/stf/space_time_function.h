#pragma once

#include <stf/common.h>

#include <array>

namespace stf {

/**
 * @brief Abstract base class for space-time functions
 *
 * This class defines the interface for functions that depend on both space and
 * time. It provides methods to evaluate the function value, its time
 * derivative, and gradient.
 *
 * @tparam dim The spatial dimension of the function
 */
template <int dim>
class SpaceTimeFunction
{
public:
    virtual ~SpaceTimeFunction() = default;

    /**
     * @brief Evaluate the function at a given position and time
     *
     * @param pos The spatial position as an array of coordinates
     * @param t The time value
     * @return Scalar The function value at the given position and time
     */
    virtual Scalar value(std::array<Scalar, dim> pos, Scalar t) const = 0;

    /**
     * @brief Compute the time derivative of the function
     *
     * @param pos The spatial position as an array of coordinates
     * @param t The time value
     * @return Scalar The time derivative at the given position and time
     */
    virtual Scalar time_derivative(std::array<Scalar, dim> pos, Scalar t) const = 0;

    /**
     * @brief Compute the gradient of the function with respect to both space
     * and time
     *
     * The gradient is returned as an array of size dim+1, where the first dim
     * elements represent the spatial gradient and the last element represents
     * the time derivative.
     *
     * @param pos The spatial position as an array of coordinates
     * @param t The time value
     * @return std::array<Scalar, dim + 1> The gradient vector
     */
    virtual std::array<Scalar, dim + 1> gradient(std::array<Scalar, dim> pos, Scalar t) const = 0;

public:
    /**
     * @brief Compute the gradient using finite differences
     *
     * This method computes the gradient of the function using finite
     * differences. It is useful for debugging and testing purposes.
     *
     * @param pos The spatial position as an array of coordinates
     * @param t The time value
     * @param delta The finite difference step size (default is 1e-6)
     * @return std::array<Scalar, dim + 1> The gradient vector
     */
    std::array<Scalar, dim + 1>
    finite_difference_gradient(std::array<Scalar, dim> pos, Scalar t, Scalar delta = 1e-6) const
    {
        std::array<Scalar, dim + 1> grad;
        for (int i = 0; i < dim; ++i) {
            std::array<Scalar, dim> pos_plus = pos;
            std::array<Scalar, dim> pos_minus = pos;
            pos_plus[i] += delta;
            pos_minus[i] -= delta;
            grad[i] = (value(pos_plus, t) - value(pos_minus, t)) / (2 * delta);
        }

        Scalar time_plus = value(pos, t + delta);
        Scalar time_minus = value(pos, t - delta);
        grad[dim] = (time_plus - time_minus) / (2 * delta);
        return grad;
    }
};

} // namespace stf
