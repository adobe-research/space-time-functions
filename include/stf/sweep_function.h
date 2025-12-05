#pragma once

#include <stf/common.h>
#include <stf/primitives/implicit_function.h>
#include <stf/space_time_function.h>
#include <stf/transforms/transform.h>

#include <array>
#include <cassert>
#include <stdexcept>

namespace stf {

/**
 * @brief Space-time function created by sweeping an implicit function through
 * space
 *
 * This class represents a space-time function created by applying a
 * transformation to an implicit function. The transformation can be
 * time-dependent, allowing the implicit function to move through space.
 *
 * The swept function F(x,t) is defined as F(x,t) = f(T(x,t)), where f is the
 * implicit function and T is the transformation. The time derivative and
 * gradient are computed using the chain rule, taking into account both the
 * spatial gradient of the implicit function and the properties of the
 * transformation.
 *
 * @tparam dim The spatial dimension of the function (2 or 3)
 */
template <int dim>
class SweepFunction : public SpaceTimeFunction<dim>
{
public:
    /**
     * @brief Construct a new SweepFunction object
     *
     * Creates a new swept function by combining an implicit function with a
     * transformation. The implicit function will be evaluated at positions
     * transformed by the given transformation.
     *
     * @param implicit_function The implicit function to be swept through space
     * @param transform The transformation to apply to the implicit function
     */
    SweepFunction(ImplicitFunction<dim>& implicit_function, Transform<dim>& transform)
        : m_implicit_function(&implicit_function)
        , m_transform(&transform)
    {}

    /**
     * @brief Evaluate the swept function at a given position and time
     *
     * Computes F(x,t) = f(T(x,t)), where f is the implicit function and T is
     * the transformation. The function first transforms the input position
     * using the transformation, then evaluates the implicit function at the
     * transformed position.
     *
     * @param pos The spatial position as an array of coordinates
     * @param t The time value
     * @return Scalar The function value at the given position and time
     */
    virtual Scalar value(std::array<Scalar, dim> pos, Scalar t) const override
    {
        assert(m_implicit_function != nullptr);
        assert(m_transform != nullptr);
        auto transformed_pos = m_transform->transform(pos, t);
        return m_implicit_function->value(transformed_pos);
    }

    /**
     * @brief Compute the time derivative of the swept function
     *
     * The time derivative is computed using the chain rule: ∂F/∂t = ∇f · ∂T/∂t,
     * where ∇f is the spatial gradient of the implicit function and ∂T/∂t is
     * the velocity of the transformation. This represents how the function
     * value changes as the implicit function moves through space.
     *
     * @param pos The spatial position as an array of coordinates
     * @param t The time value
     * @return Scalar The time derivative at the given position and time
     */
    virtual Scalar time_derivative(std::array<Scalar, dim> pos, Scalar t) const override
    {
        assert(m_implicit_function != nullptr);
        assert(m_transform != nullptr);
        auto transformed_pos = m_transform->transform(pos, t);
        auto velocity = m_transform->velocity(pos, t);
        auto spacial_grad = m_implicit_function->gradient(transformed_pos);
        if constexpr (dim == 2) {
            return spacial_grad[0] * velocity[0] + spacial_grad[1] * velocity[1];
        } else if constexpr (dim == 3) {
            return spacial_grad[0] * velocity[0] + spacial_grad[1] * velocity[1] +
                   spacial_grad[2] * velocity[2];
        } else {
            throw std::runtime_error("Unsupported dimension");
        }
    }

    /**
     * @brief Compute the gradient of the swept function
     *
     * The gradient is computed using the chain rule, taking into account both
     * the spatial gradient of the implicit function and the Jacobian of the
     * transformation. The spatial part of the gradient is computed as ∇_x F =
     * J^T ∇f, where J is the position Jacobian of the transformation and ∇f is
     * the gradient of the implicit function. The time component is computed
     * using the time_derivative method.
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
        assert(m_implicit_function != nullptr);
        assert(m_transform != nullptr);

        const auto transformed_pos = m_transform->transform(pos, t);
        const auto g_f = m_implicit_function->gradient(transformed_pos);
        const auto J = m_transform->position_Jacobian(pos, t);

        /* spatial part  ∇_x F = Jᵀ ∇f */
        std::array<Scalar, dim + 1> grad{};
        for (int i = 0; i < dim; ++i) {
            Scalar sum = 0;
            for (int k = 0; k < dim; ++k) sum += J[k][i] * g_f[k];
            grad[i] = sum;
        }

        /* time component */
        grad[dim] = time_derivative(pos, t);

        return grad;
    }

private:
    ImplicitFunction<dim>* m_implicit_function = nullptr; ///< The implicit function being swept
    Transform<dim>* m_transform = nullptr; ///< The transformation applied to the implicit function
};

} // namespace stf
