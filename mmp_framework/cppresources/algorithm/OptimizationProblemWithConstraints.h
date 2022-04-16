#ifndef _MMP_OPTIMIZATIONPROBLEM_CONSTRAINTS_
#define _MMP_OPTIMIZATIONPROBLEM_CONSTRAINTS_

#include <array>
#include <vector>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <iostream>
#include "MMP.h"
#include "../expressiontree/Function.h"
#include "../expressiontree/Expression.h"
#include "../expressiontree/Constraint.h"

template <size_t Dim>
class OptimizationProblemWithConstraints : public MMPconstraints<Dim>
{
	using typename MMPconstraints<Dim>::vtypeS;
	using typename MMPconstraints<Dim>::RBox;

	public:
	    // Konstruktor
		OptimizationProblemWithConstraints(Function* obj, std::vector<Constraint*> constraints) : MMPconstraints<Dim>()
		{
		    m_contiguousContainer = get_variant_vector();
		    m_root = getBasePointer(m_contiguousContainer.at(findElementGetIndex(m_contiguousContainer, obj->getID())));
		    m_constraints = get_variant_vector_constraints(constraints, m_contiguousContainer);
		};

        // Helperfunktion zum Debuggen, die vom Python-Interface aufgerufen werden kann
		void printObjective(){m_root->printFunction();};

	private:
	    // Membervariable für die Objective-Function im zusammenhängenden Speicherbereich
	    std::vector<variant_expression> m_contiguousContainer;
	    // Membervariable, speichert Pointer zu der Root-Funktion
	    Expression* m_root;
	    // Membervariablen für die gegebenen constraints
	    std::vector<variant_constraint> m_constraints;

		// virtuelle Funktionen der Elternklassen, die hier implementiert werden
		double MMPobj(const vtypeS& x, const vtypeS& y) const override;
		bool constraints(const vtypeS& x, const vtypeS& y) const override;

        // muss evtl. auch noch generalisiert werden?
		vtypeS feasiblePoint(const RBox& r) const override
			{ return r.lb(); }
};

template <size_t D>
double OptimizationProblemWithConstraints<D>::MMPobj(const vtypeS& x, const vtypeS& y) const
// overridden function MMPobj()
{
    std::vector<double> x_in(std::begin(x), std::end(x));
    std::vector<double> y_in(std::begin(y), std::end(y));
    return m_root->evaluate_me(x_in, y_in);
}

template <size_t D>
bool OptimizationProblemWithConstraints<D>::constraints(const vtypeS& x, const vtypeS& y) const
{
    std::vector<double> x_in(std::begin(x), std::end(x));
    std::vector<double> y_in(std::begin(y), std::end(y));

    for (auto constraint: m_constraints)
    {
        if (!fulfilledVisitor(constraint, x_in, y_in)){return false;}
    }
	return true;
}

#endif