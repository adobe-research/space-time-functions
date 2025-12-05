"""
Unit tests for base classes and module structure.
"""

import pytest
import space_time_functions as stf


class TestModuleStructure:
    """Tests for module structure and imports."""
    
    def test_main_module_exists(self):
        """Test that main stf module exists."""
        assert stf is not None
    
    def test_primitive_submodule_exists(self):
        """Test that primitive submodule exists."""
        assert hasattr(stf, 'primitive')
        assert stf.primitive is not None
    
    def test_transform_submodule_exists(self):
        """Test that transform submodule exists."""
        assert hasattr(stf, 'transform')
        assert stf.transform is not None


class TestBaseClasses:
    """Tests for base classes."""
    
    def test_space_time_function_2d_exists(self):
        """Test that SpaceTimeFunction2D base class exists."""
        assert hasattr(stf, 'SpaceTimeFunction2D')
    
    def test_space_time_function_3d_exists(self):
        """Test that SpaceTimeFunction3D base class exists."""
        assert hasattr(stf, 'SpaceTimeFunction3D')
    
    def test_implicit_function_2d_exists(self):
        """Test that ImplicitFunction2D base class exists."""
        assert hasattr(stf.primitive, 'ImplicitFunction2D')
    
    def test_implicit_function_3d_exists(self):
        """Test that ImplicitFunction3D base class exists."""
        assert hasattr(stf.primitive, 'ImplicitFunction3D')
    
    def test_transform_2d_exists(self):
        """Test that Transform2D base class exists."""
        assert hasattr(stf.transform, 'Transform2D')
    
    def test_transform_3d_exists(self):
        """Test that Transform3D base class exists."""
        assert hasattr(stf.transform, 'Transform3D')


class TestSpaceTimeFunctionInterface:
    """Tests for SpaceTimeFunction interface through concrete implementations."""
    
    def test_space_time_function_2d_interface(self):
        """Test SpaceTimeFunction2D interface through ExplicitForm2D."""
        def simple_func(pos, t):
            return pos[0] + pos[1] + t
        
        explicit = stf.ExplicitForm2D(simple_func)
        
        # Test that it has the required methods
        assert hasattr(explicit, 'value')
        assert hasattr(explicit, 'time_derivative')
        assert hasattr(explicit, 'gradient')
        
        # Test that methods work
        value = explicit.value([1.0, 2.0], 0.5)
        assert abs(value - 3.5) < 1e-10
        
        deriv = explicit.time_derivative([1.0, 2.0], 0.5)
        assert isinstance(deriv, float)
        
        grad = explicit.gradient([1.0, 2.0], 0.5)
        assert len(grad) == 3  # [df/dx, df/dy, df/dt]
    
    def test_space_time_function_3d_interface(self):
        """Test SpaceTimeFunction3D interface through ExplicitForm3D."""
        def simple_func(pos, t):
            return pos[0] + pos[1] + pos[2] + t
        
        explicit = stf.ExplicitForm3D(simple_func)
        
        # Test that it has the required methods
        assert hasattr(explicit, 'value')
        assert hasattr(explicit, 'time_derivative')
        assert hasattr(explicit, 'gradient')
        
        # Test that methods work
        value = explicit.value([1.0, 2.0, 3.0], 0.5)
        assert abs(value - 6.5) < 1e-10
        
        deriv = explicit.time_derivative([1.0, 2.0, 3.0], 0.5)
        assert isinstance(deriv, float)
        
        grad = explicit.gradient([1.0, 2.0, 3.0], 0.5)
        assert len(grad) == 4  # [df/dx, df/dy, df/dz, df/dt]


class TestImplicitFunctionInterface:
    """Tests for ImplicitFunction interface through concrete implementations."""
    
    def test_implicit_function_2d_interface(self):
        """Test ImplicitFunction2D interface through ImplicitBall2D."""
        ball = stf.primitive.ImplicitBall2D(1.0, [0.0, 0.0])
        
        # Test that it has the required methods
        assert hasattr(ball, 'value')
        assert hasattr(ball, 'gradient')
        assert hasattr(ball, 'finite_difference_gradient')
        
        # Test that methods work
        value = ball.value([0.5, 0.0])
        assert isinstance(value, float)
        
        grad = ball.gradient([0.5, 0.0])
        assert len(grad) == 2
        
        fd_grad = ball.finite_difference_gradient([0.5, 0.0])
        assert len(fd_grad) == 2
        
        # Gradient and finite difference gradient should be similar
        for i in range(2):
            assert abs(grad[i] - fd_grad[i]) < 1e-4
    
    def test_implicit_function_3d_interface(self):
        """Test ImplicitFunction3D interface through ImplicitBall3D."""
        ball = stf.primitive.ImplicitBall3D(1.0, [0.0, 0.0, 0.0])
        
        # Test that it has the required methods
        assert hasattr(ball, 'value')
        assert hasattr(ball, 'gradient')
        assert hasattr(ball, 'finite_difference_gradient')
        
        # Test that methods work
        value = ball.value([0.5, 0.0, 0.0])
        assert isinstance(value, float)
        
        grad = ball.gradient([0.5, 0.0, 0.0])
        assert len(grad) == 3
        
        fd_grad = ball.finite_difference_gradient([0.5, 0.0, 0.0])
        assert len(fd_grad) == 3
        
        # Gradient and finite difference gradient should be similar
        for i in range(3):
            assert abs(grad[i] - fd_grad[i]) < 1e-4


class TestTransformInterface:
    """Tests for Transform interface through concrete implementations."""
    
    def test_transform_2d_interface(self):
        """Test Transform2D interface through Translation2D."""
        trans = stf.transform.Translation2D([1.0, 2.0])
        
        # Test that it has the required methods
        assert hasattr(trans, 'transform')
        assert hasattr(trans, 'velocity')
        assert hasattr(trans, 'position_Jacobian')
        
        # Test that methods work
        result = trans.transform([0.0, 0.0], 1.0)
        assert len(result) == 2
        assert abs(result[0] - 1.0) < 1e-10
        assert abs(result[1] - 2.0) < 1e-10
        
        velocity = trans.velocity([0.0, 0.0], 0.5)
        assert len(velocity) == 2
        
        jacobian = trans.position_Jacobian([0.0, 0.0], 0.5)
        assert len(jacobian) == 2
        assert len(jacobian[0]) == 2
    
    def test_transform_3d_interface(self):
        """Test Transform3D interface through Translation3D."""
        trans = stf.transform.Translation3D([1.0, 2.0, 3.0])
        
        # Test that it has the required methods
        assert hasattr(trans, 'transform')
        assert hasattr(trans, 'velocity')
        assert hasattr(trans, 'position_Jacobian')
        
        # Test that methods work
        result = trans.transform([0.0, 0.0, 0.0], 1.0)
        assert len(result) == 3
        assert abs(result[0] - 1.0) < 1e-10
        assert abs(result[1] - 2.0) < 1e-10
        assert abs(result[2] - 3.0) < 1e-10
        
        velocity = trans.velocity([0.0, 0.0, 0.0], 0.5)
        assert len(velocity) == 3
        
        jacobian = trans.position_Jacobian([0.0, 0.0, 0.0], 0.5)
        assert len(jacobian) == 3
        assert len(jacobian[0]) == 3


class TestInheritanceRelationships:
    """Tests for inheritance relationships between classes."""
    
    def test_explicit_form_inherits_from_space_time_function(self):
        """Test that ExplicitForm inherits from SpaceTimeFunction."""
        def simple_func(pos, t):
            return pos[0] + pos[1] + pos[2] + t
        
        explicit = stf.ExplicitForm3D(simple_func)
        
        # Should be able to use as SpaceTimeFunction
        assert hasattr(explicit, 'value')
        assert hasattr(explicit, 'time_derivative')
        assert hasattr(explicit, 'gradient')
    
    def test_implicit_ball_inherits_from_implicit_function(self):
        """Test that ImplicitBall inherits from ImplicitFunction."""
        ball = stf.primitive.ImplicitBall3D(1.0, [0.0, 0.0, 0.0])
        
        # Should be able to use as ImplicitFunction
        assert hasattr(ball, 'value')
        assert hasattr(ball, 'gradient')
        assert hasattr(ball, 'finite_difference_gradient')
    
    def test_translation_inherits_from_transform(self):
        """Test that Translation inherits from Transform."""
        trans = stf.transform.Translation3D([1.0, 2.0, 3.0])
        
        # Should be able to use as Transform
        assert hasattr(trans, 'transform')
        assert hasattr(trans, 'velocity')
        assert hasattr(trans, 'position_Jacobian')
    
    def test_sweep_function_inherits_from_space_time_function(self):
        """Test that SweepFunction inherits from SpaceTimeFunction."""
        ball = stf.primitive.ImplicitBall3D(1.0, [0.0, 0.0, 0.0])
        trans = stf.transform.Translation3D([1.0, 0.0, 0.0])
        sweep = stf.SweepFunction3D(ball, trans)
        
        # Should be able to use as SpaceTimeFunction
        assert hasattr(sweep, 'value')
        assert hasattr(sweep, 'time_derivative')
        assert hasattr(sweep, 'gradient')


class TestPolymorphism:
    """Tests for polymorphic behavior."""
    
    def test_space_time_function_polymorphism(self):
        """Test that different SpaceTimeFunction implementations can be used interchangeably."""
        def simple_func(pos, t):
            return 1.0
        
        # Create different implementations
        explicit = stf.ExplicitForm3D(simple_func)
        ball = stf.primitive.ImplicitBall3D(1.0, [0.0, 0.0, 0.0])
        trans = stf.transform.Translation3D([0.0, 0.0, 0.0])
        sweep = stf.SweepFunction3D(ball, trans)
        
        # All should work with UnionFunction
        union1 = stf.UnionFunction3D(explicit, sweep)
        assert union1 is not None
        
        union2 = stf.UnionFunction3D(sweep, explicit)
        assert union2 is not None
    
    def test_implicit_function_polymorphism(self):
        """Test that different ImplicitFunction implementations can be used interchangeably."""
        # Create different implementations
        ball = stf.primitive.ImplicitBall3D(1.0, [0.0, 0.0, 0.0])
        torus = stf.primitive.ImplicitTorus(2.0, 0.5, [0.0, 0.0, 0.0])
        
        # All should work with ImplicitUnion
        union = stf.primitive.ImplicitUnion3D(ball, torus)
        assert union is not None
        
        # All should work with SweepFunction
        trans = stf.transform.Translation3D([1.0, 0.0, 0.0])
        sweep1 = stf.SweepFunction3D(ball, trans)
        sweep2 = stf.SweepFunction3D(torus, trans)
        assert sweep1 is not None
        assert sweep2 is not None
    
    def test_transform_polymorphism(self):
        """Test that different Transform implementations can be used interchangeably."""
        # Create different implementations
        trans = stf.transform.Translation3D([1.0, 0.0, 0.0])
        rot = stf.transform.Rotation3D([0.0, 0.0, 0.0], [0.0, 0.0, 1.0], 90.0)
        scale = stf.transform.Scale3D([2.0, 2.0, 2.0], [0.0, 0.0, 0.0])
        
        # All should work with Compose
        compose1 = stf.transform.Compose3D(trans, rot)
        compose2 = stf.transform.Compose3D(rot, scale)
        assert compose1 is not None
        assert compose2 is not None
        
        # All should work with SweepFunction
        ball = stf.primitive.ImplicitBall3D(1.0, [0.0, 0.0, 0.0])
        sweep1 = stf.SweepFunction3D(ball, trans)
        sweep2 = stf.SweepFunction3D(ball, rot)
        sweep3 = stf.SweepFunction3D(ball, scale)
        assert sweep1 is not None
        assert sweep2 is not None
        assert sweep3 is not None
