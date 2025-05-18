#pragma once

#include <stf/common.h>
#include <stf/primitives/implicit_function.h>

#include <cmath>

namespace stf {

/**
 * @brief Implicit function representing the union of two implicit functions.
 *
 * This class implements an implicit function that combines two implicit functions
 * using a smooth union operation. The smoothness of the union can be controlled
 * using the smooth_distance parameter.
 *
 * @tparam dim The dimension of the space (2 for 2D, 3 for 3D)
 */
template <int dim>
class ImplicitUnion : public ImplicitFunction<dim>
{
public:
    /**
     * @brief Constructs a new implicit union.
     *
     * @param f1 The first implicit function
     * @param f2 The second implicit function
     * @param smooth_distance The distance over which to smooth the union (0 for no smoothing)
     */
    ImplicitUnion(ImplicitFunction<dim>& f1, ImplicitFunction<dim>& f2, Scalar smooth_distance = 0)
        : m_f1(f1)
        , m_f2(f2)
        , m_smooth_distance(smooth_distance)
    {}

    /**
     * @brief Evaluates the implicit function at a given position.
     *
     * Computes the smooth union of the two implicit functions. If smooth_distance
     * is 0, this is equivalent to taking the minimum of the two functions.
     * Otherwise, the transition between the two functions is smoothed over the
     * specified distance.
     *
     * @param pos The position to evaluate at
     * @return Scalar The signed distance to the surface
     */
    Scalar value(std::array<Scalar, dim> pos) const override
    {
        Scalar a = m_f1.value(pos);
        Scalar b = m_f2.value(pos);

        if (m_smooth_distance > 0) {
            Scalar k = m_smooth_distance * 4.0;
            Scalar h = std::max(k - abs(a - b), 0.0) / k;
            return std::min(a, b) - h * h * k * (1.0 / 4.0);
        } else {
            return std::min(a, b);
        }
    }

    /**
     * @brief Computes the gradient of the implicit function at a given position.
     *
     * The gradient is computed using the chain rule, taking into account the
     * gradients of both input functions and the smoothing factor.
     *
     * @param pos The position to evaluate at
     * @return std::array<Scalar, dim> The normalized gradient vector
     */
    std::array<Scalar, dim> gradient(std::array<Scalar, dim> pos) const override
    {
        Scalar a = m_f1.value(pos);
        Scalar b = m_f2.value(pos);

        std::array<Scalar, dim> grad_a = m_f1.gradient(pos);
        std::array<Scalar, dim> grad_b = m_f2.gradient(pos);

        if (m_smooth_distance > 0) {
            Scalar k = m_smooth_distance * 4.0;
            Scalar diff = a - b;
            Scalar abs_diff = std::abs(diff);
            bool a_is_smaller = (a < b);

            if (abs_diff >= k) {
                // No blending region; just take min
                return (a < b) ? grad_a : grad_b;
            }

            Scalar h = (k - abs_diff) / k;
            Scalar sign = (a_is_smaller) ? -1.0 : 1.0;
            Scalar coeff = - h * sign / 2;

            Scalar h2k = h * h * (k * 0.25); // h^2 * k/4

            std::array<Scalar, dim> grad;
            for (int i = 0; i < dim; ++i) {
                Scalar dmin = (a_is_smaller) ? grad_a[i] : grad_b[i];
                grad[i] = dmin - coeff * (grad_a[i] - grad_b[i]);
            }

            return grad;
        } else {
            // Hard union: take gradient of the smaller
            return (a < b) ? grad_a : grad_b;
        }
    }

private:
    ImplicitFunction<dim>& m_f1; ///< The first implicit function
    ImplicitFunction<dim>& m_f2; ///< The second implicit function
    Scalar m_smooth_distance = 0; ///< The distance over which to smooth the union
};

} // namespace stf
