#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <stf/primitives/all.h>

#include <cmath>

template <int dim>
void check_gradient(
    const stf::ImplicitFunction<dim>& implicit,
    const std::array<stf::Scalar, dim>& pos,
    stf::Scalar delta = 1e-6,
    stf::Scalar epsilon = 1e-6
    )
{
    auto grad = implicit.gradient(pos);
    auto grad_fd = implicit.finite_difference_gradient(pos, delta);
    for (int i = 0; i < dim; ++i) {
        REQUIRE_THAT(grad[i], Catch::Matchers::WithinAbs(grad_fd[i], epsilon));
    }
}

TEST_CASE("primitive", "[stf]")
{
    SECTION("ball") {
        stf::ImplicitBall<3> ball(1.0, {0, 0, 0});
        REQUIRE_THAT(ball.value({0, 0, 0}), Catch::Matchers::WithinAbs(-1, 1e-6));
        REQUIRE_THAT(ball.value({1, 0, 0}), Catch::Matchers::WithinAbs(0, 1e-6));
        check_gradient(ball, {0, 0, 0});
        check_gradient(ball, {1, 0, 0});
    }

    SECTION("quadratic ball") {
        stf::ImplicitBall<3> ball(1.0, {0, 0, 0}, 2);
        REQUIRE_THAT(ball.value({0, 0, 0}), Catch::Matchers::WithinAbs(-1, 1e-6));
        REQUIRE_THAT(ball.value({1, 0, 0}), Catch::Matchers::WithinAbs(0, 1e-6));
        check_gradient(ball, {0, 0, 0});
        check_gradient(ball, {1, 0, 0});
    }

    SECTION("ball not at origin") {
        stf::ImplicitBall<3> ball(1.0, {1, 1, 1});
        REQUIRE_THAT(ball.value({0, 0, 0}), Catch::Matchers::WithinAbs(std::sqrt(3) - 1, 1e-6));
        REQUIRE_THAT(ball.value({1, 0, 0}), Catch::Matchers::WithinAbs(std::sqrt(2) - 1, 1e-6));
        check_gradient(ball, {0, 0, 0});
        check_gradient(ball, {1, 0, 0});
    }

    SECTION("quadratic 2D ball not at origin") {
        stf::ImplicitBall<2> ball(1.0, {1, 2}, 2);
        REQUIRE_THAT(ball.value({0, 0}), Catch::Matchers::WithinAbs(4, 1e-6));
        REQUIRE_THAT(ball.value({1, 0}), Catch::Matchers::WithinAbs(3, 1e-6));
        check_gradient(ball, {0, 0});
        check_gradient(ball, {1, 0});
    }

    SECTION("union") {
        stf::ImplicitBall<3> ball_1(0.5, {-0.6, 0, 0});
        stf::ImplicitBall<3> ball_2(0.5, {0.6, 0, 0});
        stf::ImplicitUnion<3> shape(ball_1, ball_2);

        REQUIRE_THAT(shape.value({0, 0, 0}), Catch::Matchers::WithinAbs(0.1, 1e-6));
        REQUIRE_THAT(shape.value({0.5, 0, 0}), Catch::Matchers::WithinAbs(-0.4, 1e-6));
        REQUIRE_THAT(shape.value({-0.5, 0, 0}), Catch::Matchers::WithinAbs(-0.4, 1e-6));
        check_gradient(shape, {0.5, 0, 0});
        check_gradient(shape, {-0.5, 0, 0});
    }

    SECTION("soft union") {
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

    SECTION("capsule") {
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

    SECTION("vipss") {
        stf::Vipss vipss(
            {{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {1, 1, 0}},
            {{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}, {13, 14, 15, 16}},
            {17, 18, 19, 20});
        check_gradient(vipss, {0.1, 0.1, 0.1});
        check_gradient(vipss, {1.0, 0.0, 0.0}, 1e-6, 1e-3);
        check_gradient(vipss, {1.1, -0.1, 0.5});
    }
}
