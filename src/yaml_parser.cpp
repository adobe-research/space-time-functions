#ifdef STF_YAML_PARSER_ENABLED

#include <stf/stf.h>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <functional>
#include <sstream>
#include <string>

namespace stf {

// Utility function implementations
template <int dim>
std::array<Scalar, dim> YamlParser<dim>::parse_array(
    const YAML::Node& node,
    const std::string& field_name)
{
    if (!node[field_name]) {
        throw YamlParseError("Missing required field: " + field_name);
    }

    if (!node[field_name].IsSequence()) {
        throw YamlParseError("Field '" + field_name + "' must be a sequence");
    }

    if (node[field_name].size() != dim) {
        throw YamlParseError(
            "Field '" + field_name + "' must have exactly " + std::to_string(dim) + " elements");
    }

    std::array<Scalar, dim> result;
    for (int i = 0; i < dim; ++i) {
        result[i] = node[field_name][i].as<Scalar>();
    }

    return result;
}

template <int dim>
Scalar YamlParser<dim>::parse_scalar(const YAML::Node& node, const std::string& field_name)
{
    if (!node[field_name]) {
        throw YamlParseError("Missing required field: " + field_name);
    }

    return node[field_name].as<Scalar>();
}

template <int dim>
std::string YamlParser<dim>::parse_string(const YAML::Node& node, const std::string& field_name)
{
    if (!node[field_name]) {
        throw YamlParseError("Missing required field: " + field_name);
    }

    return node[field_name].as<std::string>();
}

template <int dim>
int YamlParser<dim>::parse_int(const YAML::Node& node, const std::string& field_name)
{
    if (!node[field_name]) {
        throw YamlParseError("Missing required field: " + field_name);
    }

    return node[field_name].as<int>();
}

template <int dim>
int YamlParser<dim>::parse_int(
    const YAML::Node& node,
    const std::string& field_name,
    int default_value)
{
    if (!node[field_name]) {
        return default_value;
    }

    return node[field_name].as<int>();
}

template <int dim>
bool YamlParser<dim>::parse_bool(
    const YAML::Node& node,
    const std::string& field_name,
    bool default_value)
{
    if (!node[field_name]) {
        return default_value;
    }

    return node[field_name].as<bool>();
}

template <int dim>
void YamlParser<dim>::validate_dimension(const YAML::Node& node)
{
    if (node["dimension"]) {
        int yaml_dim = node["dimension"].as<int>();
        if (yaml_dim != dim) {
            throw YamlParseError(
                "Dimension mismatch: YAML specifies " + std::to_string(yaml_dim) +
                " but parser is for " + std::to_string(dim) + "D");
        }
    }
}

template <int dim>
void YamlParser<dim>::validate_required_field(const YAML::Node& node, const std::string& field_name)
{
    if (!node[field_name]) {
        throw YamlParseError("Missing required field: " + field_name);
    }
}


template <int dim>
std::unique_ptr<SpaceTimeFunction<dim>> YamlParser<dim>::parse_from_file(
    const std::string& filename)
{
    try {
        YAML::Node node = YAML::LoadFile(filename);
        // Extract directory from filename for relative path resolution
        std::filesystem::path file_path(filename);
        std::string yaml_file_dir = file_path.parent_path().string();
        return parse_from_node(node, yaml_file_dir);
    } catch (const YAML::Exception& e) {
        std::stringstream err_msg;
        err_msg << "Failed to load file '" << filename << "': " << e.what();
        throw YamlParseError(err_msg.str());
    }
}

template <int dim>
std::unique_ptr<SpaceTimeFunction<dim>> YamlParser<dim>::parse_from_string(
    const std::string& yaml_string)
{
    try {
        YAML::Node node = YAML::Load(yaml_string);
        return parse_from_node(node);
    } catch (const YAML::Exception& e) {
        std::stringstream err_msg;
        err_msg << "Failed to parse YAML string: " << e.what();
        throw YamlParseError(err_msg.str());
    }
}

template <int dim>
std::unique_ptr<SpaceTimeFunction<dim>> YamlParser<dim>::parse_from_node(
    const YAML::Node& node,
    const std::string& yaml_file_dir)
{
    validate_dimension(node);
    validate_required_field(node, "type");

    std::string type = parse_string(node, "type");

    // Create parsing context to manage lifetimes
    auto context = std::make_unique<Context<dim>>();

    std::unique_ptr<SpaceTimeFunction<dim>> function;

    if (type == "explicit") {
        function = parse_explicit_form(node, *context, yaml_file_dir);
    } else if (type == "sweep") {
        function = parse_sweep_function(node, *context, yaml_file_dir);
    } else if (type == "offset") {
        function = parse_offset_function(node, *context, yaml_file_dir);
    } else if (type == "union") {
        function = parse_union_function(node, *context, yaml_file_dir);
    } else if (type == "interpolate") {
        function = parse_interpolate_function(node, *context, yaml_file_dir);
    } else {
        throw YamlParseError("Unknown space-time function type: " + type);
    }

    // Wrap the function with lifetime management
    return std::make_unique<ManagedSpaceTimeFunction<dim>>(std::move(function), std::move(context));
}

template <int dim>
std::unique_ptr<SpaceTimeFunction<dim>> YamlParser<dim>::parse_explicit_form(
    const YAML::Node& node,
    Context<dim>& context,
    const std::string& yaml_file_dir)
{
    // For explicit forms, we would need to support function definitions in YAML
    // This is complex and would require a scripting language or mathematical expression parser
    // For now, we'll throw an error suggesting this isn't supported via YAML
    throw YamlParseError("Explicit form functions cannot be defined in YAML. Use C++ API directly "
                         "for custom functions.");
}

template <int dim>
std::unique_ptr<SpaceTimeFunction<dim>> YamlParser<dim>::parse_sweep_function(
    const YAML::Node& node,
    Context<dim>& context,
    const std::string& yaml_file_dir)
{
    validate_required_field(node, "primitive");
    validate_required_field(node, "transform");

    auto primitive = parse_primitive(node["primitive"], context, yaml_file_dir);
    auto transform = parse_transform(node["transform"], context, yaml_file_dir);

    // Store the objects and get raw pointers
    auto* primitive_ptr = context.add_primitive(std::move(primitive));
    auto* transform_ptr = context.add_transform(std::move(transform));

    return std::make_unique<SweepFunction<dim>>(*primitive_ptr, *transform_ptr);
}

template <int dim>
std::unique_ptr<SpaceTimeFunction<dim>> YamlParser<dim>::parse_offset_function(
    const YAML::Node& node,
    Context<dim>& context,
    const std::string& yaml_file_dir)
{
    validate_required_field(node, "base_function");

    // Parse the base function recursively - this will create its own ManagedSpaceTimeFunction
    auto base_function = parse_from_node(node["base_function"], yaml_file_dir);

    // Parse offset function and compute its derivative analytically
    validate_required_field(node, "offset_function");
    auto [offset_func, offset_deriv_func] =
        parse_single_variable_function_with_derivative(node, "offset_function");

    // Store the base function and get raw pointer
    auto* base_function_ptr = context.add_function(std::move(base_function));

    return std::make_unique<OffsetFunction<dim>>(
        *base_function_ptr,
        offset_func,
        offset_deriv_func);
}

template <int dim>
std::unique_ptr<SpaceTimeFunction<dim>> YamlParser<dim>::parse_union_function(
    const YAML::Node& node,
    Context<dim>& context,
    const std::string& yaml_file_dir)
{
    validate_required_field(node, "functions");

    if (!node["functions"].IsSequence()) {
        throw YamlParseError("'functions' field must be a sequence");
    }

    std::vector<std::unique_ptr<SpaceTimeFunction<dim>>> functions;
    for (const auto& func_node : node["functions"]) {
        functions.push_back(parse_from_node(func_node, yaml_file_dir));
    }

    if (functions.size() < 2) {
        throw YamlParseError("Union function requires at least 2 functions");
    }

    // Parse smooth_distance parameter (optional, defaults to 0.0)
    Scalar smooth_distance = 0.0;
    if (node["smooth_distance"]) {
        smooth_distance = parse_scalar(node, "smooth_distance");
    }

    // Store all functions and get raw pointers
    std::vector<SpaceTimeFunction<dim>*> function_ptrs;
    for (auto& func : functions) {
        function_ptrs.push_back(context.add_function(std::move(func)));
    }

    // For simplicity, we'll create a binary union tree
    auto result =
        std::make_unique<UnionFunction<dim>>(*function_ptrs[0], *function_ptrs[1], smooth_distance);

    for (size_t i = 2; i < function_ptrs.size(); ++i) {
        // Store intermediate union functions too
        auto* prev_union = context.add_function(std::move(result));
        result =
            std::make_unique<UnionFunction<dim>>(*prev_union, *function_ptrs[i], smooth_distance);
    }

    return std::move(result);
}

template <int dim>
std::unique_ptr<ImplicitFunction<dim>> YamlParser<dim>::parse_primitive(
    const YAML::Node& node,
    Context<dim>& context,
    const std::string& yaml_file_dir)
{
    validate_required_field(node, "type");

    std::string type = parse_string(node, "type");

    if (type == "ball") {
        return parse_ball(node);
    } else if (type == "capsule") {
        return parse_capsule(node);
    } else if (type == "torus") {
        return parse_torus(node);
    } else if (type == "duchon") {
        return parse_duchon(node, yaml_file_dir);
    } else if (type == "implicit_union") {
        return parse_implicit_union(node, context, yaml_file_dir);
    } else {
        throw YamlParseError("Unknown primitive type: " + type);
    }
}

template <int dim>
std::unique_ptr<Transform<dim>> YamlParser<dim>::parse_transform(
    const YAML::Node& node,
    Context<dim>& context,
    const std::string& yaml_file_dir)
{
    validate_required_field(node, "type");

    std::string type = parse_string(node, "type");

    if (type == "translation") {
        return parse_translation(node);
    } else if (type == "scale") {
        return parse_scale(node);
    } else if (type == "rotation") {
        return parse_rotation(node);
    } else if (type == "compose") {
        return parse_compose(node, context, yaml_file_dir);
    } else if (type == "polyline") {
        return parse_polyline(node, yaml_file_dir);
    } else if (type == "polybezier") {
        return parse_polybezier(node, yaml_file_dir);
    } else {
        throw YamlParseError("Unknown transform type: " + type);
    }
}

template <int dim>
std::unique_ptr<ImplicitFunction<dim>> YamlParser<dim>::parse_ball(const YAML::Node& node)
{
    Scalar radius = parse_scalar(node, "radius");
    std::array<Scalar, dim> center = parse_array(node, "center");
    int degree = parse_int(node, "degree", 1); // Default degree is 1

    return std::make_unique<ImplicitBall<dim>>(radius, center, degree);
}

template <int dim>
std::unique_ptr<ImplicitFunction<dim>> YamlParser<dim>::parse_capsule(const YAML::Node& node)
{
    if constexpr (dim != 3) {
        throw YamlParseError("Capsule primitive is only supported in 3D");
    }

    std::array<Scalar, dim> start = parse_array(node, "start");
    std::array<Scalar, dim> end = parse_array(node, "end");
    Scalar radius = parse_scalar(node, "radius");

    return std::make_unique<ImplicitCapsule<dim>>(radius, start, end);
}

template <int dim>
std::unique_ptr<ImplicitFunction<dim>> YamlParser<dim>::parse_torus(const YAML::Node& node)
{
    if constexpr (dim != 3) {
        throw YamlParseError("Torus primitive is only supported in 3D");
    }

    Scalar major_radius = parse_scalar(node, "major_radius");
    Scalar minor_radius = parse_scalar(node, "minor_radius");
    std::array<Scalar, dim> center = parse_array(node, "center");

    // ImplicitTorus is not templated, it's specifically for 3D
    if constexpr (dim == 3) {
        return std::make_unique<ImplicitTorus>(major_radius, minor_radius, center);
    }
}

template <int dim>
std::unique_ptr<Transform<dim>> YamlParser<dim>::parse_translation(const YAML::Node& node)
{
    std::array<Scalar, dim> vector = parse_array(node, "vector");
    return std::make_unique<Translation<dim>>(vector);
}

template <int dim>
std::unique_ptr<Transform<dim>> YamlParser<dim>::parse_scale(const YAML::Node& node)
{
    std::array<Scalar, dim> factors = parse_array(node, "factors");

    std::array<Scalar, dim> center{0};
    if (node["center"]) {
        center = parse_array(node, "center");
    }

    return std::make_unique<Scale<dim>>(factors, center);
}

template <int dim>
std::unique_ptr<Transform<dim>> YamlParser<dim>::parse_rotation(const YAML::Node& node)
{
    Scalar angle = parse_scalar(node, "angle");
    std::array<Scalar, dim> center{0};
    if (node["center"]) {
        center = parse_array(node, "center");
    }

    if constexpr (dim == 2) {
        // For 2D, axis is not needed
        std::array<Scalar, dim> dummy_axis{0, 1}; // Not used in 2D
        return std::make_unique<Rotation<dim>>(center, dummy_axis, angle);
    } else if constexpr (dim == 3) {
        std::array<Scalar, dim> axis = parse_array(node, "axis");
        return std::make_unique<Rotation<dim>>(center, axis, angle);
    }
}

template <int dim>
std::unique_ptr<Transform<dim>> YamlParser<dim>::parse_compose(
    const YAML::Node& node,
    Context<dim>& context,
    const std::string& yaml_file_dir)
{
    validate_required_field(node, "transforms");

    if (!node["transforms"].IsSequence()) {
        throw YamlParseError("'transforms' field must be a sequence");
    }

    std::vector<std::unique_ptr<Transform<dim>>> transforms;
    for (const auto& transform_node : node["transforms"]) {
        transforms.push_back(parse_transform(transform_node, context, yaml_file_dir));
    }

    if (transforms.size() < 2) {
        throw YamlParseError("Compose transform requires at least 2 transforms");
    }

    // Store all transforms and get raw pointers
    std::vector<Transform<dim>*> transform_ptrs;
    for (auto& transform : transforms) {
        transform_ptrs.push_back(context.add_transform(std::move(transform)));
    }

    // Create a composition chain
    auto result = std::make_unique<Compose<dim>>(*transform_ptrs[0], *transform_ptrs[1]);

    for (size_t i = 2; i < transform_ptrs.size(); ++i) {
        // Store intermediate compose transforms too
        auto* prev_compose = context.add_transform(std::move(result));
        result = std::make_unique<Compose<dim>>(*prev_compose, *transform_ptrs[i]);
    }

    return std::move(result);
}

template <int dim>
std::unique_ptr<Transform<dim>> YamlParser<dim>::parse_polyline(
    const YAML::Node& node,
    const std::string& yaml_file_dir)
{
    std::vector<std::array<Scalar, dim>> points;

    // Check if points are loaded from a file or specified inline
    if (node["points_file"]) {
        // Load points from XYZ file
        std::string points_file = parse_string(node, "points_file");
        points = load_points_from_xyz(points_file, yaml_file_dir);

    } else if (node["points"]) {
        // Load points from inline YAML array
        if (!node["points"].IsSequence()) {
            throw YamlParseError("'points' field must be a sequence");
        }

        for (const auto& point_node : node["points"]) {
            if (!point_node.IsSequence()) {
                throw YamlParseError("Each point must be a sequence");
            }

            if (point_node.size() != dim) {
                throw YamlParseError(
                    "Each point must have exactly " + std::to_string(dim) + " coordinates");
            }

            std::array<Scalar, dim> point;
            for (int i = 0; i < dim; ++i) {
                point[i] = point_node[i].as<Scalar>();
            }
            points.push_back(point);
        }

    } else {
        throw YamlParseError("Polyline requires either 'points' or 'points_file' field");
    }

    if (points.size() < 2) {
        throw YamlParseError("Polyline must have at least 2 points");
    }

    return std::make_unique<Polyline<dim>>(std::move(points));
}

template <int dim>
std::unique_ptr<Transform<dim>> YamlParser<dim>::parse_polybezier(
    const YAML::Node& node,
    const std::string& yaml_file_dir)
{
    bool follow_tangent = parse_bool(node, "follow_tangent", true);

    // Check different ways to specify points (in order of preference)
    if (node["control_points_file"]) {
        // Load control points from XYZ file
        std::string control_points_file = parse_string(node, "control_points_file");
        auto control_points = load_points_from_xyz(control_points_file, yaml_file_dir);

        if (control_points.size() < 4) {
            throw YamlParseError("PolyBezier must have at least 4 control points");
        }

        if ((control_points.size() - 1) % 3 != 0) {
            throw YamlParseError("PolyBezier must have (n * 3) + 1 control points");
        }

        return std::make_unique<PolyBezier<dim>>(std::move(control_points), follow_tangent);

    } else if (node["sample_points_file"]) {
        // Load sample points from XYZ file and create Bezier curve
        std::string sample_points_file = parse_string(node, "sample_points_file");
        auto sample_points = load_points_from_xyz(sample_points_file, yaml_file_dir);

        if (sample_points.size() < 3) {
            throw YamlParseError("PolyBezier from samples must have at least 3 sample points");
        }

        auto bezier = PolyBezier<dim>::from_samples(std::move(sample_points), follow_tangent);
        return std::make_unique<PolyBezier<dim>>(std::move(bezier));

    } else if (node["control_points"]) {
        // Direct control points specification (inline YAML)
        if (!node["control_points"].IsSequence()) {
            throw YamlParseError("'control_points' field must be a sequence");
        }

        std::vector<std::array<Scalar, dim>> control_points;
        for (const auto& point_node : node["control_points"]) {
            if (!point_node.IsSequence()) {
                throw YamlParseError("Each control point must be a sequence");
            }

            if (point_node.size() != dim) {
                throw YamlParseError(
                    "Each control point must have exactly " + std::to_string(dim) + " coordinates");
            }

            std::array<Scalar, dim> point;
            for (int i = 0; i < dim; ++i) {
                point[i] = point_node[i].as<Scalar>();
            }
            control_points.push_back(point);
        }

        if (control_points.size() < 4) {
            throw YamlParseError("PolyBezier must have at least 4 control points");
        }

        if ((control_points.size() - 1) % 3 != 0) {
            throw YamlParseError("PolyBezier must have (n * 3) + 1 control points");
        }

        return std::make_unique<PolyBezier<dim>>(std::move(control_points), follow_tangent);

    } else if (node["sample_points"]) {
        // Create from sample points (inline YAML)
        if (!node["sample_points"].IsSequence()) {
            throw YamlParseError("'sample_points' field must be a sequence");
        }

        std::vector<std::array<Scalar, dim>> sample_points;
        for (const auto& point_node : node["sample_points"]) {
            if (!point_node.IsSequence()) {
                throw YamlParseError("Each sample point must be a sequence");
            }

            if (point_node.size() != dim) {
                throw YamlParseError(
                    "Each sample point must have exactly " + std::to_string(dim) + " coordinates");
            }

            std::array<Scalar, dim> point;
            for (int i = 0; i < dim; ++i) {
                point[i] = point_node[i].as<Scalar>();
            }
            sample_points.push_back(point);
        }

        if (sample_points.size() < 3) {
            throw YamlParseError("PolyBezier from samples must have at least 3 sample points");
        }

        auto bezier = PolyBezier<dim>::from_samples(std::move(sample_points), follow_tangent);
        return std::make_unique<PolyBezier<dim>>(std::move(bezier));

    } else {
        throw YamlParseError("PolyBezier requires one of: 'control_points_file', "
                             "'sample_points_file', 'control_points', or 'sample_points' field");
    }
}

template <int dim>
std::unique_ptr<SpaceTimeFunction<dim>> YamlParser<dim>::parse_interpolate_function(
    const YAML::Node& node,
    Context<dim>& context,
    const std::string& yaml_file_dir)
{
    validate_required_field(node, "function1");
    validate_required_field(node, "function2");

    // Parse the two functions to interpolate between
    auto function1 = parse_from_node(node["function1"], yaml_file_dir);
    auto function2 = parse_from_node(node["function2"], yaml_file_dir);

    // Store the functions and get raw pointers
    auto* function1_ptr = context.add_function(std::move(function1));
    auto* function2_ptr = context.add_function(std::move(function2));

    // Parse interpolation type (optional, default is linear)
    std::string interpolation_type = "linear";
    if (node["interpolation_type"]) {
        interpolation_type = parse_string(node, "interpolation_type");
    }

    // Parse cosine interpolation parameters (optional)
    // Default num_periods = 0.5 so that the generalized formula reduces to standard cosine
    Scalar num_periods = 0.5;
    Scalar phase = 0.0;
    
    if (node["num_periods"]) {
        num_periods = node["num_periods"].as<Scalar>();
        if (num_periods <= 0) {
            throw YamlParseError("num_periods must be positive");
        }
    }
    
    if (node["phase"]) {
        phase = node["phase"].as<Scalar>();
    }
    
    // Amplitude and offset are fixed (not exposed to users)
    constexpr Scalar amplitude = 1.0;
    constexpr Scalar offset = 0.0;

    // Create interpolation functions based on type
    std::function<Scalar(Scalar)> interpolation_func;
    std::function<Scalar(Scalar)> interpolation_derivative;

    if (interpolation_type == "linear") {
        interpolation_func = [](Scalar t) { return t; };
        interpolation_derivative = [](Scalar t) { return 1.0; };
    } else if (interpolation_type == "smooth") {
        // Smooth step interpolation using polynomial: 3t² - 2t³
        interpolation_func = [](Scalar t) { return 3 * t * t - 2 * t * t * t; };
        interpolation_derivative = [](Scalar t) { return 6 * t - 6 * t * t; };
    } else if (interpolation_type == "cosine") {
        // Cosine interpolation using generalized sinusoidal function
        // Formula: offset + amplitude × (sin(t × n × 2π + phase - π/2) + 1) / 2
        // With default parameters (n=0.5, A=1, φ=0, offset=0), this reduces to:
        //   (sin(πt - π/2) + 1) / 2 = (1 - cos(πt)) / 2  (standard cosine interpolation)
        interpolation_func = [num_periods, amplitude, phase, offset](Scalar t) {
            return offset + amplitude * 
                (std::sin(t * num_periods * 2.0 * M_PI + phase - M_PI / 2.0) + 1.0) / 2.0;
        };
        // Derivative: d/dt[offset + A × (sin(t × n × 2π + φ - π/2) + 1) / 2]
        //           = A × n × π × cos(t × n × 2π + φ - π/2)
        interpolation_derivative = [num_periods, amplitude, phase](Scalar t) {
            return amplitude * num_periods * M_PI * 
                std::cos(t * num_periods * 2.0 * M_PI + phase - M_PI / 2.0);
        };
    } else if (interpolation_type == "custom") {
        // For custom interpolation, we would need to parse mathematical expressions
        // For now, throw an error suggesting this isn't supported
        throw YamlParseError("Custom interpolation functions are not yet supported in YAML. Use "
                             "'linear', 'smooth', or 'cosine'.");
    } else {
        throw YamlParseError(
            "Unknown interpolation type: " + interpolation_type +
            ". Supported types: 'linear', 'smooth', 'cosine'");
    }

    return std::make_unique<InterpolateFunction<dim>>(
        *function1_ptr,
        *function2_ptr,
        interpolation_func,
        interpolation_derivative);
}

template <int dim>
std::vector<std::array<Scalar, dim>> YamlParser<dim>::load_points_from_xyz(
    const std::string& file_path,
    const std::string& yaml_file_dir)
{
    // Handle relative paths by making them relative to the YAML file directory
    std::filesystem::path points_path(file_path);

    if (!points_path.is_absolute() && !yaml_file_dir.empty()) {
        points_path = std::filesystem::path(yaml_file_dir) / points_path;
    }

    std::ifstream file(points_path);
    if (!file.is_open()) {
        throw YamlParseError("Failed to open XYZ file: " + points_path.string());
    }

    int file_dimension;
    file >> file_dimension;

    if (file_dimension != dim) {
        throw YamlParseError(
            "XYZ file dimension (" + std::to_string(file_dimension) +
            ") does not match expected dimension (" + std::to_string(dim) + ")");
    }

    std::vector<std::array<Scalar, dim>> points;
    while (file) {
        std::array<Scalar, dim> point;
        bool valid_point = true;

        for (int i = 0; i < dim; ++i) {
            if (!(file >> point[i])) {
                valid_point = false;
                break;
            }
        }

        if (valid_point) {
            points.push_back(point);
        }
    }

    file.close();

    if (points.empty()) {
        throw YamlParseError("No valid points found in XYZ file: " + points_path.string());
    }

    return points;
}

template <int dim>
std::unique_ptr<ImplicitFunction<dim>> YamlParser<dim>::parse_duchon(
    const YAML::Node& node,
    const std::string& yaml_file_dir)
{
    if constexpr (dim != 3) {
        throw YamlParseError("Duchon primitive is only supported in 3D");
    }

    validate_required_field(node, "samples_file");
    validate_required_field(node, "coeffs_file");

    std::string samples_file = parse_string(node, "samples_file");
    std::string coeffs_file = parse_string(node, "coeffs_file");

    // Handle relative paths by making them relative to the YAML file directory
    std::filesystem::path samples_path(samples_file);
    std::filesystem::path coeffs_path(coeffs_file);

    if (!samples_path.is_absolute() && !yaml_file_dir.empty()) {
        samples_path = std::filesystem::path(yaml_file_dir) / samples_path;
    }

    if (!coeffs_path.is_absolute() && !yaml_file_dir.empty()) {
        coeffs_path = std::filesystem::path(yaml_file_dir) / coeffs_path;
    }

    // Parse optional parameters with defaults
    std::array<Scalar, dim> center;
    center.fill(0);
    if (node["center"]) {
        center = parse_array(node, "center");
    }

    Scalar radius = 1.0;
    if (node["radius"]) {
        radius = parse_scalar(node, "radius");
    }

    bool positive_inside = parse_bool(node, "positive_inside", false);

    if constexpr (dim == 3) {
        return std::make_unique<Duchon>(samples_path, coeffs_path, center, radius, positive_inside);
    }
}

template <int dim>
std::unique_ptr<ImplicitFunction<dim>> YamlParser<dim>::parse_implicit_union(
    const YAML::Node& node,
    Context<dim>& context,
    const std::string& yaml_file_dir)
{
    validate_required_field(node, "primitives");

    if (!node["primitives"].IsSequence()) {
        throw YamlParseError("'primitives' field must be a sequence");
    }

    if (node["primitives"].size() < 2) {
        throw YamlParseError("Implicit union requires at least 2 primitives");
    }

    // Parse blending function (optional, defaults to Quadratic)
    std::string blending_str = "quadratic";
    if (node["blending"]) {
        blending_str = parse_string(node, "blending");
    }

    // Parse smooth distance (optional, defaults to 0)
    Scalar smooth_distance = 0.0;
    if (node["smooth_distance"]) {
        smooth_distance = parse_scalar(node, "smooth_distance");
    }

    // Parse all primitives
    std::vector<std::unique_ptr<ImplicitFunction<dim>>> primitives;
    for (const auto& primitive_node : node["primitives"]) {
        primitives.push_back(parse_primitive(primitive_node, context, yaml_file_dir));
    }

    // Store all primitives in context and get raw pointers
    std::vector<ImplicitFunction<dim>*> primitive_ptrs;
    for (auto& primitive : primitives) {
        primitive_ptrs.push_back(context.add_primitive(std::move(primitive)));
    }

    // Create union tree based on blending function
    std::unique_ptr<ImplicitFunction<dim>> result;

    if (blending_str == "quadratic") {
        result = std::make_unique<ImplicitUnion<dim, BlendingFunction::Quadratic>>(
            *primitive_ptrs[0],
            *primitive_ptrs[1],
            smooth_distance);

        for (size_t i = 2; i < primitive_ptrs.size(); ++i) {
            auto* prev_union = context.add_primitive(std::move(result));
            result = std::make_unique<ImplicitUnion<dim, BlendingFunction::Quadratic>>(
                *prev_union,
                *primitive_ptrs[i],
                smooth_distance);
        }
    } else if (blending_str == "cubic") {
        result = std::make_unique<ImplicitUnion<dim, BlendingFunction::Cubic>>(
            *primitive_ptrs[0],
            *primitive_ptrs[1],
            smooth_distance);

        for (size_t i = 2; i < primitive_ptrs.size(); ++i) {
            auto* prev_union = context.add_primitive(std::move(result));
            result = std::make_unique<ImplicitUnion<dim, BlendingFunction::Cubic>>(
                *prev_union,
                *primitive_ptrs[i],
                smooth_distance);
        }
    } else if (blending_str == "quartic") {
        result = std::make_unique<ImplicitUnion<dim, BlendingFunction::Quartic>>(
            *primitive_ptrs[0],
            *primitive_ptrs[1],
            smooth_distance);

        for (size_t i = 2; i < primitive_ptrs.size(); ++i) {
            auto* prev_union = context.add_primitive(std::move(result));
            result = std::make_unique<ImplicitUnion<dim, BlendingFunction::Quartic>>(
                *prev_union,
                *primitive_ptrs[i],
                smooth_distance);
        }
    } else if (blending_str == "circular") {
        result = std::make_unique<ImplicitUnion<dim, BlendingFunction::Circular>>(
            *primitive_ptrs[0],
            *primitive_ptrs[1],
            smooth_distance);

        for (size_t i = 2; i < primitive_ptrs.size(); ++i) {
            auto* prev_union = context.add_primitive(std::move(result));
            result = std::make_unique<ImplicitUnion<dim, BlendingFunction::Circular>>(
                *prev_union,
                *primitive_ptrs[i],
                smooth_distance);
        }
    } else {
        throw YamlParseError(
            "Unknown blending function: " + blending_str +
            ". Supported: quadratic, cubic, quartic, circular");
    }

    return result;
}

template <int dim>
std::pair<std::function<Scalar(Scalar)>, std::function<Scalar(Scalar)>>
YamlParser<dim>::parse_single_variable_function_with_derivative(
    const YAML::Node& node,
    const std::string& field_name)
{
    if (!node[field_name]) {
        throw YamlParseError("Missing required field: " + field_name);
    }

    const auto& func_node = node[field_name];
    validate_required_field(func_node, "type");

    std::string type = parse_string(func_node, "type");

    if (type == "constant") {
        Scalar value = parse_scalar(func_node, "value");
        auto func = [value](Scalar t) { return value; };
        auto deriv = [](Scalar t) { return 0.0; }; // Derivative of constant is 0
        return std::make_pair(func, deriv);

    } else if (type == "linear") {
        Scalar a = parse_scalar(func_node, "slope");
        Scalar b = parse_scalar(func_node, "intercept");
        auto func = [a, b](Scalar t) { return a * t + b; };
        auto deriv = [a](Scalar t) { return a; }; // Derivative of at+b is a
        return std::make_pair(func, deriv);

    } else if (type == "polynomial") {
        if (!func_node["coefficients"].IsSequence()) {
            throw YamlParseError("'coefficients' field must be a sequence for polynomial function");
        }

        std::vector<Scalar> coeffs;
        for (const auto& coeff_node : func_node["coefficients"]) {
            coeffs.push_back(coeff_node.as<Scalar>());
        }

        if (coeffs.empty()) {
            throw YamlParseError("Polynomial function requires at least one coefficient");
        }

        auto func = [coeffs](Scalar t) {
            Scalar result = 0.0;
            Scalar t_power = 1.0;
            for (Scalar coeff : coeffs) {
                result += coeff * t_power;
                t_power *= t;
            }
            return result;
        };

        // Compute derivative coefficients: d/dt(a₀ + a₁t + a₂t² + ...) = a₁ + 2a₂t + 3a₃t² + ...
        auto deriv = [coeffs](Scalar t) {
            if (coeffs.size() <= 1) return 0.0; // Derivative of constant is 0

            Scalar result = 0.0;
            Scalar t_power = 1.0;
            for (size_t i = 1; i < coeffs.size(); ++i) {
                result += i * coeffs[i] * t_power;
                t_power *= t;
            }
            return result;
        };

        return std::make_pair(func, deriv);

    } else if (type == "sinusoidal") {
        Scalar amplitude = parse_scalar(func_node, "amplitude");
        Scalar frequency = parse_scalar(func_node, "frequency");
        Scalar phase = 0.0;
        if (func_node["phase"]) {
            phase = parse_scalar(func_node, "phase");
        }
        Scalar offset = 0.0;
        if (func_node["offset"]) {
            offset = parse_scalar(func_node, "offset");
        }

        auto func = [amplitude, frequency, phase, offset](Scalar t) {
            return amplitude * std::sin(frequency * t + phase) + offset;
        };

        // Derivative: d/dt(A*sin(ωt + φ) + c) = A*ω*cos(ωt + φ)
        auto deriv = [amplitude, frequency, phase](Scalar t) {
            return amplitude * frequency * std::cos(frequency * t + phase);
        };

        return std::make_pair(func, deriv);

    } else if (type == "exponential") {
        Scalar amplitude = parse_scalar(func_node, "amplitude");
        Scalar rate = parse_scalar(func_node, "rate");
        Scalar offset = 0.0;
        if (func_node["offset"]) {
            offset = parse_scalar(func_node, "offset");
        }

        auto func = [amplitude, rate, offset](Scalar t) {
            return amplitude * std::exp(rate * t) + offset;
        };

        // Derivative: d/dt(A*exp(rt) + c) = A*r*exp(rt)
        auto deriv = [amplitude, rate](Scalar t) { return amplitude * rate * std::exp(rate * t); };

        return std::make_pair(func, deriv);

    } else if (type == "polybezier") {
        if (!func_node["control_points"].IsSequence()) {
            throw YamlParseError(
                "'control_points' field must be a sequence for polybezier function");
        }

        std::vector<std::pair<Scalar, Scalar>> control_points;
        for (const auto& point_node : func_node["control_points"]) {
            if (!point_node.IsSequence() || point_node.size() != 2) {
                throw YamlParseError(
                    "Each control point in polybezier function must be [t, value]");
            }
            Scalar t = point_node[0].as<Scalar>();
            Scalar value = point_node[1].as<Scalar>();
            control_points.emplace_back(t, value);
        }

        if (control_points.size() < 4) {
            throw YamlParseError("Polybezier function requires at least 4 control points");
        }

        if ((control_points.size() - 1) % 3 != 0) {
            throw YamlParseError("Polybezier function must have (n * 3) + 1 control points");
        }

        auto func = [control_points](Scalar t) {
            size_t num_segments = (control_points.size() - 1) / 3;

            // Find the segment containing t
            size_t segment = 0;
            Scalar local_t = t;

            // Handle extrapolation
            if (t <= control_points.front().first) {
                segment = 0;
                local_t = 0.0;
            } else if (t >= control_points.back().first) {
                segment = num_segments - 1;
                local_t = 1.0;
            } else {
                // Find which segment contains t
                for (size_t i = 0; i < num_segments; ++i) {
                    size_t p0_idx = i * 3;
                    size_t p3_idx = (i + 1) * 3;

                    Scalar t0 = control_points[p0_idx].first;
                    Scalar t3 = control_points[p3_idx].first;

                    if (t >= t0 && t <= t3) {
                        segment = i;
                        // Normalize t to [0,1] within this segment
                        if (std::abs(t3 - t0) < 1e-10) {
                            local_t = 0.0;
                        } else {
                            local_t = (t - t0) / (t3 - t0);
                        }
                        break;
                    }
                }
            }

            // Get control points for this segment
            size_t base_idx = segment * 3;
            Scalar v0 = control_points[base_idx].second;
            Scalar v1 = control_points[base_idx + 1].second;
            Scalar v2 = control_points[base_idx + 2].second;
            Scalar v3 = control_points[base_idx + 3].second;

            // Evaluate cubic Bézier curve: B(t) = (1-t)³P₀ + 3(1-t)²tP₁ + 3(1-t)t²P₂ + t³P₃
            Scalar u = 1.0 - local_t;
            Scalar u2 = u * u;
            Scalar u3 = u2 * u;
            Scalar t2 = local_t * local_t;
            Scalar t3 = t2 * local_t;

            return u3 * v0 + 3.0 * u2 * local_t * v1 + 3.0 * u * t2 * v2 + t3 * v3;
        };

        // Compute derivative function for polybezier
        auto deriv = [control_points](Scalar t) {
            size_t num_segments = (control_points.size() - 1) / 3;

            // Handle extrapolation - derivative is 0 outside the range
            if (t <= control_points.front().first || t >= control_points.back().first) {
                return 0.0;
            }

            // Find which segment contains t (same logic as function evaluation)
            size_t segment = 0;
            Scalar local_t = t;

            for (size_t i = 0; i < num_segments; ++i) {
                size_t p0_idx = i * 3;
                size_t p3_idx = (i + 1) * 3;

                Scalar t0 = control_points[p0_idx].first;
                Scalar t3 = control_points[p3_idx].first;

                if (t >= t0 && t <= t3) {
                    segment = i;
                    if (std::abs(t3 - t0) < 1e-10) {
                        return 0.0; // Degenerate segment
                    }
                    local_t = (t - t0) / (t3 - t0);
                    break;
                }
            }

            // Get control points for this segment
            size_t base_idx = segment * 3;
            Scalar v0 = control_points[base_idx].second;
            Scalar v1 = control_points[base_idx + 1].second;
            Scalar v2 = control_points[base_idx + 2].second;
            Scalar v3 = control_points[base_idx + 3].second;

            // Derivative of cubic Bézier: B'(t) = 3(1-t)²(P₁-P₀) + 6(1-t)t(P₂-P₁) + 3t²(P₃-P₂)
            Scalar u = 1.0 - local_t;
            Scalar u2 = u * u;
            Scalar t2 = local_t * local_t;

            Scalar bezier_deriv =
                3.0 * u2 * (v1 - v0) + 6.0 * u * local_t * (v2 - v1) + 3.0 * t2 * (v3 - v2);

            // Chain rule: need to multiply by dt_local/dt_global
            Scalar t0 = control_points[segment * 3].first;
            Scalar t3 = control_points[(segment + 1) * 3].first;
            Scalar dt_scale = 1.0 / (t3 - t0);

            return bezier_deriv * dt_scale;
        };

        return std::make_pair(func, deriv);

    } else {
        throw YamlParseError(
            "Unknown single-variable function type: " + type +
            ". Supported: constant, linear, polynomial, sinusoidal, exponential, polybezier");
    }
}

// Explicit template instantiations
template class YamlParser<2>;
template class YamlParser<3>;

// Convenience function instantiations
template std::unique_ptr<SpaceTimeFunction<2>> parse_space_time_function_from_file<2>(
    const std::string&);
template std::unique_ptr<SpaceTimeFunction<3>> parse_space_time_function_from_file<3>(
    const std::string&);
template std::unique_ptr<SpaceTimeFunction<2>> parse_space_time_function_from_string<2>(
    const std::string&);
template std::unique_ptr<SpaceTimeFunction<3>> parse_space_time_function_from_string<3>(
    const std::string&);

} // namespace stf

#endif // STF_YAML_PARSER_ENABLED
