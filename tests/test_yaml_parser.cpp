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

#endif
