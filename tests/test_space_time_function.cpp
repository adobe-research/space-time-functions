#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <stf/stf.h>

#include <cmath>

template <int dim>
void check_gradient(
    const stf::SpaceTimeFunction<dim>& fn,
    const std::array<stf::Scalar, dim>& pos,
    const stf::Scalar t,
    const stf::Scalar delta = 1e-6,
    const stf::Scalar epsilon = 1e-6)
{
    auto grad = fn.gradient(pos, t);
    auto grad_fd = fn.finite_difference_gradient(pos, t, delta);
    auto dt = fn.time_derivative(pos, t);
    for (int i = 0; i < dim + 1; ++i) {
        REQUIRE_THAT(grad[i], Catch::Matchers::WithinAbs(grad_fd[i], epsilon));
    }
    REQUIRE_THAT(dt, Catch::Matchers::WithinAbs(grad[dim], epsilon));
}

TEST_CASE("interpolate_function", "[stf]")
{
    SECTION("two balls")
    {
        stf::ImplicitBall<2> ball_1(0.25, {0.1, 0.25});
        stf::ImplicitBall<2> ball_2(0.25, {0.9, 0.25});
        stf::Translation<2> translate({0, -0.5});
        stf::SweepFunction<2> sweep_1(ball_1, translate);
        stf::SweepFunction<2> sweep_2(ball_2, translate);
        stf::InterpolateFunction<2> op(sweep_1, sweep_2);

        REQUIRE_THAT(op.value({0.1, 0.25}, 0), Catch::Matchers::WithinAbs(-0.25, 1e-6));
        REQUIRE_THAT(op.value({0.1, 0.75}, 1), Catch::Matchers::WithinAbs(0.55, 1e-6));
        check_gradient(op, {0.1, 0.25}, 0);
        check_gradient(op, {0.1, 0.25}, 1);
        check_gradient(op, {0.5, 0.5}, 0.5);
        check_gradient(op, {0.1, 0.75}, 0);
        check_gradient(op, {0.1, 0.75}, 1);
    }

    SECTION("nonlinear interpolation")
    {
        stf::ImplicitBall<2> ball_1(0.25, {0.1, 0.25});
        stf::ImplicitBall<2> ball_2(0.25, {0.9, 0.25});
        stf::Translation<2> translate({0, -0.5});
        stf::SweepFunction<2> sweep_1(ball_1, translate);
        stf::SweepFunction<2> sweep_2(ball_2, translate);
        stf::InterpolateFunction<2> op(sweep_1, sweep_2,
            [](stf::Scalar t) { return std::sin(2 * M_PI * t); },
            [](stf::Scalar t) { return 2 * M_PI * std::cos(2 * M_PI * t); });

        check_gradient(op, {0.1, 0.25}, 0, 1e-6, 1e-5);
        check_gradient(op, {0.1, 0.25}, 1, 1e-6, 1e-5);
        check_gradient(op, {0.5, 0.5}, 0.5, 1e-6, 1e-5);
        check_gradient(op, {0.1, 0.75}, 0, 1e-6, 1e-5);
        check_gradient(op, {0.1, 0.75}, 1, 1e-6, 1e-5);
    }
}

TEST_CASE("union_function", "[stf]")
{
    SECTION("two balls")
    {
        stf::ImplicitBall<2> ball_1(0.1, {0.5, 0.0});
        stf::ImplicitBall<2> ball_2(0.1, {0.5, 0.0});
        stf::Translation<2> translate1({-0.2, -0.9});
        stf::Translation<2> translate2({0.2, -0.9});
        stf::SweepFunction<2> sweep_1(ball_1, translate1);
        stf::SweepFunction<2> sweep_2(ball_2, translate2);
        check_gradient(sweep_1, {0, 0}, 0.0);
        check_gradient(sweep_1, {0, 0}, 0.5);
        check_gradient(sweep_2, {0, 0}, 0.0);
        check_gradient(sweep_2, {0, 0}, 0.5);

        stf::UnionFunction<2> op(sweep_1, sweep_2, 0.01);

        // Off-center points
        check_gradient(op, {0.1, 0}, 0);
        check_gradient(op, {0.1, 0}, 0.5);
        check_gradient(op, {0.1, 0}, 1);
        check_gradient(op, {0.51, 0.5}, 0);
        check_gradient(op, {0.51, 0.5}, 0.5);
        check_gradient(op, {0.51, 0.5}, 1);

        // Check gradient on the center plane separating the balls
        for (size_t i=0; i <= 10; ++i) {
            stf::Scalar y = i * 0.1;
            for (size_t j=0; j<=10; ++j) {
                stf::Scalar t = j * 0.1;
                stf::Scalar dt = op.time_derivative({0.5, y}, t);
                stf::Scalar dt1 = sweep_1.time_derivative({0.5, y}, t);
                stf::Scalar dt2 = sweep_2.time_derivative({0.5, y}, t);
                REQUIRE_THAT(dt, Catch::Matchers::WithinAbs((dt1 + dt2) / 2, 1e-6));
                check_gradient(op, {0.5, y}, t);
            }
        }
    }
}
