#pragma once

#include <stf/common.h>
#include <stf/maths/all.h>
#include <stf/transforms/transform.h>

#include <array>
#include <stdexcept>
#include <tuple>
#include <vector>

namespace stf {

/**
 * @brief A transformation that follows a polyline and uses Bishop frames for orientation.
 *
 * This class represents a piecewise linear curve (polyline) in N-dimensional space and provides
 * transformation, velocity, and Jacobian computations along the polyline using Bishop frames for
 * orientation.
 *
 * @note An initial transformation is applied to align the z-axis in 3D or the y-axis in 2D with the
 * first segment.
 *
 * @tparam dim The dimension of the space (2 or 3 supported).
 */
template <int dim>
class Polyline : public Transform<dim>
{
public:
    /**
     * @brief Construct a Polyline from a sequence of points.
     * @param points The points defining the polyline. Must contain at least 2 points.
     * @throws std::runtime_error if fewer than 2 points are provided.
     */
    explicit Polyline(std::vector<std::array<Scalar, dim>> points)
        : m_points(std::move(points))
    {
        if (m_points.size() < 2) {
            throw std::runtime_error("Polyline must consist of at least 2 points.");
        }
        initialize_bishop_frames();
    }

    /**
     * @brief Transform a position along the polyline at parameter t.
     *
     * @param pos The position to transform (local coordinates).
     * @param t The parameter along the polyline in [0, 1].
     * @return The transformed position in global coordinates.
     * @throws std::runtime_error if the polyline has fewer than 2 points.
     */
    std::array<Scalar, dim> transform(std::array<Scalar, dim> pos, Scalar t) const override
    {
        if (m_points.size() < 2) {
            throw std::runtime_error("Polyline must consist of at least 2 points.");
        }

        // Find the segment of the polyline that contains the point
        auto [segment, alpha] = find_segment(t);

        // Linear interpolation between points
        auto& p0 = m_points[segment];
        auto& p1 = m_points[segment + 1];

        pos = apply_matrix(m_frames[segment], pos);

        for (int i = 0; i < dim; ++i) {
            pos[i] += p0[i] + alpha * (p1[i] - p0[i]);
        }
        return pos;
    }

    /**
     * @brief Compute the velocity along the polyline at parameter t.
     *
     * @param pos The position (unused).
     * @param t The parameter along the polyline in [0, 1].
     * @return The velocity vector at the given parameter.
     * @throws std::runtime_error if the polyline has fewer than 2 points.
     */
    std::array<Scalar, dim> velocity(std::array<Scalar, dim> pos, Scalar t) const override
    {
        if (m_points.size() < 2) {
            throw std::runtime_error("Polyline must consist of at least 2 points.");
        }

        auto [segment, alpha] = find_segment(t);

        auto& p0 = m_points[segment];
        auto& p1 = m_points[segment + 1];

        std::array<Scalar, dim> velocity;
        for (int i = 0; i < dim; ++i) {
            velocity[i] = (p1[i] - p0[i]) * (m_points.size() - 1);
        }
        return velocity;
    }

    /**
     * @brief Compute the Jacobian of the position transformation at parameter t.
     *
     * @param pos The position (unused).
     * @param t The parameter along the polyline in [0, 1].
     * @return The Jacobian matrix at the given parameter.
     * @throws std::runtime_error if the polyline has fewer than 2 points.
     */
    std::array<std::array<Scalar, dim>, dim> position_Jacobian(
        std::array<Scalar, dim> pos,
        Scalar t) const override
    {
        if (m_points.size() < 2) {
            throw std::runtime_error("Polyline must consist of at least 2 points.");
        }

        auto [segment, alpha] = find_segment(t);
        return m_frames[segment];
    }

private:
    /**
     * @brief Find the segment and interpolation parameter for a given t.
     *
     * @param t The parameter along the polyline in [0, 1].
     * @return A tuple (segment index, alpha) where alpha is the interpolation factor within the
     * segment.
     */
    std::tuple<size_t, Scalar> find_segment(Scalar t) const
    {
        size_t segment = static_cast<size_t>(t * (m_points.size() - 1));
        segment = std::max(segment, size_t(0)); // Clamp to first segment
        segment = std::min(segment, m_points.size() - 2); // Clamp to last segment
        Scalar alpha = t * (m_points.size() - 1) - segment;
        return {segment, alpha};
    }

    /**
     * @brief Initialize Bishop frames for each segment of the polyline.
     *
     * This sets up the orientation frames used for transforming positions along the polyline.
     * @throws std::runtime_error if the dimension is not 2 or 3.
     */
    void initialize_bishop_frames()
    {
        m_frames.clear();
        m_frames.reserve(m_points.size() - 1);

        if constexpr (dim == 3) {
            Vec3 from_vector{0, 0, 1}; // Align z-axis with the first segment.
            Vec3 to_vector{0, 0, 0};
            for (size_t i = 0; i + 1 < m_points.size(); ++i) {
                auto& p0 = m_points[i];
                auto& p1 = m_points[i + 1];
                to_vector = {p1[0] - p0[0], p1[1] - p0[1], p1[2] - p0[2]};
                if (m_frames.empty()) {
                    m_frames.push_back(rotation_matrix(from_vector, to_vector));
                } else {
                    m_frames.push_back(
                        multiply(rotation_matrix(from_vector, to_vector), m_frames.back()));
                }
                from_vector = to_vector;
            }
        } else if constexpr (dim == 2) {
            Vec2 from_vector{0, 1}; // Align y-axis with the first segment.
            Vec2 to_vector{0, 0};
            for (size_t i = 0; i < m_points.size() - 1; ++i) {
                auto& p0 = m_points[i];
                auto& p1 = m_points[i + 1];
                to_vector = {p1[0] - p0[0], p1[1] - p0[1]};
                if (m_frames.empty()) {
                    m_frames.push_back(rotation_matrix(from_vector, to_vector));
                } else {
                    m_frames.push_back(
                        multiply(rotation_matrix(from_vector, to_vector), m_frames.back()));
                }
                from_vector = to_vector;
            }
        } else {
            throw std::runtime_error("Bishop frames are not implemented for this dimension.");
        }
    }

private:
    std::vector<std::array<Scalar, dim>> m_points; ///< Points defining the polyline
    std::vector<std::array<std::array<Scalar, dim>, dim>>
        m_frames; ///< Bishop frames (one per segment)
};

} // namespace stf

