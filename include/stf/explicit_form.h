#pragma once

#include <stf/common.h>
#include <stf/space_time_function.h>

#include <array>
#include <functional>

namespace stf {

/**
 * @brief Concrete implementation of SpaceTimeFunction using explicit function
 * definitions
 *
 * This class allows creating space-time functions from explicit function
 * definitions for the value, time derivative, and gradient. If the time
 * derivative or gradient are not provided, they are computed using finite
 * differences.
 *
 * @tparam dim The spatial dimension of the function
 */
template <int dim>
class ExplicitForm : public SpaceTimeFunction<dim>
{
public:
    /**
     * @brief Construct a new ExplicitForm object
     *
     * @param func The function defining the value
     * @param time_derivative Optional function defining the time derivative
     * @param gradient Optional function defining the gradient
     */
    ExplicitForm(
        std::function<Scalar(std::array<Scalar, dim>, Scalar)> func,
        std::function<Scalar(std::array<Scalar, dim>, Scalar)> time_derivative = nullptr,
        std::function<std::array<Scalar, dim + 1>(std::array<Scalar, dim>, Scalar)> gradient =
            nullptr)
        : m_function(func)
        , m_time_derivative(time_derivative)
        , m_gradient(gradient)
    {
        assert(m_function != nullptr);
    }

    /**
     * @brief Evaluate the function at a given position and time
     *
     * Evaluates the function using the provided function object. This is a
     * direct evaluation of the function without any numerical approximation.
     *
     * @param pos The spatial position as an array of coordinates
     * @param t The time value
     * @return Scalar The function value at the given position and time
     */
    virtual Scalar value(std::array<Scalar, dim> pos, Scalar t) const override
    {
        return m_function(pos, t);
    }

    /**
     * @brief Compute the time derivative of the function
     *
     * If a time derivative function was provided during construction, it is
     * used directly. Otherwise, the time derivative is approximated using a
     * forward finite difference with a small time step (1e-6).
     *
     * @param pos The spatial position as an array of coordinates
     * @param t The time value
     * @return Scalar The time derivative at the given position and time
     */
    virtual Scalar time_derivative(std::array<Scalar, dim> pos, Scalar t) const override
    {
        if (m_time_derivative == nullptr) {
            // Finite difference
            auto delta_t = 1e-6;
            auto value1 = m_function(pos, t);
            auto value2 = m_function(pos, t + delta_t);
            return (value2 - value1) / delta_t;
        } else {
            return m_time_derivative(pos, t);
        }
    }

    /**
     * @brief Compute the gradient of the function
     *
     * If a gradient function was provided during construction, it is used
     * directly. Otherwise, the gradient is approximated using forward finite
     * differences with a small step size (1e-6) for each spatial dimension. The
     * time component of the gradient is computed using the time_derivative
     * method.
     *
     * @param pos The spatial position as an array of coordinates
     * @param t The time value
     * @return std::array<Scalar, dim + 1> The gradient vector, where the first
     * dim elements represent the spatial gradient and the last element
     * represents the time derivative
     */
    virtual std::array<Scalar, dim + 1> gradient(std::array<Scalar, dim> pos, Scalar t)
        const override
    {
        if (m_gradient == nullptr) {
            // Finite difference
            auto delta = 1e-6;
            std::array<Scalar, dim + 1> gradient;
            for (int i = 0; i < dim; ++i) {
                auto pos_delta = pos;
                pos_delta[i] += delta;
                auto value1 = m_function(pos, t);
                auto value2 = m_function(pos_delta, t);
                gradient[i] = (value2 - value1) / delta;
            }
            gradient[dim] = time_derivative(pos, t);
            return gradient;
        } else {
            return m_gradient(pos, t);
        }
    }

private:
    std::function<Scalar(std::array<Scalar, dim>, Scalar)>
        m_function; ///< The function defining the value
    std::function<Scalar(std::array<Scalar, dim>, Scalar)>
        m_time_derivative; ///< Optional function defining the time derivative
    std::function<std::array<Scalar, dim + 1>(std::array<Scalar, dim>, Scalar)>
        m_gradient; ///< Optional function defining the gradient
};

} // namespace stf
