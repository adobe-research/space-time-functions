"""
Unit tests for transform classes.
"""

import pytest
import math
import space_time_functions as stf


class TestTranslation:
    """Tests for Translation classes."""
    
    def test_translation_2d_creation(self):
        """Test creating 2D translations."""
        trans = stf.transform.Translation2D([1.0, 2.0])
        assert trans is not None
    
    def test_translation_3d_creation(self):
        """Test creating 3D translations."""
        trans = stf.transform.Translation3D([1.0, 2.0, 3.0])
        assert trans is not None
    
    def test_translation_2d_transform(self):
        """Test 2D translation transformation."""
        trans = stf.transform.Translation2D([1.0, 2.0])
        
        # Test at t=0 (no translation)
        result = trans.transform([0.0, 0.0], 0.0)
        expected = [0.0, 0.0]
        for i in range(2):
            assert abs(result[i] - expected[i]) < 1e-10
        
        # Test at t=1 (full translation)
        result = trans.transform([0.0, 0.0], 1.0)
        expected = [1.0, 2.0]
        for i in range(2):
            assert abs(result[i] - expected[i]) < 1e-10
        
        # Test at t=0.5 (half translation)
        result = trans.transform([0.0, 0.0], 0.5)
        expected = [0.5, 1.0]
        for i in range(2):
            assert abs(result[i] - expected[i]) < 1e-10
    
    def test_translation_3d_transform(self):
        """Test 3D translation transformation."""
        trans = stf.transform.Translation3D([1.0, 2.0, 3.0])
        
        # Test at t=1
        result = trans.transform([0.0, 0.0, 0.0], 1.0)
        expected = [1.0, 2.0, 3.0]
        for i in range(3):
            assert abs(result[i] - expected[i]) < 1e-10
    
    def test_translation_velocity(self):
        """Test translation velocity computation."""
        trans = stf.transform.Translation3D([1.0, 2.0, 3.0])
        
        # Velocity should be constant and equal to translation vector
        velocity = trans.velocity([0.0, 0.0, 0.0], 0.5)
        expected = [1.0, 2.0, 3.0]
        for i in range(3):
            assert abs(velocity[i] - expected[i]) < 1e-10
    
    def test_translation_jacobian(self):
        """Test translation Jacobian computation."""
        trans = stf.transform.Translation3D([1.0, 2.0, 3.0])
        
        # Jacobian should be identity matrix
        jacobian = trans.position_Jacobian([0.0, 0.0, 0.0], 0.5)
        for i in range(3):
            for j in range(3):
                expected = 1.0 if i == j else 0.0
                assert abs(jacobian[i][j] - expected) < 1e-10


class TestRotation:
    """Tests for Rotation classes."""
    
    def test_rotation_2d_creation(self):
        """Test creating 2D rotations."""
        rot = stf.transform.Rotation2D([0.0, 0.0], [0.0, 1.0], 90.0)
        assert rot is not None
    
    def test_rotation_3d_creation(self):
        """Test creating 3D rotations."""
        rot = stf.transform.Rotation3D([0.0, 0.0, 0.0], [0.0, 0.0, 1.0], 90.0)
        assert rot is not None
    
    def test_rotation_3d_transform(self):
        """Test 3D rotation transformation."""
        # 90 degree rotation around z-axis
        rot = stf.transform.Rotation3D([0.0, 0.0, 0.0], [0.0, 0.0, 1.0], 90.0)
        
        # Test at t=0 (no rotation)
        result = rot.transform([1.0, 0.0, 0.0], 0.0)
        expected = [1.0, 0.0, 0.0]
        for i in range(3):
            assert abs(result[i] - expected[i]) < 1e-10
        
        # Test at t=1 (full 90 degree rotation)
        result = rot.transform([1.0, 0.0, 0.0], 1.0)
        expected = [0.0, 1.0, 0.0]  # Should rotate to y-axis
        for i in range(3):
            assert abs(result[i] - expected[i]) < 1e-6  # Allow for numerical precision
    
    def test_rotation_velocity(self):
        """Test rotation velocity computation."""
        rot = stf.transform.Rotation3D([0.0, 0.0, 0.0], [0.0, 0.0, 1.0], 90.0)
        
        # Test velocity at a point
        velocity = rot.velocity([1.0, 0.0, 0.0], 0.0)
        assert len(velocity) == 3
    
    def test_rotation_jacobian(self):
        """Test rotation Jacobian computation."""
        rot = stf.transform.Rotation3D([0.0, 0.0, 0.0], [0.0, 0.0, 1.0], 90.0)
        
        jacobian = rot.position_Jacobian([1.0, 0.0, 0.0], 0.0)
        assert len(jacobian) == 3
        assert len(jacobian[0]) == 3


class TestScale:
    """Tests for Scale classes."""
    
    def test_scale_2d_creation(self):
        """Test creating 2D scales."""
        scale = stf.transform.Scale2D([2.0, 3.0], [0.0, 0.0])
        assert scale is not None
    
    def test_scale_3d_creation(self):
        """Test creating 3D scales."""
        scale = stf.transform.Scale3D([2.0, 3.0, 4.0], [0.0, 0.0, 0.0])
        assert scale is not None
    
    def test_scale_3d_transform(self):
        """Test 3D scale transformation."""
        scale = stf.transform.Scale3D([2.0, 3.0, 4.0], [0.0, 0.0, 0.0])
        
        # Test at t=0 (no scaling)
        result = scale.transform([1.0, 1.0, 1.0], 0.0)
        expected = [1.0, 1.0, 1.0]
        for i in range(3):
            assert abs(result[i] - expected[i]) < 1e-10
        
        # Test at t=1 (full scaling)
        result = scale.transform([1.0, 1.0, 1.0], 1.0)
        expected = [2.0, 3.0, 4.0]
        for i in range(3):
            assert abs(result[i] - expected[i]) < 1e-10
    
    def test_scale_velocity(self):
        """Test scale velocity computation."""
        scale = stf.transform.Scale3D([2.0, 3.0, 4.0], [0.0, 0.0, 0.0])
        
        velocity = scale.velocity([1.0, 1.0, 1.0], 0.5)
        expected = [1.0, 2.0, 3.0]  # (factor - 1) * position
        for i in range(3):
            assert abs(velocity[i] - expected[i]) < 1e-10
    
    def test_scale_jacobian(self):
        """Test scale Jacobian computation."""
        scale = stf.transform.Scale3D([2.0, 3.0, 4.0], [0.0, 0.0, 0.0])
        
        jacobian = scale.position_Jacobian([1.0, 1.0, 1.0], 0.5)
        assert len(jacobian) == 3
        assert len(jacobian[0]) == 3


class TestCompose:
    """Tests for Compose classes."""
    
    def test_compose_2d_creation(self):
        """Test creating 2D compositions."""
        trans = stf.transform.Translation2D([1.0, 0.0])
        scale = stf.transform.Scale2D([2.0, 2.0], [0.0, 0.0])
        
        compose = stf.transform.Compose2D(trans, scale)
        assert compose is not None
    
    def test_compose_3d_creation(self):
        """Test creating 3D compositions."""
        trans = stf.transform.Translation3D([1.0, 0.0, 0.0])
        scale = stf.transform.Scale3D([2.0, 2.0, 2.0], [0.0, 0.0, 0.0])
        
        compose = stf.transform.Compose3D(trans, scale)
        assert compose is not None
    
    def test_compose_3d_transform(self):
        """Test 3D composition transformation."""
        trans = stf.transform.Translation3D([1.0, 0.0, 0.0])
        scale = stf.transform.Scale3D([2.0, 2.0, 2.0], [0.0, 0.0, 0.0])
        
        compose = stf.transform.Compose3D(trans, scale)
        
        # Test composition: first translate, then scale
        result = compose.transform([0.0, 0.0, 0.0], 1.0)
        # First: translate by [1,0,0] -> [1,0,0]
        # Then: scale by 2 -> [2,0,0]
        expected = [2.0, 0.0, 0.0]
        for i in range(3):
            assert abs(result[i] - expected[i]) < 1e-10
    
    def test_compose_velocity(self):
        """Test composition velocity computation."""
        trans = stf.transform.Translation3D([1.0, 0.0, 0.0])
        scale = stf.transform.Scale3D([2.0, 2.0, 2.0], [0.0, 0.0, 0.0])
        
        compose = stf.transform.Compose3D(trans, scale)
        
        velocity = compose.velocity([0.0, 0.0, 0.0], 0.5)
        assert len(velocity) == 3
    
    def test_compose_jacobian(self):
        """Test composition Jacobian computation."""
        trans = stf.transform.Translation3D([1.0, 0.0, 0.0])
        scale = stf.transform.Scale3D([2.0, 2.0, 2.0], [0.0, 0.0, 0.0])
        
        compose = stf.transform.Compose3D(trans, scale)
        
        jacobian = compose.position_Jacobian([0.0, 0.0, 0.0], 0.5)
        assert len(jacobian) == 3
        assert len(jacobian[0]) == 3


class TestPolyline:
    """Tests for Polyline classes."""
    
    def test_polyline_2d_creation(self):
        """Test creating 2D polylines."""
        points = [[0.0, 0.0], [1.0, 0.0], [1.0, 1.0]]
        polyline = stf.transform.Polyline2D(points)
        assert polyline is not None
    
    def test_polyline_3d_creation(self):
        """Test creating 3D polylines."""
        points = [[0.0, 0.0, 0.0], [1.0, 0.0, 0.0], [1.0, 1.0, 0.0]]
        polyline = stf.transform.Polyline3D(points)
        assert polyline is not None
    
    def test_polyline_3d_transform(self):
        """Test 3D polyline transformation."""
        points = [[0.0, 0.0, 0.0], [2.0, 0.0, 0.0]]
        polyline = stf.transform.Polyline3D(points)
        
        # Test at t=0 (start of polyline)
        result = polyline.transform([0.0, 0.0, 0.0], 0.0)
        # Should be at first point with some transformation applied
        assert len(result) == 3
        
        # Test at t=1 (end of polyline)
        result = polyline.transform([0.0, 0.0, 0.0], 1.0)
        assert len(result) == 3
    
    def test_polyline_velocity(self):
        """Test polyline velocity computation."""
        points = [[0.0, 0.0, 0.0], [2.0, 0.0, 0.0]]
        polyline = stf.transform.Polyline3D(points)
        
        velocity = polyline.velocity([0.0, 0.0, 0.0], 0.5)
        assert len(velocity) == 3
    
    def test_polyline_jacobian(self):
        """Test polyline Jacobian computation."""
        points = [[0.0, 0.0, 0.0], [2.0, 0.0, 0.0]]
        polyline = stf.transform.Polyline3D(points)
        
        jacobian = polyline.position_Jacobian([0.0, 0.0, 0.0], 0.5)
        assert len(jacobian) == 3
        assert len(jacobian[0]) == 3


class TestPolyBezier:
    """Tests for PolyBezier classes."""
    
    def test_polybezier_2d_creation(self):
        """Test creating 2D PolyBezier curves."""
        samples = [[0.0, 0.0], [1.0, 0.0], [2.0, 1.0]]
        bezier = stf.transform.PolyBezier2D.from_samples(samples)
        assert bezier is not None
    
    def test_polybezier_3d_creation(self):
        """Test creating 3D PolyBezier curves."""
        samples = [[0.0, 0.0, 0.0], [1.0, 0.0, 0.0], [2.0, 1.0, 0.0]]
        bezier = stf.transform.PolyBezier3D.from_samples(samples)
        assert bezier is not None
    
    def test_polybezier_3d_transform(self):
        """Test 3D PolyBezier transformation."""
        samples = [[0.0, 0.0, 0.0], [1.0, 0.0, 0.0], [2.0, 0.0, 0.0]]
        bezier = stf.transform.PolyBezier3D.from_samples(samples)
        
        # Test transformation
        result = bezier.transform([0.0, 0.0, 0.0], 0.5)
        assert len(result) == 3
    
    def test_polybezier_velocity(self):
        """Test PolyBezier velocity computation."""
        samples = [[0.0, 0.0, 0.0], [1.0, 0.0, 0.0], [2.0, 0.0, 0.0]]
        bezier = stf.transform.PolyBezier3D.from_samples(samples)
        
        velocity = bezier.velocity([0.0, 0.0, 0.0], 0.5)
        assert len(velocity) == 3
    
    def test_polybezier_jacobian(self):
        """Test PolyBezier Jacobian computation."""
        samples = [[0.0, 0.0, 0.0], [1.0, 0.0, 0.0], [2.0, 0.0, 0.0]]
        bezier = stf.transform.PolyBezier3D.from_samples(samples)
        
        jacobian = bezier.position_Jacobian([0.0, 0.0, 0.0], 0.5)
        assert len(jacobian) == 3
        assert len(jacobian[0]) == 3

