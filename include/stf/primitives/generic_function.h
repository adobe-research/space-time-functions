#pragma once

#include <stf/common.h>

#include <array>
#include <functional>

namespace stf {

/**
 * @brief A generic implicit function that takes function pointers for value and gradient
 * computation.
 *
 * This class allows creating implicit functions by providing function pointers for both
 * the value and gradient computations. This is useful for creating custom implicit functions
 * without having to create a new class.
 *
 * @tparam dim The dimension of the space (2 for 2D, 3 for 3D)
 */
template <int dim>
class GenericFunction : public ImplicitFunction<dim>
{
public:
    /**
     * @brief Constructs a new generic implicit function.
     *
     * @param value_func Function that computes the value at a given position
     * @param gradient_func Function that computes the gradient at a given position
     * @throws std::invalid_argument if either function pointer is null
     */
    GenericFunction(
        std::function<Scalar(std::array<Scalar, dim>)> value_func,
        std::function<std::array<Scalar, dim>(std::array<Scalar, dim>)> gradient_func)
        : m_value_func(value_func)
        , m_gradient_func(gradient_func)
    {
        if (!value_func) {
            throw std::invalid_argument("value_func cannot be null");
        }
        if (!gradient_func) {
            throw std::invalid_argument("gradient_func cannot be null");
        }
    }

    virtual ~GenericFunction() = default;

    Scalar value(std::array<Scalar, dim> pos) const override { return m_value_func(pos); }

    std::array<Scalar, dim> gradient(std::array<Scalar, dim> pos) const override
    {
        return m_gradient_func(pos);
    }

private:
    std::function<Scalar(std::array<Scalar, dim>)> m_value_func;
    std::function<std::array<Scalar, dim>(std::array<Scalar, dim>)> m_gradient_func;
};

} // namespace stf
