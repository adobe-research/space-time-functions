#pragma once

#include <stf/common.h>
#include <stf/space_time_function.h>

#include <array>

namespace stf {

/**
 * @brief A class that linearly interpolates two space-time functions at each time point.
 *
 * This class creates a new space-time function by linearly interpolating between two input
 * functions f1 and f2 at each time point. At t=0, the result is f1, and at t=1, the result is f2.
 * For values of t between 0 and 1, the result is a linear interpolation of the two functions.
 *
 * @tparam dim The dimensionality of the space-time function
 */
template <int dim>
class InterpolateFunction : public SpaceTimeFunction<dim>
{
public:
    /**
     * @brief Construct a new Interpolate Function object
     *
     * @param f1 The first space-time function (used at t=0)
     * @param f2 The second space-time function (used at t=1)
     */
    InterpolateFunction(SpaceTimeFunction<dim>& f1, SpaceTimeFunction<dim>& f2)
        : m_f1(f1)
        , m_f2(f2)
    {}

    /**
     * @brief Compute the interpolated value at the given position and time
     *
     * @param pos The spatial position
     * @param t The time parameter (0 to 1)
     * @return Scalar The interpolated value: f1(pos,t) * (1-t) + f2(pos,t) * t
     */
    Scalar value(std::array<Scalar, dim> pos, Scalar t) const override
    {
        return m_f1.value(pos, t) * (1 - t) + m_f2.value(pos, t) * t;
    }

    /**
     * @brief Compute the time derivative of the interpolated function
     *
     * @param pos The spatial position
     * @param t The time parameter (0 to 1)
     * @return Scalar The interpolated time derivative: f1'(pos,t) * (1-t) + f2'(pos,t) * t
     */
    Scalar time_derivative(std::array<Scalar, dim> pos, Scalar t) const override
    {
        // The time derivative of the interpolated function is computed using the product rule:
        // d/dt [f1(pos,t) * (1-t) + f2(pos,t) * t] =
        //     f1'(pos,t) * (1-t) + f2'(pos,t) * t - f1(pos,t) + f2(pos,t)
        return m_f1.time_derivative(pos, t) * (1 - t) + m_f2.time_derivative(pos, t) * t -
               m_f1.value(pos, t) + m_f2.value(pos, t);
    }

    /**
     * @brief Compute the gradient of the interpolated function
     *
     * @param pos The spatial position
     * @param t The time parameter (0 to 1)
     * @return std::array<Scalar, dim + 1> The interpolated gradient vector
     */
    std::array<Scalar, dim + 1> gradient(std::array<Scalar, dim> pos, Scalar t) const override
    {
        std::array<Scalar, dim + 1> grad_f1 = m_f1.gradient(pos, t);
        std::array<Scalar, dim + 1> grad_f2 = m_f2.gradient(pos, t);

        for (int i = 0; i < dim; ++i) {
            grad_f1[i] = grad_f1[i] * (1 - t) + grad_f2[i] * t;
        }
        grad_f1[dim] = time_derivative(pos, t);

        return grad_f1;
    }

private:
    SpaceTimeFunction<dim>& m_f1; ///< The first function (used at t=0)
    SpaceTimeFunction<dim>& m_f2; ///< The second function (used at t=1)
};

} // namespace stf
