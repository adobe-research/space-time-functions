#pragma once

#include <stf/common.h>
#include <stf/primitives/implicit_function.h>

#include <cmath>
#include <stdexcept>

namespace stf {

/**
 * Enumeration of different soft union blending functions.
 *
 * We support four types of soft union blending functions from the Clamped Difference (CD) family.
 * Please refer to https://iquilezles.org/articles/smin/ for detailed explanations.
 *
 * In summary, `Circular` provides the "roundest" blending, followed by "Qradratic", "Quartic" and
 * "Cubic" in decreasing order of "roundness".
 */
enum class BlendingFunction {
    /**
     * Quadratic blending function.
     */
    Quadratic,

    /**
     * Quartic blending function.
     */
    Quartic,

    /**
     * Cubic blending function.
     */
    Cubic,

    /**
     * Circular blending function.
     */
    Circular
};

/**
 * @brief Implicit function representing the union of two implicit functions.
 *
 * This class implements an implicit function that combines two implicit functions
 * using a smooth union operation. The smoothness of the union can be controlled
 * using the smooth_distance parameter.
 *
 * @tparam dim The dimension of the space (2 for 2D, 3 for 3D)
 */
template <int dim, BlendingFunction UnionType = BlendingFunction::Quadratic>
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
            if constexpr (UnionType == BlendingFunction::Quadratic) {
                Scalar k = m_smooth_distance * 4.0;
                Scalar h = std::max(k - abs(a - b), 0.0) / k;
                return std::min(a, b) - h * h * k * (1.0 / 4.0);
            } else if constexpr (UnionType == BlendingFunction::Cubic) {
                Scalar k = m_smooth_distance * 6.0;
                Scalar h = std::max(k - abs(a - b), 0.0) / k;
                return std::min(a, b) - h * h * h * k * (1.0 / 6.0);
            } else if constexpr (UnionType == BlendingFunction::Quartic) {
                Scalar k = m_smooth_distance * 16.0 / 3.0;
                Scalar h = std::max(k - abs(a - b), 0.0) / k;
                return std::min(a, b) - h * h * h * (4.0 - h) * k * (1.0 / 16.0);
            } else if constexpr (UnionType == BlendingFunction::Circular) {
                Scalar k = m_smooth_distance * 1.0 / (1.0 - sqrt(0.5));
                Scalar h = std::max(k - abs(a - b), 0.0) / k;
                return std::min(a, b) - k * 0.5 * (1.0 + h - sqrt(1.0 - h * (h - 2.0)));
            } else {
                static_assert(always_false<bool>, "Unsupported BlendingFunction");
            }
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
            if constexpr (UnionType == BlendingFunction::Quadratic) {
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
                Scalar coeff = -h * sign / 2;

                std::array<Scalar, dim> grad;
                for (int i = 0; i < dim; ++i) {
                    Scalar dmin = (a_is_smaller) ? grad_a[i] : grad_b[i];
                    grad[i] = dmin - coeff * (grad_a[i] - grad_b[i]);
                }

                return grad;
            } else if constexpr (UnionType == BlendingFunction::Cubic) {
                Scalar k = m_smooth_distance * 6.0;
                Scalar diff = a - b;
                Scalar abs_diff = std::abs(diff);
                bool a_is_smaller = (a < b);

                if (abs_diff >= k) {
                    // No blending region; just take min
                    return (a < b) ? grad_a : grad_b;
                }

                Scalar h = (k - abs_diff) / k;
                Scalar sign = (a_is_smaller) ? -1.0 : 1.0;
                Scalar coeff = -h * h * sign / 2;

                std::array<Scalar, dim> grad;
                for (int i = 0; i < dim; ++i) {
                    Scalar dmin = (a_is_smaller) ? grad_a[i] : grad_b[i];
                    grad[i] = dmin - coeff * (grad_a[i] - grad_b[i]);
                }

                return grad;
            } else if constexpr (UnionType == BlendingFunction::Quartic) {
                Scalar k = m_smooth_distance * 16.0 / 3.0;
                Scalar diff = a - b;
                Scalar abs_diff = std::abs(diff);
                bool a_is_smaller = (a < b);

                if (abs_diff >= k) {
                    // No blending region; just take min
                    return (a < b) ? grad_a : grad_b;
                }

                Scalar h = (k - abs_diff) / k;
                Scalar sign = (a_is_smaller) ? -1.0 : 1.0;
                Scalar coeff = -(3.0 / 16.0 * h * h * (4 - h) - h * h * h / 16.0) * sign;

                std::array<Scalar, dim> grad;
                for (int i = 0; i < dim; ++i) {
                    Scalar dmin = (a_is_smaller) ? grad_a[i] : grad_b[i];
                    grad[i] = dmin - coeff * (grad_a[i] - grad_b[i]);
                }

                return grad;
            } else if constexpr (UnionType == BlendingFunction::Circular) {
                Scalar k = m_smooth_distance * 1.0 / (1.0 - sqrt(0.5));
                Scalar diff = a - b;
                Scalar abs_diff = std::abs(diff);
                bool a_is_smaller = (a < b);

                if (abs_diff >= k) {
                    // No blending region; just take min
                    return (a < b) ? grad_a : grad_b;
                }

                Scalar h = (k - abs_diff) / k;
                Scalar sign = (a_is_smaller) ? -1.0 : 1.0;
                Scalar coeff = -0.5 * (1 + (h - 1) / sqrt(1 - h * (h - 2))) * sign;

                std::array<Scalar, dim> grad;
                for (int i = 0; i < dim; ++i) {
                    Scalar dmin = (a_is_smaller) ? grad_a[i] : grad_b[i];
                    grad[i] = dmin - coeff * (grad_a[i] - grad_b[i]);
                }

                return grad;
            } else {
                static_assert(always_false<bool>, "Unsupported BlendingFunction");
            }
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
