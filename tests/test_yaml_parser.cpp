#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <stf/stf.h>
#include <sstream>
#include <fstream>
#include <filesystem>

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

TEST_CASE("YamlParser can parse Duchon primitive", "[yaml_parser]") {
    // Create temporary test files for Duchon
    std::string samples_content = "3\n0.0 0.0 0.0\n1.0 0.0 0.0\n0.0 1.0 0.0\n0.0 0.0 1.0\n";
    std::string coeffs_content = "1.0 0.5 0.2 0.1\n0.8 0.3 0.1 0.0\n0.6 0.2 0.0 0.1\n0.4 0.1 0.0 0.0\n0.1 0.2 0.3 0.4\n";
    
    // Write temporary files
    std::ofstream samples_file("test_samples.xyz");
    samples_file << samples_content;
    samples_file.close();
    
    std::ofstream coeffs_file("test_coeffs.txt");
    coeffs_file << coeffs_content;
    coeffs_file.close();
    
    SECTION("Duchon with absolute paths") {
        std::string yaml_content = R"(
type: sweep
dimension: 3
primitive:
  type: duchon
  samples_file: test_samples.xyz
  coeffs_file: test_coeffs.txt
  center: [0.0, 0.0, 0.0]
  radius: 1.0
  positive_inside: false
transform:
  type: translation
  vector: [0.0, 0.0, 0.0]
)";

        auto func = YamlParser<3>::parse_from_string(yaml_content);
        REQUIRE(func != nullptr);
        
        // Test function evaluation
        std::array<Scalar, 3> pos = {0.1, 0.1, 0.1};
        Scalar t = 0.0;
        
        Scalar value = func->value(pos, t);
        REQUIRE(std::isfinite(value));
        
        // Test gradient computation
        auto gradient = func->gradient(pos, t);
        REQUIRE(std::isfinite(gradient[0]));
        REQUIRE(std::isfinite(gradient[1]));
        REQUIRE(std::isfinite(gradient[2]));
        REQUIRE(std::isfinite(gradient[3])); // time derivative
    }
    
    SECTION("Duchon with default parameters") {
        std::string yaml_content = R"(
type: sweep
dimension: 3
primitive:
  type: duchon
  samples_file: test_samples.xyz
  coeffs_file: test_coeffs.txt
  # center, radius, and positive_inside use defaults
transform:
  type: translation
  vector: [0.0, 0.0, 0.0]
)";

        auto func = YamlParser<3>::parse_from_string(yaml_content);
        REQUIRE(func != nullptr);
        
        // Test function evaluation
        std::array<Scalar, 3> pos = {0.0, 0.0, 0.0};
        Scalar t = 0.0;
        
        Scalar value = func->value(pos, t);
        REQUIRE(std::isfinite(value));
    }
    
    SECTION("Duchon in 2D should throw error") {
        std::string yaml_content = R"(
type: sweep
dimension: 2
primitive:
  type: duchon
  samples_file: test_samples.xyz
  coeffs_file: test_coeffs.txt
transform:
  type: translation
  vector: [0.0, 0.0]
)";

        REQUIRE_THROWS_AS(YamlParser<2>::parse_from_string(yaml_content), YamlParseError);
    }
    
    // Clean up temporary files
    std::filesystem::remove("test_samples.xyz");
    std::filesystem::remove("test_coeffs.txt");
}

TEST_CASE("YamlParser handles relative paths correctly", "[yaml_parser]") {
    // Create a subdirectory for test files
    std::filesystem::create_directory("test_data");
    
    // Create test files in subdirectory
    std::string samples_content = "3\n0.0 0.0 0.0\n1.0 0.0 0.0\n0.0 1.0 0.0\n0.0 0.0 1.0\n";
    std::string coeffs_content = "1.0 0.5 0.2 0.1\n0.8 0.3 0.1 0.0\n0.6 0.2 0.0 0.1\n0.4 0.1 0.0 0.0\n0.1 0.2 0.3 0.4\n";
    
    std::ofstream samples_file("test_data/samples.xyz");
    samples_file << samples_content;
    samples_file.close();
    
    std::ofstream coeffs_file("test_data/coeffs.txt");
    coeffs_file << coeffs_content;
    coeffs_file.close();
    
    // Create YAML file with relative paths
    std::string yaml_content = R"(
type: sweep
dimension: 3
primitive:
  type: duchon
  samples_file: samples.xyz
  coeffs_file: coeffs.txt
  center: [0.0, 0.0, 0.0]
  radius: 1.0
transform:
  type: translation
  vector: [0.0, 0.0, 0.0]
)";
    
    std::ofstream yaml_file("test_data/test.yaml");
    yaml_file << yaml_content;
    yaml_file.close();
    
    // Parse from file - relative paths should be resolved relative to YAML file location
    auto func = YamlParser<3>::parse_from_file("test_data/test.yaml");
    REQUIRE(func != nullptr);
    
    // Test function evaluation
    std::array<Scalar, 3> pos = {0.1, 0.1, 0.1};
    Scalar t = 0.0;
    
    Scalar value = func->value(pos, t);
    REQUIRE(std::isfinite(value));
    
    // Clean up
    std::filesystem::remove("test_data/samples.xyz");
    std::filesystem::remove("test_data/coeffs.txt");
    std::filesystem::remove("test_data/test.yaml");
    std::filesystem::remove_all("test_data");
}

TEST_CASE("YamlParser can load polyline points from XYZ file", "[yaml_parser]") {
    // Create test directory and XYZ file
    std::filesystem::create_directory("test_polyline_data");
    
    std::string points_content_2d = "2\n0.0 0.0\n1.0 0.0\n1.0 1.0\n0.0 1.0\n";
    std::string points_content_3d = "3\n0.0 0.0 0.0\n1.0 0.0 0.0\n1.0 1.0 0.0\n0.0 1.0 1.0\n";
    
    std::ofstream points_file_2d("test_polyline_data/points_2d.xyz");
    points_file_2d << points_content_2d;
    points_file_2d.close();
    
    std::ofstream points_file_3d("test_polyline_data/points_3d.xyz");
    points_file_3d << points_content_3d;
    points_file_3d.close();
    
    SECTION("Polyline 2D from XYZ file") {
        std::string yaml_content = R"(
type: sweep
dimension: 2
primitive:
  type: ball
  radius: 0.2
  center: [0.0, 0.0]
  degree: 1
transform:
  type: polyline
  points_file: points_2d.xyz
)";
        
        std::ofstream yaml_file("test_polyline_data/test_2d.yaml");
        yaml_file << yaml_content;
        yaml_file.close();
        
        auto func = YamlParser<2>::parse_from_file("test_polyline_data/test_2d.yaml");
        REQUIRE(func != nullptr);
        
        // Test function evaluation
        std::array<Scalar, 2> pos = {0.5, 0.0};
        Scalar t = 0.25;
        
        Scalar value = func->value(pos, t);
        REQUIRE(std::isfinite(value));
    }
    
    SECTION("Polyline 3D from XYZ file") {
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
  points_file: points_3d.xyz
)";
        
        std::ofstream yaml_file("test_polyline_data/test_3d.yaml");
        yaml_file << yaml_content;
        yaml_file.close();
        
        auto func = YamlParser<3>::parse_from_file("test_polyline_data/test_3d.yaml");
        REQUIRE(func != nullptr);
        
        // Test function evaluation
        std::array<Scalar, 3> pos = {0.5, 0.0, 0.0};
        Scalar t = 0.25;
        
        Scalar value = func->value(pos, t);
        REQUIRE(std::isfinite(value));
    }
    
    SECTION("Polyline with dimension mismatch should throw error") {
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
  points_file: points_2d.xyz
)";
        
        std::ofstream yaml_file("test_polyline_data/test_mismatch.yaml");
        yaml_file << yaml_content;
        yaml_file.close();
        
        REQUIRE_THROWS_AS(YamlParser<3>::parse_from_file("test_polyline_data/test_mismatch.yaml"), YamlParseError);
    }
    
    // Clean up
    std::filesystem::remove("test_polyline_data/points_2d.xyz");
    std::filesystem::remove("test_polyline_data/points_3d.xyz");
    std::filesystem::remove("test_polyline_data/test_2d.yaml");
    std::filesystem::remove("test_polyline_data/test_3d.yaml");
    std::filesystem::remove("test_polyline_data/test_mismatch.yaml");
    std::filesystem::remove_all("test_polyline_data");
}

TEST_CASE("YamlParser can load polybezier points from XYZ file", "[yaml_parser]") {
    // Create test directory and XYZ files
    std::filesystem::create_directory("test_bezier_data");
    
    // Control points for a simple cubic Bezier curve (4 points)
    std::string control_points_content = "3\n0.0 0.0 0.0\n0.5 0.0 0.0\n0.5 0.5 0.0\n1.0 0.5 0.0\n";
    
    // Sample points for curve fitting
    std::string sample_points_content = "3\n0.0 0.0 0.0\n0.25 0.1 0.0\n0.5 0.3 0.0\n0.75 0.4 0.0\n1.0 0.5 0.0\n";
    
    std::ofstream control_file("test_bezier_data/control_points.xyz");
    control_file << control_points_content;
    control_file.close();
    
    std::ofstream sample_file("test_bezier_data/sample_points.xyz");
    sample_file << sample_points_content;
    sample_file.close();
    
    SECTION("PolyBezier from control points file") {
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
  control_points_file: control_points.xyz
  follow_tangent: true
)";
        
        std::ofstream yaml_file("test_bezier_data/test_control.yaml");
        yaml_file << yaml_content;
        yaml_file.close();
        
        auto func = YamlParser<3>::parse_from_file("test_bezier_data/test_control.yaml");
        REQUIRE(func != nullptr);
        
        // Test function evaluation
        std::array<Scalar, 3> pos = {0.5, 0.25, 0.0};
        Scalar t = 0.5;
        
        Scalar value = func->value(pos, t);
        REQUIRE(std::isfinite(value));
    }
    
    SECTION("PolyBezier from sample points file") {
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
  sample_points_file: sample_points.xyz
  follow_tangent: false
)";
        
        std::ofstream yaml_file("test_bezier_data/test_sample.yaml");
        yaml_file << yaml_content;
        yaml_file.close();
        
        auto func = YamlParser<3>::parse_from_file("test_bezier_data/test_sample.yaml");
        REQUIRE(func != nullptr);
        
        // Test function evaluation
        std::array<Scalar, 3> pos = {0.5, 0.3, 0.0};
        Scalar t = 0.5;
        
        Scalar value = func->value(pos, t);
        REQUIRE(std::isfinite(value));
    }
    
    SECTION("PolyBezier with insufficient control points should throw error") {
        // Create file with only 3 control points (need at least 4)
        std::string insufficient_points = "3\n0.0 0.0 0.0\n0.5 0.0 0.0\n1.0 0.5 0.0\n";
        
        std::ofstream insufficient_file("test_bezier_data/insufficient.xyz");
        insufficient_file << insufficient_points;
        insufficient_file.close();
        
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
  control_points_file: insufficient.xyz
)";
        
        std::ofstream yaml_file("test_bezier_data/test_insufficient.yaml");
        yaml_file << yaml_content;
        yaml_file.close();
        
        REQUIRE_THROWS_AS(YamlParser<3>::parse_from_file("test_bezier_data/test_insufficient.yaml"), YamlParseError);
        
        std::filesystem::remove("test_bezier_data/insufficient.xyz");
        std::filesystem::remove("test_bezier_data/test_insufficient.yaml");
    }
    
    // Clean up
    std::filesystem::remove("test_bezier_data/control_points.xyz");
    std::filesystem::remove("test_bezier_data/sample_points.xyz");
    std::filesystem::remove("test_bezier_data/test_control.yaml");
    std::filesystem::remove("test_bezier_data/test_sample.yaml");
    std::filesystem::remove_all("test_bezier_data");
}

TEST_CASE("YamlParser handles missing XYZ files gracefully", "[yaml_parser]") {
    SECTION("Missing polyline points file") {
        std::string yaml_content = R"(
type: sweep
dimension: 2
primitive:
  type: ball
  radius: 0.2
  center: [0.0, 0.0]
  degree: 1
transform:
  type: polyline
  points_file: nonexistent.xyz
)";

        REQUIRE_THROWS_AS(YamlParser<2>::parse_from_string(yaml_content), YamlParseError);
    }
    
    SECTION("Missing polybezier control points file") {
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
  control_points_file: nonexistent.xyz
)";

        REQUIRE_THROWS_AS(YamlParser<3>::parse_from_string(yaml_content), YamlParseError);
    }
}

TEST_CASE("YamlParser can parse implicit union primitive", "[yaml_parser]") {
    SECTION("Simple implicit union with two balls") {
        std::string yaml_content = R"(
type: sweep
dimension: 3
primitive:
  type: implicit_union
  primitives:
    - type: ball
      radius: 0.5
      center: [0.0, 0.0, 0.0]
      degree: 1
    - type: ball
      radius: 0.3
      center: [0.8, 0.0, 0.0]
      degree: 1
  smooth_distance: 0.2
  blending: quadratic
transform:
  type: translation
  vector: [0.0, 0.0, 0.0]
)";

        auto func = YamlParser<3>::parse_from_string(yaml_content);
        REQUIRE(func != nullptr);
        
        // Test function evaluation
        std::array<Scalar, 3> pos = {0.4, 0.0, 0.0};
        Scalar t = 0.0;
        
        Scalar value = func->value(pos, t);
        REQUIRE(std::isfinite(value));
        
        // Test gradient computation
        auto gradient = func->gradient(pos, t);
        REQUIRE(std::isfinite(gradient[0]));
        REQUIRE(std::isfinite(gradient[1]));
        REQUIRE(std::isfinite(gradient[2]));
        REQUIRE(std::isfinite(gradient[3])); // time derivative
    }
    
    SECTION("Implicit union with different blending functions") {
        std::vector<std::string> blending_functions = {"quadratic", "cubic", "quartic", "circular"};
        
        for (const auto& blending : blending_functions) {
            std::string yaml_content = R"(
type: sweep
dimension: 2
primitive:
  type: implicit_union
  primitives:
    - type: ball
      radius: 0.4
      center: [0.0, 0.0]
      degree: 1
    - type: ball
      radius: 0.3
      center: [0.6, 0.0]
      degree: 1
  smooth_distance: 0.1
  blending: )" + blending + R"(
transform:
  type: translation
  vector: [0.0, 0.0]
)";

            auto func = YamlParser<2>::parse_from_string(yaml_content);
            REQUIRE(func != nullptr);
            
            // Test function evaluation
            std::array<Scalar, 2> pos = {0.3, 0.0};
            Scalar t = 0.0;
            
            Scalar value = func->value(pos, t);
            REQUIRE(std::isfinite(value));
        }
    }
    
    SECTION("Implicit union with multiple primitives") {
        std::string yaml_content = R"(
type: sweep
dimension: 3
primitive:
  type: implicit_union
  primitives:
    - type: ball
      radius: 0.3
      center: [0.0, 0.0, 0.0]
      degree: 1
    - type: ball
      radius: 0.2
      center: [0.5, 0.0, 0.0]
      degree: 1
    - type: ball
      radius: 0.25
      center: [0.0, 0.5, 0.0]
      degree: 1
    - type: capsule
      start: [0.0, 0.0, 0.0]
      end: [0.0, 0.0, 0.5]
      radius: 0.1
  smooth_distance: 0.15
  blending: circular
transform:
  type: translation
  vector: [0.0, 0.0, 0.0]
)";

        auto func = YamlParser<3>::parse_from_string(yaml_content);
        REQUIRE(func != nullptr);
        
        // Test function evaluation
        std::array<Scalar, 3> pos = {0.1, 0.1, 0.1};
        Scalar t = 0.0;
        
        Scalar value = func->value(pos, t);
        REQUIRE(std::isfinite(value));
    }
    
    SECTION("Implicit union with default parameters") {
        std::string yaml_content = R"(
type: sweep
dimension: 2
primitive:
  type: implicit_union
  primitives:
    - type: ball
      radius: 0.4
      center: [0.0, 0.0]
      degree: 1
    - type: ball
      radius: 0.3
      center: [0.6, 0.0]
      degree: 1
  # smooth_distance and blending use defaults
transform:
  type: translation
  vector: [0.0, 0.0]
)";

        auto func = YamlParser<2>::parse_from_string(yaml_content);
        REQUIRE(func != nullptr);
        
        // Test function evaluation
        std::array<Scalar, 2> pos = {0.3, 0.0};
        Scalar t = 0.0;
        
        Scalar value = func->value(pos, t);
        REQUIRE(std::isfinite(value));
    }
    
    SECTION("Implicit union with hard union (smooth_distance = 0)") {
        std::string yaml_content = R"(
type: sweep
dimension: 3
primitive:
  type: implicit_union
  primitives:
    - type: ball
      radius: 0.5
      center: [0.0, 0.0, 0.0]
      degree: 1
    - type: ball
      radius: 0.3
      center: [1.0, 0.0, 0.0]
      degree: 1
  smooth_distance: 0.0
  blending: quadratic
transform:
  type: translation
  vector: [0.0, 0.0, 0.0]
)";

        auto func = YamlParser<3>::parse_from_string(yaml_content);
        REQUIRE(func != nullptr);
        
        // Test function evaluation
        std::array<Scalar, 3> pos = {0.5, 0.0, 0.0};
        Scalar t = 0.0;
        
        Scalar value = func->value(pos, t);
        REQUIRE(std::isfinite(value));
    }
}

TEST_CASE("YamlParser handles implicit union errors correctly", "[yaml_parser]") {
    SECTION("Implicit union with insufficient primitives") {
        std::string yaml_content = R"(
type: sweep
dimension: 2
primitive:
  type: implicit_union
  primitives:
    - type: ball
      radius: 0.4
      center: [0.0, 0.0]
      degree: 1
  # Only one primitive - need at least 2
transform:
  type: translation
  vector: [0.0, 0.0]
)";

        REQUIRE_THROWS_AS(YamlParser<2>::parse_from_string(yaml_content), YamlParseError);
    }
    
    SECTION("Implicit union with unknown blending function") {
        std::string yaml_content = R"(
type: sweep
dimension: 2
primitive:
  type: implicit_union
  primitives:
    - type: ball
      radius: 0.4
      center: [0.0, 0.0]
      degree: 1
    - type: ball
      radius: 0.3
      center: [0.6, 0.0]
      degree: 1
  blending: unknown_blending
transform:
  type: translation
  vector: [0.0, 0.0]
)";

        REQUIRE_THROWS_AS(YamlParser<2>::parse_from_string(yaml_content), YamlParseError);
    }
    
    SECTION("Implicit union with non-sequence primitives field") {
        std::string yaml_content = R"(
type: sweep
dimension: 2
primitive:
  type: implicit_union
  primitives: not_a_sequence
transform:
  type: translation
  vector: [0.0, 0.0]
)";

        REQUIRE_THROWS_AS(YamlParser<2>::parse_from_string(yaml_content), YamlParseError);
    }
}

#endif
