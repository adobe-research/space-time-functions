# YAML Specification for Space-Time Functions

This document describes the YAML format for defining space-time functions in the STF library.

## Table of Contents

1. [Basic Structure](#basic-structure)
2. [Space-Time Function Types](#space-time-function-types)
3. [Primitive Types](#primitive-types)
4. [Transform Types](#transform-types)
5. [External File Support](#external-file-support)
6. [Examples](#examples)

## Basic Structure

All YAML files must specify the following top-level fields:

```yaml
type: <function_type>
dimension: <2|3>
# Additional fields depend on the function type
```

### Required Fields

- `type`: The type of space-time function (see [Space-Time Function Types](#space-time-function-types))
- `dimension`: The spatial dimension (2 or 3)

## Space-Time Function Types

### Sweep Function

Creates a space-time function by sweeping a primitive along a transform path.

```yaml
type: sweep
dimension: <2|3>
primitive:
  # Primitive definition (see Primitive Types)
transform:
  # Transform definition (see Transform Types)
```

### Offset Function

Adds a time-dependent offset to another space-time function.

```yaml
type: offset
dimension: <2|3>
base_function:
  # Nested space-time function definition
offset: <scalar>              # Constant offset value
offset_derivative: <scalar>   # Constant offset derivative
```

### Union Function

Combines multiple space-time functions using smooth union.

```yaml
type: union
dimension: <2|3>
functions:
  - # First space-time function
  - # Second space-time function
  # ... additional functions
smooth_distance: <scalar>    # Optional, defaults to 0.0 (hard union)
```

#### Parameters

- `functions`: Array of space-time function definitions (minimum 2 required)
- `smooth_distance`: Distance over which to smooth the union (0 = hard union, >0 = smooth union)

### Interpolate Function

Linearly interpolates between two space-time functions over time.

```yaml
type: interpolate
dimension: <2|3>
function1:
  # First space-time function (at t=0)
function2:
  # Second space-time function (at t=1)
interpolation_type: <linear|smooth|cosine>  # Optional, defaults to linear
```

## Primitive Types

### Ball

A sphere (3D) or circle (2D) primitive.

```yaml
type: ball
radius: <scalar>
center: [<x>, <y>]           # 2D
center: [<x>, <y>, <z>]      # 3D
degree: <integer>            # Optional, defaults to 1
```

### Capsule (3D only)

A capsule primitive defined by two endpoints and a radius.

```yaml
type: capsule
start: [<x>, <y>, <z>]
end: [<x>, <y>, <z>]
radius: <scalar>
```

### Torus (3D only)

A torus primitive with major and minor radii.

```yaml
type: torus
major_radius: <scalar>
minor_radius: <scalar>
center: [<x>, <y>, <z>]
```

### Duchon (3D only)

A Duchon interpolant loaded from external files.

```yaml
type: duchon
samples_file: <path>         # Path to .xyz file with sample points
coeffs_file: <path>          # Path to coefficients file
center: [<x>, <y>, <z>]      # Optional, defaults to [0, 0, 0]
radius: <scalar>             # Optional, defaults to 1.0
positive_inside: <boolean>   # Optional, defaults to false
```

### Implicit Union

A smooth union of multiple implicit primitives using various blending functions.

```yaml
type: implicit_union
primitives:
  - # First primitive definition
  - # Second primitive definition
  # ... additional primitives (minimum 2 required)
smooth_distance: <scalar>    # Optional, defaults to 0.0 (hard union)
blending: <function>         # Optional, defaults to "quadratic"
```

#### Blending Functions

The `blending` parameter supports the following functions (in order of "roundness"):

- `circular`: Most rounded blending
- `quadratic`: Smooth quadratic blending (default)
- `quartic`: Quartic blending
- `cubic`: Least rounded blending

#### Parameters

- `primitives`: Array of primitive definitions (any combination of ball, capsule, torus, duchon, or nested implicit_union)
- `smooth_distance`: Distance over which to smooth the union (0 = hard union, >0 = smooth union)
- `blending`: Blending function type for smooth transitions

## Transform Types

### Translation

Translates along a vector over time.

```yaml
type: translation
vector: [<x>, <y>]           # 2D
vector: [<x>, <y>, <z>]      # 3D
```

### Scale

Scales by factors over time.

```yaml
type: scale
factors: [<fx>, <fy>]        # 2D
factors: [<fx>, <fy>, <fz>]  # 3D
center: [<x>, <y>]           # Optional, defaults to origin
center: [<x>, <y>, <z>]      # Optional, defaults to origin
```

### Rotation

Rotates around an axis (3D) or point (2D) over time.

```yaml
# 2D rotation
type: rotation
angle: <scalar>              # Rotation angle in radians
center: [<x>, <y>]           # Optional, defaults to origin

# 3D rotation
type: rotation
angle: <scalar>              # Rotation angle in radians
axis: [<x>, <y>, <z>]        # Rotation axis (normalized)
center: [<x>, <y>, <z>]      # Optional, defaults to origin
```

### Compose

Composes multiple transforms in sequence.

```yaml
type: compose
transforms:
  - # First transform
  - # Second transform
  # ... additional transforms
```

### Polyline

Moves along a polyline path defined by connected line segments.

#### Inline Points

```yaml
type: polyline
points:
  - [<x1>, <y1>]             # 2D
  - [<x2>, <y2>]
  # ... additional points (minimum 2 required)

# OR for 3D
points:
  - [<x1>, <y1>, <z1>]       # 3D
  - [<x2>, <y2>, <z2>]
  # ... additional points
```

#### External XYZ File

```yaml
type: polyline
points_file: <path>          # Path to XYZ file containing points
```

### PolyBezier

Moves along a piecewise cubic Bézier curve.

#### Control Points (Inline)

```yaml
type: polybezier
control_points:
  - [<x1>, <y1>, <z1>]       # First control point
  - [<x2>, <y2>, <z2>]       # First control handle
  - [<x3>, <y3>, <z3>]       # Second control handle
  - [<x4>, <y4>, <z4>]       # Second control point
  # ... additional control points (must be (n*3)+1 total)
follow_tangent: <boolean>    # Optional, defaults to true
```

#### Control Points (External File)

```yaml
type: polybezier
control_points_file: <path>  # Path to XYZ file with control points
follow_tangent: <boolean>    # Optional, defaults to true
```

#### Sample Points (Inline)

```yaml
type: polybezier
sample_points:
  - [<x1>, <y1>, <z1>]       # Points to fit curve through
  - [<x2>, <y2>, <z2>]
  # ... additional sample points (minimum 3 required)
follow_tangent: <boolean>    # Optional, defaults to true
```

#### Sample Points (External File)

```yaml
type: polybezier
sample_points_file: <path>   # Path to XYZ file with sample points
follow_tangent: <boolean>    # Optional, defaults to true
```

## External File Support

The STF YAML parser supports loading point data from external XYZ files for several use cases:

### XYZ File Format

XYZ files follow this format:

```
<dimension>
<x1> <y1> [<z1>]
<x2> <y2> [<z2>]
...
<xn> <yn> [<zn>]
```

**Example 2D XYZ file:**
```
2
0.0 0.0
1.0 0.0
1.0 1.0
0.0 1.0
```

**Example 3D XYZ file:**
```
3
0.0 0.0 0.0
1.0 0.0 0.0
1.0 1.0 0.0
0.0 1.0 1.0
```

### Relative Path Resolution

All file paths in YAML are resolved relative to the directory containing the YAML file:

```yaml
# If this YAML is in /path/to/config.yaml
type: sweep
dimension: 3
primitive:
  type: duchon
  samples_file: data/samples.xyz     # Resolves to /path/to/data/samples.xyz
  coeffs_file: ../shared/coeffs.txt  # Resolves to /path/shared/coeffs.txt
```

### Supported File Loading

| Transform/Primitive | Field | File Type | Description |
|-------------------|-------|-----------|-------------|
| `polyline` | `points_file` | XYZ | Polyline vertex coordinates |
| `polybezier` | `control_points_file` | XYZ | Bézier control point coordinates |
| `polybezier` | `sample_points_file` | XYZ | Sample points for curve fitting |
| `duchon` | `samples_file` | XYZ | 3D sample point coordinates |
| `duchon` | `coeffs_file` | Text | RBF and affine coefficients |
| `implicit_union` | (nested primitives) | - | Can contain primitives with file references |

### Advantages of External Files

1. **Large datasets**: Handle thousands of points without cluttering YAML
2. **Data reuse**: Share point datasets across multiple configurations
3. **Tool integration**: Generate points from CAD software, simulations, etc.
4. **Version control**: Track point data changes separately from configuration
5. **Performance**: Faster parsing of large point sets

### Error Handling

The parser provides clear error messages for common issues:

- **Missing files**: "Failed to open XYZ file: /path/to/file.xyz"
- **Dimension mismatch**: "XYZ file dimension (2) does not match expected dimension (3)"
- **Invalid format**: "No valid points found in XYZ file"
- **Insufficient points**: "Polyline must have at least 2 points"

## Examples

### Simple 2D Sweep with Ball

```yaml
type: sweep
dimension: 2
primitive:
  type: ball
  radius: 0.5
  center: [0.0, 0.0]
transform:
  type: translation
  vector: [1.0, 0.0]
```

### 3D Polyline from External File

```yaml
type: sweep
dimension: 3
primitive:
  type: ball
  radius: 0.2
  center: [0.0, 0.0, 0.0]
transform:
  type: polyline
  points_file: data/path_points.xyz
```

### PolyBezier with Control Points from File

```yaml
type: sweep
dimension: 3
primitive:
  type: capsule
  start: [0.0, 0.0, 0.0]
  end: [0.0, 0.0, 0.1]
  radius: 0.05
transform:
  type: polybezier
  control_points_file: data/bezier_controls.xyz
  follow_tangent: true
```

### Duchon Surface with External Data

```yaml
type: sweep
dimension: 3
primitive:
  type: duchon
  samples_file: data/surface_samples.xyz
  coeffs_file: data/surface_coeffs.txt
  center: [0.0, 0.0, 0.0]
  radius: 2.0
  positive_inside: false
transform:
  type: scale
  factors: [1.0, 1.0, 2.0]
```

### Complex Union with Multiple Functions

```yaml
type: union
dimension: 3
functions:
  - type: sweep
    primitive:
      type: ball
      radius: 0.3
      center: [0.0, 0.0, 0.0]
    transform:
      type: translation
      vector: [1.0, 0.0, 0.0]
  - type: sweep
    primitive:
      type: torus
      major_radius: 0.8
      minor_radius: 0.2
      center: [0.0, 0.0, 0.0]
    transform:
      type: rotation
      angle: 1.57
      axis: [0.0, 1.0, 0.0]
smooth_distance: 0.3         # Smooth blending between functions
```

### Interpolation Between Two Functions

```yaml
type: interpolate
dimension: 2
function1:
  type: sweep
  primitive:
    type: ball
    radius: 0.2
    center: [0.0, 0.0]
  transform:
    type: translation
    vector: [1.0, 0.0]
function2:
  type: sweep
  primitive:
    type: ball
    radius: 0.4
    center: [0.0, 0.0]
  transform:
    type: translation
    vector: [0.0, 1.0]
interpolation_type: smooth
```

### Implicit Union Examples

#### Simple Smooth Union

```yaml
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
```

#### Complex Multi-Primitive Union

```yaml
type: sweep
dimension: 3
primitive:
  type: implicit_union
  primitives:
    - type: ball
      radius: 0.4
      center: [0.0, 0.0, 0.0]
      degree: 1
    - type: capsule
      start: [0.5, 0.0, 0.0]
      end: [0.5, 0.0, 0.8]
      radius: 0.2
    - type: torus
      major_radius: 0.6
      minor_radius: 0.1
      center: [0.0, 0.8, 0.0]
  smooth_distance: 0.3
  blending: circular
transform:
  type: scale
  factors: [1.0, 1.0, 1.0]
```

#### Hard Union (No Smoothing)

```yaml
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
  smooth_distance: 0.0  # Hard union
  blending: quadratic
transform:
  type: translation
  vector: [0.0, 0.0]
```

#### Nested Implicit Unions

```yaml
type: sweep
dimension: 3
primitive:
  type: implicit_union
  primitives:
    - type: implicit_union
      primitives:
        - type: ball
          radius: 0.3
          center: [0.0, 0.0, 0.0]
          degree: 1
        - type: ball
          radius: 0.2
          center: [0.4, 0.0, 0.0]
          degree: 1
      smooth_distance: 0.1
      blending: quadratic
    - type: capsule
      start: [0.0, 0.5, 0.0]
      end: [0.4, 0.5, 0.0]
      radius: 0.15
  smooth_distance: 0.2
  blending: circular
transform:
  type: translation
  vector: [0.0, 0.0, 0.0]
```