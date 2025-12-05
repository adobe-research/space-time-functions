"""
Unit tests for primitive implicit function classes.
"""

import pytest
import math
import space_time_functions as stf


class TestImplicitBall:
    """Tests for ImplicitBall classes."""
    
    def test_implicit_ball_2d_creation(self):
        """Test creating 2D implicit balls."""
        ball = stf.primitive.ImplicitBall2D(1.0, [0.0, 0.0])
        assert ball is not None
        
        # Test with different parameters
        ball2 = stf.primitive.ImplicitBall2D(2.5, [1.0, -1.0], 2)
        assert ball2 is not None
    
    def test_implicit_ball_3d_creation(self):
        """Test creating 3D implicit balls."""
        ball = stf.primitive.ImplicitBall3D(1.0, [0.0, 0.0, 0.0])
        assert ball is not None
        
        # Test with different parameters
        ball2 = stf.primitive.ImplicitBall3D(3.0, [1.0, 2.0, -1.0], 3)
        assert ball2 is not None
    
    def test_implicit_ball_2d_value(self, simple_2d_points):
        """Test 2D ball value computation."""
        ball = stf.primitive.ImplicitBall2D(1.0, [0.0, 0.0])
        
        # Test center point (should be -1.0)
        assert abs(ball.value([0.0, 0.0]) - (-1.0)) < 1e-10
        
        # Test point on surface (should be 0.0)
        assert abs(ball.value([1.0, 0.0])) < 1e-10
        
        # Test point outside (should be positive)
        assert ball.value([2.0, 0.0]) > 0
        
        # Test point inside (should be negative)
        assert ball.value([0.5, 0.0]) < 0
    
    def test_implicit_ball_3d_value(self, simple_3d_points):
        """Test 3D ball value computation."""
        ball = stf.primitive.ImplicitBall3D(1.0, [0.0, 0.0, 0.0])
        
        # Test center point (should be -1.0)
        assert abs(ball.value([0.0, 0.0, 0.0]) - (-1.0)) < 1e-10
        
        # Test point on surface (should be 0.0)
        assert abs(ball.value([1.0, 0.0, 0.0])) < 1e-10
        
        # Test point outside (should be positive)
        assert ball.value([2.0, 0.0, 0.0]) > 0
        
        # Test point inside (should be negative)
        assert ball.value([0.5, 0.0, 0.0]) < 0
    
    def test_implicit_ball_gradient(self):
        """Test ball gradient computation."""
        ball = stf.primitive.ImplicitBall3D(1.0, [0.0, 0.0, 0.0])
        
        # Test gradient at point on x-axis
        grad = ball.gradient([0.5, 0.0, 0.0])
        expected = [1.0, 0.0, 0.0]  # Should point outward
        for i in range(3):
            assert abs(grad[i] - expected[i]) < 1e-10
    
    def test_implicit_ball_aliases(self):
        """Test ImplicitCircle and ImplicitSphere aliases."""
        # Test that aliases exist and work
        circle = stf.primitive.ImplicitCircle(1.0, [0.0, 0.0])
        assert circle is not None
        assert abs(circle.value([1.0, 0.0])) < 1e-10
        
        sphere = stf.primitive.ImplicitSphere(1.0, [0.0, 0.0, 0.0])
        assert sphere is not None
        assert abs(sphere.value([1.0, 0.0, 0.0])) < 1e-10


class TestImplicitCapsule:
    """Tests for ImplicitCapsule classes."""
    
    def test_implicit_capsule_2d_creation(self):
        """Test creating 2D implicit capsules."""
        capsule = stf.primitive.ImplicitCapsule2D(0.5, [0.0, 0.0], [2.0, 0.0])
        assert capsule is not None
    
    def test_implicit_capsule_3d_creation(self):
        """Test creating 3D implicit capsules."""
        capsule = stf.primitive.ImplicitCapsule3D(0.5, [0.0, 0.0, 0.0], [2.0, 0.0, 0.0])
        assert capsule is not None
    
    def test_implicit_capsule_3d_value(self):
        """Test 3D capsule value computation."""
        capsule = stf.primitive.ImplicitCapsule3D(0.5, [0.0, 0.0, 0.0], [2.0, 0.0, 0.0])
        
        # Test point at center of line segment
        value = capsule.value([1.0, 0.0, 0.0])
        assert abs(value - (-0.5)) < 1e-10  # Should be -radius
        
        # Test point on surface
        value = capsule.value([1.0, 0.5, 0.0])
        assert abs(value) < 1e-10
    
    def test_implicit_capsule_gradient(self):
        """Test capsule gradient computation."""
        capsule = stf.primitive.ImplicitCapsule3D(0.5, [0.0, 0.0, 0.0], [2.0, 0.0, 0.0])
        
        # Test gradient at a point
        grad = capsule.gradient([1.0, 0.5, 0.0])
        # Gradient should point outward from the capsule surface
        assert len(grad) == 3
        assert abs(grad[1] - 1.0) < 1e-10  # Should be 1 in y direction


class TestImplicitTorus:
    """Tests for ImplicitTorus class."""
    
    def test_implicit_torus_creation(self):
        """Test creating implicit torus."""
        torus = stf.primitive.ImplicitTorus(2.0, 0.5, [0.0, 0.0, 0.0])
        assert torus is not None
    
    def test_implicit_torus_value(self):
        """Test torus value computation."""
        torus = stf.primitive.ImplicitTorus(2.0, 0.5, [0.0, 0.0, 0.0])
        
        # Test point on the torus (major radius in x-y plane)
        value = torus.value([2.5, 0.0, 0.0])  # R + r from center
        assert abs(value) < 1e-10
        
        # Test center point (should be positive)
        value = torus.value([0.0, 0.0, 0.0])
        assert value > 0
    
    def test_implicit_torus_gradient(self):
        """Test torus gradient computation."""
        torus = stf.primitive.ImplicitTorus(2.0, 0.5, [0.0, 0.0, 0.0])
        
        # Test gradient at a point
        grad = torus.gradient([2.5, 0.0, 0.0])
        assert len(grad) == 3
        # Gradient should point outward
        assert grad[0] > 0


class TestGenericFunction:
    """Tests for GenericFunction classes."""
    
    def test_generic_function_2d_creation(self, simple_implicit_2d, simple_implicit_gradient_2d):
        """Test creating 2D generic functions."""
        func = stf.primitive.GenericFunction2D(simple_implicit_2d, simple_implicit_gradient_2d)
        assert func is not None
    
    def test_generic_function_3d_creation(self, simple_implicit_3d, simple_implicit_gradient_3d):
        """Test creating 3D generic functions."""
        func = stf.primitive.GenericFunction3D(simple_implicit_3d, simple_implicit_gradient_3d)
        assert func is not None
    
    def test_generic_function_2d_value(self, simple_implicit_2d, simple_implicit_gradient_2d):
        """Test 2D generic function value computation."""
        func = stf.primitive.GenericFunction2D(simple_implicit_2d, simple_implicit_gradient_2d)
        
        # Test that it behaves like a unit circle
        assert abs(func.value([1.0, 0.0])) < 1e-10  # On surface
        assert func.value([2.0, 0.0]) > 0  # Outside
        assert func.value([0.5, 0.0]) < 0  # Inside
    
    def test_generic_function_3d_value(self, simple_implicit_3d, simple_implicit_gradient_3d):
        """Test 3D generic function value computation."""
        func = stf.primitive.GenericFunction3D(simple_implicit_3d, simple_implicit_gradient_3d)
        
        # Test that it behaves like a unit sphere
        assert abs(func.value([1.0, 0.0, 0.0])) < 1e-10  # On surface
        assert func.value([2.0, 0.0, 0.0]) > 0  # Outside
        assert func.value([0.5, 0.0, 0.0]) < 0  # Inside
    
    def test_generic_function_gradient(self, simple_implicit_3d, simple_implicit_gradient_3d):
        """Test generic function gradient computation."""
        func = stf.primitive.GenericFunction3D(simple_implicit_3d, simple_implicit_gradient_3d)
        
        grad = func.gradient([1.0, 0.0, 0.0])
        expected = [1.0, 0.0, 0.0]  # Normalized gradient
        for i in range(3):
            assert abs(grad[i] - expected[i]) < 1e-10


class TestImplicitUnion:
    """Tests for ImplicitUnion classes."""
    
    def test_implicit_union_2d_creation(self):
        """Test creating 2D implicit unions."""
        ball1 = stf.primitive.ImplicitBall2D(1.0, [0.0, 0.0])
        ball2 = stf.primitive.ImplicitBall2D(1.0, [2.0, 0.0])
        
        # Sharp union
        union = stf.primitive.ImplicitUnion2D(ball1, ball2, 0.0)
        assert union is not None
        
        # Smooth union
        smooth_union = stf.primitive.ImplicitUnion2D(ball1, ball2, 0.5)
        assert smooth_union is not None
    
    def test_implicit_union_3d_creation(self):
        """Test creating 3D implicit unions."""
        ball1 = stf.primitive.ImplicitBall3D(1.0, [0.0, 0.0, 0.0])
        ball2 = stf.primitive.ImplicitBall3D(1.0, [2.0, 0.0, 0.0])
        
        union = stf.primitive.ImplicitUnion3D(ball1, ball2, 0.0)
        assert union is not None
    
    def test_implicit_union_value(self):
        """Test union value computation."""
        ball1 = stf.primitive.ImplicitBall3D(1.0, [0.0, 0.0, 0.0])
        ball2 = stf.primitive.ImplicitBall3D(1.0, [3.0, 0.0, 0.0])
        
        union = stf.primitive.ImplicitUnion3D(ball1, ball2, 0.0)
        
        # Test point inside first ball
        value = union.value([0.0, 0.0, 0.0])
        assert value < 0  # Should be inside
        
        # Test point between balls
        value = union.value([1.5, 0.0, 0.0])
        assert value > 0  # Should be outside both
    
    def test_implicit_union_gradient(self):
        """Test union gradient computation."""
        ball1 = stf.primitive.ImplicitBall3D(1.0, [0.0, 0.0, 0.0])
        ball2 = stf.primitive.ImplicitBall3D(1.0, [3.0, 0.0, 0.0])
        
        union = stf.primitive.ImplicitUnion3D(ball1, ball2, 0.0)
        
        grad = union.gradient([0.5, 0.0, 0.0])
        assert len(grad) == 3

