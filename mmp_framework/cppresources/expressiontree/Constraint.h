#ifndef _CONSTRAINTS_
#define _CONSTRAINTS_

#include <variant>
#include <vector>
#include "Expression.h"
#include "Function.h"

class Constraint {
  public:
    virtual void printConstraint() {m_constraint->printFunction();};
    Expression* getConstraint(){return m_constraint;};

  protected:
    explicit Constraint(Expression* constraint) {
        m_constraint = constraint;
        m_dim = m_constraint->getDim();
    };
    Expression* m_constraint;
    int m_dim;
};

class InequalityConstraint: public Constraint {
  public:
    explicit InequalityConstraint(Expression* constraint) : Constraint(constraint){};

    bool fulfilled(const std::vector<double>& x_in, const std::vector<double>& y_in) {
        m_constraint->evaluate(x_in, y_in);

        for (int i=0; i<m_dim; ++i)
        {
            if (m_constraint->getValue()->at(i) == 0){return false;}
        }
        return true;
    };
};

class GTOEConstraint: public Constraint {
  public:
    explicit GTOEConstraint(Expression* constraint) : Constraint(constraint){};

    bool fulfilled(const std::vector<double>& x_in, const std::vector<double>& y_in) {
        m_constraint->evaluate(x_in, y_in);
        for (int i=0; i<m_dim; ++i)
        {
            if (m_constraint->getValue()->at(i) > 0){return false;}
        }
        return true;
    };
};

class LTOEConstraint: public Constraint {
  public:
    explicit LTOEConstraint(Expression* constraint) : Constraint(constraint){};

    bool fulfilled(const std::vector<double>& x_in, const std::vector<double>& y_in) {
        m_constraint->evaluate(x_in, y_in);
        for (int i=0; i<m_dim; ++i)
        {
            if (m_constraint->getValue()->at(i) < 0){return false;}
        }
        return true;
    };
};

/*  Down below are Methods for creating a contiguous storage container for Constraints in an objective
 *  ----------------------------------------------------------------------------------------------------
 */

typedef std::variant<InequalityConstraint, GTOEConstraint, LTOEConstraint> variant_constraint;

bool fulfilledVisitor(variant_constraint& variant, const std::vector<double>& x, const std::vector<double>& y) {
    return std::visit([x,y](auto&& c) -> bool {return c.fulfilled(x,y);}, variant);
}

std::vector<variant_constraint> get_variant_vector_constraints(const std::vector<Constraint*>& constraints,
                                                               std::vector<variant_expression>& list_of_expressions) {
    // The return vector
    std::vector<variant_constraint> contiguousStorageContainer;

    // Reserve Space, so pointers do not misbehave
    contiguousStorageContainer.reserve(constraints.size());

    // Für JEDEN Constraint des gegebenen Objectives
    for (auto constr : constraints) {

        // 1. Falls InequalityConstraint:
        if (auto iec = dynamic_cast<InequalityConstraint*>(constr)) {
            Expression *basePointer;
            // Suche den constr in dem variant_container aller Expressions
            int idToFind = iec->getConstraint()->getID();
            size_t index = findElementGetIndex(list_of_expressions, idToFind);
            if (index != -1) {
                // Und speichere einen Pointer zu diesem
                basePointer = getBasePointer(list_of_expressions.at(index));
            }
            contiguousStorageContainer.push_back(InequalityConstraint(basePointer));
        }
        // 2. Falls GTOEConstraint:
        if (auto iec = dynamic_cast<GTOEConstraint*>(constr)) {
            Expression *basePointer;
            // Suche den constr in dem variant_container aller Expressions
            int idToFind = iec->getConstraint()->getID();
            size_t index = findElementGetIndex(list_of_expressions, idToFind);
            if (index != -1) {
                // Und speichere einen Pointer zu diesem
                basePointer = getBasePointer(list_of_expressions.at(index));
            }

            contiguousStorageContainer.push_back(GTOEConstraint(basePointer));
        }
        // 3. Falls LTOEConstraint:
        if (auto iec = dynamic_cast<LTOEConstraint*>(constr)) {
            Expression *basePointer;
            // Suche den constr in dem variant_container aller Expressions
            int idToFind = iec->getConstraint()->getID();
            size_t index = findElementGetIndex(list_of_expressions, idToFind);
            if (index != -1) {
                // Und speichere einen Pointer zu diesem
                basePointer = getBasePointer(list_of_expressions.at(index));
            }

            contiguousStorageContainer.push_back(LTOEConstraint{basePointer});
        }
    }
    return contiguousStorageContainer;
}

#endif