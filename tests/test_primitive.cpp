#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <stf/primitives/all.h>

#include <cmath>

template <int dim>
void check_gradient(
    const stf::ImplicitFunction<dim>& implicit,
    const std::array<stf::Scalar, dim>& pos,
    stf::Scalar delta = 1e-6,
    stf::Scalar epsilon = 1e-6)
{
    auto grad = implicit.gradient(pos);
    auto grad_fd = implicit.finite_difference_gradient(pos, delta);
    for (int i = 0; i < dim; ++i) {
        REQUIRE_THAT(grad[i], Catch::Matchers::WithinAbs(grad_fd[i], epsilon));
    }
}

TEST_CASE("primitive", "[stf]")
{
    SECTION("ball")
    {
        stf::ImplicitBall<3> ball(1.0, {0, 0, 0});
        REQUIRE_THAT(ball.value({0, 0, 0}), Catch::Matchers::WithinAbs(-1, 1e-6));
        REQUIRE_THAT(ball.value({1, 0, 0}), Catch::Matchers::WithinAbs(0, 1e-6));
        check_gradient(ball, {0, 0, 0});
        check_gradient(ball, {1, 0, 0});
    }

    SECTION("quadratic ball")
    {
        stf::ImplicitBall<3> ball(1.0, {0, 0, 0}, 2);
        REQUIRE_THAT(ball.value({0, 0, 0}), Catch::Matchers::WithinAbs(-1, 1e-6));
        REQUIRE_THAT(ball.value({1, 0, 0}), Catch::Matchers::WithinAbs(0, 1e-6));
        check_gradient(ball, {0, 0, 0});
        check_gradient(ball, {1, 0, 0});
    }

    SECTION("ball not at origin")
    {
        stf::ImplicitBall<3> ball(1.0, {1, 1, 1});
        REQUIRE_THAT(ball.value({0, 0, 0}), Catch::Matchers::WithinAbs(std::sqrt(3) - 1, 1e-6));
        REQUIRE_THAT(ball.value({1, 0, 0}), Catch::Matchers::WithinAbs(std::sqrt(2) - 1, 1e-6));
        check_gradient(ball, {0, 0, 0});
        check_gradient(ball, {1, 0, 0});
    }

    SECTION("quadratic 2D ball not at origin")
    {
        stf::ImplicitBall<2> ball(1.0, {1, 2}, 2);
        REQUIRE_THAT(ball.value({0, 0}), Catch::Matchers::WithinAbs(4, 1e-6));
        REQUIRE_THAT(ball.value({1, 0}), Catch::Matchers::WithinAbs(3, 1e-6));
        check_gradient(ball, {0, 0});
        check_gradient(ball, {1, 0});
    }

    SECTION("union")
    {
        stf::ImplicitBall<3> ball_1(0.5, {-0.6, 0, 0});
        stf::ImplicitBall<3> ball_2(0.5, {0.6, 0, 0});
        stf::ImplicitUnion<3> shape(ball_1, ball_2);

        REQUIRE_THAT(shape.value({0, 0, 0}), Catch::Matchers::WithinAbs(0.1, 1e-6));
        REQUIRE_THAT(shape.value({0.5, 0, 0}), Catch::Matchers::WithinAbs(-0.4, 1e-6));
        REQUIRE_THAT(shape.value({-0.5, 0, 0}), Catch::Matchers::WithinAbs(-0.4, 1e-6));
        check_gradient(shape, {0.5, 0, 0});
        check_gradient(shape, {-0.5, 0, 0});
    }

    SECTION("union quadratic")
    {
        stf::ImplicitBall<3> ball_1(0.5, {-0.6, 0, 0});
        stf::ImplicitBall<3> ball_2(0.5, {0.6, 0, 0});
        stf::ImplicitUnion<3> shape(ball_1, ball_2, 0.2);

        REQUIRE(shape.value({0, 0, 0}) < 0);
        REQUIRE_THAT(shape.value({0.5, 0, 0}), Catch::Matchers::WithinAbs(-0.4, 1e-6));
        REQUIRE_THAT(shape.value({-0.5, 0, 0}), Catch::Matchers::WithinAbs(-0.4, 1e-6));
        check_gradient(shape, {0.0, 0, 0});
        check_gradient(shape, {0.5, 0, 0});
        check_gradient(shape, {-0.5, 0, 0});
        check_gradient(shape, {1, 1, 1});
    }

    SECTION("union cubic")
    {
        stf::ImplicitBall<3> ball_1(0.5, {-0.6, 0, 0});
        stf::ImplicitBall<3> ball_2(0.5, {0.6, 0, 0});
        stf::ImplicitUnion<3, stf::BlendingFunction::Cubic> shape(ball_1, ball_2, 0.2);

        REQUIRE(shape.value({0, 0, 0}) < 0);
        check_gradient(shape, {0.0, 0, 0});
        check_gradient(shape, {0.5, 0, 0});
        check_gradient(shape, {-0.5, 0, 0});
        check_gradient(shape, {1, 1, 1});
    }

    SECTION("union quartic")
    {
        stf::ImplicitBall<3> ball_1(0.5, {-0.6, 0, 0});
        stf::ImplicitBall<3> ball_2(0.5, {0.6, 0, 0});
        stf::ImplicitUnion<3, stf::BlendingFunction::Quartic> shape(ball_1, ball_2, 0.2);

        REQUIRE(shape.value({0, 0, 0}) < 0);
        check_gradient(shape, {0.0, 0, 0});
        check_gradient(shape, {0.5, 0, 0});
        check_gradient(shape, {-0.5, 0, 0});
        check_gradient(shape, {1, 1, 1});
    }

    SECTION("union circular")
    {
        stf::ImplicitBall<3> ball_1(0.5, {-0.6, 0, 0});
        stf::ImplicitBall<3> ball_2(0.5, {0.6, 0, 0});
        stf::ImplicitUnion<3, stf::BlendingFunction::Circular> shape(ball_1, ball_2, 0.2);

        REQUIRE(shape.value({0, 0, 0}) < 0);
        check_gradient(shape, {0.0, 0, 0});
        check_gradient(shape, {0.5, 0, 0});
        check_gradient(shape, {-0.5, 0, 0});
        check_gradient(shape, {1, 1, 1});
    }

    SECTION("capsule")
    {
        stf::ImplicitCapsule<3> capsule(0.5, {0, 0, 0}, {1, 0, 0});

        REQUIRE_THAT(capsule.value({0, 0, 0}), Catch::Matchers::WithinAbs(-0.5, 1e-6));
        REQUIRE_THAT(capsule.value({1, 0, 0}), Catch::Matchers::WithinAbs(-0.5, 1e-6));
        REQUIRE_THAT(capsule.value({0.5, 0, 0}), Catch::Matchers::WithinAbs(-0.5, 1e-6));
        REQUIRE_THAT(capsule.value({0, 1, 0}), Catch::Matchers::WithinAbs(0.5, 1e-6));
        REQUIRE_THAT(capsule.value({0.5, 1, 0}), Catch::Matchers::WithinAbs(0.5, 1e-6));
        REQUIRE_THAT(capsule.value({1, 1, 0}), Catch::Matchers::WithinAbs(0.5, 1e-6));
        REQUIRE_THAT(capsule.value({-0.5, 0, 0}), Catch::Matchers::WithinAbs(0, 1e-6));
        REQUIRE_THAT(capsule.value({1.5, 0, 0}), Catch::Matchers::WithinAbs(0, 1e-6));

        check_gradient(capsule, {0, 1, 0});
        check_gradient(capsule, {0.5, 1, 0});
        check_gradient(capsule, {1, 1, 0});
        check_gradient(capsule, {-0.5, 0, 0});
        check_gradient(capsule, {1.5, 0, 0});
    }

    SECTION("vipss")
    {
        stf::Duchon vipss(
            {{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {1, 1, 0}},
            {{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}, {13, 14, 15, 16}},
            {17, 18, 19, 20});
        check_gradient(vipss, {0.1, 0.1, 0.1});
        check_gradient(vipss, {1.0, 0.0, 0.0}, 1e-6, 1e-3);
        check_gradient(vipss, {1.1, -0.1, 0.5});
    }

    SECTION("vipss negated")
    {
        stf::Duchon vipss(
            {{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {1, 1, 0}},
            {{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}, {13, 14, 15, 16}},
            {17, 18, 19, 20},
            {0, 0, 0},
            1.0,
            true);
        check_gradient(vipss, {0.1, 0.1, 0.1});
        check_gradient(vipss, {1.0, 0.0, 0.0}, 1e-6, 1e-3);
        check_gradient(vipss, {1.1, -0.1, 0.5});
        check_gradient(vipss, {0.0, 0.0, 0.0});
    }

    SECTION("vipss with transformation")
    {
        stf::Duchon vipss(
            {{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {1, 1, 0}},
            {{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}, {13, 14, 15, 16}},
            {17, 18, 19, 20});
        stf::Duchon vipss_transformed(
            {{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {1, 1, 0}},
            {{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}, {13, 14, 15, 16}},
            {17, 18, 19, 20},
            {1, 1, 1}, // translation
            0.5);
        check_gradient(vipss_transformed, {0.1, 0.1, 0.1});
        check_gradient(vipss_transformed, {1.0, 0.0, 0.0}, 1e-6, 1e-3);
        check_gradient(vipss_transformed, {1.1, -0.1, 0.5});

        SECTION("Evaluate at center")
        {
            auto v = vipss.value({0, 0, 0});
            auto r = vipss_transformed.value({1, 1, 1});
            REQUIRE_THAT(v, Catch::Matchers::WithinAbs(r, 1e-6));
        }
        SECTION("Evaluate at bbox_min")
        {
            auto v = vipss.value({-0.5, -0.5, -0.5});
            auto r = vipss_transformed.value({0.75, 0.75, 0.75});
            REQUIRE_THAT(v, Catch::Matchers::WithinAbs(r, 1e-6));
        }
        SECTION("Evaluate at bbox_max")
        {
            auto v = vipss.value({0.5, 0.5, 0.5});
            auto r = vipss_transformed.value({1.25, 1.25, 1.25});
            REQUIRE_THAT(v, Catch::Matchers::WithinAbs(r, 1e-6));
        }
    }

    SECTION("torus - default orientation (XY plane)")
    {
        // Torus with major radius 1.0, minor radius 0.3, centered at origin
        // Default normal is {0, 0, 1}, so torus lies in XY plane
        stf::ImplicitTorus torus(1.0, 0.3, {0, 0, 0});

        // Center should be at distance = major_radius - minor_radius = 0.7
        REQUIRE_THAT(torus.value({0, 0, 0}), Catch::Matchers::WithinAbs(0.7, 1e-6));

        // Point on the torus tube (major circle at radius 1.0, minor circle at radius 0.3)
        // On XY plane at (1.3, 0, 0) should be on surface
        REQUIRE_THAT(torus.value({1.3, 0, 0}), Catch::Matchers::WithinAbs(0.0, 1e-5));

        // On XY plane at (0.7, 0, 0) should be on surface (inner side)
        REQUIRE_THAT(torus.value({0.7, 0, 0}), Catch::Matchers::WithinAbs(0.0, 1e-5));

        // Point on major circle in XY plane at (1, 0, 0.3) should be on surface
        REQUIRE_THAT(torus.value({1, 0, 0.3}), Catch::Matchers::WithinAbs(0.0, 1e-5));

        // Point inside torus tube
        REQUIRE(torus.value({1.0, 0, 0}) < 0);

        // Point far outside
        REQUIRE(torus.value({5, 5, 5}) > 0);

        // Check gradients at various points
        check_gradient(torus, {1.3, 0, 0});
        check_gradient(torus, {0.7, 0, 0});
        check_gradient(torus, {1, 0, 0.3});
        check_gradient(torus, {1, 0.5, 0.2});
        check_gradient(torus, {0, 1, 0.15});
    }

    SECTION("torus - YZ plane (normal along X)")
    {
        // Torus with normal along X-axis {1, 0, 0}
        // Should lie in YZ plane
        stf::ImplicitTorus torus(1.0, 0.3, {0, 0, 0}, {1, 0, 0});

        // Center should be at distance 0.7
        REQUIRE_THAT(torus.value({0, 0, 0}), Catch::Matchers::WithinAbs(0.7, 1e-6));

        // Points on the torus surface in YZ plane
        REQUIRE_THAT(torus.value({0, 1.3, 0}), Catch::Matchers::WithinAbs(0.0, 1e-5));
        REQUIRE_THAT(torus.value({0, 0.7, 0}), Catch::Matchers::WithinAbs(0.0, 1e-5));
        REQUIRE_THAT(torus.value({0.3, 1.0, 0}), Catch::Matchers::WithinAbs(0.0, 1e-5));

        // Check gradients
        check_gradient(torus, {0, 1.3, 0});
        check_gradient(torus, {0, 0.7, 0});
        check_gradient(torus, {0.3, 1.0, 0});
        check_gradient(torus, {0.2, 0, 1.0});
    }

    SECTION("torus - XZ plane (normal along Y)")
    {
        // Torus with normal along Y-axis {0, 1, 0}
        // Should lie in XZ plane
        stf::ImplicitTorus torus(1.0, 0.3, {0, 0, 0}, {0, 1, 0});

        // Center should be at distance 0.7
        REQUIRE_THAT(torus.value({0, 0, 0}), Catch::Matchers::WithinAbs(0.7, 1e-6));

        // Points on the torus surface in XZ plane
        REQUIRE_THAT(torus.value({1.3, 0, 0}), Catch::Matchers::WithinAbs(0.0, 1e-5));
        REQUIRE_THAT(torus.value({0.7, 0, 0}), Catch::Matchers::WithinAbs(0.0, 1e-5));
        REQUIRE_THAT(torus.value({1.0, 0.3, 0}), Catch::Matchers::WithinAbs(0.0, 1e-5));

        // Check gradients
        check_gradient(torus, {1.3, 0, 0});
        check_gradient(torus, {0.7, 0, 0});
        check_gradient(torus, {1.0, 0.3, 0});
        check_gradient(torus, {0, 0.2, 1.0});
    }

    SECTION("torus - angled orientation")
    {
        // Torus at 45 degrees between X and Y axes
        stf::Scalar sqrt2_inv = 1.0 / std::sqrt(2.0);
        stf::ImplicitTorus torus(1.0, 0.3, {0, 0, 0}, {sqrt2_inv, sqrt2_inv, 0});

        // Center should still be at distance 0.7
        REQUIRE_THAT(torus.value({0, 0, 0}), Catch::Matchers::WithinAbs(0.7, 1e-6));

        // The torus should be symmetric about the Z-axis for this orientation
        // Check gradients at various points
        check_gradient(torus, {1.0, 0, 0});
        check_gradient(torus, {0, 1.0, 0});
        check_gradient(torus, {0.5, 0.5, 0.3});
        check_gradient(torus, {-0.5, 0.5, 0.2});
    }

    SECTION("torus - translated center")
    {
        // Torus centered at (1, 2, 3) with default normal
        stf::ImplicitTorus torus(1.0, 0.3, {1, 2, 3});

        // Center should be at distance 0.7
        REQUIRE_THAT(torus.value({1, 2, 3}), Catch::Matchers::WithinAbs(0.7, 1e-6));

        // Point on surface (offset from center)
        REQUIRE_THAT(torus.value({2.3, 2, 3}), Catch::Matchers::WithinAbs(0.0, 1e-5));

        // Check gradients
        check_gradient(torus, {2.3, 2, 3});
        check_gradient(torus, {1, 3, 3.3});
        check_gradient(torus, {0.5, 2, 2.8});
    }

    SECTION("torus - small and large")
    {
        // Small torus
        stf::ImplicitTorus small_torus(0.5, 0.1, {0, 0, 0});
        REQUIRE_THAT(small_torus.value({0, 0, 0}), Catch::Matchers::WithinAbs(0.4, 1e-6));
        check_gradient(small_torus, {0.6, 0, 0});
        check_gradient(small_torus, {0.5, 0, 0.1});

        // Large torus
        stf::ImplicitTorus large_torus(5.0, 1.0, {0, 0, 0});
        REQUIRE_THAT(large_torus.value({0, 0, 0}), Catch::Matchers::WithinAbs(4.0, 1e-6));
        check_gradient(large_torus, {6.0, 0, 0});
        check_gradient(large_torus, {5.0, 0, 1.0});
    }
}
