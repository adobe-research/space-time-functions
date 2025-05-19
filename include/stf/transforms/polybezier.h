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
 * @brief A class representing a piecewise cubic Bezier curve in n-dimensional space.
 *
 * The PolyBezier class implements a transform that maps points along a piecewise cubic Bezier
 * curve. Each segment of the curve is defined by 4 control points, and the segments are connected
 * to form a continuous curve. The class uses Bishop frames to maintain a consistent orientation
 * along the curve.
 *
 * @tparam dim The dimension of the space (2 or 3)
 */
template <int dim>
class PolyBezier : public Transform<dim>
{
public:
    /**
     * @brief Constructs a PolyBezier from a set of sample points.
     *
     * This method generates a piecewise cubic Bezier curve that passes through the given
     * sample points. Tangents are computed automatically to ensure smooth transitions
     * between curve segments.
     *
     * @param samples        A vector of sample points, where each point is represented as an
     *                       array of Scalars in n-dimensional space.
     * @param follow_tangent If true, the transform will add rotation so that the Z-axis of the
     *                       input coordinate system follows the tangent of the curve.
     * @return A PolyBezier object representing the constructed curve.
     * @throws std::runtime_error If fewer than 3 sample points are provided.
     */
    static PolyBezier<dim> from_samples(
        std::vector<std::array<Scalar, dim>> samples,
        bool follow_tangent = true)
    {
        const size_t n = samples.size();
        if (n < 3) {
            throw std::runtime_error(
                "A minimum of 3 samples is required for PolyBezier construction.");
        }

        std::vector<std::array<Scalar, dim>> tangents(n);
        for (size_t i = 1; i < n - 1; ++i) {
            tangents[i] = scale(subtract(samples[i + 1], samples[i - 1]), 0.5);
        }
        tangents[0] = subtract(samples[1], samples[0]);
        tangents[n - 1] = subtract(samples[n - 1], samples[n - 2]);

        std::vector<std::array<Scalar, dim>> points;
        points.reserve((n - 1) * 3 + 1);
        for (size_t i = 0; i < n - 1; i++) {
            points.push_back(samples[i]);
            points.push_back(add(samples[i], scale(tangents[i], 1 / 3.0)));
            points.push_back(add(samples[i + 1], scale(tangents[i + 1], -1 / 3.0)));
        }
        points.push_back(samples[n - 1]);

        PolyBezier bezier(points, follow_tangent);
        return bezier;
    }

public:
    /**
     * @brief Constructs a PolyBezier curve from a set of control points.
     *
     * @param points Vector of control points. Must have at least 4 points and follow the pattern (n
     * * 3) + 1
     * @param follow_tangent If true, the transform will add rotation so that the Z-axis of the
     * input coordinate system follows the tangent of the curve.
     *
     * @throws std::runtime_error if points.size() < 4 or if (points.size() - 1) % 3 != 0
     */
    explicit PolyBezier(std::vector<std::array<Scalar, dim>> points, bool follow_tangent = true)
        : m_points(std::move(points))
        , m_follow_tangent(follow_tangent)
    {
        if (m_points.size() < 4) {
            throw std::runtime_error("PolyBezier must consist of at least 4 points.");
        }
        if ((m_points.size() - 1) % 3 != 0) {
            throw std::runtime_error("PolyBezier must consist of a (n * 3) + 1 points.");
        }
        initialize_bishop_frames();
    }

    /**
     * @brief Transforms a point along the Bezier curve.
     *
     * @param pos The input position to transform
     * @param t The parameter along the curve [0,1]
     * @return The transformed position
     */
    std::array<Scalar, dim> transform(std::array<Scalar, dim> pos, Scalar t) const override
    {
        if (m_follow_tangent) {
            auto [segment, alpha] = find_bezier(t);

            std::span<const std::array<Scalar, dim>, 4> control_points{
                m_points.data() + segment * 3,
                4};

            auto bezier_point = bezier(control_points, alpha);
            auto bezier_frame = get_frame(segment, alpha);

            for (int i = 0; i < dim; i++) {
                pos[i] -= bezier_point[i];
            }
            pos = apply_matrix(transpose(bezier_frame), pos);
            return pos;
        } else {
            auto [segment, alpha] = find_bezier(t);

            std::span<const std::array<Scalar, dim>, 4> control_points{
                m_points.data() + segment * 3,
                4};

            auto bezier_point = bezier(control_points, alpha);
            if constexpr (dim == 3) {
                return {pos[0]-bezier_point[0], pos[1]-bezier_point[1], pos[2]-bezier_point[2]};
            } else {
                static_assert(dim == 2, "PolyBezier only support 2D and 3D.");
                return {pos[0]-bezier_point[0], pos[1]-bezier_point[1]};
            }
        }
    }

    /**
     * @brief Computes the velocity of a point along the Bezier curve.
     *
     * @param pos The input position
     * @param t The parameter along the curve [0,1]
     * @return The velocity vector at the given position and time
     */
    std::array<Scalar, dim> velocity(std::array<Scalar, dim> pos, Scalar t) const override
    {
        if (m_follow_tangent) {
            size_t num_beziers = (m_points.size() - 1) / 3;
            auto [segment, alpha] = find_bezier(t);

            std::span<const std::array<Scalar, dim>, 4> control_points(
                m_points.data() + segment * 3,
                4);
            auto bezier_point = bezier(control_points, alpha);
            auto bezier_velocity = bezier_derivative(control_points, alpha);
            auto bezier_acceleration = bezier_second_derivative(control_points, alpha);

            auto frame = get_frame(segment, alpha);
            auto frame_derivative =
                get_frame_derivative(frame, bezier_velocity, bezier_acceleration);

            auto p = pos;
            for (int i = 0; i < dim; ++i) {
                p[i] -= bezier_point[i];
            }
            p = apply_matrix(transpose(frame), p);
            p = apply_matrix(frame_derivative, p);
            p = apply_matrix(transpose(frame), p);

            auto v = apply_matrix(transpose(frame), bezier_velocity);

            std::array<Scalar, dim> result;
            for (int i = 0; i < dim; i++) {
                result[i] = (-p[i] - v[i]) * num_beziers;
            }

            return result;
        } else {
            auto [segment, alpha] = find_bezier(t);

            std::span<const std::array<Scalar, dim>, 4> control_points{
                m_points.data() + segment * 3,
                4};
            size_t num_beziers = (m_points.size() - 1) / 3;
            auto bezier_velocity = bezier_derivative(control_points, alpha);

            if constexpr (dim == 3) {
                return {
                    -bezier_velocity[0] * num_beziers,
                    -bezier_velocity[1] * num_beziers,
                    -bezier_velocity[2] * num_beziers};
            } else {
                static_assert(dim == 2, "PolyBezier only support 2D and 3D.");
                return {-bezier_velocity[0] * num_beziers, -bezier_velocity[1] * num_beziers};
            }
        }
    }

    /**
     * @brief Computes the Jacobian matrix of the transformation.
     *
     * @param pos The input position
     * @param t The parameter along the curve [0,1]
     * @return The Jacobian matrix of the transformation
     */
    std::array<std::array<Scalar, dim>, dim> position_Jacobian(
        std::array<Scalar, dim> pos,
        Scalar t) const override
    {
        if (m_follow_tangent) {
            auto [segment, alpha] = find_bezier(t);
            auto bezier_frame = get_frame(segment, alpha);
            return transpose(bezier_frame);
        } else {
            std::array<std::array<Scalar, dim>, dim> jacobian{};
            for (int i = 0; i < dim; ++i) {
                jacobian[i][i] = 1;
            }
            return jacobian;
        }
    }

private:
    /**
     * @brief Finds the Bezier segment and local parameter for a given curve parameter.
     *
     * @param t The global curve parameter [0,1]
     * @return A tuple containing the segment index and local parameter alpha
     *
     * @note This function will return the first/last bezier if t is out of [0, 1] bound.
     * alpha will also be out of [0, 1] bound to allow extrapolation.
     */
    std::tuple<size_t, Scalar> find_bezier(Scalar t) const
    {
        size_t num_beziers = (m_points.size() - 1) / 3;

        size_t segment = static_cast<size_t>(std::max(Scalar(0), t) * num_beziers);
        segment = std::min(segment, num_beziers - 1); // Clamp to last bezier
        Scalar alpha = t * num_beziers - segment;
        return {segment, alpha};
    }

    /**
     * @brief Gets the Bishop frame at a given segment and parameter.
     *
     * @param segment The segment index
     * @param alpha The local parameter within the segment [0,1]
     * @return The Bishop frame matrix
     */
    std::array<std::array<Scalar, dim>, dim> get_frame(size_t segment, Scalar alpha) const
    {
        size_t num_beziers = (m_points.size() - 1) / 3;
        assert(segment < num_beziers);

        size_t frame_index =
            segment * m_frames_per_bezier + static_cast<size_t>(alpha * (m_frames_per_bezier - 1));
        const auto& ref_frame = m_frames[frame_index];

        std::array<Scalar, dim> from_vec;
        for (size_t i = 0; i < dim; ++i) {
            from_vec[i] = ref_frame[i][2];
        }

        std::span<const std::array<Scalar, dim>, 4> control_points{
            m_points.data() + segment * 3,
            4};
        auto to_vec = bezier_derivative(control_points, alpha);

        return multiply(rotation_matrix(from_vec, to_vec), ref_frame);
    }

    /**
     * @brief Computes the derivative of the Bishop frame.
     *
     * @param frame The Bishop frame matrix
     * @param bezier_velocity The velocity of the Bezier curve
     * @param bezier_acceleration The acceleration of the Bezier curve
     *
     * @return The derivative of the Bishop frame matrix
     */
    std::array<std::array<Scalar, dim>, dim> get_frame_derivative(
        const std::array<std::array<Scalar, dim>, dim>& frame,
        const std::array<Scalar, dim>& bezier_velocity,
        const std::array<Scalar, dim>& bezier_acceleration) const
    {
        auto bezier_speed = norm(bezier_velocity);

        if (bezier_speed < 1e-10) {
            std::array<std::array<Scalar, dim>, dim> zero_matrix;
            for (size_t i = 0; i < dim; ++i) {
                for (size_t j = 0; j < dim; ++j) {
                    zero_matrix[i][j] = 0;
                }
            }
            return zero_matrix;
        }

        if constexpr (dim == 3) {
            auto [N1, N2, T] = transpose(frame);

            auto acc_T = dot(bezier_acceleration, T);
            Vec3 dT{
                (bezier_acceleration[0] - acc_T * T[0]) / bezier_speed,
                (bezier_acceleration[1] - acc_T * T[1]) / bezier_speed,
                (bezier_acceleration[2] - acc_T * T[2]) / bezier_speed};
            Scalar kappa1 = dot(dT, N1);
            Scalar kappa2 = dot(dT, N2);
            Vec3 dN1{-kappa1 * T[0], -kappa1 * T[1], -kappa1 * T[2]};
            Vec3 dN2{-kappa2 * T[0], -kappa2 * T[1], -kappa2 * T[2]};

            Mat3 result;
            result[0] = {dN1[0], dN2[0], dT[0]};
            result[1] = {dN1[1], dN2[1], dT[1]};
            result[2] = {dN1[2], dN2[2], dT[2]};

            return result;
        } else if constexpr (dim == 2) {
            auto [N1, T] = transpose(frame);

            auto acc_T = dot(bezier_acceleration, T);
            Vec2 dT{
                (bezier_acceleration[0] - acc_T * T[0]) / bezier_speed,
                (bezier_acceleration[1] - acc_T * T[1]) / bezier_speed};
            Scalar kappa = bezier_speed * dot(dT, N1);
            Vec2 dN1{-kappa * T[0], -kappa * T[1]};

            Mat2 result;
            result[0] = {dN1[0], dT[0]};
            result[1] = {dN1[1], dT[1]};
            return result;
        } else {
            throw std::runtime_error("PolyBezier only support 2D and 3D.");
        }
    }

    /**
     * @brief Initializes the Bishop frames along the curve.
     *
     * This method computes a set of orthonormal frames along the curve that minimize rotation.
     * The frames are used to maintain a consistent orientation along the curve.
     */
    void initialize_bishop_frames()
    {
        size_t num_beziers = (m_points.size() - 1) / 3;
        m_frames.clear();
        m_frames.reserve(num_beziers * m_frames_per_bezier);

        if constexpr (dim == 3) {
            Vec3 from_vector{0, 0, 1}; // Align z-axis with the first segment.
            Vec3 to_vector{0, 0, 0};
            for (size_t i = 0; i < num_beziers; ++i) {
                std::span<const Vec3, 4> control_points{m_points.data() + i * 3, 4};
                for (size_t j = 0; j < m_frames_per_bezier; ++j) {
                    Scalar t = static_cast<Scalar>(j) / (m_frames_per_bezier - 1);
                    to_vector = bezier_derivative(control_points, t);
                    if (m_frames.empty()) {
                        m_frames.push_back(rotation_matrix(from_vector, to_vector));
                    } else {
                        m_frames.push_back(
                            multiply(rotation_matrix(from_vector, to_vector), m_frames.back()));
                    }
                    from_vector = to_vector;
                }
            }
        } else if constexpr (dim == 2) {
            Vec2 from_vector{0, 1}; // Align y-axis with the first segment.
            Vec2 to_vector{0, 0};
            for (size_t i = 0; i < num_beziers; ++i) {
                std::span<const Vec2, 4> control_points{m_points.data() + i * 3, 4};
                for (size_t j = 0; j < m_frames_per_bezier; ++j) {
                    Scalar t = static_cast<Scalar>(j) / (m_frames_per_bezier - 1);
                    to_vector = bezier_derivative(control_points, t);
                    if (m_frames.empty()) {
                        m_frames.push_back(rotation_matrix(from_vector, to_vector));
                    } else {
                        m_frames.push_back(
                            multiply(rotation_matrix(from_vector, to_vector), m_frames.back()));
                    }
                    from_vector = to_vector;
                }
            }
        } else {
            throw std::runtime_error("Bishop frames are not implemented for this dimension.");
        }
    }

private:
    std::vector<std::array<Scalar, dim>> m_points; ///< Points defining the polyline
    std::vector<std::array<std::array<Scalar, dim>, dim>>
        m_frames; ///< Bishop frames (One frame per control point)
    constexpr static size_t m_frames_per_bezier =
        4; ///< Number of frames to sample per bezier segment
    bool m_follow_tangent = true; ///< Whether to follow the tangent of the curve
};

} // namespace stf
