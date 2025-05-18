#pragma once

#include <stf/common.h>
#include <stf/space_time_function.h>

#include <array>
#include <stdexcept>

namespace stf {

/**
 * @brief A class representing the union of two space-time functions.
 * 
 * This class implements the union operation between two space-time functions,
 * which can be either a sharp union (when smooth_distance = 0) or a smooth union
 * (when smooth_distance > 0). The smooth union provides a continuous transition
 * between the two functions.
 * 
 * @tparam dim The dimension of the space (2 or 3)
 */
template <int dim>
class UnionFunction : public SpaceTimeFunction<dim>
{
public:
    /**
     * @brief Constructs a UnionFunction from two space-time functions.
     * 
     * @param f1 The first space-time function
     * @param f2 The second space-time function
     * @param smooth_distance The distance over which to smooth the union. 
     *                       If 0, performs a sharp union (min operation).
     *                       If > 0, performs a smooth union over this distance.
     */
    UnionFunction(
        SpaceTimeFunction<dim>& f1,
        SpaceTimeFunction<dim>& f2,
        Scalar smooth_distance = 0)
        : m_f1(f1)
        , m_f2(f2)
        , m_smooth_distance(smooth_distance)
    {
        if (smooth_distance < 0) {
            throw std::invalid_argument("smooth_distance must be non-negative");
        }
    }

    /**
     * @brief Evaluates the union function at a given position and time.
     * 
     * For smooth_distance = 0, returns min(f1, f2).
     * For smooth_distance > 0, returns a smooth interpolation between f1 and f2.
     * 
     * @param pos The spatial position to evaluate at
     * @param t The time to evaluate at
     * @return The value of the union function
     */
    Scalar value(std::array<Scalar, dim> pos, Scalar t) const override
    {
        Scalar a = m_f1.value(pos, t);
        Scalar b = m_f2.value(pos, t);

        if (m_smooth_distance > 0) {
            Scalar k = m_smooth_distance * 4.0;
            Scalar h = std::max(k - std::abs(a - b), 0.0) / k;
            return std::min(a, b) - h * h * k * (1.0 / 4.0);
        } else {
            return std::min(a, b);
        }
    }

    /**
     * @brief Computes the time derivative of the union function.
     * 
     * For smooth_distance = 0, returns the derivative of the minimum function.
     * For smooth_distance > 0, returns the derivative of the smooth interpolation.
     * 
     * @param pos The spatial position to evaluate at
     * @param t The time to evaluate at
     * @return The time derivative of the union function
     */
    Scalar time_derivative(std::array<Scalar, dim> pos, Scalar t) const override
    {
        Scalar a = m_f1.value(pos, t);
        Scalar b = m_f2.value(pos, t);
        Scalar da = m_f1.time_derivative(pos, t);
        Scalar db = m_f2.time_derivative(pos, t);

        if (m_smooth_distance > 0) {
            Scalar k = m_smooth_distance * 4.0;
            Scalar diff = a - b;
            Scalar abs_diff = std::abs(diff);
            bool a_is_smaller = (a < b);

            if (abs_diff >= k) {
                // Outside smoothing zone
                return (a < b) ? da : db;
            } else {
                // Inside smoothing zone
                // Compute dh/dpos = -(1/k) * sign(a - b) * (grad_a - grad_b)
                Scalar h = (k - abs_diff) / k;
                Scalar sign = (a_is_smaller) ? -1.0 : 1.0;
                Scalar coeff = - h * sign / 2;

                return (a_is_smaller ? da : db) - coeff * (da - db);
            }
        } else {
            if (a < b)
                return da;
            else if (b < a)
                return db;
            else {
                return (da + db) / 2;
            }
        }
    }

    /**
     * @brief Computes the gradient of the union function.
     * 
     * Returns the spatial and temporal gradients of the union function.
     * For smooth_distance = 0, returns the gradient of the minimum function.
     * For smooth_distance > 0, returns the gradient of the smooth interpolation.
     * 
     * @param pos The spatial position to evaluate at
     * @param t The time to evaluate at
     * @return An array containing the spatial gradients followed by the time derivative
     */
    std::array<Scalar, dim + 1> gradient(std::array<Scalar, dim> pos, Scalar t) const override
    {
        Scalar a = m_f1.value(pos, t);
        Scalar b = m_f2.value(pos, t);
        std::array<Scalar, dim + 1> grad_a = m_f1.gradient(pos, t);
        std::array<Scalar, dim + 1> grad_b = m_f2.gradient(pos, t);

        if (m_smooth_distance > 0) {
            Scalar k = m_smooth_distance * 4.0;
            Scalar diff = a - b;
            Scalar abs_diff = std::abs(diff);
            bool a_is_smaller = (a < b);

            if (abs_diff >= k) {
                // Outside smoothing zone
                return (a_is_smaller) ? grad_a : grad_b;
            } else {
                // Inside smoothing zone
                // Compute dh/dpos = -(1/k) * sign(a - b) * (grad_a - grad_b)
                Scalar h = (k - abs_diff) / k;
                Scalar sign = (a_is_smaller) ? -1.0 : 1.0;
                Scalar coeff = - h * sign / 2;

                std::array<Scalar, dim + 1> grad_result;
                for (int i = 0; i <= dim; ++i) {
                    Scalar dmin = (a_is_smaller) ? grad_a[i] : grad_b[i];
                    grad_result[i] = dmin - coeff * (grad_a[i] - grad_b[i]);
                }
                return grad_result;
            }
        } else {
            if (a < b)
                return grad_a;
            else if (b < a)
                return grad_b;
            else {
                if constexpr (dim == 3) {
                    return {
                        (grad_a[0] + grad_b[0]) / 2,
                        (grad_a[1] + grad_b[1]) / 2,
                        (grad_a[2] + grad_b[2]) / 2,
                        (grad_a[3] + grad_b[3]) / 2,
                    };
                } else {
                    static_assert(dim == 2, "Only 2D and 3D are supported");
                    return {
                        (grad_a[0] + grad_b[0]) / 2,
                        (grad_a[1] + grad_b[1]) / 2,
                        (grad_a[2] + grad_b[2]) / 2,
                    };
                }
            }
        }
    }

private:
    SpaceTimeFunction<dim>& m_f1;
    SpaceTimeFunction<dim>& m_f2;
    Scalar m_smooth_distance = 0;
};

} // namespace stf
