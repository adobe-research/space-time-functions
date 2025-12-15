#pragma once

#include <stf/common.h>
#include <stf/primitives/implicit_function.h>

namespace stf {

/**
 * @brief Implicit function representing a 3D torus.
 *
 * The torus is defined by two radii (R and r), a center point, and a normal direction.
 * R is the distance from the center of the tube to the center of the torus,
 * and r is the radius of the tube.
 * The torus lies in a plane orthogonal to the normal direction.
 */
class ImplicitTorus : public ImplicitFunction<3>
{
public:
    /**
     * @brief Constructs a new implicit torus.
     *
     * @param R The major radius (distance from center of tube to center of
     * torus)
     * @param r The minor radius (radius of the tube)
     * @param center The center point of the torus
     * @param normal The normal direction (torus is orthogonal to this). Defaults to {0, 0, 1}.
     */
    ImplicitTorus(Scalar R, Scalar r, std::array<Scalar, 3> center, 
                  std::array<Scalar, 3> normal = {0, 0, 1})
        : m_R(R)
        , m_r(r)
        , m_center(center)
    {
        // Normalize the normal vector
        Scalar len = std::sqrt(normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2]);
        if (len < 1e-10) {
            // Default to Z-axis if invalid normal
            m_normal = {0, 0, 1};
        } else {
            m_normal = {normal[0] / len, normal[1] / len, normal[2] / len};
        }
        
        // Compute basis vectors for the torus plane (orthogonal to normal)
        compute_basis();
    }

    Scalar value(std::array<Scalar, 3> pos) const override
    {
        // Transform to local coordinates
        auto local = to_local(pos);
        
        // In local coordinates, torus is in XY plane (orthogonal to Z)
        Scalar x = local[0];
        Scalar y = local[1];
        Scalar z = local[2];
        Scalar len_xy = std::sqrt(x * x + y * y);

        return std::sqrt(z * z + (len_xy - m_R) * (len_xy - m_R)) - m_r;
    }

    std::array<Scalar, 3> gradient(std::array<Scalar, 3> pos) const override
    {
        // Transform to local coordinates
        auto local = to_local(pos);
        
        Scalar x = local[0];
        Scalar y = local[1];
        Scalar z = local[2];

        Scalar len_xy = std::sqrt(x * x + y * y);

        // Avoid division by zero (if point is at z-axis)
        if (len_xy < 1e-6f) {
            // Gradient in local coordinates
            std::array<Scalar, 3> local_grad = {0, 0, static_cast<Scalar>(z >= 0 ? 1 : -1)};
            // Transform back to world coordinates
            return to_world(local_grad);
        }

        Scalar a = len_xy - m_R;
        Scalar q_len = std::sqrt(a * a + z * z);

        // Again avoid division by zero if exactly on torus surface
        if (q_len < 1e-6f) {
            return {0, 0, 0}; // Undefined
        }

        Scalar dx = (a / q_len) * (x / len_xy);
        Scalar dy = (a / q_len) * (y / len_xy);
        Scalar dz = z / q_len;

        // Gradient in local coordinates
        std::array<Scalar, 3> local_grad = {dx, dy, dz};
        
        // Transform back to world coordinates
        return to_world(local_grad);
    }

private:
    /**
     * @brief Computes orthonormal basis vectors for the torus plane.
     * 
     * Creates two vectors (u and v) that are orthogonal to the normal and to each other.
     * Together with the normal, they form a right-handed coordinate system.
     */
    void compute_basis()
    {
        // Find a vector not parallel to normal
        std::array<Scalar, 3> ref = {1, 0, 0};
        if (std::abs(m_normal[0]) > 0.9) {
            ref = {0, 1, 0};
        }
        
        // Compute first basis vector (u) using cross product
        m_u[0] = ref[1] * m_normal[2] - ref[2] * m_normal[1];
        m_u[1] = ref[2] * m_normal[0] - ref[0] * m_normal[2];
        m_u[2] = ref[0] * m_normal[1] - ref[1] * m_normal[0];
        
        // Normalize u
        Scalar len_u = std::sqrt(m_u[0] * m_u[0] + m_u[1] * m_u[1] + m_u[2] * m_u[2]);
        m_u[0] /= len_u;
        m_u[1] /= len_u;
        m_u[2] /= len_u;
        
        // Compute second basis vector (v) using cross product with normal
        m_v[0] = m_normal[1] * m_u[2] - m_normal[2] * m_u[1];
        m_v[1] = m_normal[2] * m_u[0] - m_normal[0] * m_u[2];
        m_v[2] = m_normal[0] * m_u[1] - m_normal[1] * m_u[0];
        
        // v should already be normalized since normal and u are orthonormal
    }
    
    /**
     * @brief Transforms a world position to local torus coordinates.
     * 
     * In local coordinates, the torus is in the XY plane (orthogonal to Z).
     */
    std::array<Scalar, 3> to_local(std::array<Scalar, 3> pos) const
    {
        // Translate to origin
        Scalar dx = pos[0] - m_center[0];
        Scalar dy = pos[1] - m_center[1];
        Scalar dz = pos[2] - m_center[2];
        
        // Project onto basis vectors
        return {
            dx * m_u[0] + dy * m_u[1] + dz * m_u[2],  // x component
            dx * m_v[0] + dy * m_v[1] + dz * m_v[2],  // y component
            dx * m_normal[0] + dy * m_normal[1] + dz * m_normal[2]  // z component
        };
    }
    
    /**
     * @brief Transforms a local vector to world coordinates.
     * 
     * Used for transforming gradients back to world space.
     */
    std::array<Scalar, 3> to_world(std::array<Scalar, 3> local) const
    {
        return {
            local[0] * m_u[0] + local[1] * m_v[0] + local[2] * m_normal[0],
            local[0] * m_u[1] + local[1] * m_v[1] + local[2] * m_normal[1],
            local[0] * m_u[2] + local[1] * m_v[2] + local[2] * m_normal[2]
        };
    }

    Scalar m_R; ///< The major radius of the torus
    Scalar m_r; ///< The minor radius of the torus
    std::array<Scalar, 3> m_center; ///< The center point of the torus
    std::array<Scalar, 3> m_normal; ///< The normal direction (torus is orthogonal to this)
    std::array<Scalar, 3> m_u; ///< First basis vector in the torus plane
    std::array<Scalar, 3> m_v; ///< Second basis vector in the torus plane
};

} // namespace stf
