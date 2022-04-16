"""Module for providing Variable, Parameter and MultiDimParameter Class."""

from abc import abstractmethod

import cppyy
import numpy as np

cppyy.include('Parameter.h')
cppyy.include('Variable.h')
cppyy.include('Expression.h')


class Expression:
    """Base abstract class for the Expression Tree Structure."""

    @abstractmethod
    def __init__(self):
        """
        Create an Expression-instance (abstract class).

        :param dim: The dimension of it's value.
        """
        self.cpprep = None
        self.sign = cppyy.gbl.sign.unknown

    def __getitem__(self, key):
        """
        Override the operator [], for accessing parts of an expression.
        Creates a Sliced-Expression.
        """
        return cppyy.gbl.Sliced(self.cpprep, key)

    def eval(self, x, y):
        """
        Evaluates the expression with input values x and y.
        Prints the whole expression-tree with it's evaluated values of this expression.

        :param x: input value x
        :param y: input value y
        :return: The result (vector/list of doubles) of the evaluation
        """
        self.cpprep.evaluate(x, y)
        self.cpprep.printFunction()
        return self.cpprep.getValue()

    def get_dim(self):
        """ Returns the dimension of this Expression
        :return: The dimension of this Expression
        """
        return self.cpprep.getDim()

    def get_sign(self):
        """ Returns the sign of this Expression
        :return: The sign of this Expression
        """
        return self.cpprep.getSign()


class Leaf(Expression):
    """
    Abstract class, derived from Expression.
    It represents Leaf-Nodes on the produced Expression-Trees, that are
    either Variables or Constants (see derived classes Variable and Parameter).
    """

    @abstractmethod
    def __init__(self):
        """
        Create a Leaf-instance (abstract class).

        :param dim: dimension of it's value.
        :param value: the value.
        """
        super().__init__()


class Variable(Leaf):
    """
    The Variable class is a concrete class derived from Expression->Leaf.
    A variable-object represents a variable, that is a parameter whose value
    can change.
    """

    def __init__(self, dim, lb, ub):
        """
        Create a variable instance.

        :param dim: The dimension of the variable
        :param lb: The lower bound for this variable
        :param ub: The upper bound for this variable
        """
        super().__init__()
        self.lb = _get_value(lb)
        self.ub = _get_value(ub)
        self.sign = None

        if _get_length(self.lb) != _get_length(self.ub) and \
                _get_length(self.lb) != 1 and _get_length(self.ub) != 1:
            raise AssertionError("The dimensions of upper/lower bound are "
                                 "not fitting!")
        elif (_get_length(self.lb) == 1 and _get_length(self.ub) == 1) or \
                (_get_length(self.lb) == dim and _get_length(self.ub) == dim):
            if self.ub < self.lb:
                raise AssertionError("Upper bound can not be less than "
                                     "lower bound!")
        elif _get_length(self.lb) == 1 and _get_length(self.ub) == dim:
            if any(v < self.lb for v in self.ub):
                raise AssertionError("Upper bound can not be less than "
                                     "lower bound!")
        elif _get_length(self.lb) == dim and _get_length(self.ub) == 1:
            if any(self.ub < v for v in self.lb):
                raise AssertionError("Upper bound can not be less than "
                                     "lower bound!")
        else:
            raise AssertionError("Can not interpret input for bounds("
                                 "lb:" + str(self.lb) + ";ub:" + str(self.ub) +
                                 ")! (dim=" + str(dim) + ";lb.dim=" +
                                 str(_get_length(self.lb)) + ";ub.dim=" +
                                 str(_get_length(self.ub)) + ")")

        if isinstance(self.lb, (int, float)):
            if self.lb >= 0:
                self.sign = cppyy.gbl.sign.NN
        elif isinstance(self.lb, (list, tuple, np.ndarray)):
            if all(v >= 0 for v in self.lb):
                self.sign = cppyy.gbl.sign.NN
        if isinstance(self.ub, (int, float)):
            if self.ub < 0:
                self.sign = cppyy.gbl.sign.NP
        elif isinstance(self.ub, (list, tuple, np.ndarray)):
            if all(v <= 0 for v in self.ub):
                self.sign = cppyy.gbl.sign.NP
        if self.sign is None:
            self.sign = cppyy.gbl.sign.unknown

        # Die C++ Datenstruktur
        self.cpprep = cppyy.gbl.Variable(dim, self.sign)

    def get_lb(self):
        return self.lb

    def get_ub(self):
        return self.ub


class Parameter(Leaf):
    """
    The Parameter class is a concrete class derived from Expression->Leaf.
    All constants, whether vectors or scalar or matrices
    (see subclass MultiDimParameter), need to be initialized as a
    Parameter-Class before they can be used in the framework.

    Moreover, the Parameter class handles the initialization of a list of
    Expressions. Internally, the C++-Class Expression::Function::List
    (hierarchically) depicts such a list/vector of any number of expressions.
    """

    def __init__(self, value):
        """
        Create a parameter-instance.

        :param value: The value of the Constant.
        :param dim: The dimension of the Constant.
        """
        super().__init__()

        value = _convert_to_vector(value)
        # check, if expression of this (dim/value) already exists:
        par_id = cppyy.gbl.Expression.getParameterId(value)
        if par_id != -1:
            # if yes, then return a pointer of respective Parameter
            self.cpprep = cppyy.gbl.Expression.getExpression(par_id)
        else:
            # Else create a new Parameter
            self.cpprep = cppyy.gbl.Parameter(value)


class MultiDimParameter(Leaf):
    """
    Concrete class derived from Expression->Leaf->Parameter.
    All Matrices have to be initialized as an MultiDimParameter in order to
    be used by the framework.
    """

    def __init__(self, value, dim=None):
        """
        Create a MultiDimParameter-instance.

        :param value: The value of the Matrix.
        :param dim: The row-dimension of the Matrix.
        """
        self.value = value

        self.vecvalue = []
        if isinstance(self.value, (list, tuple, np.ndarray)):
            rows = len(self.value)
            cols = len(self.value[0])
            for i in range(rows):
                for j in range(cols):
                    self.vecvalue.append(value[i][j])
        else:
            raise TypeError(
                "Unknown type (" + str(type(self.value)) +
                ") as argument of MultiDimParameter!")

        self.cpprep = cppyy.gbl.MultiDimParameter(rows, cols, self.vecvalue)


class Function(Expression):
    """
    Abstract class, which is derived from Expression.
    It represents all Nodes in the Expression-Tree, which have further
    children.
    All valid and usable mmp_framework-functions must derive from this class.
    """

    @abstractmethod
    def __init__(self, name, args=None):
        """
        Create a Function-instance (abstract class).

        :param name: It's name.
        :param dim: The dimension of it's value.
        :param args: arguments.
        """
        super().__init__()
        self.name = name
        self.args = args
        self.cpprep = None

    def get_variables(self, variables: "set") -> "set":
        """ Get the amount of variables in this function.
            Should always be 1, therefore not needed."""
        for arg in self.args:
            if isinstance(arg, Function):
                variables.union(arg.get_variables(variables))
            elif isinstance(arg, Variable):
                variables.add(arg)
        return variables


def _get_value(a: ["int", "float", "list", "tuple", "np.ndarray"]):
    """
    Return a single value if list of length 1.

    :param a: The input int, float, list, tuple, numpy.ndarray
    :return: The length of a
    """
    if _get_length(a) == 1 and isinstance(a, (list, tuple, np.ndarray)):
        return a[0]
    return a


def _get_length(a: ["int", "float", "list", "tuple", "np.ndarray"]):
    """
    Return the length of a value: int, float, list, tuple, numpy.ndarray.

    :param a: The input int, float, list, tuple, numpy.ndarray
    :return: The length of a
    """
    if not isinstance(a, (int, float, list, tuple, np.ndarray)):
        raise TypeError("Not a valid input type!")
    if isinstance(a, (int, float)):
        return 1
    return len(a)


def _convert_to_vector(a: ["int", "float", "list", "tuple", "np.ndarray"]):
    """ Convert the input into a python list (if it is not already),
        which then is used as input for C++ std::vector type.

    :param a: The input value
    :return: Python list
    """
    if not isinstance(a, (int, float, list, tuple, np.ndarray)):
        raise TypeError("Not a valid input type!")
    if isinstance(a, (int, float)):
        return [a]
    return a
