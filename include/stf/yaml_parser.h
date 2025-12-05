#pragma once

#ifdef STF_YAML_PARSER_ENABLED

#include <stf/explicit_form.h>
#include <stf/offset_function.h>
#include <stf/space_time_function.h>
#include <stf/sweep_function.h>
#include <stf/union_function.h>
#include <stf/primitives/duchon.h>
#include <stf/primitives/implicit_union.h>
#include <yaml-cpp/yaml.h>

#include <cassert>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace stf {

/**
 * @brief Exception thrown when YAML parsing fails
 */
class YamlParseError : public std::runtime_error
{
public:
    explicit YamlParseError(const std::string& message)
        : std::runtime_error("YAML Parse Error: " + message)
    {}
};

/**
 * @brief Parsing context that manages object lifetimes
 *
 * This class holds all the parsed primitives and transforms to ensure they
 * remain alive for the lifetime of the space-time function that uses them.
 */
template <int dim>
class Context
{
public:
    std::vector<std::unique_ptr<ImplicitFunction<dim>>> primitives;
    std::vector<std::unique_ptr<Transform<dim>>> transforms;
    std::vector<std::unique_ptr<SpaceTimeFunction<dim>>> functions;
    
    // Add objects and return raw pointers for use
    ImplicitFunction<dim>* add_primitive(std::unique_ptr<ImplicitFunction<dim>> primitive) {
        auto* ptr = primitive.get();
        primitives.push_back(std::move(primitive));
        return ptr;
    }
    
    Transform<dim>* add_transform(std::unique_ptr<Transform<dim>> transform) {
        auto* ptr = transform.get();
        transforms.push_back(std::move(transform));
        return ptr;
    }
    
    SpaceTimeFunction<dim>* add_function(std::unique_ptr<SpaceTimeFunction<dim>> function) {
        auto* ptr = function.get();
        functions.push_back(std::move(function));
        return ptr;
    }
};

/**
 * @brief Space-time function wrapper that manages object lifetimes
 *
 * This wrapper ensures that all parsed objects (primitives, transforms, etc.)
 * remain alive for the lifetime of the space-time function.
 */
template <int dim>
class ManagedSpaceTimeFunction : public SpaceTimeFunction<dim>
{
public:
    ManagedSpaceTimeFunction(
        std::unique_ptr<SpaceTimeFunction<dim>> function,
        std::unique_ptr<Context<dim>> context)
        : m_function(std::move(function))
        , m_context(std::move(context))
    {}

    Scalar value(std::array<Scalar, dim> pos, Scalar t) const override {
        return m_function->value(pos, t);
    }

    Scalar time_derivative(std::array<Scalar, dim> pos, Scalar t) const override {
        return m_function->time_derivative(pos, t);
    }

    std::array<Scalar, dim + 1> gradient(std::array<Scalar, dim> pos, Scalar t) const override {
        return m_function->gradient(pos, t);
    }

private:
    std::unique_ptr<SpaceTimeFunction<dim>> m_function;
    std::unique_ptr<Context<dim>> m_context;
};

/**
 * @brief Factory class for parsing space-time functions from YAML
 *
 * This class provides functionality to parse space-time functions from YAML files.
 * It supports various types of space-time functions including explicit forms,
 * sweep functions, offset functions, and union functions.
 *
 * Example YAML format:
 * ```yaml
 * type: sweep
 * dimension: 3
 * primitive:
 *   type: ball
 *   radius: 0.5
 *   center: [0.0, 0.0, 0.0]
 * transform:
 *   type: translation
 *   vector: [1.0, 0.0, 0.0]
 * ```
 */
template <int dim>
class YamlParser
{
public:
    /**
     * @brief Parse a space-time function from a YAML file
     *
     * @param filename Path to the YAML file
     * @return std::unique_ptr<SpaceTimeFunction<dim>> Parsed space-time function
     * @throws YamlParseError if parsing fails
     */
    static std::unique_ptr<SpaceTimeFunction<dim>> parse_from_file(const std::string& filename);

    /**
     * @brief Parse a space-time function from a YAML string
     *
     * @param yaml_string YAML content as string
     * @return std::unique_ptr<SpaceTimeFunction<dim>> Parsed space-time function
     * @throws YamlParseError if parsing fails
     */
    static std::unique_ptr<SpaceTimeFunction<dim>> parse_from_string(
        const std::string& yaml_string);

    /**
     * @brief Parse a space-time function from a YAML node
     *
     * @param node YAML node containing the function definition
     * @param yaml_file_dir Directory containing the YAML file (for resolving relative paths)
     * @return std::unique_ptr<SpaceTimeFunction<dim>> Parsed space-time function
     * @throws YamlParseError if parsing fails
     */
    static std::unique_ptr<SpaceTimeFunction<dim>> parse_from_node(const YAML::Node& node, const std::string& yaml_file_dir = "");

private:
    // Helper methods for parsing different components
    static std::unique_ptr<ImplicitFunction<dim>> parse_primitive(
        const YAML::Node& node, Context<dim>& context, const std::string& yaml_file_dir = "");
    static std::unique_ptr<Transform<dim>> parse_transform(
        const YAML::Node& node, Context<dim>& context, const std::string& yaml_file_dir = "");

    // Specific parsers for different space-time function types
    static std::unique_ptr<SpaceTimeFunction<dim>> parse_explicit_form(
        const YAML::Node& node, Context<dim>& context, const std::string& yaml_file_dir = "");
    static std::unique_ptr<SpaceTimeFunction<dim>> parse_sweep_function(
        const YAML::Node& node, Context<dim>& context, const std::string& yaml_file_dir = "");
    static std::unique_ptr<SpaceTimeFunction<dim>> parse_offset_function(
        const YAML::Node& node, Context<dim>& context, const std::string& yaml_file_dir = "");
    static std::unique_ptr<SpaceTimeFunction<dim>> parse_union_function(
        const YAML::Node& node, Context<dim>& context, const std::string& yaml_file_dir = "");
    static std::unique_ptr<SpaceTimeFunction<dim>> parse_interpolate_function(
        const YAML::Node& node, Context<dim>& context, const std::string& yaml_file_dir = "");

    // Specific parsers for primitives
    static std::unique_ptr<ImplicitFunction<dim>> parse_ball(const YAML::Node& node);
    static std::unique_ptr<ImplicitFunction<dim>> parse_capsule(const YAML::Node& node);
    static std::unique_ptr<ImplicitFunction<dim>> parse_torus(const YAML::Node& node);
    static std::unique_ptr<ImplicitFunction<dim>> parse_duchon(const YAML::Node& node, const std::string& yaml_file_dir = "");
    static std::unique_ptr<ImplicitFunction<dim>> parse_implicit_union(const YAML::Node& node, Context<dim>& context, const std::string& yaml_file_dir = "");

    // Specific parsers for transforms
    static std::unique_ptr<Transform<dim>> parse_translation(const YAML::Node& node);
    static std::unique_ptr<Transform<dim>> parse_scale(const YAML::Node& node);
    static std::unique_ptr<Transform<dim>> parse_rotation(const YAML::Node& node);
    static std::unique_ptr<Transform<dim>> parse_compose(
        const YAML::Node& node, Context<dim>& context, const std::string& yaml_file_dir = "");
    static std::unique_ptr<Transform<dim>> parse_polyline(const YAML::Node& node, const std::string& yaml_file_dir = "");
    static std::unique_ptr<Transform<dim>> parse_polybezier(const YAML::Node& node, const std::string& yaml_file_dir = "");

    // Utility functions
    static std::array<Scalar, dim> parse_array(
        const YAML::Node& node,
        const std::string& field_name);
    static Scalar parse_scalar(const YAML::Node& node, const std::string& field_name);
    static std::string parse_string(const YAML::Node& node, const std::string& field_name);
    static int parse_int(const YAML::Node& node, const std::string& field_name);
    static int parse_int(const YAML::Node& node, const std::string& field_name, int default_value);
    static bool
    parse_bool(const YAML::Node& node, const std::string& field_name, bool default_value = false);

    static void validate_dimension(const YAML::Node& node);
    static void validate_required_field(const YAML::Node& node, const std::string& field_name);
    
    // Helper function to load points from XYZ file
    static std::vector<std::array<Scalar, dim>> load_points_from_xyz(
        const std::string& file_path, const std::string& yaml_file_dir = "");
};

// Convenience functions for common use cases
template <int dim>
std::unique_ptr<SpaceTimeFunction<dim>> parse_space_time_function_from_file(
    const std::string& filename)
{
    return YamlParser<dim>::parse_from_file(filename);
}

template <int dim>
std::unique_ptr<SpaceTimeFunction<dim>> parse_space_time_function_from_string(
    const std::string& yaml_string)
{
    return YamlParser<dim>::parse_from_string(yaml_string);
}

} // namespace stf

#endif // STF_YAML_PARSER_ENABLED
