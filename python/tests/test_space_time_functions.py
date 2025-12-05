"""
Unit tests for space-time function classes.
"""

import pytest
import math
import space_time_functions as stf


class TestExplicitForm:
    """Tests for ExplicitForm classes."""
    
    def test_explicit_form_2d_creation(self, simple_function_2d):
        """Test creating 2D explicit forms."""
        explicit = stf.ExplicitForm2D(simple_function_2d)
        assert explicit is not None
    
    def test_explicit_form_3d_creation(self, simple_function_3d):
        """Test creating 3D explicit forms."""
        explicit = stf.ExplicitForm3D(simple_function_3d)
        assert explicit is not None
    
    def test_explicit_form_2d_with_derivatives(self, simple_function_2d, simple_gradient_2d):
        """Test creating 2D explicit forms with derivatives."""
        def time_deriv(pos, t):
            return 1.0  # df/dt = 1 for our simple function
        
        explicit = stf.ExplicitForm2D(simple_function_2d, time_deriv, simple_gradient_2d)
        assert explicit is not None
    
    def test_explicit_form_3d_with_derivatives(self, simple_function_3d, simple_gradient_3d):
        """Test creating 3D explicit forms with derivatives."""
        def time_deriv(pos, t):
            return 1.0  # df/dt = 1 for our simple function
        
        explicit = stf.ExplicitForm3D(simple_function_3d, time_deriv, simple_gradient_3d)
        assert explicit is not None
    
    def test_explicit_form_2d_value(self, simple_function_2d, simple_2d_points, simple_times):
        """Test 2D explicit form value computation."""
        explicit = stf.ExplicitForm2D(simple_function_2d)
        
        # Test with simple function: f(x,y,t) = x + y + t
        for pos in simple_2d_points:
            for t in simple_times:
                expected = pos[0] + pos[1] + t
                result = explicit.value(pos, t)
                assert abs(result - expected) < 1e-10
    
    def test_explicit_form_3d_value(self, simple_function_3d, simple_3d_points, simple_times):
        """Test 3D explicit form value computation."""
        explicit = stf.ExplicitForm3D(simple_function_3d)
        
        # Test with simple function: f(x,y,z,t) = x + y + z + t
        for pos in simple_3d_points:
            for t in simple_times:
                expected = pos[0] + pos[1] + pos[2] + t
                result = explicit.value(pos, t)
                assert abs(result - expected) < 1e-10
    
    def test_explicit_form_time_derivative(self, simple_function_3d):
        """Test explicit form time derivative computation."""
        explicit = stf.ExplicitForm3D(simple_function_3d)
        
        # Time derivative should be computed using finite differences
        deriv = explicit.time_derivative([1.0, 1.0, 1.0], 0.5)
        # For f(x,y,z,t) = x + y + z + t, df/dt = 1
        assert abs(deriv - 1.0) < 1e-4  # Allow for finite difference error
    
    def test_explicit_form_gradient(self, simple_function_3d):
        """Test explicit form gradient computation."""
        explicit = stf.ExplicitForm3D(simple_function_3d)
        
        # Gradient should be computed using finite differences
        grad = explicit.gradient([1.0, 1.0, 1.0], 0.5)
        # For f(x,y,z,t) = x + y + z + t, gradient = [1, 1, 1, 1]
        expected = [1.0, 1.0, 1.0, 1.0]
        for i in range(4):
            assert abs(grad[i] - expected[i]) < 1e-4  # Allow for finite difference error


class TestSweepFunction:
    """Tests for SweepFunction classes."""
    
    def test_sweep_function_2d_creation(self):
        """Test creating 2D sweep functions."""
        ball = stf.primitive.ImplicitBall2D(1.0, [0.0, 0.0])
        trans = stf.transform.Translation2D([1.0, 0.0])
        
        sweep = stf.SweepFunction2D(ball, trans)
        assert sweep is not None
    
    def test_sweep_function_3d_creation(self):
        """Test creating 3D sweep functions."""
        ball = stf.primitive.ImplicitBall3D(1.0, [0.0, 0.0, 0.0])
        trans = stf.transform.Translation3D([1.0, 0.0, 0.0])
        
        sweep = stf.SweepFunction3D(ball, trans)
        assert sweep is not None
    
    def test_sweep_function_3d_value(self):
        """Test 3D sweep function value computation."""
        ball = stf.primitive.ImplicitBall3D(1.0, [0.0, 0.0, 0.0])
        trans = stf.transform.Translation3D([2.0, 0.0, 0.0])
        
        sweep = stf.SweepFunction3D(ball, trans)
        
        # At t=0, ball is at origin
        value = sweep.value([0.0, 0.0, 0.0], 0.0)
        assert value < 0  # Inside the ball
        
        # At t=1, ball is translated by [2,0,0]
        value = sweep.value([-2.0, 0.0, 0.0], 1.0)
        assert value < 0  # Inside the translated ball
    
    def test_sweep_function_time_derivative(self):
        """Test sweep function time derivative computation."""
        ball = stf.primitive.ImplicitBall3D(1.0, [0.0, 0.0, 0.0])
        trans = stf.transform.Translation3D([1.0, 0.0, 0.0])
        
        sweep = stf.SweepFunction3D(ball, trans)
        
        deriv = sweep.time_derivative([0.5, 0.0, 0.0], 0.5)
        # Should be non-zero due to the movement
        assert isinstance(deriv, float)
    
    def test_sweep_function_gradient(self):
        """Test sweep function gradient computation."""
        ball = stf.primitive.ImplicitBall3D(1.0, [0.0, 0.0, 0.0])
        trans = stf.transform.Translation3D([1.0, 0.0, 0.0])
        
        sweep = stf.SweepFunction3D(ball, trans)
        
        grad = sweep.gradient([0.5, 0.0, 0.0], 0.5)
        assert len(grad) == 4  # [df/dx, df/dy, df/dz, df/dt]


class TestUnionFunction:
    """Tests for UnionFunction classes."""
    
    def test_union_function_2d_creation(self, simple_function_2d):
        """Test creating 2D union functions."""
        f1 = stf.ExplicitForm2D(simple_function_2d)
        f2 = stf.ExplicitForm2D(simple_function_2d)
        
        union = stf.UnionFunction2D(f1, f2, 0.0)
        assert union is not None
        
        # Test smooth union
        smooth_union = stf.UnionFunction2D(f1, f2, 0.5)
        assert smooth_union is not None
    
    def test_union_function_3d_creation(self, simple_function_3d):
        """Test creating 3D union functions."""
        f1 = stf.ExplicitForm3D(simple_function_3d)
        f2 = stf.ExplicitForm3D(simple_function_3d)
        
        union = stf.UnionFunction3D(f1, f2, 0.0)
        assert union is not None
    
    def test_union_function_value(self, simple_function_3d):
        """Test union function value computation."""
        def func1(pos, t):
            return pos[0] + t
        
        def func2(pos, t):
            return pos[1] + t
        
        f1 = stf.ExplicitForm3D(func1)
        f2 = stf.ExplicitForm3D(func2)
        
        union = stf.UnionFunction3D(f1, f2, 0.0)
        
        # Sharp union should return minimum of the two functions
        value = union.value([1.0, 2.0, 0.0], 0.0)
        # f1 = 1.0, f2 = 2.0, min = 1.0
        assert abs(value - 1.0) < 1e-10
    
    def test_union_function_time_derivative(self, simple_function_3d):
        """Test union function time derivative computation."""
        f1 = stf.ExplicitForm3D(simple_function_3d)
        f2 = stf.ExplicitForm3D(simple_function_3d)
        
        union = stf.UnionFunction3D(f1, f2, 0.0)
        
        deriv = union.time_derivative([1.0, 1.0, 1.0], 0.5)
        assert isinstance(deriv, float)
    
    def test_union_function_gradient(self, simple_function_3d):
        """Test union function gradient computation."""
        f1 = stf.ExplicitForm3D(simple_function_3d)
        f2 = stf.ExplicitForm3D(simple_function_3d)
        
        union = stf.UnionFunction3D(f1, f2, 0.0)
        
        grad = union.gradient([1.0, 1.0, 1.0], 0.5)
        assert len(grad) == 4


class TestInterpolateFunction:
    """Tests for InterpolateFunction classes."""
    
    def test_interpolate_function_2d_creation(self, simple_function_2d):
        """Test creating 2D interpolate functions."""
        f1 = stf.ExplicitForm2D(simple_function_2d)
        f2 = stf.ExplicitForm2D(simple_function_2d)
        
        interp = stf.InterpolateFunction2D(f1, f2)
        assert interp is not None
    
    def test_interpolate_function_3d_creation(self, simple_function_3d):
        """Test creating 3D interpolate functions."""
        f1 = stf.ExplicitForm3D(simple_function_3d)
        f2 = stf.ExplicitForm3D(simple_function_3d)
        
        interp = stf.InterpolateFunction3D(f1, f2)
        assert interp is not None
    
    def test_interpolate_function_value(self):
        """Test interpolate function value computation."""
        def func1(pos, t):
            return 1.0  # Constant function
        
        def func2(pos, t):
            return 3.0  # Constant function
        
        f1 = stf.ExplicitForm3D(func1)
        f2 = stf.ExplicitForm3D(func2)
        
        interp = stf.InterpolateFunction3D(f1, f2)
        
        # At t=0, should return f1
        value = interp.value([0.0, 0.0, 0.0], 0.0)
        assert abs(value - 1.0) < 1e-10
        
        # At t=1, should return f2
        value = interp.value([0.0, 0.0, 0.0], 1.0)
        assert abs(value - 3.0) < 1e-10
        
        # At t=0.5, should return average
        value = interp.value([0.0, 0.0, 0.0], 0.5)
        assert abs(value - 2.0) < 1e-10
    
    def test_interpolate_function_time_derivative(self, simple_function_3d):
        """Test interpolate function time derivative computation."""
        f1 = stf.ExplicitForm3D(simple_function_3d)
        f2 = stf.ExplicitForm3D(simple_function_3d)
        
        interp = stf.InterpolateFunction3D(f1, f2)
        
        deriv = interp.time_derivative([1.0, 1.0, 1.0], 0.5)
        assert isinstance(deriv, float)
    
    def test_interpolate_function_gradient(self, simple_function_3d):
        """Test interpolate function gradient computation."""
        f1 = stf.ExplicitForm3D(simple_function_3d)
        f2 = stf.ExplicitForm3D(simple_function_3d)
        
        interp = stf.InterpolateFunction3D(f1, f2)
        
        grad = interp.gradient([1.0, 1.0, 1.0], 0.5)
        assert len(grad) == 4


class TestOffsetFunction:
    """Tests for OffsetFunction classes."""
    
    def test_offset_function_2d_creation(self, simple_function_2d):
        """Test creating 2D offset functions."""
        f = stf.ExplicitForm2D(simple_function_2d)
        
        offset = stf.OffsetFunction2D(f)
        assert offset is not None
    
    def test_offset_function_3d_creation(self, simple_function_3d):
        """Test creating 3D offset functions."""
        f = stf.ExplicitForm3D(simple_function_3d)
        
        offset = stf.OffsetFunction3D(f)
        assert offset is not None
    
    def test_offset_function_value(self):
        """Test offset function value computation."""
        def base_func(pos, t):
            return 1.0  # Constant function
        
        def offset_func(t):
            return 2.0 * t  # Linear offset
        
        f = stf.ExplicitForm3D(base_func)
        offset = stf.OffsetFunction3D(f, offset_func)
        
        # At t=0, should return base function + 0
        value = offset.value([0.0, 0.0, 0.0], 0.0)
        assert abs(value - 1.0) < 1e-10
        
        # At t=1, should return base function + 2
        value = offset.value([0.0, 0.0, 0.0], 1.0)
        assert abs(value - 3.0) < 1e-10
    
    def test_offset_function_time_derivative(self):
        """Test offset function time derivative computation."""
        def base_func(pos, t):
            return 1.0  # Constant function
        
        def offset_func(t):
            return 2.0 * t  # Linear offset
        
        def offset_deriv(t):
            return 2.0  # Derivative of offset
        
        f = stf.ExplicitForm3D(base_func)
        offset = stf.OffsetFunction3D(f, offset_func, offset_deriv)
        
        # Time derivative should include offset derivative
        deriv = offset.time_derivative([0.0, 0.0, 0.0], 0.5)
        # Base function has 0 time derivative, offset derivative is 2
        assert abs(deriv - 2.0) < 1e-4
    
    def test_offset_function_gradient(self, simple_function_3d):
        """Test offset function gradient computation."""
        f = stf.ExplicitForm3D(simple_function_3d)
        offset = stf.OffsetFunction3D(f)
        
        grad = offset.gradient([1.0, 1.0, 1.0], 0.5)
        assert len(grad) == 4

