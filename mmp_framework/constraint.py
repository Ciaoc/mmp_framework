"""
A Constraint consists of a left operand, an operator and a right operand.
The left operand should be an Expression.
The right operand should be an Expression.
The Operator differs for each kind of constraint.
Requirements for a valid constraint: The dimensions of the operands have
to be identical.
"""

from abc import abstractmethod
import cppyy
import numpy as np

from .expression import Expression
from .expression import Parameter
from .atoms import add, neg

cppyy.include('Constraint.h')


class Constraint:
    """Abstract Class for handling constraints to the optimization problem."""

    @abstractmethod
    def __init__(self, left_operand: ["Expression", "int", "float", "list",
                                      "tuple", "np.ndarray"],
                 right_operand: ["Expression", "int", "float", "list",
                                 "tuple", "np.ndarray"]):
        """
        Create a constraint-instance (abstract class).

        :param left_operand: The left operand.
        :param right_operand: The right operand.
        """
        self.left_operand = _check_type(left_operand)
        self.right_operand = _check_type(right_operand)
        self._check_dimensions()
        self.mmp = add(self.right_operand, neg(self.left_operand))
        self.cpprep = None

    def _check_dimensions(self):
        if self.left_operand.get_dim() != self.right_operand.get_dim():
            raise AssertionError("Constraint operands have "
                                 "incompatible dimensions! (left operand: " +
                                 str(self.left_operand.get_dim()) + ", "
                                                                   "right operand: " +
                                 str(self.right_operand.get_dim()))

    def _getconstr(self):
        """Getter. """
        return self.mmp

    def eval(self, x, y):
        """Helper for Debugging purposes."""
        a = self.cpprep.fulfilled(x, y)
        self.cpprep.printConstraint()
        print(a)
        return a


def _check_type(operand):
    if isinstance(operand, (int, float, list, tuple, np.ndarray)):
        operand = Parameter(operand)
    elif isinstance(operand, Expression):
        pass
    else:
        raise AssertionError("Invalid argument type for constraint! ("
                             "Given type: " + str(type(operand)))
    return operand


class InequalityConstraint(Constraint):
    """
    Inequality-constraint.
    The left operand value must NOT be equal to the value of the right operand.
    """

    def __init__(self, left_operand: ["Expression", "int", "float", "list",
                                      "tuple", "np.ndarray"],
                 right_operand: ["Expression", "int", "float", "list",
                                 "tuple", "np.ndarray"]):
        """
        Create an InequalityConstraint-instance.

        :param left_operand: The left operand.
        :param right_operand: The right operand.
        """
        super().__init__(left_operand, right_operand)
        self.cpprep = cppyy.gbl.InequalityConstraint(self._getconstr().cpprep)


class GTOEConstraint(Constraint):
    """
    Greater-than-or-equal-constraint (>=).
    The left operand value must be greater than or equal to the value of the
    right operand.
    """

    def __init__(self, left_operand: ["Expression", "int", "float", "list",
                                      "tuple", "np.ndarray"],
                 right_operand: ["Expression", "int", "float", "list",
                                 "tuple", "np.ndarray"]):
        """
        Create a GTOEConstraint-instance.

        :param left_operand: The left operand.
        :param right_operand: The right operand.
        """
        super().__init__(left_operand, right_operand)
        self.cpprep = cppyy.gbl.GTOEConstraint(self._getconstr().cpprep)


class LTOEConstraint(Constraint):
    """
    Less-than-or-equal-constraint (<=).
    The left operand value must be less than or equal to the value of the
    right operand.
    """

    def __init__(self, left_operand: ["Expression", "int", "float", "list",
                                      "tuple", "np.ndarray"],
                 right_operand: ["Expression", "int", "float", "list",
                                 "tuple", "np.ndarray"]):
        """
        Create an LTOEConstraint-instance.

        :param left_operand: The left operand.
        :param right_operand: The right operand.
        """
        super().__init__(left_operand, right_operand)
        self.cpprep = cppyy.gbl.LTOEConstraint(self._getconstr().cpprep)
