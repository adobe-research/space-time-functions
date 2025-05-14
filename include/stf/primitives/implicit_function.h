#pragma once

#include <stf/common.h>

#include <array>

namespace stf {

/**
 * @brief Base class for implicit functions in N-dimensional space.
 *
 * An implicit function defines a surface as the zero level set of a scalar
 * function. The function returns positive values outside the surface, negative
 * values inside, and zero on the surface.
 *
 * @tparam dim The dimension of the space (2 for 2D, 3 for 3D)
 */
template <int dim>
class ImplicitFunction {
   public:
    /**
     * @brief Evaluates the implicit function at a given position.
     *
     * @param pos The position to evaluate at
     * @return Scalar The signed distance to the surface (positive outside,
     * negative inside)
     */
    virtual Scalar value(std::array<Scalar, dim> pos) const = 0;

    /**
     * @brief Computes the gradient of the implicit function at a given
     * position.
     *
     * @param pos The position to evaluate at
     * @return std::array<Scalar, dim> The normalized gradient vector
     */
    virtual std::array<Scalar, dim> gradient(
        std::array<Scalar, dim> pos) const = 0;
};

}  // namespace stf
