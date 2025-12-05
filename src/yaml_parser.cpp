#ifdef STF_YAML_PARSER_ENABLED

#include <stf/yaml_parser.h>
#include <stf/primitives/all.h>
#include <stf/transforms/all.h>

#include <fstream>
#include <sstream>
#include <string>
#include <cmath>

namespace stf {

template <int dim>
std::unique_ptr<SpaceTimeFunction<dim>> YamlParser<dim>::parse_from_file(const std::string& filename) {
    try {
        YAML::Node node = YAML::LoadFile(filename);
        return parse_from_node(node);
    } catch (const YAML::Exception& e) {
        std::stringstream err_msg;
        err_msg << "Failed to load file '" << filename << "': " << e.what();
        throw YamlParseError(err_msg.str());
    }
}

template <int dim>
std::unique_ptr<SpaceTimeFunction<dim>> YamlParser<dim>::parse_from_string(const std::string& yaml_string) {
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
std::unique_ptr<SpaceTimeFunction<dim>> YamlParser<dim>::parse_from_node(const YAML::Node& node) {
    validate_dimension(node);
    validate_required_field(node, "type");
    
    std::string type = parse_string(node, "type");
    
    // Create parsing context to manage lifetimes
    auto context = std::make_unique<Context<dim>>();
    
    std::unique_ptr<SpaceTimeFunction<dim>> function;
    
    if (type == "explicit") {
        function = parse_explicit_form(node, *context);
    } else if (type == "sweep") {
        function = parse_sweep_function(node, *context);
    } else if (type == "offset") {
        function = parse_offset_function(node, *context);
    } else if (type == "union") {
        function = parse_union_function(node, *context);
    } else if (type == "interpolate") {
        function = parse_interpolate_function(node, *context);
    } else {
        throw YamlParseError("Unknown space-time function type: " + type);
    }
    
    // Wrap the function with lifetime management
    return std::make_unique<ManagedSpaceTimeFunction<dim>>(std::move(function), std::move(context));
}

template <int dim>
std::unique_ptr<SpaceTimeFunction<dim>> YamlParser<dim>::parse_explicit_form(const YAML::Node& node, Context<dim>& context) {
    // For explicit forms, we would need to support function definitions in YAML
    // This is complex and would require a scripting language or mathematical expression parser
    // For now, we'll throw an error suggesting this isn't supported via YAML
    throw YamlParseError("Explicit form functions cannot be defined in YAML. Use C++ API directly for custom functions.");
}

template <int dim>
std::unique_ptr<SpaceTimeFunction<dim>> YamlParser<dim>::parse_sweep_function(const YAML::Node& node, Context<dim>& context) {
    validate_required_field(node, "primitive");
    validate_required_field(node, "transform");
    
    auto primitive = parse_primitive(node["primitive"], context);
    auto transform = parse_transform(node["transform"], context);
    
    // Store the objects and get raw pointers
    auto* primitive_ptr = context.add_primitive(std::move(primitive));
    auto* transform_ptr = context.add_transform(std::move(transform));
    
    return std::make_unique<SweepFunction<dim>>(*primitive_ptr, *transform_ptr);
}

template <int dim>
std::unique_ptr<SpaceTimeFunction<dim>> YamlParser<dim>::parse_offset_function(const YAML::Node& node, Context<dim>& context) {
    validate_required_field(node, "base_function");
    
    // Parse the base function recursively - this will create its own ManagedSpaceTimeFunction
    auto base_function = parse_from_node(node["base_function"]);
    
    // For now, we'll support simple constant offsets
    Scalar offset = parse_scalar(node, "offset");
    Scalar offset_derivative = parse_scalar(node, "offset_derivative");
    
    auto offset_func = [offset](Scalar t) { return offset; };
    auto offset_deriv_func = [offset_derivative](Scalar t) { return offset_derivative; };
    
    // Store the base function and get raw pointer
    auto* base_function_ptr = context.add_function(std::move(base_function));
    
    return std::make_unique<OffsetFunction<dim>>(*base_function_ptr, offset_func, offset_deriv_func);
}

template <int dim>
std::unique_ptr<SpaceTimeFunction<dim>> YamlParser<dim>::parse_union_function(const YAML::Node& node, Context<dim>& context) {
    validate_required_field(node, "functions");
    
    if (!node["functions"].IsSequence()) {
        throw YamlParseError("'functions' field must be a sequence");
    }
    
    std::vector<std::unique_ptr<SpaceTimeFunction<dim>>> functions;
    for (const auto& func_node : node["functions"]) {
        functions.push_back(parse_from_node(func_node));
    }
    
    if (functions.size() < 2) {
        throw YamlParseError("Union function requires at least 2 functions");
    }
    
    // Store all functions and get raw pointers
    std::vector<SpaceTimeFunction<dim>*> function_ptrs;
    for (auto& func : functions) {
        function_ptrs.push_back(context.add_function(std::move(func)));
    }
    
    // For simplicity, we'll create a binary union tree
    auto result = std::make_unique<UnionFunction<dim>>(*function_ptrs[0], *function_ptrs[1]);
    
    for (size_t i = 2; i < function_ptrs.size(); ++i) {
        // Store intermediate union functions too
        auto* prev_union = context.add_function(std::move(result));
        result = std::make_unique<UnionFunction<dim>>(*prev_union, *function_ptrs[i]);
    }
    
    return std::move(result);
}

template <int dim>
std::unique_ptr<ImplicitFunction<dim>> YamlParser<dim>::parse_primitive(const YAML::Node& node, Context<dim>& context) {
    validate_required_field(node, "type");
    
    std::string type = parse_string(node, "type");
    
    if (type == "ball") {
        return parse_ball(node);
    } else if (type == "capsule") {
        return parse_capsule(node);
    } else if (type == "torus") {
        return parse_torus(node);
    } else {
        throw YamlParseError("Unknown primitive type: " + type);
    }
}

template <int dim>
std::unique_ptr<Transform<dim>> YamlParser<dim>::parse_transform(const YAML::Node& node, Context<dim>& context) {
    validate_required_field(node, "type");
    
    std::string type = parse_string(node, "type");
    
    if (type == "translation") {
        return parse_translation(node);
    } else if (type == "scale") {
        return parse_scale(node);
    } else if (type == "rotation") {
        return parse_rotation(node);
    } else if (type == "compose") {
        return parse_compose(node, context);
    } else if (type == "polyline") {
        return parse_polyline(node);
    } else if (type == "polybezier") {
        return parse_polybezier(node);
    } else {
        throw YamlParseError("Unknown transform type: " + type);
    }
}

template <int dim>
std::unique_ptr<ImplicitFunction<dim>> YamlParser<dim>::parse_ball(const YAML::Node& node) {
    Scalar radius = parse_scalar(node, "radius");
    std::array<Scalar, dim> center = parse_array(node, "center");
    int degree = parse_int(node, "degree", 1);  // Default degree is 1
    
    return std::make_unique<ImplicitBall<dim>>(radius, center, degree);
}

template <int dim>
std::unique_ptr<ImplicitFunction<dim>> YamlParser<dim>::parse_capsule(const YAML::Node& node) {
    if constexpr (dim != 3) {
        throw YamlParseError("Capsule primitive is only supported in 3D");
    }
    
    std::array<Scalar, dim> start = parse_array(node, "start");
    std::array<Scalar, dim> end = parse_array(node, "end");
    Scalar radius = parse_scalar(node, "radius");
    
    return std::make_unique<ImplicitCapsule<dim>>(radius, start, end);
}

template <int dim>
std::unique_ptr<ImplicitFunction<dim>> YamlParser<dim>::parse_torus(const YAML::Node& node) {
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
std::unique_ptr<Transform<dim>> YamlParser<dim>::parse_translation(const YAML::Node& node) {
    std::array<Scalar, dim> vector = parse_array(node, "vector");
    return std::make_unique<Translation<dim>>(vector);
}

template <int dim>
std::unique_ptr<Transform<dim>> YamlParser<dim>::parse_scale(const YAML::Node& node) {
    std::array<Scalar, dim> factors = parse_array(node, "factors");
    
    std::array<Scalar, dim> center{0};
    if (node["center"]) {
        center = parse_array(node, "center");
    }
    
    return std::make_unique<Scale<dim>>(factors, center);
}

template <int dim>
std::unique_ptr<Transform<dim>> YamlParser<dim>::parse_rotation(const YAML::Node& node) {
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
std::unique_ptr<Transform<dim>> YamlParser<dim>::parse_compose(const YAML::Node& node, Context<dim>& context) {
    validate_required_field(node, "transforms");
    
    if (!node["transforms"].IsSequence()) {
        throw YamlParseError("'transforms' field must be a sequence");
    }
    
    std::vector<std::unique_ptr<Transform<dim>>> transforms;
    for (const auto& transform_node : node["transforms"]) {
        transforms.push_back(parse_transform(transform_node, context));
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
std::unique_ptr<Transform<dim>> YamlParser<dim>::parse_polyline(const YAML::Node& node) {
    validate_required_field(node, "points");
    
    if (!node["points"].IsSequence()) {
        throw YamlParseError("'points' field must be a sequence");
    }
    
    std::vector<std::array<Scalar, dim>> points;
    for (const auto& point_node : node["points"]) {
        if (!point_node.IsSequence()) {
            throw YamlParseError("Each point must be a sequence");
        }
        
        if (point_node.size() != dim) {
            throw YamlParseError("Each point must have exactly " + std::to_string(dim) + " coordinates");
        }
        
        std::array<Scalar, dim> point;
        for (int i = 0; i < dim; ++i) {
            point[i] = point_node[i].as<Scalar>();
        }
        points.push_back(point);
    }
    
    if (points.size() < 2) {
        throw YamlParseError("Polyline must have at least 2 points");
    }
    
    return std::make_unique<Polyline<dim>>(std::move(points));
}

template <int dim>
std::unique_ptr<Transform<dim>> YamlParser<dim>::parse_polybezier(const YAML::Node& node) {
    // Check if we have control_points (direct specification) or sample_points (from samples)
    if (node["control_points"]) {
        // Direct control points specification
        if (!node["control_points"].IsSequence()) {
            throw YamlParseError("'control_points' field must be a sequence");
        }
        
        std::vector<std::array<Scalar, dim>> control_points;
        for (const auto& point_node : node["control_points"]) {
            if (!point_node.IsSequence()) {
                throw YamlParseError("Each control point must be a sequence");
            }
            
            if (point_node.size() != dim) {
                throw YamlParseError("Each control point must have exactly " + std::to_string(dim) + " coordinates");
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
        
        bool follow_tangent = parse_bool(node, "follow_tangent", true);
        
        return std::make_unique<PolyBezier<dim>>(std::move(control_points), follow_tangent);
        
    } else if (node["sample_points"]) {
        // Create from sample points
        if (!node["sample_points"].IsSequence()) {
            throw YamlParseError("'sample_points' field must be a sequence");
        }
        
        std::vector<std::array<Scalar, dim>> sample_points;
        for (const auto& point_node : node["sample_points"]) {
            if (!point_node.IsSequence()) {
                throw YamlParseError("Each sample point must be a sequence");
            }
            
            if (point_node.size() != dim) {
                throw YamlParseError("Each sample point must have exactly " + std::to_string(dim) + " coordinates");
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
        
        bool follow_tangent = parse_bool(node, "follow_tangent", true);
        
        auto bezier = PolyBezier<dim>::from_samples(std::move(sample_points), follow_tangent);
        return std::make_unique<PolyBezier<dim>>(std::move(bezier));
        
    } else {
        throw YamlParseError("PolyBezier requires either 'control_points' or 'sample_points' field");
    }
}

template <int dim>
std::unique_ptr<SpaceTimeFunction<dim>> YamlParser<dim>::parse_interpolate_function(const YAML::Node& node, Context<dim>& context) {
    validate_required_field(node, "function1");
    validate_required_field(node, "function2");
    
    // Parse the two functions to interpolate between
    auto function1 = parse_from_node(node["function1"]);
    auto function2 = parse_from_node(node["function2"]);
    
    // Store the functions and get raw pointers
    auto* function1_ptr = context.add_function(std::move(function1));
    auto* function2_ptr = context.add_function(std::move(function2));
    
    // Parse interpolation type (optional, default is linear)
    std::string interpolation_type = "linear";
    if (node["interpolation_type"]) {
        interpolation_type = parse_string(node, "interpolation_type");
    }
    
    // Create interpolation functions based on type
    std::function<Scalar(Scalar)> interpolation_func;
    std::function<Scalar(Scalar)> interpolation_derivative;
    
    if (interpolation_type == "linear") {
        interpolation_func = [](Scalar t) { return t; };
        interpolation_derivative = [](Scalar t) { return 1.0; };
    } else if (interpolation_type == "smooth") {
        // Smooth step interpolation: 3t² - 2t³
        interpolation_func = [](Scalar t) { return 3 * t * t - 2 * t * t * t; };
        interpolation_derivative = [](Scalar t) { return 6 * t - 6 * t * t; };
    } else if (interpolation_type == "smoother") {
        // Smoother step interpolation: 6t⁵ - 15t⁴ + 10t³
        interpolation_func = [](Scalar t) { 
            return 6 * t * t * t * t * t - 15 * t * t * t * t + 10 * t * t * t; 
        };
        interpolation_derivative = [](Scalar t) { 
            return 30 * t * t * t * t - 60 * t * t * t + 30 * t * t; 
        };
    } else if (interpolation_type == "cosine") {
        // Cosine interpolation: (1 - cos(πt)) / 2
        interpolation_func = [](Scalar t) { 
            return (1.0 - std::cos(M_PI * t)) / 2.0; 
        };
        interpolation_derivative = [](Scalar t) { 
            return M_PI * std::sin(M_PI * t) / 2.0; 
        };
    } else if (interpolation_type == "custom") {
        // For custom interpolation, we would need to parse mathematical expressions
        // For now, throw an error suggesting this isn't supported
        throw YamlParseError("Custom interpolation functions are not yet supported in YAML. Use 'linear', 'smooth', 'smoother', or 'cosine'.");
    } else {
        throw YamlParseError("Unknown interpolation type: " + interpolation_type + ". Supported types: 'linear', 'smooth', 'smoother', 'cosine'");
    }
    
    return std::make_unique<InterpolateFunction<dim>>(*function1_ptr, *function2_ptr, interpolation_func, interpolation_derivative);
}

// Utility function implementations
template <int dim>
std::array<Scalar, dim> YamlParser<dim>::parse_array(const YAML::Node& node, const std::string& field_name) {
    if (!node[field_name]) {
        throw YamlParseError("Missing required field: " + field_name);
    }
    
    if (!node[field_name].IsSequence()) {
        throw YamlParseError("Field '" + field_name + "' must be a sequence");
    }
    
    if (node[field_name].size() != dim) {
        throw YamlParseError("Field '" + field_name + "' must have exactly " + std::to_string(dim) + " elements");
    }
    
    std::array<Scalar, dim> result;
    for (int i = 0; i < dim; ++i) {
        result[i] = node[field_name][i].as<Scalar>();
    }
    
    return result;
}

template <int dim>
Scalar YamlParser<dim>::parse_scalar(const YAML::Node& node, const std::string& field_name) {
    if (!node[field_name]) {
        throw YamlParseError("Missing required field: " + field_name);
    }
    
    return node[field_name].as<Scalar>();
}

template <int dim>
std::string YamlParser<dim>::parse_string(const YAML::Node& node, const std::string& field_name) {
    if (!node[field_name]) {
        throw YamlParseError("Missing required field: " + field_name);
    }
    
    return node[field_name].as<std::string>();
}

template <int dim>
int YamlParser<dim>::parse_int(const YAML::Node& node, const std::string& field_name) {
    if (!node[field_name]) {
        throw YamlParseError("Missing required field: " + field_name);
    }
    
    return node[field_name].as<int>();
}

template <int dim>
int YamlParser<dim>::parse_int(const YAML::Node& node, const std::string& field_name, int default_value) {
    if (!node[field_name]) {
        return default_value;
    }
    
    return node[field_name].as<int>();
}

template <int dim>
bool YamlParser<dim>::parse_bool(const YAML::Node& node, const std::string& field_name, bool default_value) {
    if (!node[field_name]) {
        return default_value;
    }
    
    return node[field_name].as<bool>();
}

template <int dim>
void YamlParser<dim>::validate_dimension(const YAML::Node& node) {
    if (node["dimension"]) {
        int yaml_dim = node["dimension"].as<int>();
        if (yaml_dim != dim) {
            throw YamlParseError("Dimension mismatch: YAML specifies " + std::to_string(yaml_dim) + 
                               " but parser is for " + std::to_string(dim) + "D");
        }
    }
}

template <int dim>
void YamlParser<dim>::validate_required_field(const YAML::Node& node, const std::string& field_name) {
    if (!node[field_name]) {
        throw YamlParseError("Missing required field: " + field_name);
    }
}

// Explicit template instantiations
template class YamlParser<2>;
template class YamlParser<3>;

// Convenience function instantiations
template std::unique_ptr<SpaceTimeFunction<2>> parse_space_time_function_from_file<2>(const std::string&);
template std::unique_ptr<SpaceTimeFunction<3>> parse_space_time_function_from_file<3>(const std::string&);
template std::unique_ptr<SpaceTimeFunction<2>> parse_space_time_function_from_string<2>(const std::string&);
template std::unique_ptr<SpaceTimeFunction<3>> parse_space_time_function_from_string<3>(const std::string&);

} // namespace stf

#endif // STF_YAML_PARSER_ENABLED
