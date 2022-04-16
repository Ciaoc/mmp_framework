"""
Available MMP-Atoms for constructing an objective function.
These should be called from the User.
"""
import inspect

import numpy as np

import mmp_framework._functions as f
import mmp_framework.expression as expr


def add(a: ["expr.Expression", "int", "float", "list", "tuple", "np.ndarray"],
        b: ["expr.Expression", "int", "float", "list", "tuple", "np.ndarray"])\
        -> "expr.Function":
    """
    An atom for simple addition.
    Supports vector + vector, vector + scalar, scalar + scalar.

    :param a: first argument
    :param b: second argument
    :return: an object of type 'Function'
    """
    if isinstance(a, expr.MultiDimParameter) or \
            isinstance(b, expr.MultiDimParameter):
        raise TypeError("Invalid argument types for add(a,b)! \na: "
                        + str(type(a)) + "\nb: " + str(type(b)))

    a = _check_and_convert_input(a)
    b = _check_and_convert_input(b)

    obj = f._Add(a, b)

    # TODO: MultiDimParameter Handling?! elementwise
    return obj


def mult(a: ["expr.Expression", "int", "float", "list", "tuple", "np.ndarray"],
         b: ["expr.Expression", "int", "float", "list", "tuple",
             "np.ndarray"]) \
        -> "expr.Function":
    """
    Atom for multiplication of it's arguments.
    SUpports vec*vec, vec*scalar, scalar*scalar.

    :param a: first argument
    :param b: second argument
    :return: an object of type 'Function'
    """

    a = _check_and_convert_input(a)
    b = _check_and_convert_input(b)

    if isinstance(a, expr.MultiDimParameter) and \
            isinstance(b, (expr.Function, expr.Parameter, expr.Variable)):
        obj = f._MultMatVec(a, b)
    elif isinstance(a, (expr.Function, expr.Parameter, expr.Variable)) and \
            isinstance(b, expr.MultiDimParameter):
        raise AssertionError("Currently no Vector-Matrix multiplication "
                             "supported!")
        #obj = f._MultVecMat(a, b)
    elif isinstance(a, (expr.Function, expr.Parameter, expr.Variable)) and \
            isinstance(b, (expr.Function, expr.Parameter, expr.Variable)):
        if a.get_dim() == b.get_dim():
            obj = f._MultVecVec(a, b)
        elif a.get_dim() > b.get_dim() == 1:
            obj = f._MultVecScalar(a, b)
        elif b.get_dim() > a.get_dim() == 1:
            obj = f._MultVecScalar(b, a)
        else:
            raise AssertionError(
                "Invalid argument dimensions for mult(a,b)! (a: "
                + str(a.get_dim()) + ", b: " + str(b.get_dim())
                + ")")
    # Error-throwing if unsupported input-types (e.g. mult(matrix, matrix))
    else:
        raise TypeError(
            "Invalid combination of arguments for mult(a,b)! \na: "
            + str(type(a)) + "\nb: " + str(type(b)))
    return obj


def dot(a: ["expr.Expression", "int", "float", "list", "tuple", "np.ndarray"],
        b: ["expr.Expression", "int", "float", "list", "tuple", "np.ndarray"])\
        -> "expr.Function":
    """
    Atom for dot-product (inner product) of it's arguments.

    :param a: first argument
    :param b: second argument
    :return: an object of type 'Function'
    """
    a = _check_and_convert_input(a)
    b = _check_and_convert_input(b)

    # Case checking: Creating the correct function-class
    if isinstance(a, (expr.Function, expr.Parameter, expr.Variable)) and \
            isinstance(b, (expr.Function, expr.Parameter, expr.Variable)):
        obj = f._Dot(b, a)

    # Error-throwing if unsupported input-types (e.g. dot(matrix, matrix))
    else:
        raise TypeError(
            "Invalid combination of arguments for dot(a,b)! \na: "
            + str(type(a)) + "\nb: " + str(type(b)))
    return obj


def div(a: ["expr.Expression", "int", "float", "list", "tuple", "np.ndarray"],
        b: ["expr.Expression", "int", "float", "list", "tuple", "np.ndarray"])\
        -> "expr.Function":
    """
    Atom for Division.
    Supports    vector/vector (elementwise),
                vector/scalar (elementwise),
                scalar/vector (elementwise),
                scalar/scalar.

    :param a: numerator
    :param b: denominator
    :return: an object of type 'Function'
    """
    if isinstance(a, expr.MultiDimParameter) or\
            isinstance(b, expr.MultiDimParameter):
        raise TypeError("Invalid argument types for add(a,b)! \na: "
                        + str(type(a)) + "\nb: " + str(type(b)))

    a = _check_and_convert_input(a)
    b = _check_and_convert_input(b)

    if a.get_dim() == b.get_dim():
        obj = f._DivVecVec(a, b)
    elif a.get_dim() > b.get_dim() == 1:
        obj = f._DivVecScalar(a, b)
    elif b.get_dim() > a.get_dim() == 1:
        obj = f._DivScalarVec(a, b)
    else:
        raise AssertionError(
            "Invalid argument dimensions for div(a,b)! (a: "
            + str(a.get_dim()) + ", b: " + str(b.get_dim())
            + ")")
    return obj


def log2(a: ["expr.Expression", "int", "float", "list", "tuple",
             "np.ndarray"]) -> "expr.Function":
    """
    Atom for logarithm to base 2.
    Can be used on vector (elementwise) or on a scalar.

    :param a: argument
    :return: an object of type 'Function'
    """
    if isinstance(a, expr.MultiDimParameter):
        raise TypeError("Invalid argument types for log2(a)! (a: "
                        + str(type(a)) + ")")

    a = _check_and_convert_input(a)
    obj = f._Log2(a)
    return obj


def ln(a: ["expr.Expression", "int", "float", "list", "tuple", "np.ndarray"]) \
        -> "expr.Function":
    """
    Atom for logarithm to base e.
    Can be used on vector (elementwise) or on scalar.

    :param a: argument
    :return: an object of type 'Function'
    """
    if isinstance(a, expr.MultiDimParameter):
        raise TypeError("Invalid argument types for ln(a)! (a: "
                        + str(type(a)) + ")")

    a = _check_and_convert_input(a)
    obj = f._LogN(a)
    return obj


def inv(a: ["expr.Expression", "int", "float", "list", "tuple",
            "np.ndarray"]) -> "expr.Function":
    """
    Atom for inversion of an expression.
    Can be used on vector (elementwise) or on scalar.

    :param a: argument
    :return: an object of type 'Function'
    """
    if isinstance(a, expr.MultiDimParameter):
        raise TypeError("Invalid argument types for inv(a)! (a: "
                        + str(type(a)) + ")")

    a = _check_and_convert_input(a)
    obj = f._Inv(a)
    return obj


def neg(a: ["expr.Expression", "int", "float", "list", "tuple",
            "np.ndarray"]) -> "expr.Function":
    """
    Atom for negation of an expression.
    Can be used on vector (elementwise) or on scalar.

    :param a: argument
    :return: an object of type 'Function'
    """
    if isinstance(a, expr.MultiDimParameter):
        raise TypeError("Invalid argument types for neg(a)! (a: "
                        + str(type(a)) + ")")

    a = _check_and_convert_input(a)
    obj = f._Neg(a)
    return obj


def prod(a: ["expr.Expression", "int", "float", "list", "tuple",
             "np.ndarray"]) -> "expr.Function":
    """
    Atom for the product of a vector.

    :param a: argument vector
    :return: an object of type 'Function'
    """
    if isinstance(a, expr.MultiDimParameter):
        raise TypeError("Invalid argument types for log2(a)! (a: "
                        + str(type(a)) + ")")

    a = _check_and_convert_input(a)
    obj = f._Prod(a)
    return obj


def sum(a: ["expr.Expression", "int", "float", "list", "tuple", "np.ndarray"])\
        -> "expr.Function":
    """
    Atom for the sum of a vector.

    :param a: argument vector
    :return: an object of type 'Function'
    """
    if isinstance(a, expr.MultiDimParameter):
        raise TypeError("Invalid argument types for log2(a)! (a: "
                        + str(type(a)) + ")")

    a = _check_and_convert_input(a)
    obj = f._Sum(a)
    return obj


"""
-------------------------------------------------------------------------------
Below are internal/private helper methods to check
    _is_expression_list:
        if list/tuple/numpy.ndarray consist of
        scalars     for -> Parameter-Initialization
        Expressions for -> ListExpression-Initialization.
        
    _check_and_convert_input:
        if inputs are valid and potentially convert inputs.
        
    _dim:
        dimensions of lists, tuples, np.ndarrays.
        
    _get_dim:
        the dimensions of inputs.
    
-------------------------------------------------------------------------------
"""


def _is_expression_list(a: ["list", "tuple", "np.ndarray"]):
    """
    Check if the given list consists only of expression-types.

    :param a: The given list
    :return: True, if only expression-type values are in a
    """
    if not isinstance(a, (list, tuple, np.ndarray)):
        return False
    if all(isinstance(ele, expr.Expression) for ele in a):
        return True
    else:
        return False


def _check_and_convert_input(a):
    """
    Check if the given input argument is a valid one for the atoms,
    and if not convert the input to the fitting datatype.

    :param a: The given argument
    :return: (potentially converted) given argument
    """
    # Typechecking
    if not isinstance(a, (expr.Expression, int, float, list, tuple,
                          np.ndarray)):
        raise TypeError("Invalid argument-type! \nargtype: " + str(type(a)) +
                        "in function " + str(inspect.stack()[1].function))
    # Handling conversion from list to internal expression-structure
    if isinstance(a, (int, float, list, tuple, np.ndarray)):
        if _get_dim(a) < 2:
            if _is_expression_list(a):
                a = f._ListOfExpressions(a)
            else:
                a = expr.Parameter(a)
        elif _get_dim(a) == 2:
            a = expr.MultiDimParameter(a)
        else:
            raise AssertionError("3-Dimensional data not supported!")
    return a


def _dim(a: ["list", "tuple", "np.ndarray"]):
    """
    Return the Dimensions of a list, tuple, numpy.ndarray.
    Helper method to get dimensions of list, tuple or np.ndarray type.

    :param a: The input list, tuple, numpy.ndarray
    :return: The dimensions of 'a' as a list
    """
    if not isinstance(a, (list, tuple, np.ndarray)):
        return []
    return [len(a)] + _dim(a[0])


def _get_dim(a: ["list", "tuple", "np.ndarray"]):
    """
    Return the number of Dimensions of a list, tuple, numpy.ndarray.
    Helper method to get dimensions of list, tuple or np.ndarray type.

    :param a: The input list, tuple, np.ndarray
    :return: the number of dimensions of a
    """
    return len(_dim(a))
