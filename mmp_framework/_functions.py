import cppyy
import numpy as np

import mmp_framework.expression as expr

cppyy.include('Function.h')
cppyy.include('Parameter.h')
cppyy.include('Expression.h')


class _Add(expr.Function):
    """
    Class for handling additions of two Expression.
    """

    def __init__(self, a: "expr.Expression", b: "expr.Expression"):
        """
        Create an Add-Expression.

        :param a: first argument
        :param b: second argument
        """
        super().__init__(name="add", args=[a, b])
        if a.get_dim() == b.get_dim():
            self.cpprep = cppyy.gbl.Add_vec_vec(a.cpprep, b.cpprep)
        elif a.get_dim() > b.get_dim():
            self.cpprep = cppyy.gbl.Add_vec_scalar(a.cpprep, b.cpprep)
        elif a.get_dim() < b.get_dim():
            self.cpprep = cppyy.gbl.Add_vec_scalar(b.cpprep, a.cpprep)
        else:
            raise AssertionError(
                "Invalid argument dimensions for add(a,b)! (a:"
                + str(a.get_dim()) + ", b:" +
                str(b.get_dim()) + ")")


class _MultVecVec(expr.Function):
    """ Class for handling Multiplication of two Vector-Expressions."""

    def __init__(self, a: "expr.Expression", b: "expr.Expression"):
        """
        Create a MultVecVec-Expression.

        :param a: first argument (any Expression).
        :param b: second argument (any Expression).
        """
        super().__init__(name="mult", args=[a, b])
        if a.get_sign() == cppyy.gbl.sign.NN and \
                b.get_sign() == cppyy.gbl.sign.NN:
            self.cpprep = cppyy.gbl.Mult_vecNN_vecNN(a.cpprep, b.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.NP and \
                b.get_sign() == cppyy.gbl.sign.NP:
            self.cpprep = cppyy.gbl.Mult_vecNP_vecNP(a.cpprep, b.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.NN and \
                b.get_sign() == cppyy.gbl.sign.NP:
            self.cpprep = cppyy.gbl.Mult_vecNN_vecNP(a.cpprep, b.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.NP and \
                b.get_sign() == cppyy.gbl.sign.NN:
            self.cpprep = cppyy.gbl.Mult_vecNN_vecNP(b.cpprep, a.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.unknown and \
                b.get_sign() == cppyy.gbl.sign.NN:
            self.cpprep = cppyy.gbl.Mult_vecU_vecNN(a.cpprep, b.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.NN and \
                b.get_sign() == cppyy.gbl.sign.unknown:
            self.cpprep = cppyy.gbl.Mult_vecU_vecNN(b.cpprep, a.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.unknown and \
                b.get_sign() == cppyy.gbl.sign.NP:
            self.cpprep = cppyy.gbl.Mult_vecU_vecNP(a.cpprep, b.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.NP and \
                b.get_sign() == cppyy.gbl.sign.unknown:
            self.cpprep = cppyy.gbl.Mult_vecU_vecNP(b.cpprep, a.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.unknown and \
                b.get_sign() == cppyy.gbl.sign.unknown:
            self.cpprep = cppyy.gbl.Mult_vecU_vecU(b.cpprep, a.cpprep)
        else:
            raise AssertionError("Operation not supported for given signs: a "
                                 "is " + str(_get_sign_string(a.get_sign()))
                                 + " and b is " +
                                 str(_get_sign_string(b.get_sign())))


class _MultVecScalar(expr.Function):
    """ Class for handling Multiplication of two Expressions, of which one
    is a vector and the other is a scalar."""

    def __init__(self, a: "expr.Expression", b: "expr.Expression"):
        """
        Create a MultVecScalar-Expression.

        :param a: first argument (any Expression).
        :param b: second argument (any Expression).
        """
        super().__init__(name="mult", args=[a, b])

        if a.get_sign() == cppyy.gbl.sign.NN and \
                b.get_sign() == cppyy.gbl.sign.NN:
            self.cpprep = cppyy.gbl.Mult_vecNN_scalarNN(a.cpprep, b.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.NP and \
                b.get_sign() == cppyy.gbl.sign.NP:
            self.cpprep = cppyy.gbl.Mult_vecNP_scalarNP(a.cpprep, b.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.NN and \
                b.get_sign() == cppyy.gbl.sign.NP:
            self.cpprep = cppyy.gbl.Mult_vecNN_scalarNP(a.cpprep, b.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.NP and \
                b.get_sign() == cppyy.gbl.sign.NN:
            self.cpprep = cppyy.gbl.Mult_vecNP_scalarNN(a.cpprep, b.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.unknown and \
                b.get_sign() == cppyy.gbl.sign.NN:
            self.cpprep = cppyy.gbl.Mult_vecU_scalarNN(a.cpprep, b.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.NN and \
                b.get_sign() == cppyy.gbl.sign.unknown:
            self.cpprep = cppyy.gbl.Mult_vecNN_scalarU(a.cpprep, b.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.unknown and \
                b.get_sign() == cppyy.gbl.sign.NP:
            self.cpprep = cppyy.gbl.Mult_vecU_scalarNP(a.cpprep, b.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.NP and \
                b.get_sign() == cppyy.gbl.sign.unknown:
            self.cpprep = cppyy.gbl.Mult_vecNP_scalarU(a.cpprep, b.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.unknown and \
                b.get_sign() == cppyy.gbl.sign.unknown:
            self.cpprep = cppyy.gbl.Mult_vecU_scalarU(a.cpprep, b.cpprep)
        else:
            raise AssertionError("Operation not supported for given signs: a "
                                 "is " + str(_get_sign_string(a.get_sign()))
                                 + " and b is " +
                                 str(_get_sign_string(b.get_sign())))


class _MultMatVec(expr.Function):
    """
    Class for handling dotproduct of two Expressions.
    One argument is a MultiDimParameter.
    """

    def __init__(self, a: "expr.MultiDimParameter", b: "expr.Expression"):
        """
        Create a DotMMMat-Expression.

        :param a: first argument (MultiDimParameter).
        :param b: second argument (Expression).
        """
        super().__init__(name="dot_mat", args=[a, b])

        # Dimension checking
        if a.get_dim()[1] != b.get_dim():
            raise AssertionError("Inputs have different dimensions! (a:"
                                 + str(a.get_dim()) + ", b:" +
                                 str(b.get_dim()) + ")")

        if a.get_sign() == cppyy.gbl.sign.NN and \
                b.get_sign() == cppyy.gbl.sign.NN:
            self.cpprep = cppyy.gbl.Mult_MatNN_VecNN(a.cpprep, b.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.NP and \
                b.get_sign() == cppyy.gbl.sign.NP:
            self.cpprep = cppyy.gbl.Mult_MatNP_VecNP(a.cpprep, b.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.NN and \
                b.get_sign() == cppyy.gbl.sign.NP:
            self.cpprep = cppyy.gbl.Mult_MatNN_VecNP(a.cpprep, b.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.NP and \
                b.get_sign() == cppyy.gbl.sign.NN:
            self.cpprep = cppyy.gbl.Mult_MatNP_VecNN(a.cpprep, b.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.NN and \
                b.get_sign() == cppyy.gbl.sign.unknown:
            self.cpprep = cppyy.gbl.Mult_MatNN_VecU(a.cpprep, b.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.NP and \
                b.get_sign() == cppyy.gbl.sign.unknown:
            self.cpprep = cppyy.gbl.Mult_MatNP_VecU(a.cpprep, b.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.unknown and \
                b.get_sign() == cppyy.gbl.sign.NN:
            self.cpprep = cppyy.gbl.Mult_MatU_VecNN(a.cpprep, b.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.unknown and \
                b.get_sign() == cppyy.gbl.sign.NP:
            self.cpprep = cppyy.gbl.Mult_MatU_VecNP(a.cpprep, b.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.unknown and \
                b.get_sign() == cppyy.gbl.sign.unknown:
            self.cpprep = cppyy.gbl.Mult_MatU_VecU(a.cpprep, b.cpprep)
        else:
            raise AssertionError("Operation not supported for given signs: a "
                                 "is " + str(_get_sign_string(a.get_sign()))
                                 + " and b is " +
                                 str(_get_sign_string(b.get_sign())))


class _Dot(expr.Function):
    """
    Class for handling dot-product of two Expressions.
    """

    def __init__(self, a: "expr.Expression", b: "expr.Expression"):
        """
        Create a DotMM-Expression.

        :param a: first argument (Expression).
        :param b: second argument (Expression).
        """
        super().__init__(name="dot", args=[a, b])

        # Dimension checking
        if a.get_dim() != b.get_dim():
            raise AssertionError("Input Vectors have different dimensions! (a:"
                                 + str(a.get_dim()) + ", b:" +
                                 str(b.get_dim()) + ")")

        if a.get_sign() == cppyy.gbl.sign.NN and \
                b.get_sign() == cppyy.gbl.sign.NN:
            self.cpprep = cppyy.gbl.Dot_NN_NN(a.cpprep, b.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.NP and \
                b.get_sign() == cppyy.gbl.sign.NP:
            self.cpprep = cppyy.gbl.Dot_NP_NP(a.cpprep, b.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.NN and \
                b.get_sign() == cppyy.gbl.sign.NP:
            self.cpprep = cppyy.gbl.Dot_NN_NP(a.cpprep, b.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.NP and \
                b.get_sign() == cppyy.gbl.sign.NN:
            self.cpprep = cppyy.gbl.Dot_NN_NP(b.cpprep, a.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.unknown and \
                b.get_sign() == cppyy.gbl.sign.NN:
            self.cpprep = cppyy.gbl.Dot_U_NN(a.cpprep, b.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.NN and \
                b.get_sign() == cppyy.gbl.sign.unknown:
            self.cpprep = cppyy.gbl.Dot_U_NN(b.cpprep, a.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.unknown and \
                b.get_sign() == cppyy.gbl.sign.NP:
            self.cpprep = cppyy.gbl.Dot_U_NP(a.cpprep, b.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.NP and \
                b.get_sign() == cppyy.gbl.sign.unknown:
            self.cpprep = cppyy.gbl.Dot_U_NP(b.cpprep, a.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.unknown and \
                b.get_sign() == cppyy.gbl.sign.unknown:
            self.cpprep = cppyy.gbl.Dot_U_U(a.cpprep, b.cpprep)
        else:
            raise AssertionError("Operation not supported for given signs: a "
                                 "is " + str(_get_sign_string(a.get_sign()))
                                 + " and b is " +
                                 str(_get_sign_string(b.get_sign())))


class _DivVecScalar(expr.Function):
    """Class for handling vector/scalar division of two Expressions."""

    def __init__(self, a: "expr.Expression", b: "expr.Expression"):
        """
        Create a DivVecScalar-Expression.

        :param a: first argument (any Expression).
        :param b: second argument (any Expression).
        """
        super().__init__(name="div", args=[a, b])
        if a.get_sign() == cppyy.gbl.sign.NN and b.get_sign() ==\
                cppyy.gbl.sign.NN:
            self.cpprep = cppyy.gbl.Div_VecNN_ScalarNN(a.cpprep, b.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.NP and b.get_sign() ==\
                cppyy.gbl.sign.NP:
            self.cpprep = cppyy.gbl.Div_VecNP_ScalarNP(a.cpprep, b.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.NN and b.get_sign() ==\
                cppyy.gbl.sign.NP:
            self.cpprep = cppyy.gbl.Div_VecNN_ScalarNP(a.cpprep, b.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.NP and b.get_sign() ==\
                cppyy.gbl.sign.NN:
            self.cpprep = cppyy.gbl.Div_VecNP_ScalarNN(a.cpprep, b.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.unknown and b.get_sign() == \
                cppyy.gbl.sign.NN:
            self.cpprep = cppyy.gbl.Div_VecU_ScalarNN(a.cpprep, b.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.unknown and b.get_sign() == \
                cppyy.gbl.sign.NP:
            self.cpprep = cppyy.gbl.Div_VecU_ScalarNP(a.cpprep, b.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.NN and b.get_sign() \
                == cppyy.gbl.sign.unknown:
            self.cpprep = cppyy.gbl.Div_VecNN_ScalarU(a.cpprep, b.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.NP and b.get_sign() \
                == cppyy.gbl.sign.unknown:
            self.cpprep = cppyy.gbl.Div_VecNP_ScalarU(a.cpprep, b.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.unknown and b.get_sign() \
                == cppyy.gbl.sign.unknown:
            self.cpprep = cppyy.gbl.Div_VecU_ScalarU(a.cpprep, b.cpprep)
        else:
            raise AssertionError("Operation not supported for given signs: a "
                                 "is " + str(_get_sign_string(a.get_sign()))
                                 + " and b is " +
                                 str(_get_sign_string(b.get_sign())))


class _DivVecVec(expr.Function):
    """Class for handling vector/vector division of two Expressions."""

    def __init__(self, a: "expr.Expression", b: "expr.Expression"):
        """
        Create a DivVecVec-Expression.

        :param a: first argument (any Expression).
        :param b: second argument (any Expression).
        """
        super().__init__(name="div", args=[a, b])
        if a.get_sign() == cppyy.gbl.sign.NN and b.get_sign() == \
                cppyy.gbl.sign.NN:
            self.cpprep = cppyy.gbl.Div_VecNN_VecNN(a.cpprep, b.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.NP and b.get_sign() == \
                cppyy.gbl.sign.NP:
            self.cpprep = cppyy.gbl.Div_VecNP_VecNP(a.cpprep, b.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.NN and b.get_sign() == \
                cppyy.gbl.sign.NP:
            self.cpprep = cppyy.gbl.Div_VecNN_VecNP(a.cpprep, b.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.NP and b.get_sign() == \
                cppyy.gbl.sign.NN:
            self.cpprep = cppyy.gbl.Div_VecNP_VecNN(a.cpprep, b.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.unknown and \
                b.get_sign() == cppyy.gbl.sign.NN:
            self.cpprep = cppyy.gbl.Div_VecU_VecNN(a.cpprep, b.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.unknown and \
                b.get_sign() == cppyy.gbl.sign.NP:
            self.cpprep = cppyy.gbl.Div_VecU_VecNP(a.cpprep, b.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.NN and \
                b.get_sign() == cppyy.gbl.sign.unknown:
            self.cpprep = cppyy.gbl.Div_VecNN_VecU(a.cpprep, b.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.NP and \
                b.get_sign() == cppyy.gbl.sign.unknown:
            self.cpprep = cppyy.gbl.Div_VecNP_VecU(a.cpprep, b.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.unknown and \
                b.get_sign() == cppyy.gbl.sign.unknown:
            self.cpprep = cppyy.gbl.Div_VecU_VecU(a.cpprep, b.cpprep)
        else:
            raise AssertionError("Operation not supported for given signs: a "
                                 "is " + str(_get_sign_string(a.get_sign()))
                                 + " and b is " +
                                 str(_get_sign_string(b.get_sign())))


class _DivScalarVec(expr.Function):
    """Class for handling scalar/vector division of two Expressions."""

    def __init__(self, a: "expr.Expression", b: "expr.Expression"):
        """
        Create a DivScalarVec-Expression.

        :param a: first argument (any Expression).
        :param b: second argument (any Expression).
        """
        super().__init__(name="div", args=[a, b])
        if a.get_sign() == cppyy.gbl.sign.NN and b.get_sign() == \
                cppyy.gbl.sign.NN:
            self.cpprep = cppyy.gbl.Div_ScalarNN_VecNN(a.cpprep, b.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.NP and b.get_sign() == \
                cppyy.gbl.sign.NP:
            self.cpprep = cppyy.gbl.Div_ScalarNP_VecNP(a.cpprep, b.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.NN and b.get_sign() == \
                cppyy.gbl.sign.NP:
            self.cpprep = cppyy.gbl.Div_ScalarNN_VecNP(a.cpprep, b.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.NP and b.get_sign() == \
                cppyy.gbl.sign.NN:
            self.cpprep = cppyy.gbl.Div_ScalarNP_VecNN(a.cpprep, b.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.unknown and b.get_sign() == \
                cppyy.gbl.sign.NN:
            self.cpprep = cppyy.gbl.Div_ScalarU_VecNN(a.cpprep, b.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.unknown and b.get_sign() == \
                cppyy.gbl.sign.NP:
            self.cpprep = cppyy.gbl.Div_ScalarU_VecNP(a.cpprep, b.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.NN and b.get_sign() \
                == cppyy.gbl.sign.unknown:
            self.cpprep = cppyy.gbl.Div_ScalarNN_VecU(a.cpprep, b.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.NP and b.get_sign() \
                == cppyy.gbl.sign.unknown:
            self.cpprep = cppyy.gbl.Div_ScalarNP_VecU(a.cpprep, b.cpprep)
        elif a.get_sign() == cppyy.gbl.sign.unknown and b.get_sign() \
                == cppyy.gbl.sign.unknown:
            self.cpprep = cppyy.gbl.Div_ScalarU_VecU(a.cpprep, b.cpprep)
        else:
            raise AssertionError("Operation not supported for given signs: a "
                                 "is " + str(_get_sign_string(a.get_sign()))
                                 + " and b is " +
                                 str(_get_sign_string(b.get_sign())))


class _Log2(expr.Function):
    """Class for handling the logarithm to base 2 of an Expression."""

    def __init__(self, a: "expr.Expression"):
        """
        Create a Log2-Expression.

        :param a: argument (any Expression).
        """
        super().__init__(name="log2", args=[a])
        self.cpprep = cppyy.gbl.Log2(a.cpprep)


class _LogN(expr.Function):
    """Class for handling the natural logarithm (base e) of an Expression."""

    def __init__(self, a: "expr.Expression"):
        """
        Create a LogN-Expression.

        :param a: argument (any Expression).
        """
        super().__init__(name="logN", args=[a])
        self.cpprep = cppyy.gbl.LogN(a.cpprep)


class _Inv(expr.Function):
    """Class for handling inversion of Expressions"""

    def __init__(self, a: "expr.Expression"):
        """
        Create an Inv-Expression.

        :param a: argument (any Expression).
        """
        super().__init__(name="inverse", args=[a])
        self.cpprep = cppyy.gbl.Inv(a.cpprep)


class _Neg(expr.Function):
    """Class for handling negation of Expressions"""

    def __init__(self, a: "expr.Expression"):
        """
        Create a Neg-Expression.

        :param a: argument (any Expression).
        """
        super().__init__(name="negate", args=[a])
        self.cpprep = cppyy.gbl.Neg(a.cpprep)


class _Prod(expr.Function):
    """Class for handling the product of an Expression."""

    def __init__(self, a: "expr.Expression"):
        """
        Create a Prod-Expression.

        :param a: argument (any NN Expression).
        """
        super().__init__(name="product", args=[a])
        if a.get_sign() != cppyy.gbl.sign.NN:
            raise AssertionError("The product can only be used on NN "
                                 "expressions!")
        self.cpprep = cppyy.gbl.Prod(a.cpprep)


class _Sum(expr.Function):
    """Class for handling the sum of an Expression."""

    def __init__(self, a: "expr.Expression"):
        """
        Create a Sum-Expression.

        :param a: argument (any Expression).
        """
        super().__init__(name="sum", args=[a])
        self.cpprep = cppyy.gbl.Sum(a.cpprep)


class _ListOfExpressions(expr.Function):
    """Class for handling a list of multiple expressions."""

    def __init__(self, a: ["list", "tuple", "np.ndarray"]):
        """
        Create a List-Expression.

        :param a: argument (list/tuple/numpy.ndarray of Expressions).
        """
        super().__init__(name="ListOfExpressions", args=a)
        cppexprlist = []
        for pyexpr in a:
            cppexprlist.append(pyexpr.cpprep)
        self.cpprep = cppyy.gbl.List(cppexprlist)


def _get_sign_string(a):
    if a == 0:
        return "NN"
    elif a == 1:
        return "NP"
    elif a == 2:
        return "unknown"
    else:
        raise AssertionError("Can not convert input into sign-string!")