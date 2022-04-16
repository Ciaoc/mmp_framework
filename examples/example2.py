from mmp_framework.expression import Variable
from mmp_framework.problem import Problem
from mmp_framework.atoms import add, mult, neg, ln
from mmp_framework.constraint import GTOEConstraint


def test_ALOHA():
    """ Example Slotted ALOHA : Problem IV.F from
    Bho Matthiesen, Christoph Hellings, Eduard A. Jorswieck, and Wolfgang
    Utschick, "Mixed Monotonic Programming for Fast Global Optimization,"
    submitted to IEEE  Transactions on Signal Processing."""
    # Have some problem data (Python-list, Python-tuple or numpy.ndarray)
    ck = [1.46459273, 3.56878364, 3.5327645]
    Rmin = [0.2065572, 0.39122164, 0.33131569]
    dim = 3

    # define objective:
    x = Variable(dim, 0, 1)
    value = add(1, neg(x))
    product = [1] * dim
    for k in range(dim):
        for j in range(dim):
            if k != j:
                product[k] = mult(product[k], value[j])

    rk = mult(mult(ck, x), product)
    obj = sum(ln(rk))

    # define constraints:
    constraints = []
    constr_1 = GTOEConstraint(rk, Rmin)  # rk >= Rmin_const (elementwise)
    constraints.append(constr_1)

    # Set up Problem-instance
    p = Problem(objective=obj, optvar=x, constraints=constraints)
    p.optimize()
