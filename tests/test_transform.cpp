#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <stf/transforms/all.h>

template <int dim>
void check_velocity(
    const stf::Transform<dim>& transform,
    const std::array<stf::Scalar, dim>& pos,
    stf::Scalar t,
    stf::Scalar delta = 1e-6,
    stf::Scalar epsilon = 1e-6)
{
    auto v = transform.velocity(pos, t);
    auto v_fd = transform.finite_difference_velocity(pos, t, delta);
    for (int i = 0; i < dim; ++i) {
        REQUIRE_THAT(v[i], Catch::Matchers::WithinAbs(v_fd[i], epsilon));
    }
}

template <int dim>
void check_jacobian(
    const stf::Transform<dim>& transform,
    const std::array<stf::Scalar, dim>& pos,
    stf::Scalar t)
{
    auto J = transform.position_Jacobian(pos, t);
    auto J_fd = transform.finite_difference_Jacobian(pos, t);
    for (int i = 0; i < dim; ++i)
        for (int j = 0; j < dim; ++j) {
            REQUIRE_THAT(J[i][j], Catch::Matchers::WithinAbs(J_fd[i][j], 1e-6));
        }
}

TEST_CASE("transform", "[stf]")
{
    SECTION("Rotation 2D")
    {
        stf::Rotation<2> rotation({0.0, 0.0}, {0, 0});

        auto p0 = rotation.transform({1, 0}, 0);
        REQUIRE_THAT(p0[0], Catch::Matchers::WithinAbs(1, 1e-6));
        REQUIRE_THAT(p0[1], Catch::Matchers::WithinAbs(0, 1e-6));
        check_velocity(rotation, {1, 0}, 0);
        check_jacobian(rotation, {1, 0}, 0);

        auto p1 = rotation.transform({1, 0}, 0.5);
        REQUIRE_THAT(p1[0], Catch::Matchers::WithinAbs(-1, 1e-6));
        REQUIRE_THAT(p1[1], Catch::Matchers::WithinAbs(0, 1e-6));
        check_velocity(rotation, {1, 0}, 0.5);
        check_jacobian(rotation, {1, 0}, 0.5);

        auto p2 = rotation.transform({1, 0}, 0.25);
        REQUIRE_THAT(p2[0], Catch::Matchers::WithinAbs(0, 1e-6));
        REQUIRE_THAT(p2[1], Catch::Matchers::WithinAbs(1, 1e-6));
        check_velocity(rotation, {1, 0}, 0.25);
        check_jacobian(rotation, {1, 0}, 0.25);

        check_velocity(rotation, {1, 1}, 0.75);
        check_jacobian(rotation, {1, 1}, 0.75);
    }

    SECTION("Compose")
    {
        stf::Translation<3> translation({1, 0, 0});
        stf::Rotation<3> rotation({0, 0, 0}, {0, 0, 1});
        stf::Compose<3> compose(rotation, translation);

        SECTION("Origin at t=0")
        {
            auto p0 = compose.transform({0, 0, 0}, 0);
            REQUIRE_THAT(p0[0], Catch::Matchers::WithinAbs(0, 1e-6));
            REQUIRE_THAT(p0[1], Catch::Matchers::WithinAbs(0, 1e-6));
            REQUIRE_THAT(p0[2], Catch::Matchers::WithinAbs(0, 1e-6));
            check_velocity(compose, {0, 0, 0}, 0);
            check_jacobian(compose, {0, 0, 0}, 0);
        }

        SECTION("Origin at t=0.5")
        {
            auto p0 = compose.transform({0, 0, 0}, 0.5);
            REQUIRE_THAT(p0[0], Catch::Matchers::WithinAbs(0.5, 1e-6));
            REQUIRE_THAT(p0[1], Catch::Matchers::WithinAbs(0, 1e-6));
            REQUIRE_THAT(p0[2], Catch::Matchers::WithinAbs(0, 1e-6));
            check_velocity(compose, {0, 0, 0}, 0.5);
            check_jacobian(compose, {0, 0, 0}, 0.5);
        }

        SECTION("Origin at t=1")
        {
            auto p0 = compose.transform({0, 0, 0}, 1);
            REQUIRE_THAT(p0[0], Catch::Matchers::WithinAbs(1, 1e-6));
            REQUIRE_THAT(p0[1], Catch::Matchers::WithinAbs(0, 1e-6));
            REQUIRE_THAT(p0[2], Catch::Matchers::WithinAbs(0, 1e-6));
            check_velocity(compose, {0, 0, 0}, 1);
            check_jacobian(compose, {0, 0, 0}, 1);
        }

        SECTION("[1, 0, 0] at t=0")
        {
            auto p0 = compose.transform({1, 0, 0}, 0);
            REQUIRE_THAT(p0[0], Catch::Matchers::WithinAbs(1, 1e-6));
            REQUIRE_THAT(p0[1], Catch::Matchers::WithinAbs(0, 1e-6));
            REQUIRE_THAT(p0[2], Catch::Matchers::WithinAbs(0, 1e-6));
            check_velocity(compose, {1, 0, 0}, 0);
            check_jacobian(compose, {1, 0, 0}, 0);
        }

        SECTION("[1, 0, 0] at t=0.5")
        {
            auto p0 = compose.transform({1, 0, 0}, 0.5);
            REQUIRE_THAT(p0[0], Catch::Matchers::WithinAbs(-0.5, 1e-6));
            REQUIRE_THAT(p0[1], Catch::Matchers::WithinAbs(0, 1e-6));
            REQUIRE_THAT(p0[2], Catch::Matchers::WithinAbs(0, 1e-6));
            check_velocity(compose, {1, 0, 0}, 0.5);
            check_jacobian(compose, {1, 0, 0}, 0.5);
        }

        SECTION("[1, 0, 0] at t=1.0")
        {
            auto p0 = compose.transform({1, 0, 0}, 1.0);
            REQUIRE_THAT(p0[0], Catch::Matchers::WithinAbs(2, 1e-6));
            REQUIRE_THAT(p0[1], Catch::Matchers::WithinAbs(0, 1e-6));
            REQUIRE_THAT(p0[2], Catch::Matchers::WithinAbs(0, 1e-6));
            check_velocity(compose, {1, 0, 0}, 1.0);
            check_jacobian(compose, {1, 0, 0}, 1.0);
        }
    }

    SECTION("Polyline")
    {
        stf::Polyline<3> transform({{0, 0, 0}, {1, 0, 0}, {0, 1, 0}});

        SECTION("[0, 0, 0] at t=0")
        {
            auto p0 = transform.transform({0, 0, 0}, 0);
            REQUIRE_THAT(p0[0], Catch::Matchers::WithinAbs(0, 1e-6));
            REQUIRE_THAT(p0[1], Catch::Matchers::WithinAbs(0, 1e-6));
            REQUIRE_THAT(p0[2], Catch::Matchers::WithinAbs(0, 1e-6));
            check_velocity(transform, {0, 0, 0}, 0);
            check_jacobian(transform, {0, 0, 0}, 0);
        }

        SECTION("[0, 0, 0] at t=0.25")
        {
            auto p0 = transform.transform({0, 0, 0}, 0.25);
            REQUIRE_THAT(p0[0], Catch::Matchers::WithinAbs(0, 1e-6));
            REQUIRE_THAT(p0[1], Catch::Matchers::WithinAbs(0, 1e-6));
            REQUIRE_THAT(p0[2], Catch::Matchers::WithinAbs(-0.5, 1e-6));
            check_velocity(transform, {0, 0, 0}, 0.25);
            check_jacobian(transform, {0, 0, 0}, 0.25);
        }

        SECTION("[0, 0, 0] at t=1.0")
        {
            auto p0 = transform.transform({0, 0, 0}, 1.0);
            REQUIRE_THAT(p0[0], Catch::Matchers::WithinAbs(0, 1e-6));
            REQUIRE_THAT(p0[1], Catch::Matchers::WithinAbs(std::sqrt(2) / 2, 1e-6));
            REQUIRE_THAT(p0[2], Catch::Matchers::WithinAbs(-std::sqrt(2) / 2, 1e-6));
            check_velocity(transform, {0, 0, 0}, 1.0);
            check_jacobian(transform, {0, 0, 0}, 1);
        }

        SECTION("[1, 0, 0] at t=0.25")
        {
            auto p0 = transform.transform({1, 0, 0}, 0.25);
            REQUIRE_THAT(p0[0], Catch::Matchers::WithinAbs(0, 1e-6));
            REQUIRE_THAT(p0[1], Catch::Matchers::WithinAbs(0, 1e-6));
            REQUIRE_THAT(p0[2], Catch::Matchers::WithinAbs(0.5, 1e-6));
            check_velocity(transform, {1, 0, 0}, 0.25);
            check_jacobian(transform, {1, 0, 0}, 0.25);
        }

        SECTION("[1, 0, 0] at t=0.75")
        {
            auto p0 = transform.transform({1, 0, 0}, 0.75);
            REQUIRE_THAT(p0[0], Catch::Matchers::WithinAbs(0, 1e-6));
            REQUIRE_THAT(p0[1], Catch::Matchers::WithinAbs(0, 1e-6));
            REQUIRE_THAT(p0[2], Catch::Matchers::WithinAbs(-std::sqrt(2) / 2, 1e-6));
            check_velocity(transform, {1, 0, 0}, 0.75);
            check_jacobian(transform, {1, 0, 0}, 0.75);
        }
    }

    SECTION("Polyline without follow_tangent")
    {
        // Polyline with follow_tangent = false should use identity transformations
        stf::Polyline<3> transform({{0, 0, 0}, {1, 0, 0}, {1, 1, 0}}, false);

        SECTION("[0, 0, 0] at t=0")
        {
            auto p0 = transform.transform({0, 0, 0}, 0);
            REQUIRE_THAT(p0[0], Catch::Matchers::WithinAbs(0, 1e-6));
            REQUIRE_THAT(p0[1], Catch::Matchers::WithinAbs(0, 1e-6));
            REQUIRE_THAT(p0[2], Catch::Matchers::WithinAbs(0, 1e-6));
            check_velocity(transform, {0, 0, 0}, 0);
            check_jacobian(transform, {0, 0, 0}, 0);
        }

        SECTION("[1, 0, 0] at t=0.25")
        {
            // At t=0.25, we're at point (0.5, 0, 0)
            // Input point [1, 0, 0] becomes [-0.5, 0, 0] with identity transform
            auto p0 = transform.transform({1, 0, 0}, 0.25);
            REQUIRE_THAT(p0[0], Catch::Matchers::WithinAbs(0.5, 1e-6));
            REQUIRE_THAT(p0[1], Catch::Matchers::WithinAbs(0, 1e-6));
            REQUIRE_THAT(p0[2], Catch::Matchers::WithinAbs(0, 1e-6));
            check_velocity(transform, {1, 0, 0}, 0.25);
            check_jacobian(transform, {1, 0, 0}, 0.25);
        }
    }

    SECTION("polybezier")
    {
        stf::PolyBezier<3> transform({
            {0, 0, 0}, // 0
            {1, 0, 0}, // 1
            {1, 1, 0}, // 2
            {0, 1, 0}, // 3
            {-1, 1, 0}, // 4
            {-1, 0, 0}, // 5
            {0, 0, 0} // 6
        });

        SECTION("[0, 0, 0] at t=0")
        {
            auto p0 = transform.transform({0, 0, 0}, 0);
            REQUIRE_THAT(p0[0], Catch::Matchers::WithinAbs(0, 1e-6));
            REQUIRE_THAT(p0[1], Catch::Matchers::WithinAbs(0, 1e-6));
            REQUIRE_THAT(p0[2], Catch::Matchers::WithinAbs(0, 1e-6));
            check_velocity(transform, {0, 0, 0}, 0);
            check_jacobian(transform, {0, 0, 0}, 0);
        }
        SECTION("[0, 0, 0] at t=0.25")
        {
            auto p0 = transform.transform({0, 0, 0}, 0.25);
            REQUIRE_THAT(p0[0], Catch::Matchers::WithinAbs(0, 1e-6));
            REQUIRE_THAT(p0[1], Catch::Matchers::WithinAbs(0.75, 1e-6));
            REQUIRE_THAT(p0[2], Catch::Matchers::WithinAbs(-0.5, 1e-6));
            check_velocity(transform, {0, 0, 0}, 0.25);
            check_jacobian(transform, {0, 0, 0}, 0.25);
        }
        SECTION("[0, 0, 0] at t=0.5")
        {
            auto p0 = transform.transform({0, 0, 0}, 0.5);
            REQUIRE_THAT(p0[0], Catch::Matchers::WithinAbs(0, 1e-6));
            REQUIRE_THAT(p0[1], Catch::Matchers::WithinAbs(1, 1e-6));
            REQUIRE_THAT(p0[2], Catch::Matchers::WithinAbs(0, 1e-6));
            check_velocity(transform, {0, 0, 0}, 0.5, 1e-3, 1e-1);
            check_jacobian(transform, {0, 0, 0}, 0.5);
        }
        SECTION("[0, 0, 0] at t=1")
        {
            auto p0 = transform.transform({0, 0, 0}, 1.0);
            REQUIRE_THAT(p0[0], Catch::Matchers::WithinAbs(0, 1e-6));
            REQUIRE_THAT(p0[1], Catch::Matchers::WithinAbs(0, 1e-6));
            REQUIRE_THAT(p0[2], Catch::Matchers::WithinAbs(0, 1e-6));
            check_velocity(transform, {0, 0, 0}, 1, 1e-3, 1e-1);
            check_jacobian(transform, {0, 0, 0}, 1);
        }
        SECTION("[0, 1, 0] at t=0")
        {
            auto p0 = transform.transform({0, 1, 0}, 0);
            REQUIRE_THAT(p0[0], Catch::Matchers::WithinAbs(0, 1e-6));
            REQUIRE_THAT(p0[1], Catch::Matchers::WithinAbs(1, 1e-6));
            REQUIRE_THAT(p0[2], Catch::Matchers::WithinAbs(0, 1e-6));
            check_velocity(transform, {0, 1, 0}, 0, 1e-3, 1e-3);
            check_jacobian(transform, {0, 1, 0}, 0);
        }
        SECTION("[0, 1, 0] at t=0.25")
        {
            auto p0 = transform.transform({0, 1, 0}, 0.25);
            REQUIRE_THAT(p0[0], Catch::Matchers::WithinAbs(0, 1e-6));
            REQUIRE_THAT(p0[1], Catch::Matchers::WithinAbs(0.75, 1e-6));
            REQUIRE_THAT(p0[2], Catch::Matchers::WithinAbs(0.5, 1e-6));
            check_velocity(transform, {0, 1, 0}, 0.25, 1e-3, 1e-3);
            check_jacobian(transform, {0, 1, 0}, 0.25);
        }
        SECTION("[0, 1, 0] at t=0.5")
        {
            auto p0 = transform.transform({0, 1, 0}, 0.5);
            REQUIRE_THAT(p0[0], Catch::Matchers::WithinAbs(0, 1e-6));
            REQUIRE_THAT(p0[1], Catch::Matchers::WithinAbs(0, 1e-6));
            REQUIRE_THAT(p0[2], Catch::Matchers::WithinAbs(0, 1e-6));
            check_velocity(transform, {0, 1, 0}, 0.5, 1e-3, 1e-1);
            check_jacobian(transform, {0, 1, 0}, 0.5);
        }
        SECTION("[0, 1, 0] at t=1")
        {
            auto p0 = transform.transform({0, 1, 0}, 1);
            REQUIRE_THAT(p0[0], Catch::Matchers::WithinAbs(0, 1e-6));
            REQUIRE_THAT(p0[1], Catch::Matchers::WithinAbs(1, 1e-6));
            REQUIRE_THAT(p0[2], Catch::Matchers::WithinAbs(0, 1e-6));
            check_velocity(transform, {0, 1, 0}, 1, 1e-3, 1e-1);
            check_jacobian(transform, {0, 1, 0}, 1);
        }
    }

    SECTION("polybezier from samples")
    {
        std::vector<std::array<stf::Scalar, 3>> samples = {
            {0, 0, 0},
            {1, 0, 0},
            {1, 1, 0},
            {0, 1, 0}};
        auto transform = stf::PolyBezier<3>::from_samples(samples);

        SECTION("[0, 0, 0] at t=0")
        {
            auto p0 = transform.transform({0, 0, 0}, 0);
            REQUIRE_THAT(p0[0], Catch::Matchers::WithinAbs(0, 1e-6));
            REQUIRE_THAT(p0[1], Catch::Matchers::WithinAbs(0, 1e-6));
            REQUIRE_THAT(p0[2], Catch::Matchers::WithinAbs(0, 1e-6));
            check_velocity(transform, {0, 0, 0}, 0);
            check_jacobian(transform, {0, 0, 0}, 0);
        }

        SECTION("[0, 0, 0] at t=1")
        {
            auto p0 = transform.transform({0, 0, 0}, 1);
            REQUIRE_THAT(p0[0], Catch::Matchers::WithinAbs(0, 1e-6));
            REQUIRE_THAT(p0[1], Catch::Matchers::WithinAbs(1, 1e-6));
            REQUIRE_THAT(p0[2], Catch::Matchers::WithinAbs(0, 1e-6));
            check_velocity(transform, {0, 0, 0}, 1, 1e-3, 1e-3);
            check_jacobian(transform, {0, 0, 0}, 1);
        }
    }

    SECTION("polybezier translation only")
    {
        stf::PolyBezier<3> transform({
            {0, 0, 1}, // 0
            {1, 0, 1}, // 1
            {1, 1, 1}, // 2
            {0, 1, 1}, // 3
            {-1, 1, 1}, // 4
            {-1, 0, 1}, // 5
            {0, 0, 1} // 6
        }, false);

        SECTION("[0, 0, 0] at t=0") {
            auto p0 = transform.transform({0, 0, 0}, 0);
            REQUIRE_THAT(p0[0], Catch::Matchers::WithinAbs(0, 1e-6));
            REQUIRE_THAT(p0[1], Catch::Matchers::WithinAbs(0, 1e-6));
            REQUIRE_THAT(p0[2], Catch::Matchers::WithinAbs(-1, 1e-6));
            check_velocity(transform, {0, 0, 0}, 0);
            check_jacobian(transform, {0, 0, 0}, 0);
        }
        SECTION("[0, 0, 0] at t=0.25") {
            auto p0 = transform.transform({0, 0, 0}, 0.25);
            REQUIRE_THAT(p0[0], Catch::Matchers::WithinAbs(-0.75, 1e-6));
            REQUIRE_THAT(p0[1], Catch::Matchers::WithinAbs(-0.5, 1e-6));
            REQUIRE_THAT(p0[2], Catch::Matchers::WithinAbs(-1, 1e-6));
            check_velocity(transform, {0, 0, 0}, 0.25);
            check_jacobian(transform, {0, 0, 0}, 0.25);
        }
        SECTION("[0, 0, 0] at t=0.5") {
            auto p0 = transform.transform({0, 0, 0}, 0.5);
            REQUIRE_THAT(p0[0], Catch::Matchers::WithinAbs(0, 1e-6));
            REQUIRE_THAT(p0[1], Catch::Matchers::WithinAbs(-1, 1e-6));
            REQUIRE_THAT(p0[2], Catch::Matchers::WithinAbs(-1, 1e-6));
            check_velocity(transform, {0, 0, 0}, 0.5, 1e-6, 1e-3);
            check_jacobian(transform, {0, 0, 0}, 0.5);
        }
        SECTION("[0, 0, 0] at t=0.75") {
            auto p0 = transform.transform({0, 0, 0}, 0.75);
            REQUIRE_THAT(p0[0], Catch::Matchers::WithinAbs(0.75, 1e-6));
            REQUIRE_THAT(p0[1], Catch::Matchers::WithinAbs(-0.5, 1e-6));
            REQUIRE_THAT(p0[2], Catch::Matchers::WithinAbs(-1, 1e-6));
            check_velocity(transform, {0, 0, 0}, 0.75, 1e-6, 1e-3);
            check_jacobian(transform, {0, 0, 0}, 0.75);
        }
    }
}
