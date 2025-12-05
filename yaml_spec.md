# YAML Parser for Space-Time Functions

The STF library now supports parsing space-time functions from YAML files, making it easy to define complex space-time functions declaratively without writing C++ code.

## Basic Usage

```cpp
#include <stf/stf.h>

// Parse from file
auto func = stf::parse_space_time_function_from_file<3>("my_function.yaml");

// Parse from string
std::string yaml_content = "...";
auto func = stf::parse_space_time_function_from_string<3>(yaml_content);

// Use the function
std::array<stf::Scalar, 3> pos = {1.0, 0.0, 0.0};
stf::Scalar t = 0.5;
auto value = func->value(pos, t);
auto gradient = func->gradient(pos, t);
```

## YAML Format

### Basic Structure

All YAML files must specify:
- `type`: The type of space-time function
- `dimension`: The spatial dimension (2 or 3)

### Sweep Functions

Sweep functions are created by sweeping a primitive shape through space using a transform:

```yaml
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
```

## Supported Primitives

### Ball (2D/3D)
```yaml
primitive:
  type: ball
  radius: 0.5
  center: [0.0, 0.0, 0.0]  # [x, y] for 2D, [x, y, z] for 3D
  degree: 1                # Distance function degree
```

### Capsule (3D only)
```yaml
primitive:
  type: capsule
  radius: 0.2
  start: [0.0, 0.0, 0.0]   # Start point of capsule axis
  end: [0.0, 0.0, 1.0]     # End point of capsule axis
```

### Torus (3D only)
```yaml
primitive:
  type: torus
  major_radius: 1.0        # Distance from center to tube center
  minor_radius: 0.3        # Tube radius
  center: [0.0, 0.0, 0.0]
```

## Supported Transforms

### Translation
```yaml
transform:
  type: translation
  vector: [1.0, 0.0, 0.0]  # Translation vector per unit time
```

### Scale
```yaml
transform:
  type: scale
  factors: [2.0, 1.0, 1.0] # Scale factors per dimension
  center: [0.0, 0.0, 0.0]  # Optional: center of scaling (default: origin)
```

### Rotation

For 2D:
```yaml
transform:
  type: rotation
  angle: 90.0              # Total rotation in degrees
  center: [0.0, 0.0]       # Optional: center of rotation
```

For 3D:
```yaml
transform:
  type: rotation
  axis: [0.0, 0.0, 1.0]    # Rotation axis
  angle: 90.0              # Total rotation in degrees
  center: [0.0, 0.0, 0.0]  # Optional: center of rotation
```

### Compose (Multiple Transforms)
```yaml
transform:
  type: compose
  transforms:
    - type: translation
      vector: [1.0, 0.0, 0.0]
    - type: rotation
      axis: [0.0, 0.0, 1.0]
      angle: 180.0
      center: [0.0, 0.0, 0.0]
```

## Examples

### Moving Ball
A ball that moves along the x-axis:
```yaml
type: sweep
dimension: 3
primitive:
  type: ball
  radius: 0.5
  center: [0.0, 0.0, 0.0]
  degree: 1
transform:
  type: translation
  vector: [2.0, 0.0, 0.0]
```

### Rotating Capsule
A capsule that rotates around the z-axis:
```yaml
type: sweep
dimension: 3
primitive:
  type: capsule
  radius: 0.2
  start: [1.0, 0.0, -0.5]
  end: [1.0, 0.0, 0.5]
transform:
  type: rotation
  axis: [0.0, 0.0, 1.0]
  angle: 360.0
  center: [0.0, 0.0, 0.0]
```

### Complex Motion
A ball with both rotation and translation:
```yaml
type: sweep
dimension: 3
primitive:
  type: ball
  radius: 0.3
  center: [1.0, 0.0, 0.0]
  degree: 1
transform:
  type: compose
  transforms:
    - type: rotation
      axis: [0.0, 0.0, 1.0]
      angle: 180.0
      center: [0.0, 0.0, 0.0]
    - type: translation
      vector: [0.0, 0.0, 1.0]
```

## Error Handling

The parser provides detailed error messages for common issues:

- Missing required fields
- Invalid field types or values
- Dimension mismatches
- Unknown primitive or transform types
- Invalid YAML syntax

```cpp
try {
    auto func = stf::parse_space_time_function_from_file<3>("invalid.yaml");
} catch (const stf::YamlParseError& e) {
    std::cerr << "Parse error: " << e.what() << std::endl;
}
```

## Limitations

- Explicit form functions cannot be defined in YAML (use C++ API for custom functions)
- Union and offset functions are not yet supported in YAML format
- Function parameters cannot be expressions (only literal values)

## Building with YAML Support

The YAML parser requires yaml-cpp. It's automatically included when building the library:

```bash
mkdir build && cd build
cmake .. -DSTF_BUILD_TESTS=ON
make
```

The yaml-cpp dependency is handled automatically by the CMake configuration.
