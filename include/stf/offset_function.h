#pragma once

#include <stf/common.h>
#include <stf/space_time_function.h>

#include <array>
#include <functional>

namespace stf {

/**
 * @brief A space-time function that adds a time-dependent offset to another space-time function.
 *
 * This class wraps a space-time function and adds a time-dependent offset to its values.
 * The offset and its time derivative can be specified through function objects.
 *
 * @tparam dim The spatial dimension of the function
 */
template <int dim>
class OffsetFunction : public SpaceTimeFunction<dim>
{
public:
    /**
     * @brief Constructs an OffsetFunction with the given base function and offset functions.
     *
     * @param f The base space-time function to be offset
     * @param offset_func Function that computes the time-dependent offset value
     * @param offset_derivative Function that computes the time derivative of the offset
     */
    OffsetFunction(
        SpaceTimeFunction<dim>& f,
        std::function<Scalar(Scalar)> offset_func = [](Scalar t) { return 0; },
        std::function<Scalar(Scalar)> offset_derivative = [](Scalar t) { return 0; })
        : m_f(f)
        , m_offset_func(offset_func)
        , m_offset_derivative(offset_derivative)
    {}

    /**
     * @brief Evaluates the function value at the given position and time.
     *
     * @param pos The spatial position
     * @param t The time
     * @return The sum of the base function value and the offset at time t
     */
    Scalar value(std::array<Scalar, dim> pos, Scalar t) const override
    {
        return m_f.value(pos, t) + m_offset_func(t);
    }

    /**
     * @brief Computes the time derivative of the function at the given position and time.
     *
     * @param pos The spatial position
     * @param t The time
     * @return The sum of the base function's time derivative and the offset's time derivative
     */
    Scalar time_derivative(std::array<Scalar, dim> pos, Scalar t) const override
    {
        return m_f.time_derivative(pos, t) + m_offset_derivative(t);
    }

    /**
     * @brief Computes the gradient of the function at the given position and time.
     *
     * The gradient includes both spatial derivatives and the time derivative.
     * The time derivative component is modified by adding the offset's time derivative.
     *
     * @param pos The spatial position
     * @param t The time
     * @return The gradient vector, where the last component is the time derivative
     */
    std::array<Scalar, dim + 1> gradient(std::array<Scalar, dim> pos, Scalar t) const override
    {
        auto grad = m_f.gradient(pos, t);
        grad[dim] += m_offset_derivative(t);
        return grad;
    }

private:
    SpaceTimeFunction<dim>& m_f; ///< Reference to the base space-time function
    std::function<Scalar(Scalar)> m_offset_func; ///< Function computing the time-dependent offset
    std::function<Scalar(Scalar)>
        m_offset_derivative; ///< Function computing the offset's time derivative
};

} // namespace stf

