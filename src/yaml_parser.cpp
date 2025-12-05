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
    } else {
        throw YamlParseError("Unknown transform type: " + type);
    }
}

template <int dim>
std::unique_ptr<ImplicitFunction<dim>> YamlParser<dim>::parse_ball(const YAML::Node& node) {
    Scalar radius = parse_scalar(node, "radius");
    std::array<Scalar, dim> center = parse_array(node, "center");
    int degree = parse_int(node, "degree");
    
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
