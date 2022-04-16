#ifndef EXPRTREE_EXPRESSION_H
#define EXPRTREE_EXPRESSION_H

#include <string>
#include <utility>
#include <vector>

enum sign{NN,NP,unknown};
std::ostream& operator<<(std::ostream& o, sign s)
{
    if (s == 0) {
        std::cout << "NN";
    } else if (s == 1) {
        std::cout << "NP";
    } else {
        std::cout << "U";
    }
    return o;
}

class Expression
{
  public:
    // Ctor for abstract class, gets called from derived classes
    explicit Expression(const std::string& name) {
        this->m_id = getNewID();
        this->m_name = name;
        expression_list.push_back(this);
    };
    // Copy-Ctor
    Expression(const Expression &old) = default;

    // Evaluation function (to be called from optimization Problem class)
    double evaluate_me(const std::vector<double>& x_in, const std::vector<double>& y_in) {
        this->evaluate(x_in, y_in);
        return m_value.at(0);
    };

    // Pure Virtual function evaluate, all derived classes need to implement
    virtual void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) = 0;

    // Getter for private members
    std::string getName() {return m_name;};
    std::vector<double>* getValue() {return &m_value;};
    sign getSign(){return m_sign;};
    int getDim() const{return m_dim;};
    int getID() const {return m_id;};
    static std::vector<Expression*> getExpressionList() {return expression_list;};

    // Helper methods to print expression
    virtual void printFunction(){printExpression();};
    void printExpression() {
        std::cout << "ID " << std::to_string(m_id) << ": ";
        std::cout << m_name << "(dim=" << std::to_string(m_dim) << ", Sign=" << m_sign <<"): ";
        this->printValue();
    };
    virtual void printValue() {
        std::cout << "[";
        for (int i=0; i<m_value.size(); ++i) {
            if (i==m_value.size()-1) {
                printf("%.3f]\n", m_value.at(i));
            } else {
                printf("%.3f,", m_value.at(i));
            }
        }
    };

    // Helper methods to not create duplicate Parameters
    static int getParameterId(const std::vector<double>& value) {
        for (auto expr : Expression::expression_list) {
            if (expr->m_dim == value.size() && expr->m_name == "Parameter") {
                if (*expr->getValue() == value) {
                    return expr->getID();
                }
            }
        }
        return -1;
    };
    static Expression* getExpression(int id) {
        for (auto expr : Expression::expression_list) {
            if (expr->getID() == id) {
                return expr;
            }
        }
    }

    // Methods for list all created Expressions in an objective (only needed, if one does not want to utilize
    // the static member Expression::expression_list, which is recommended.
    virtual void add_to_expr_list(std::vector<Expression*>& expr_list) {
        expr_list.push_back(this);
    }
    std::vector<Expression*> get_expression_list() {
        std::vector<Expression*> list_of_expressions;

        // recursive call to add expressions
        this->add_to_expr_list(list_of_expressions);

        // remove all duplicate elements (Parameters, Variables only need to be stored once)
        for (int i=0; i<list_of_expressions.size(); ++i) {
            for (int j=i+1; j<list_of_expressions.size();j++) {
                if (list_of_expressions.at(i)->getID() == list_of_expressions.at(j)->getID()) {
                    list_of_expressions.erase(list_of_expressions.begin() + j);
                }
            }
        }

        return list_of_expressions;
    }

  protected:
    //protected, so that subclasses have access to these members
    int m_dim = 0;
    std::vector<double> m_value;
    sign m_sign = unknown;
  private:
    std::string m_name;
    static int getNewID(){++uid_counter; return uid_counter;};
    int m_id;
    static int uid_counter;
    static std::vector<Expression*> expression_list;
};

int Expression::uid_counter = 0;
std::vector<Expression*> Expression::expression_list = {};

#endif //EXPRTREE_EXPRESSION_H