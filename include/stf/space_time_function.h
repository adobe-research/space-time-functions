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
class SpaceTimeFunction {
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
    virtual Scalar time_derivative(std::array<Scalar, dim> pos,
                                   Scalar t) const = 0;

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
    virtual std::array<Scalar, dim + 1> gradient(std::array<Scalar, dim> pos,
                                                 Scalar t) const = 0;
};

}  // namespace stf
