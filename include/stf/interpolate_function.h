#pragma once

#include <stf/common.h>
#include <stf/space_time_function.h>

#include <array>
#include <functional>

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
     * @param interpolation_func The interpolation function (default is linear)
     * @param interpolation_derivative The derivative of the interpolation function (default is 1)
     */
    InterpolateFunction(
        SpaceTimeFunction<dim>& f1,
        SpaceTimeFunction<dim>& f2,
        std::function<Scalar(Scalar)> interpolation_func = [](Scalar t) { return t; },
        std::function<Scalar(Scalar)> interpolation_derivative = [](Scalar t) { return 1; })
        : m_f1(f1)
        , m_f2(f2)
        , m_interpolation_func(interpolation_func)
        , m_interpolation_derivative(interpolation_derivative)
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
        Scalar s = m_interpolation_func(t);
        return m_f1.value(pos, t) * (1 - s) + m_f2.value(pos, t) * s;
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
        Scalar s = m_interpolation_func(t);
        Scalar ds_dt = m_interpolation_derivative(t);
        // The time derivative of the interpolated function is computed using the product rule:
        // d/dt [f1(pos,t) * (1-s) + f2(pos,t) * s] =
        //     f1'(pos,t) * (1-s) + f2'(pos,t) * s - f1(pos,t) ds/dt + f2(pos,t) ds/dt
        return m_f1.time_derivative(pos, t) * (1 - s) + m_f2.time_derivative(pos, t) * s -
               m_f1.value(pos, t) * ds_dt + m_f2.value(pos, t) * ds_dt;
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

        Scalar s = m_interpolation_func(t);

        for (int i = 0; i < dim; ++i) {
            grad_f1[i] = grad_f1[i] * (1 - s) + grad_f2[i] * s;
        }
        grad_f1[dim] = time_derivative(pos, t);

        return grad_f1;
    }

private:
    SpaceTimeFunction<dim>& m_f1; ///< The first function (used at t=0)
    SpaceTimeFunction<dim>& m_f2; ///< The second function (used at t=1)

    ///< The interpolation function
    std::function<Scalar(Scalar)> m_interpolation_func;

    /// The derivative of the interpolation function
    std::function<Scalar(Scalar)> m_interpolation_derivative;
};

} // namespace stf
