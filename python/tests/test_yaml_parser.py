"""Tests for YAML parser Python bindings."""

import pytest
import tempfile
import os
from pathlib import Path

try:
    import space_time_functions as stf
    YAML_PARSER_AVAILABLE = hasattr(stf, 'parse_space_time_function_from_string')
except ImportError:
    YAML_PARSER_AVAILABLE = False

pytestmark = pytest.mark.skipif(
    not YAML_PARSER_AVAILABLE, 
    reason="YAML parser not available (STF_YAML_PARSER not enabled)"
)


class TestYamlParser:
    """Test YAML parser Python bindings."""

    def test_parse_from_string_3d(self):
        """Test parsing 3D space-time function from YAML string."""
        yaml_content = """
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
"""
        
        func = stf.parse_space_time_function_from_string(yaml_content)
        assert func is not None
        
        # Test function evaluation
        pos = [0.5, 0.0, 0.0]
        t = 0.5
        value = func.value(pos, t)
        
        # At t=0.5, ball center should be at (0.5, 0, 0)
        # Distance from (0.5, 0, 0) should be -0.5 (inside ball)
        assert abs(value + 0.5) < 1e-6

    def test_parse_from_string_2d(self):
        """Test parsing 2D space-time function from YAML string."""
        yaml_content = """
type: sweep
dimension: 2
primitive:
  type: ball
  radius: 0.4
  center: [0.0, 0.0]
  degree: 1
transform:
  type: translation
  vector: [-1.0, -1.0]
"""
        
        func = stf.parse_space_time_function_from_string_2d(yaml_content)
        assert func is not None
        
        # Test function evaluation
        pos = [0.5, 0.5]
        t = 0.5
        value = func.value(pos, t)
        
        # At t=0.5, circle center should be at (0.5, 0.5)
        # Distance from (0.5, 0.5) should be -0.4 (inside circle)
        assert abs(value + 0.4) < 1e-6

    def test_parse_from_file_3d(self):
        """Test parsing 3D space-time function from YAML file."""
        yaml_content = """
type: sweep
dimension: 3
primitive:
  type: ball
  radius: 0.3
  center: [0.0, 0.0, 0.0]
  degree: 1
transform:
  type: scale
  factors: [2.0, 1.0, 1.0]
  center: [0.0, 0.0, 0.0]
"""
        
        # Create temporary file
        with tempfile.NamedTemporaryFile(mode='w', suffix='.yaml', delete=False) as f:
            f.write(yaml_content)
            temp_filename = f.name
        
        try:
            func = stf.parse_space_time_function_from_file(temp_filename)
            assert func is not None
            
            # Test function evaluation
            pos = [1.0, 0.0, 0.0]
            t = 0.5
            value = func.value(pos, t)
            
            # Function should return a finite value
            assert abs(value) < float('inf')
            
        finally:
            os.unlink(temp_filename)

    def test_parse_from_file_2d(self):
        """Test parsing 2D space-time function from YAML file."""
        yaml_content = """
type: sweep
dimension: 2
primitive:
  type: ball
  radius: 0.6
  center: [0.0, 0.0]
  degree: 1
transform:
  type: rotation
  angle: 90.0
  center: [0.0, 0.0]
"""
        
        # Create temporary file
        with tempfile.NamedTemporaryFile(mode='w', suffix='.yaml', delete=False) as f:
            f.write(yaml_content)
            temp_filename = f.name
        
        try:
            func = stf.parse_space_time_function_from_file_2d(temp_filename)
            assert func is not None
            
            # Test function evaluation
            pos = [0.0, 0.0]
            t = 0.25
            value = func.value(pos, t)
            
            # At center, should be negative (inside)
            assert value < 0
            
        finally:
            os.unlink(temp_filename)

    def test_yaml_parse_error(self):
        """Test that YamlParseError is raised for invalid YAML."""
        invalid_yaml = """
type: unknown_type
dimension: 3
"""
        
        with pytest.raises(stf.YamlParseError):
            stf.parse_space_time_function_from_string(invalid_yaml)

    def test_yaml_parse_error_missing_field(self):
        """Test that YamlParseError is raised for missing required fields."""
        invalid_yaml = """
type: sweep
dimension: 3
primitive:
  type: ball
  radius: 0.5
  # Missing center and degree
transform:
  type: translation
  vector: [1.0, 0.0, 0.0]
"""
        
        with pytest.raises(stf.YamlParseError):
            stf.parse_space_time_function_from_string(invalid_yaml)

    def test_yaml_parse_error_dimension_mismatch(self):
        """Test that YamlParseError is raised for dimension mismatch."""
        invalid_yaml = """
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
"""
        
        with pytest.raises(stf.YamlParseError):
            stf.parse_space_time_function_from_string_2d(invalid_yaml)

    def test_complex_function(self):
        """Test parsing a more complex function with composed transforms."""
        yaml_content = """
type: sweep
dimension: 3
primitive:
  type: capsule
  radius: 0.2
  start: [0.0, 0.0, -0.5]
  end: [0.0, 0.0, 0.5]
transform:
  type: compose
  transforms:
    - type: rotation
      axis: [0.0, 0.0, 1.0]
      angle: 180.0
      center: [0.0, 0.0, 0.0]
    - type: translation
      vector: [0.0, 1.0, 0.0]
"""
        
        func = stf.parse_space_time_function_from_string(yaml_content)
        assert func is not None
        
        # Test multiple evaluations to ensure object lifetime management works
        positions = [
            [0.0, 0.5, 0.0],
            [0.0, 1.0, 0.0], 
            [0.0, 1.5, 0.0]
        ]
        
        for pos in positions:
            for t in [0.0, 0.5, 1.0]:
                value = func.value(pos, t)
                assert abs(value) < float('inf'), f"Invalid value at pos={pos}, t={t}"

    def test_function_methods(self):
        """Test that all SpaceTimeFunction methods work on parsed functions."""
        yaml_content = """
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
"""
        
        func = stf.parse_space_time_function_from_string(yaml_content)
        pos = [0.5, 0.0, 0.0]
        t = 0.5
        
        # Test value method
        value = func.value(pos, t)
        assert isinstance(value, float)
        
        # Test time_derivative method
        time_deriv = func.time_derivative(pos, t)
        assert isinstance(time_deriv, float)
        
        # Test gradient method
        gradient = func.gradient(pos, t)
        assert len(gradient) == 4  # [df/dx, df/dy, df/dz, df/dt]
        assert all(isinstance(g, float) for g in gradient)

    def test_polyline_transform(self):
        """Test parsing polyline transform."""
        yaml_content = """
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
"""
        
        func = stf.parse_space_time_function_from_string(yaml_content)
        assert func is not None
        
        # Test function evaluation
        pos = [0.5, 0.0, 0.0]
        t = 0.25
        value = func.value(pos, t)
        assert abs(value) < float('inf')

    def test_polyline_2d_transform(self):
        """Test parsing 2D polyline transform."""
        yaml_content = """
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
"""
        
        func = stf.parse_space_time_function_from_string_2d(yaml_content)
        assert func is not None
        
        # Test function evaluation
        pos = [1.0, 0.0]
        t = 0.25
        value = func.value(pos, t)
        assert abs(value) < float('inf')

    def test_polybezier_control_points(self):
        """Test parsing polybezier with control points."""
        yaml_content = """
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
"""
        
        func = stf.parse_space_time_function_from_string(yaml_content)
        assert func is not None
        
        # Test function evaluation
        pos = [0.5, 0.25, 0.0]
        t = 0.5
        value = func.value(pos, t)
        assert abs(value) < float('inf')

    def test_polybezier_from_samples(self):
        """Test parsing polybezier from sample points."""
        yaml_content = """
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
"""
        
        func = stf.parse_space_time_function_from_string(yaml_content)
        assert func is not None
        
        # Test function evaluation
        pos = [1.0, 0.5, 0.25]
        t = 0.5
        value = func.value(pos, t)
        assert abs(value) < float('inf')

    def test_polyline_error_handling(self):
        """Test error handling for invalid polyline."""
        yaml_content = """
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
"""
        
        with pytest.raises(stf.YamlParseError):
            stf.parse_space_time_function_from_string(yaml_content)

    def test_polybezier_error_handling(self):
        """Test error handling for invalid polybezier."""
        yaml_content = """
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
"""
        
        with pytest.raises(stf.YamlParseError):
            stf.parse_space_time_function_from_string(yaml_content)

    def test_interpolate_function_linear(self):
        """Test parsing interpolate function with linear interpolation."""
        yaml_content = """
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
"""
        
        func = stf.parse_space_time_function_from_string(yaml_content)
        assert func is not None
        
        # Test function evaluation at different interpolation points
        pos = [0.0, 0.0, 0.0]
        
        # At t=0, should be closer to function1
        value_0 = func.value(pos, 0.0)
        assert abs(value_0) < float('inf')
        
        # At t=1, should be closer to function2
        value_1 = func.value(pos, 1.0)
        assert abs(value_1) < float('inf')
        
        # At t=0.5, should be interpolated
        value_half = func.value(pos, 0.5)
        assert abs(value_half) < float('inf')

    def test_interpolate_function_smooth(self):
        """Test parsing interpolate function with smooth interpolation."""
        yaml_content = """
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
"""
        
        func = stf.parse_space_time_function_from_string_2d(yaml_content)
        assert func is not None
        
        # Test function evaluation
        pos = [0.5, 0.5]
        t = 0.5
        value = func.value(pos, t)
        assert abs(value) < float('inf')
        
        # Test gradient computation
        gradient = func.gradient(pos, t)
        assert len(gradient) == 3  # [df/dx, df/dy, df/dt]
        assert all(abs(g) < float('inf') for g in gradient)

    def test_interpolate_function_cosine(self):
        """Test parsing interpolate function with cosine interpolation."""
        yaml_content = """
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
"""
        
        func = stf.parse_space_time_function_from_string(yaml_content)
        assert func is not None
        
        # Test function evaluation
        pos = [0.0, 0.0, 0.0]
        t = 0.25
        value = func.value(pos, t)
        assert abs(value) < float('inf')
        
        # Test time derivative
        time_deriv = func.time_derivative(pos, t)
        assert abs(time_deriv) < float('inf')

    def test_interpolate_function_default_linear(self):
        """Test parsing interpolate function with default linear interpolation."""
        yaml_content = """
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
"""
        
        func = stf.parse_space_time_function_from_string(yaml_content)
        assert func is not None
        
        # Test function evaluation
        pos = [0.0, 0.0, 0.0]
        t = 0.5
        value = func.value(pos, t)
        assert abs(value) < float('inf')

    def test_interpolate_function_error_handling(self):
        """Test error handling for invalid interpolate function."""
        yaml_content = """
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
"""
        
        with pytest.raises(stf.YamlParseError):
            stf.parse_space_time_function_from_string(yaml_content)

    def test_interpolate_function_unknown_type_error(self):
        """Test error handling for unknown interpolation type."""
        yaml_content = """
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
"""
        
        with pytest.raises(stf.YamlParseError):
            stf.parse_space_time_function_from_string(yaml_content)

    def test_ball_optional_degree_parameter(self):
        """Test that degree parameter is optional for ball primitive with default value 1."""
        # Test with explicit degree
        yaml_explicit = """
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
"""
        
        # Test with default degree (omitted)
        yaml_default = """
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
"""
        
        func_explicit = stf.parse_space_time_function_from_string_2d(yaml_explicit)
        func_default = stf.parse_space_time_function_from_string_2d(yaml_default)
        
        assert func_explicit is not None
        assert func_default is not None
        
        # Both should give the same result
        pos = [0.5, 0.0]
        t = 0.0
        
        value_explicit = func_explicit.value(pos, t)
        value_default = func_default.value(pos, t)
        
        # Values should be approximately equal
        assert abs(value_explicit - value_default) < 1e-10
        
        # For degree 1, distance from center (0,0) to (0.5,0) should be 0.5 - 1.0 = -0.5
        assert abs(value_default - (-0.5)) < 1e-6

    def test_ball_different_degree_values(self):
        """Test that different degree values produce different results."""
        yaml_degree_1 = """
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
"""
        
        yaml_degree_2 = """
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
"""
        
        func_deg1 = stf.parse_space_time_function_from_string_2d(yaml_degree_1)
        func_deg2 = stf.parse_space_time_function_from_string_2d(yaml_degree_2)
        
        assert func_deg1 is not None
        assert func_deg2 is not None
        
        # Different degrees should produce different results
        pos = [0.5, 0.0]
        t = 0.0
        
        value_deg1 = func_deg1.value(pos, t)
        value_deg2 = func_deg2.value(pos, t)
        
        # Values should be different for different degrees
        assert abs(value_deg1 - value_deg2) > 1e-6

    def test_duchon_primitive_parsing(self):
        """Test parsing Duchon primitive with file paths."""
        import tempfile
        import os
        
        # Create temporary test files
        samples_content = "3\n0.0 0.0 0.0\n1.0 0.0 0.0\n0.0 1.0 0.0\n0.0 0.0 1.0\n"
        coeffs_content = "1.0 0.5 0.2 0.1\n0.8 0.3 0.1 0.0\n0.6 0.2 0.0 0.1\n0.4 0.1 0.0 0.0\n0.1 0.2 0.3 0.4\n"
        
        with tempfile.TemporaryDirectory() as temp_dir:
            samples_file = os.path.join(temp_dir, "samples.xyz")
            coeffs_file = os.path.join(temp_dir, "coeffs.txt")
            
            with open(samples_file, 'w') as f:
                f.write(samples_content)
            with open(coeffs_file, 'w') as f:
                f.write(coeffs_content)
            
            yaml_content = f"""
type: sweep
dimension: 3
primitive:
  type: duchon
  samples_file: {samples_file}
  coeffs_file: {coeffs_file}
  center: [0.0, 0.0, 0.0]
  radius: 1.0
  positive_inside: false
transform:
  type: translation
  vector: [0.0, 0.0, 0.0]
"""
            
            func = stf.parse_space_time_function_from_string(yaml_content)
            assert func is not None
            
            # Test function evaluation
            pos = [0.1, 0.1, 0.1]
            t = 0.0
            value = func.value(pos, t)
            assert abs(value) < float('inf')
            
            # Test gradient computation
            gradient = func.gradient(pos, t)
            assert len(gradient) == 4  # [df/dx, df/dy, df/dz, df/dt]
            assert all(abs(g) < float('inf') for g in gradient)

    def test_duchon_relative_paths(self):
        """Test that relative paths are resolved relative to YAML file directory."""
        import tempfile
        import os
        
        # Create temporary test files
        samples_content = "3\n0.0 0.0 0.0\n1.0 0.0 0.0\n0.0 1.0 0.0\n0.0 0.0 1.0\n"
        coeffs_content = "1.0 0.5 0.2 0.1\n0.8 0.3 0.1 0.0\n0.6 0.2 0.0 0.1\n0.4 0.1 0.0 0.0\n0.1 0.2 0.3 0.4\n"
        
        with tempfile.TemporaryDirectory() as temp_dir:
            # Create subdirectory for data files
            data_dir = os.path.join(temp_dir, "data")
            os.makedirs(data_dir)
            
            samples_file = os.path.join(data_dir, "samples.xyz")
            coeffs_file = os.path.join(data_dir, "coeffs.txt")
            yaml_file = os.path.join(data_dir, "test.yaml")
            
            with open(samples_file, 'w') as f:
                f.write(samples_content)
            with open(coeffs_file, 'w') as f:
                f.write(coeffs_content)
            
            # YAML with relative paths
            yaml_content = """
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
"""
            
            with open(yaml_file, 'w') as f:
                f.write(yaml_content)
            
            # Parse from file - relative paths should be resolved
            func = stf.parse_space_time_function_from_file(yaml_file)
            assert func is not None
            
            # Test function evaluation
            pos = [0.1, 0.1, 0.1]
            t = 0.0
            value = func.value(pos, t)
            assert abs(value) < float('inf')

    def test_duchon_2d_error(self):
        """Test that Duchon in 2D throws an error."""
        yaml_content = """
type: sweep
dimension: 2
primitive:
  type: duchon
  samples_file: dummy.xyz
  coeffs_file: dummy.txt
transform:
  type: translation
  vector: [0.0, 0.0]
"""
        
        with pytest.raises(stf.YamlParseError):
            stf.parse_space_time_function_from_string_2d(yaml_content)

    def test_duchon_default_parameters(self):
        """Test Duchon with default parameters."""
        import tempfile
        import os
        
        # Create temporary test files
        samples_content = "3\n0.0 0.0 0.0\n1.0 0.0 0.0\n0.0 1.0 0.0\n0.0 0.0 1.0\n"
        coeffs_content = "1.0 0.5 0.2 0.1\n0.8 0.3 0.1 0.0\n0.6 0.2 0.0 0.1\n0.4 0.1 0.0 0.0\n0.1 0.2 0.3 0.4\n"
        
        with tempfile.TemporaryDirectory() as temp_dir:
            samples_file = os.path.join(temp_dir, "samples.xyz")
            coeffs_file = os.path.join(temp_dir, "coeffs.txt")
            
            with open(samples_file, 'w') as f:
                f.write(samples_content)
            with open(coeffs_file, 'w') as f:
                f.write(coeffs_content)
            
            yaml_content = f"""
type: sweep
dimension: 3
primitive:
  type: duchon
  samples_file: {samples_file}
  coeffs_file: {coeffs_file}
  # center, radius, and positive_inside use defaults
transform:
  type: translation
  vector: [0.0, 0.0, 0.0]
"""
            
            func = stf.parse_space_time_function_from_string(yaml_content)
            assert func is not None
            
            # Test function evaluation
            pos = [0.0, 0.0, 0.0]
            t = 0.0
            value = func.value(pos, t)
            assert abs(value) < float('inf')


if __name__ == "__main__":
    pytest.main([__file__])
