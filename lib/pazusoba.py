# This file was automatically generated by SWIG (http://www.swig.org).
# Version 4.0.2
#
# Do not make changes to this file unless you know what you are doing--modify
# the SWIG interface file instead.

from sys import version_info as _swig_python_version_info
if _swig_python_version_info < (2, 7, 0):
    raise RuntimeError("Python 2.7 or later required")

# Import the low-level C/C++ module
if __package__ or "." in __name__:
    from . import _pazusoba
else:
    import _pazusoba

try:
    import builtins as __builtin__
except ImportError:
    import __builtin__

def _swig_repr(self):
    try:
        strthis = "proxy of " + self.this.__repr__()
    except __builtin__.Exception:
        strthis = ""
    return "<%s.%s; %s >" % (self.__class__.__module__, self.__class__.__name__, strthis,)


def _swig_setattr_nondynamic_instance_variable(set):
    def set_instance_attr(self, name, value):
        if name == "thisown":
            self.this.own(value)
        elif name == "this":
            set(self, name, value)
        elif hasattr(self, name) and isinstance(getattr(type(self), name), property):
            set(self, name, value)
        else:
            raise AttributeError("You cannot add instance attributes to %s" % self)
    return set_instance_attr


def _swig_setattr_nondynamic_class_variable(set):
    def set_class_attr(cls, name, value):
        if hasattr(cls, name) and not isinstance(getattr(cls, name), property):
            set(cls, name, value)
        else:
            raise AttributeError("You cannot add class attributes to %s" % cls)
    return set_class_attr


def _swig_add_metaclass(metaclass):
    """Class decorator for adding a metaclass to a SWIG wrapped class - a slimmed down version of six.add_metaclass"""
    def wrapper(cls):
        return metaclass(cls.__name__, cls.__bases__, cls.__dict__.copy())
    return wrapper


class _SwigNonDynamicMeta(type):
    """Meta class to enforce nondynamic attributes (no new attributes) for a class"""
    __setattr__ = _swig_setattr_nondynamic_class_variable(type.__setattr__)


class PSolver(object):
    thisown = property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc="The membership flag")
    __repr__ = _swig_repr
    row = property(_pazusoba.PSolver_row_get, _pazusoba.PSolver_row_set)
    column = property(_pazusoba.PSolver_column_get, _pazusoba.PSolver_column_set)
    board = property(_pazusoba.PSolver_board_get, _pazusoba.PSolver_board_set)

    def __init__(self, *args):
        _pazusoba.PSolver_swiginit(self, _pazusoba.new_PSolver(*args))

    def solve(self) -> "std::vector< Route >":
        return _pazusoba.PSolver_solve(self)

    def setBoardFrom(self, board: "std::string &") -> "void":
        return _pazusoba.PSolver_setBoardFrom(self, board)

    def setRandomBoard(self, row: "int", column: "int") -> "void":
        return _pazusoba.PSolver_setRandomBoard(self, row, column)

    def setBeamSize(self, size: "int") -> "void":
        return _pazusoba.PSolver_setBeamSize(self, size)

    def setStepLimit(self, step: "int") -> "void":
        return _pazusoba.PSolver_setStepLimit(self, step)
    __swig_destroy__ = _pazusoba.delete_PSolver

# Register PSolver in _pazusoba:
_pazusoba.PSolver_swigregister(PSolver)



