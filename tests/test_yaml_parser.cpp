#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <stf/stf.h>
#include <sstream>
#include <fstream>

using namespace stf;

#ifdef STF_YAML_PARSER_ENABLED

TEST_CASE("YamlParser can parse simple sweep function with ball primitive", "[yaml_parser]") {
    std::string yaml_content = R"(
type: sweep
dimension: 3
primitive:
  type: ball
  radius: 0.5
  center: [0.0, 0.0, 0.0]
  degree: 1
transform:
  type: translation
  vector: [-1.0, 0.0, 0.0]
)";

    auto func = YamlParser<3>::parse_from_string(yaml_content);
    REQUIRE(func != nullptr);
    
    // Test function evaluation
    std::array<Scalar, 3> pos = {0.5, 0.0, 0.0};
    Scalar t = 0.5;
    
    Scalar value = func->value(pos, t);
    // At t=0.5, the ball center should be at (0.5, 0, 0), so distance from (0.5, 0, 0) should be -0.5
    REQUIRE(value == Catch::Approx(-0.5).epsilon(1e-6));
}

TEST_CASE("YamlParser can parse sweep function with capsule primitive", "[yaml_parser]") {
    std::string yaml_content = R"(
type: sweep
dimension: 3
primitive:
  type: capsule
  radius: 0.2
  start: [0.0, 0.0, 0.0]
  end: [0.0, 0.0, 1.0]
transform:
  type: translation
  vector: [0.0, 1.0, 0.0]
)";

    auto func = YamlParser<3>::parse_from_string(yaml_content);
    REQUIRE(func != nullptr);
    
    // Test that the function can be evaluated
    std::array<Scalar, 3> pos = {0.0, 0.5, 0.5};
    Scalar t = 0.5;
    
    Scalar value = func->value(pos, t);
    // Just check that it returns a reasonable value
    REQUIRE(std::isfinite(value));
}

TEST_CASE("YamlParser can parse sweep function with torus primitive", "[yaml_parser]") {
    std::string yaml_content = R"(
type: sweep
dimension: 3
primitive:
  type: torus
  major_radius: 1.0
  minor_radius: 0.3
  center: [0.0, 0.0, 0.0]
transform:
  type: scale
  factors: [2.0, 2.0, 2.0]
  center: [0.0, 0.0, 0.0]
)";

    auto func = YamlParser<3>::parse_from_string(yaml_content);
    REQUIRE(func != nullptr);
    
    // Test that the function can be evaluated
    std::array<Scalar, 3> pos = {1.0, 0.0, 0.0};
    Scalar t = 0.5;
    
    Scalar value = func->value(pos, t);
    REQUIRE(std::isfinite(value));
}

TEST_CASE("YamlParser can parse sweep function with rotation transform", "[yaml_parser]") {
    std::string yaml_content = R"(
type: sweep
dimension: 3
primitive:
  type: ball
  radius: 0.5
  center: [1.0, 0.0, 0.0]
  degree: 1
transform:
  type: rotation
  axis: [0.0, 0.0, 1.0]
  angle: 90.0
  center: [0.0, 0.0, 0.0]
)";

    auto func = YamlParser<3>::parse_from_string(yaml_content);
    REQUIRE(func != nullptr);
    
    // Test that the function can be evaluated
    std::array<Scalar, 3> pos = {0.0, 1.0, 0.0};
    Scalar t = 1.0;
    
    Scalar value = func->value(pos, t);
    REQUIRE(std::isfinite(value));
}

TEST_CASE("YamlParser can parse 2D sweep function", "[yaml_parser]") {
    std::string yaml_content = R"(
type: sweep
dimension: 2
primitive:
  type: ball
  radius: 0.5
  center: [0.0, 0.0]
  degree: 1
transform:
  type: translation
  vector: [-1.0, 0.0]
)";

    auto func = YamlParser<2>::parse_from_string(yaml_content);
    REQUIRE(func != nullptr);
    
    // Test function evaluation
    std::array<Scalar, 2> pos = {0.5, 0.0};
    Scalar t = 0.5;
    
    Scalar value = func->value(pos, t);
    // At t=0.5, the circle center should be at (0.5, 0), so distance from (0.5, 0) should be -0.5
    REQUIRE(value == Catch::Approx(-0.5).epsilon(1e-6));
}

TEST_CASE("YamlParser throws error for missing required fields", "[yaml_parser]") {
    std::string yaml_content = R"(
type: sweep
dimension: 3
primitive:
  type: ball
  radius: 0.5
  # Missing center and degree
transform:
  type: translation
  vector: [1.0, 0.0, 0.0]
)";

    REQUIRE_THROWS_AS(YamlParser<3>::parse_from_string(yaml_content), YamlParseError);
}

TEST_CASE("YamlParser throws error for unknown function type", "[yaml_parser]") {
    std::string yaml_content = R"(
type: unknown_type
dimension: 3
)";

    REQUIRE_THROWS_AS(YamlParser<3>::parse_from_string(yaml_content), YamlParseError);
}

TEST_CASE("YamlParser throws error for unknown primitive type", "[yaml_parser]") {
    std::string yaml_content = R"(
type: sweep
dimension: 3
primitive:
  type: unknown_primitive
transform:
  type: translation
  vector: [1.0, 0.0, 0.0]
)";

    REQUIRE_THROWS_AS(YamlParser<3>::parse_from_string(yaml_content), YamlParseError);
}

TEST_CASE("YamlParser throws error for dimension mismatch", "[yaml_parser]") {
    std::string yaml_content = R"(
type: sweep
dimension: 2
primitive:
  type: ball
  radius: 0.5
  center: [0.0, 0.0, 0.0]  # 3D center for 2D function
  degree: 1
transform:
  type: translation
  vector: [1.0, 0.0]
)";

    REQUIRE_THROWS_AS(YamlParser<2>::parse_from_string(yaml_content), YamlParseError);
}

TEST_CASE("YamlParser can parse composed transforms", "[yaml_parser]") {
    std::string yaml_content = R"(
type: sweep
dimension: 3
primitive:
  type: ball
  radius: 0.5
  center: [0.0, 0.0, 0.0]
  degree: 1
transform:
  type: compose
  transforms:
    - type: translation
      vector: [1.0, 0.0, 0.0]
    - type: scale
      factors: [2.0, 1.0, 1.0]
      center: [0.0, 0.0, 0.0]
)";

    auto func = YamlParser<3>::parse_from_string(yaml_content);
    REQUIRE(func != nullptr);
    
    // Test that the function can be evaluated
    std::array<Scalar, 3> pos = {1.0, 0.0, 0.0};
    Scalar t = 0.5;
    
    Scalar value = func->value(pos, t);
    REQUIRE(std::isfinite(value));
}

TEST_CASE("YamlParser can parse from file", "[yaml_parser]") {
    // Create a temporary YAML file
    std::string filename = "test_function.yaml";
    std::ofstream file(filename);
    file << R"(
type: sweep
dimension: 3
primitive:
  type: ball
  radius: 0.5
  center: [0.0, 0.0, 0.0]
  degree: 1
transform:
  type: translation
  vector: [1.0, 0.0, 0.0]
)";
    file.close();

    auto func = YamlParser<3>::parse_from_file(filename);
    REQUIRE(func != nullptr);
    
    // Clean up
    std::remove(filename.c_str());
}

TEST_CASE("YamlParser convenience functions work", "[yaml_parser]") {
    std::string yaml_content = R"(
type: sweep
dimension: 3
primitive:
  type: ball
  radius: 0.5
  center: [0.0, 0.0, 0.0]
  degree: 1
transform:
  type: translation
  vector: [1.0, 0.0, 0.0]
)";

    auto func = parse_space_time_function_from_string<3>(yaml_content);
    REQUIRE(func != nullptr);
}

TEST_CASE("YamlParser can parse polyline transform", "[yaml_parser]") {
    std::string yaml_content = R"(
type: sweep
dimension: 3
primitive:
  type: ball
  radius: 0.2
  center: [0.0, 0.0, 0.0]
  degree: 1
transform:
  type: polyline
  points:
    - [0.0, 0.0, 0.0]
    - [1.0, 0.0, 0.0]
    - [1.0, 1.0, 0.0]
    - [1.0, 1.0, 1.0]
)";

    auto func = YamlParser<3>::parse_from_string(yaml_content);
    REQUIRE(func != nullptr);
    
    // Test function evaluation at different times
    std::array<Scalar, 3> pos = {0.5, 0.0, 0.0};
    Scalar t = 0.25;
    
    Scalar value = func->value(pos, t);
    REQUIRE(std::isfinite(value));
}

TEST_CASE("YamlParser can parse 2D polyline transform", "[yaml_parser]") {
    std::string yaml_content = R"(
type: sweep
dimension: 2
primitive:
  type: ball
  radius: 0.3
  center: [0.0, 0.0]
  degree: 1
transform:
  type: polyline
  points:
    - [0.0, 0.0]
    - [2.0, 0.0]
    - [2.0, 2.0]
    - [0.0, 2.0]
)";

    auto func = YamlParser<2>::parse_from_string(yaml_content);
    REQUIRE(func != nullptr);
    
    // Test function evaluation
    std::array<Scalar, 2> pos = {1.0, 0.0};
    Scalar t = 0.25;
    
    Scalar value = func->value(pos, t);
    REQUIRE(std::isfinite(value));
}

TEST_CASE("YamlParser can parse polybezier with control points", "[yaml_parser]") {
    std::string yaml_content = R"(
type: sweep
dimension: 3
primitive:
  type: ball
  radius: 0.15
  center: [0.0, 0.0, 0.0]
  degree: 1
transform:
  type: polybezier
  control_points:
    - [0.0, 0.0, 0.0]
    - [0.5, 0.0, 0.0]
    - [0.5, 0.5, 0.0]
    - [1.0, 0.5, 0.0]
  follow_tangent: true
)";

    auto func = YamlParser<3>::parse_from_string(yaml_content);
    REQUIRE(func != nullptr);
    
    // Test function evaluation
    std::array<Scalar, 3> pos = {0.5, 0.25, 0.0};
    Scalar t = 0.5;
    
    Scalar value = func->value(pos, t);
    REQUIRE(std::isfinite(value));
}

TEST_CASE("YamlParser can parse polybezier from sample points", "[yaml_parser]") {
    std::string yaml_content = R"(
type: sweep
dimension: 3
primitive:
  type: ball
  radius: 0.1
  center: [0.0, 0.0, 0.0]
  degree: 1
transform:
  type: polybezier
  sample_points:
    - [0.0, 0.0, 0.0]
    - [1.0, 0.0, 0.5]
    - [2.0, 1.0, 0.5]
    - [2.5, 2.0, 0.0]
  follow_tangent: false
)";

    auto func = YamlParser<3>::parse_from_string(yaml_content);
    REQUIRE(func != nullptr);
    
    // Test function evaluation
    std::array<Scalar, 3> pos = {1.0, 0.5, 0.25};
    Scalar t = 0.5;
    
    Scalar value = func->value(pos, t);
    REQUIRE(std::isfinite(value));
}

TEST_CASE("YamlParser throws error for invalid polyline", "[yaml_parser]") {
    std::string yaml_content = R"(
type: sweep
dimension: 3
primitive:
  type: ball
  radius: 0.2
  center: [0.0, 0.0, 0.0]
  degree: 1
transform:
  type: polyline
  points:
    - [0.0, 0.0, 0.0]
    # Only one point - should fail
)";

    REQUIRE_THROWS_AS(YamlParser<3>::parse_from_string(yaml_content), YamlParseError);
}

TEST_CASE("YamlParser throws error for invalid polybezier control points", "[yaml_parser]") {
    std::string yaml_content = R"(
type: sweep
dimension: 3
primitive:
  type: ball
  radius: 0.15
  center: [0.0, 0.0, 0.0]
  degree: 1
transform:
  type: polybezier
  control_points:
    - [0.0, 0.0, 0.0]
    - [0.5, 0.0, 0.0]
    # Only 2 points - should fail (need at least 4)
)";

    REQUIRE_THROWS_AS(YamlParser<3>::parse_from_string(yaml_content), YamlParseError);
}

TEST_CASE("YamlParser throws error for polybezier with wrong number of control points", "[yaml_parser]") {
    std::string yaml_content = R"(
type: sweep
dimension: 3
primitive:
  type: ball
  radius: 0.15
  center: [0.0, 0.0, 0.0]
  degree: 1
transform:
  type: polybezier
  control_points:
    - [0.0, 0.0, 0.0]
    - [0.5, 0.0, 0.0]
    - [0.5, 0.5, 0.0]
    - [1.0, 0.5, 0.0]
    - [1.5, 0.5, 0.0]
    # 5 points - should fail (need (n*3)+1 points)
)";

    REQUIRE_THROWS_AS(YamlParser<3>::parse_from_string(yaml_content), YamlParseError);
}

TEST_CASE("YamlParser throws error for polybezier with insufficient sample points", "[yaml_parser]") {
    std::string yaml_content = R"(
type: sweep
dimension: 3
primitive:
  type: ball
  radius: 0.1
  center: [0.0, 0.0, 0.0]
  degree: 1
transform:
  type: polybezier
  sample_points:
    - [0.0, 0.0, 0.0]
    - [1.0, 0.0, 0.5]
    # Only 2 sample points - should fail (need at least 3)
)";

    REQUIRE_THROWS_AS(YamlParser<3>::parse_from_string(yaml_content), YamlParseError);
}

TEST_CASE("YamlParser can parse interpolate function with linear interpolation", "[yaml_parser]") {
    std::string yaml_content = R"(
type: interpolate
dimension: 3
function1:
  type: sweep
  primitive:
    type: ball
    radius: 0.3
    center: [0.0, 0.0, 0.0]
    degree: 1
  transform:
    type: translation
    vector: [1.0, 0.0, 0.0]
function2:
  type: sweep
  primitive:
    type: ball
    radius: 0.5
    center: [0.0, 0.0, 0.0]
    degree: 1
  transform:
    type: translation
    vector: [0.0, 1.0, 0.0]
interpolation_type: linear
)";

    auto func = YamlParser<3>::parse_from_string(yaml_content);
    REQUIRE(func != nullptr);
    
    // Test function evaluation at different interpolation points
    std::array<Scalar, 3> pos = {0.0, 0.0, 0.0};
    
    // At t=0, should be closer to function1
    Scalar value_0 = func->value(pos, 0.0);
    REQUIRE(std::isfinite(value_0));
    
    // At t=1, should be closer to function2
    Scalar value_1 = func->value(pos, 1.0);
    REQUIRE(std::isfinite(value_1));
    
    // At t=0.5, should be interpolated
    Scalar value_half = func->value(pos, 0.5);
    REQUIRE(std::isfinite(value_half));
}

TEST_CASE("YamlParser can parse interpolate function with smooth interpolation", "[yaml_parser]") {
    std::string yaml_content = R"(
type: interpolate
dimension: 2
function1:
  type: sweep
  primitive:
    type: ball
    radius: 0.2
    center: [0.0, 0.0]
    degree: 1
  transform:
    type: translation
    vector: [1.0, 0.0]
function2:
  type: sweep
  primitive:
    type: ball
    radius: 0.4
    center: [0.0, 0.0]
    degree: 1
  transform:
    type: translation
    vector: [0.0, 1.0]
interpolation_type: smooth
)";

    auto func = YamlParser<2>::parse_from_string(yaml_content);
    REQUIRE(func != nullptr);
    
    // Test function evaluation
    std::array<Scalar, 2> pos = {0.5, 0.5};
    Scalar t = 0.5;
    
    Scalar value = func->value(pos, t);
    REQUIRE(std::isfinite(value));
    
    // Test gradient computation
    auto gradient = func->gradient(pos, t);
    REQUIRE(gradient.size() == 3); // [df/dx, df/dy, df/dt]
    REQUIRE(std::isfinite(gradient[0]));
    REQUIRE(std::isfinite(gradient[1]));
    REQUIRE(std::isfinite(gradient[2]));
}

TEST_CASE("YamlParser can parse interpolate function with cosine interpolation", "[yaml_parser]") {
    std::string yaml_content = R"(
type: interpolate
dimension: 3
function1:
  type: sweep
  primitive:
    type: ball
    radius: 0.3
    center: [1.0, 0.0, 0.0]
    degree: 1
  transform:
    type: scale
    factors: [1.0, 1.0, 1.0]
    center: [0.0, 0.0, 0.0]
function2:
  type: sweep
  primitive:
    type: ball
    radius: 0.3
    center: [-1.0, 0.0, 0.0]
    degree: 1
  transform:
    type: scale
    factors: [2.0, 2.0, 2.0]
    center: [0.0, 0.0, 0.0]
interpolation_type: cosine
)";

    auto func = YamlParser<3>::parse_from_string(yaml_content);
    REQUIRE(func != nullptr);
    
    // Test function evaluation
    std::array<Scalar, 3> pos = {0.0, 0.0, 0.0};
    Scalar t = 0.25;
    
    Scalar value = func->value(pos, t);
    REQUIRE(std::isfinite(value));
    
    // Test time derivative
    Scalar time_deriv = func->time_derivative(pos, t);
    REQUIRE(std::isfinite(time_deriv));
}

TEST_CASE("YamlParser can parse interpolate function with default linear interpolation", "[yaml_parser]") {
    std::string yaml_content = R"(
type: interpolate
dimension: 3
function1:
  type: sweep
  primitive:
    type: ball
    radius: 0.2
    center: [0.0, 0.0, 0.0]
    degree: 1
  transform:
    type: translation
    vector: [1.0, 0.0, 0.0]
function2:
  type: sweep
  primitive:
    type: ball
    radius: 0.2
    center: [0.0, 0.0, 0.0]
    degree: 1
  transform:
    type: translation
    vector: [-1.0, 0.0, 0.0]
# No interpolation_type specified - should default to linear
)";

    auto func = YamlParser<3>::parse_from_string(yaml_content);
    REQUIRE(func != nullptr);
    
    // Test function evaluation
    std::array<Scalar, 3> pos = {0.0, 0.0, 0.0};
    Scalar t = 0.5;
    
    Scalar value = func->value(pos, t);
    REQUIRE(std::isfinite(value));
}

TEST_CASE("YamlParser throws error for interpolate function with missing functions", "[yaml_parser]") {
    std::string yaml_content = R"(
type: interpolate
dimension: 3
function1:
  type: sweep
  primitive:
    type: ball
    radius: 0.2
    center: [0.0, 0.0, 0.0]
    degree: 1
  transform:
    type: translation
    vector: [1.0, 0.0, 0.0]
# Missing function2 - should fail
)";

    REQUIRE_THROWS_AS(YamlParser<3>::parse_from_string(yaml_content), YamlParseError);
}

TEST_CASE("YamlParser throws error for interpolate function with unknown interpolation type", "[yaml_parser]") {
    std::string yaml_content = R"(
type: interpolate
dimension: 3
function1:
  type: sweep
  primitive:
    type: ball
    radius: 0.2
    center: [0.0, 0.0, 0.0]
    degree: 1
  transform:
    type: translation
    vector: [1.0, 0.0, 0.0]
function2:
  type: sweep
  primitive:
    type: ball
    radius: 0.2
    center: [0.0, 0.0, 0.0]
    degree: 1
  transform:
    type: translation
    vector: [-1.0, 0.0, 0.0]
interpolation_type: unknown_type
)";

    REQUIRE_THROWS_AS(YamlParser<3>::parse_from_string(yaml_content), YamlParseError);
}

TEST_CASE("YamlParser handles optional degree parameter for ball primitive", "[yaml_parser]") {
    SECTION("Ball with explicit degree parameter") {
        std::string yaml_content = R"(
type: sweep
dimension: 2
primitive:
  type: ball
  radius: 1.0
  center: [0.0, 0.0]
  degree: 2
transform:
  type: translation
  vector: [0.0, 0.0]
)";

        auto func = YamlParser<2>::parse_from_string(yaml_content);
        REQUIRE(func != nullptr);
        
        // Test function evaluation - with degree 2, the ball should have different behavior
        std::array<Scalar, 2> pos = {0.5, 0.0};
        Scalar t = 0.0;
        
        Scalar value = func->value(pos, t);
        // For degree 2, the distance function is different from degree 1
        REQUIRE(std::isfinite(value));
    }
    
    SECTION("Ball with default degree parameter (should be 1)") {
        std::string yaml_content = R"(
type: sweep
dimension: 2
primitive:
  type: ball
  radius: 1.0
  center: [0.0, 0.0]
  # degree parameter omitted - should default to 1
transform:
  type: translation
  vector: [0.0, 0.0]
)";

        auto func = YamlParser<2>::parse_from_string(yaml_content);
        REQUIRE(func != nullptr);
        
        // Test function evaluation - with default degree 1
        std::array<Scalar, 2> pos = {0.5, 0.0};
        Scalar t = 0.0;
        
        Scalar value = func->value(pos, t);
        // For degree 1 (default), distance from center (0,0) to (0.5,0) should be 0.5 - 1.0 = -0.5
        REQUIRE(value == Catch::Approx(-0.5).epsilon(1e-6));
    }
    
    SECTION("Compare explicit degree=1 with default degree") {
        std::string yaml_explicit = R"(
type: sweep
dimension: 2
primitive:
  type: ball
  radius: 1.0
  center: [0.0, 0.0]
  degree: 1
transform:
  type: translation
  vector: [0.0, 0.0]
)";

        std::string yaml_default = R"(
type: sweep
dimension: 2
primitive:
  type: ball
  radius: 1.0
  center: [0.0, 0.0]
  # degree parameter omitted - should default to 1
transform:
  type: translation
  vector: [0.0, 0.0]
)";

        auto func_explicit = YamlParser<2>::parse_from_string(yaml_explicit);
        auto func_default = YamlParser<2>::parse_from_string(yaml_default);
        
        REQUIRE(func_explicit != nullptr);
        REQUIRE(func_default != nullptr);
        
        // Both should give the same result
        std::array<Scalar, 2> pos = {0.5, 0.0};
        Scalar t = 0.0;
        
        Scalar value_explicit = func_explicit->value(pos, t);
        Scalar value_default = func_default->value(pos, t);
        
        REQUIRE(value_explicit == Catch::Approx(value_default).epsilon(1e-10));
    }
}

#endif
