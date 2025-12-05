# YAML Parser for Space-Time Functions

The STF library now supports parsing space-time functions from YAML files, making it easy to define complex space-time functions declaratively without writing C++ code. The parser supports various primitives (ball, capsule, torus) and transforms including basic transforms (translation, rotation, scale), path-based transforms (polyline, polybezier), and transform composition.

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

### Polyline
Creates piecewise linear paths through a sequence of points:
```yaml
transform:
  type: polyline
  points:
    - [0.0, 0.0, 0.0]    # First point
    - [1.0, 0.0, 0.0]    # Second point
    - [1.0, 1.0, 0.0]    # Third point
    - [1.0, 1.0, 1.0]    # Fourth point
    # ... more points
```

**Requirements:**
- Minimum 2 points required
- Each point must have correct number of coordinates for the dimension
- Uses Bishop frames for consistent orientation along the path

### PolyBezier
Creates smooth curved paths using piecewise cubic Bezier curves.

**Option 1: Direct control points specification**
```yaml
transform:
  type: polybezier
  control_points:
    # First Bezier segment (4 control points)
    - [0.0, 0.0, 0.0]    # P0 - start point
    - [0.5, 0.0, 0.0]    # P1 - control point
    - [0.5, 0.5, 0.0]    # P2 - control point
    - [1.0, 0.5, 0.0]    # P3 - end point
    # Second Bezier segment (3 more points, P3 is shared)
    - [1.5, 0.5, 0.0]    # P4 - control point
    - [1.5, 1.0, 0.5]    # P5 - control point
    - [1.0, 1.0, 1.0]    # P6 - end point
  follow_tangent: true   # Optional, default: true
```

**Option 2: Generate from sample points**
```yaml
transform:
  type: polybezier
  sample_points:
    - [0.0, 0.0, 0.0]    # Point 1 (curve passes through)
    - [1.0, 0.0, 0.5]    # Point 2 (curve passes through)
    - [2.0, 1.0, 0.5]    # Point 3 (curve passes through)
    - [2.5, 2.0, 0.0]    # Point 4 (curve passes through)
    # ... more sample points
  follow_tangent: true   # Optional, default: true
```

**Parameters:**
- `control_points`: Direct Bezier control points (minimum 4, must follow (n×3)+1 pattern)
- `sample_points`: Points the curve passes through (minimum 3, control points generated automatically)
- `follow_tangent`: Whether to align coordinate system with curve tangent (default: true)

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

### Polyline Path
A ball following a piecewise linear path:
```yaml
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
    - [0.0, 0.0, 0.0]    # Start point
    - [1.0, 0.0, 0.0]    # Move along x-axis
    - [1.0, 1.0, 0.0]    # Turn along y-axis
    - [1.0, 1.0, 1.0]    # Move up along z-axis
    - [0.0, 1.0, 1.0]    # Move back along x-axis
```

### Smooth Bezier Curve
A capsule following a smooth curved path:
```yaml
type: sweep
dimension: 3
primitive:
  type: capsule
  radius: 0.1
  start: [0.0, 0.0, -0.2]
  end: [0.0, 0.0, 0.2]
transform:
  type: polybezier
  sample_points:
    - [0.0, 0.0, 0.0]    # Curve passes through these points
    - [1.0, 0.0, 0.5]
    - [2.0, 1.0, 0.5]
    - [2.5, 2.0, 0.0]
    - [2.0, 3.0, -0.5]
    - [1.0, 3.5, -0.5]
    - [0.0, 3.0, 0.0]
  follow_tangent: true
```

### 2D Polyline Path
A 2D circle following a square path:
```yaml
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
    - [0.0, 0.0]     # Start point
    - [2.0, 0.0]     # Move right
    - [2.0, 2.0]     # Move up
    - [0.0, 2.0]     # Move left
    - [0.0, 0.0]     # Return to start
```

## Error Handling

The parser provides detailed error messages for common issues:

- Missing required fields
- Invalid field types or values
- Dimension mismatches
- Unknown primitive or transform types
- Invalid YAML syntax
- **Polyline errors**: Too few points (< 2), incorrect point dimensions
- **PolyBezier errors**: Too few control points (< 4), wrong control point pattern (not (n×3)+1), too few sample points (< 3)

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
