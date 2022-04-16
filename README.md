# A Python interface to the MMP-Framework
 https://github.com/bmatthiesen/mixed-monotonic
---
## Usage:

Consider the Problem IV.A from

_Bho Matthiesen, Christoph Hellings, Eduard A. Jorswieck, and Wolfgang
Utschick, "Mixed Monotonic Programming for Fast Global Optimization,"
submitted to IEEE  Transactions on Signal Processing._:
---
_Weighted Sum Rates in a K-User Interference Channel_

---


### Preparation Part 1 (Imports):
* Import the needed assets. Usable modules and their assets are listed in the following:

```Python 
from mmp_framework.expression import Variable
from mmp_framework.constraint import InequalityConstraint, LTOEConstraint, GTOEConstraint
from mmp_framework.problem import Problem
from mmp_framework.atoms import add, dot, mult, div, inv, neg, log2, ln, prod, sum
```

### Preparation Part 2 (Data):
* Have some given data:

```Python
# either by having a python-object (int, float or list->[], tuple->() of int or floats) :
alpha = [8.3401758e+02, 3.9182301e+01, 8.7457578e+03, 1.2471862e+02]
beta  = [[0, 5.9968562e+00, 9.5184622e+00, 6.0737956e-01],
        [1.3587096e+00, 0, 2.0014184e-02, 1.6249435e+00],
        [3.8521406e-01, 4.6761915e-01, 0, 1.8704400e+00],
        [1.2729254e-01, 2.1447293e-02, 3.1017335e-02, 0]]
Rmin = [0.2065572, 0.39122164, 0.33131569]
sigma = 1e-2
weights = [1, 1, 1, 1]

# or having a numpy.ndarray-object
import numpy as np
numpyarray_alpha = np.array(alpha)
numpyarray_beta = np.array(beta)
```

### Construct the needed Functions and Constraints:
1. Define the optimization variable with the known dimension (here dimension=3)
and also the lower and upper bounds of its domain (here lb=0, ub=1):
```Python
x = Variable(3, 0, 1)
```
  
2. Construct the objective function with the available MMP-atoms, which are (up to now):
    _add, mult, dot, div, inv, neg, log2, ln, prod, sum_
```Python
# Note: Split the objective as much as needed
#   (e.g. for adding constraints to parts of the function)

numerator   = mult(alpha, x)
denominator = add(sigma, dot(beta, x))
fraction    = div(numerator, denominator)
rk          = log2(add(1, fraction))
obj         = dot(weights, rk)
```

3. Construct the constraints. There are 3 available Constraint class,
which are _GTOEConstraint, LTOEConstraint, InequalityConstraint_:
```Python
    # Note: Constraints take as input either data or MMP-Expressions and
    #   !!!the dimensions of the operands have to match!!!
    constr_1 = GTOEConstraint(rk, Rmin) # rk >= Rmin_const (elementwise)
    
    # Then add all defined constraints to a list
    constraints = []
    constraints.append(constr_1)
```

### The Optimization Problem:
*  Finally, set up the Problem-class:
     The Problem-class takes as input\
       1. the objective function (always maximization problem)\
       2. the optimization variable\
       3. All given constraints in a python-list (or None)
```Python
    p = Problem(objective=objective, optvar=x, constraints=constraints)

    # Now the problem has to be set up:
    p.set_console_output(True) # default
    p.set_precision(1e-3)     # default
    p.use_relative_tolerance(False)       # default
    p.disable_reduction(True) # default
    p.output_every(1000000)   # default
    
    # and at last the solver is called by:
    p.optimize()
```

### Gathering results:
If the method _setConsoleOutput(False)_ was not called, 
calling Problem.optimize() prints out the current status to the console.

---
For accessing separate results, following functions are available:

```Python
    p.print_results()
    p.get_status()
    p.get_optimalValue()
    p.get_optimalInput()
    p.get_runtime()
    p.get_total_iterations()
    p.get_last_update_iteration()
```


## Extending the framework:

How to implement more atoms:
1. C++
   * In `mmp_framework/cppresources/expressiontree/Function.h`
   implement the Atom as subclass of the 'Function'-class. Potentially multiple
   classes need to be implemented due to case differentiations.
   * In `mmp_framework/cppresources/expressiontree/Function.h` extend the `std::variant`-container `variant_expression`
    with the implemented atom-classes. Then, adapt the method `getBasePointer` below, according to
    the order of insertion. Then add an `else if` condition in the two functions `get_variant_vector()`
    below according to the implemented atoms.
2. Python Bindings
   * In `mmp_framework/_function_mapping/_functions.py`
        implement the python-class, which calls the previously implemented Atom.
        Again, this class handles case differentiations

2. Finally, In `mmp_framework/atoms.py` make the atom available
as a python-function, handling all valid input types/dimensions and
raising errors if an illegal call to the atom happens.
