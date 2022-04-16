"""Module for providing the Problem class."""

import cppyy

import mmp_framework.expression as expr

cppyy.include('OptimizationProblem.h')
cppyy.include('OptimizationProblemWithConstraints.h')


class Problem:
    """
    Interface to the MMP-solver.
    The Constructor takes exactly one objective function, and an arbitrary
    amount of constraints as a list.
    Once a problem has been instantiated, it can/should no longer be changed or
    manipulated.
    """

    def __init__(self, objective: "expr.Function", optvar: "expr.Variable",
                 constraints=None):
        """
        Create a problem-instance.

        :param objective: The objective function to be optimized
        :param optvar: The optimization variable
        :param constraints: The constraint list for the problem.
        """
        if constraints is None:
            constraints = []
        self.obj = objective
        self.optvar = optvar
        self.constraints = constraints

        if len(self.constraints) == 0:
            self.mmp_object = cppyy.gbl.OptimizationProblem[
                self.optvar.get_dim()](
                self.obj.cpprep)
        else:
            cppconstraintlist = []
            for constr in self.constraints:
                cppconstraintlist.append(constr.cpprep)
            self.mmp_object = cppyy.gbl.OptimizationProblemWithConstraints[
                self.optvar.get_dim()](self.obj.cpprep, cppconstraintlist)

        self.mmp_object.setUB(optvar.ub)
        self.mmp_object.setLB(optvar.lb)
        self.set_precision(1e-2)
        self.use_relative_tolerance(False)
        self.disable_reduction(True)
        self.output_every(1000000)

    def print(self):
        """ Print the Objective-Function to console. """
        self.mmp_object.printObjective()

    # Interface functions for the MMP-class
    # ----------------------------------------------
    def set_console_output(self, a):
        """
        Set the console output, when calling the optimize()-method.

        :param a: boolean value True or False
        """
        self.mmp_object.setOutput(a)

    def set_precision(self, a):
        """
        Set the wanted precision.

        :param a: the wanted precision value.
        """
        self.mmp_object.setPrecision(a)

    def use_relative_tolerance(self, a):
        """
        Set the useRelativeTolerance-flag.

        :param a: the flag value (boolean).
        """
        self.mmp_object.useRelTol = a

    def disable_reduction(self, a):
        """
        Set the disableReduction-flag.

        :param a: the flag value (boolean).
        """
        self.mmp_object.disableReduction = a

    def output_every(self, a):
        """
        Set the output-period for the solver.

        :param a: the iteration-period value.
        """
        self.mmp_object.outputEvery = a

    def optimize(self):
        """Start the solver for this problem instance."""
        self.mmp_object.optimize()

    # -------------------------------------------------------------------------
    # getter-methods for results

    def print_results(self):
        """Print the Solver-results to console."""
        print("\nThe Solver-Results are as follows:\n")
        self.mmp_object.printResult()

    def get_status(self):
        """
        Get the Status (Optimal, Infeasible, Unsolved) as string.

        :return: The status-string.
        """
        return self.mmp_object.statusStr

    def get_optimal_value(self):
        """
        Get the optimal value of the problem.

        :return: the optimal value
        """
        return self.mmp_object.optval

    def get_optimal_input(self):
        """
        Get the input-vector for the optimal value.

        :return: The optimal input.
        """
        return self.mmp_object.get_xopt()

    def get_runtime(self):
        """
        Get the runtime of the solver in seconds.

        :return: The runtime.
        """
        return self.mmp_object.runtime

    def get_total_iterations(self):
        """
        Get the total number of needed iterations.

        :return: number of iterations.
        """
        return self.mmp_object.iter

    def get_last_update_iteration(self):
        """
        Get the Iteration number, at which the optimal solution was found.

        :return: Iteration number of optimal solution.
        """
        return self.mmp_object.lastUpdate
