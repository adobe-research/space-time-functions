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


if __name__ == "__main__":
    pytest.main([__file__])
