"""
Pytest configuration and fixtures for space-time-functions tests.
"""

import sys
import os
import pytest


@pytest.fixture
def simple_2d_points():
    """Fixture providing simple 2D points for testing."""
    return [
        [0.0, 0.0],
        [1.0, 0.0],
        [0.0, 1.0],
        [1.0, 1.0],
        [-1.0, -1.0]
    ]

@pytest.fixture
def simple_3d_points():
    """Fixture providing simple 3D points for testing."""
    return [
        [0.0, 0.0, 0.0],
        [1.0, 0.0, 0.0],
        [0.0, 1.0, 0.0],
        [0.0, 0.0, 1.0],
        [1.0, 1.0, 1.0],
        [-1.0, -1.0, -1.0]
    ]

@pytest.fixture
def simple_times():
    """Fixture providing simple time values for testing."""
    return [0.0, 0.5, 1.0, -0.5, 2.0]

@pytest.fixture
def simple_function_2d():
    """Fixture providing a simple 2D function: f(x,y,t) = x + y + t."""
    def func(pos, t):
        return pos[0] + pos[1] + t
    return func

@pytest.fixture
def simple_function_3d():
    """Fixture providing a simple 3D function: f(x,y,z,t) = x + y + z + t."""
    def func(pos, t):
        return pos[0] + pos[1] + pos[2] + t
    return func

@pytest.fixture
def simple_gradient_2d():
    """Fixture providing gradient for simple 2D function: [1, 1, 1]."""
    def grad(pos, t):
        return [1.0, 1.0, 1.0]  # [df/dx, df/dy, df/dt]
    return grad

@pytest.fixture
def simple_gradient_3d():
    """Fixture providing gradient for simple 3D function: [1, 1, 1, 1]."""
    def grad(pos, t):
        return [1.0, 1.0, 1.0, 1.0]  # [df/dx, df/dy, df/dz, df/dt]
    return grad

@pytest.fixture
def simple_implicit_2d():
    """Fixture providing a simple 2D implicit function: circle at origin with radius 1."""
    def func(pos):
        return (pos[0]**2 + pos[1]**2)**0.5 - 1.0
    return func

@pytest.fixture
def simple_implicit_3d():
    """Fixture providing a simple 3D implicit function: sphere at origin with radius 1."""
    def func(pos):
        return (pos[0]**2 + pos[1]**2 + pos[2]**2)**0.5 - 1.0
    return func

@pytest.fixture
def simple_implicit_gradient_2d():
    """Fixture providing gradient for simple 2D implicit function."""
    def grad(pos):
        r = (pos[0]**2 + pos[1]**2)**0.5
        if r < 1e-10:
            return [0.0, 0.0]
        return [pos[0]/r, pos[1]/r]
    return grad

@pytest.fixture
def simple_implicit_gradient_3d():
    """Fixture providing gradient for simple 3D implicit function."""
    def grad(pos):
        r = (pos[0]**2 + pos[1]**2 + pos[2]**2)**0.5
        if r < 1e-10:
            return [0.0, 0.0, 0.0]
        return [pos[0]/r, pos[1]/r, pos[2]/r]
    return grad
