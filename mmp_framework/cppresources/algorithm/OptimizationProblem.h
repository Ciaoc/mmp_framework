#ifndef _MMP_OPTIMIZATIONPROBLEM_
#define _MMP_OPTIMIZATIONPROBLEM_

#include <array>
#include <vector>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <iostream>
#include "MMP.h"
#include "../expressiontree/Function.h"
#include "../expressiontree/Expression.h"

template <size_t Dim, bool propFair = false>
class OptimizationProblem : public MMP<Dim>
{
	using typename MMP<Dim>::vtypeS;

	public:
	    // Konstruktor
		OptimizationProblem(Function* obj) : MMP<Dim>() {
		    //m_contiguousContainer = get_variant_vector(obj);
		    //m_root = getBasePointer(m_contiguousContainer.back());
		    m_root = obj;
		};

        // Helperfunktion zum Debuggen, die vom Python-Interface aufgerufen werden kann
		void printObjective(){m_root->printFunction();};

	private:
	    // Membervariable für die Objective-Function im zusammenhängenden Speicherbereich
	    std::vector<variant_expression> m_contiguousContainer;
	    // Membervariable, speichert Pointer zu der Root-Funktion
	    Expression* m_root;

		double MMPobj(const vtypeS& x_in, const vtypeS& y_in) const override;
		double evaluate(const std::vector<double>& x, const std::vector<double>& y) const override;
};

template <size_t D, bool propFair>
double OptimizationProblem<D,propFair>::MMPobj(const vtypeS& x_in, const vtypeS& y_in) const
// overridden function MMPobj()
{
    std::vector<double> x(std::begin(x_in), std::end(x_in));
    std::vector<double> y(std::begin(y_in), std::end(y_in));
    return m_root->evaluate_me(x, y);
}

template <size_t D, bool propFair>
double OptimizationProblem<D,propFair>::evaluate(const std::vector<double>& x, const std::vector<double>& y) const
// overridden function MMPobj()
{
    return m_root->evaluate_me(x, y);
}


#endif