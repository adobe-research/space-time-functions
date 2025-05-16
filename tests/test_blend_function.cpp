#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <stf/stf.h>

#include <cmath>

template <int dim>
void check_gradient(
    const stf::SpaceTimeFunction<dim>& fn,
    const std::array<stf::Scalar, dim>& pos,
    const stf::Scalar t)
{
    constexpr stf::Scalar epsilon = 1e-6;
    auto grad = fn.gradient(pos, t);
    auto grad_fd = fn.finite_difference_gradient(pos, t);
    for (int i = 0; i < dim + 1; ++i) {
        REQUIRE_THAT(grad[i], Catch::Matchers::WithinAbs(grad_fd[i], epsilon));
    }
}

TEST_CASE("blend_function", "[stf]")
{
    SECTION("two balls") {
        stf::ImplicitBall<2> ball_1(0.25, {0.1, 0.25});
        stf::ImplicitBall<2> ball_2(0.25, {0.9, 0.25});
        stf::Translation<2> translate({0, -0.5});
        stf::SweepFunction<2> sweep_1(ball_1, translate);
        stf::SweepFunction<2> sweep_2(ball_2, translate);
        stf::BlendFunction<2> blend(sweep_1, sweep_2);

        REQUIRE_THAT(blend.value({0.1, 0.25}, 0), Catch::Matchers::WithinAbs(-0.25, 1e-6));
        REQUIRE_THAT(blend.value({0.1, 0.75}, 1), Catch::Matchers::WithinAbs(0.55, 1e-6));
        check_gradient(blend, {0.1, 0.25}, 0);
        check_gradient(blend, {0.1, 0.25}, 1);
        check_gradient(blend, {0.5, 0.5}, 0.5);
        check_gradient(blend, {0.1, 0.75}, 0);
        check_gradient(blend, {0.1, 0.75}, 1);
    }
}
