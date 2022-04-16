#ifndef EXPRTREE_FUNCTION_H
#define EXPRTREE_FUNCTION_H


#include <stdexcept>
#include <utility>
#include <variant>
#include <vector>
#include <deque>
#include <stack>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <functional>
#include <iostream>
#include "Expression.h"
#include "Variable.h"
#include "Parameter.h"

class Function : public Expression
{
    //TODO: split header files(documentation) from source(implementation)

  public:
    // single argument constructor
    explicit Function(Expression *a, const std::string& name) : Expression(name) {
        m_arguments.push_back(a);
    };
    // double argument constructor (calls the single argument constructor)
    Function(Expression *a, Expression *b, const std::string& name) : Function(a, name) {
        m_arguments.push_back(b);
    };
    // vector constructor
    Function(std::vector<Expression*> a, const std::string& name) : Expression(name) {
        for (auto expr : a) {
            m_arguments.push_back(expr);
        }
    };

    // copy-Ctor for contiguous storage container
    Function(const Function &old, const std::vector<Expression*>& arguments) : Expression(old) {
        for (auto expr: arguments) {
            m_arguments.push_back(expr);
        }
    };

    // Helper virtual Method
    void add_to_expr_list(std::vector<Expression*>& expr_list) override {
        for (auto arg : m_arguments) {
            arg->add_to_expr_list(expr_list);
        }
        expr_list.push_back(this);
    }

    void printFunction() override {
        for (auto el : m_arguments){el->printFunction();}
        this->printExpression();
        std::cout << "    f(";
        for (auto el : m_arguments){std::cout << " " << el->getID() << ",";}
        std::cout << ")\n";
    }

    // getter for m_arguments
    std::vector<Expression*> getArgs() { return m_arguments; };

    // This is the recursive pure virtual function
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override = 0;
  protected:
    // A function has always arguments. Pointers to those are stored in following vector
    std::vector<Expression*> m_arguments;

};


class Add_vec_vec: public Function
{
  public:
    Add_vec_vec(Expression *a, Expression *b) : Function(a, b, "add_vec_vec") {
        this->m_arg1val = a->getValue();
        this->m_arg2val = b->getValue();

        this->m_dim = a->getDim();
        this->m_value.resize(m_dim);

        if (a->getSign() == NN && b->getSign() == NN) {
            this->m_sign = NN;
        } else if (a->getSign() == NP && b->getSign() == NP) {
            this->m_sign = NP;
        } else {
            this->m_sign = unknown;
        }
    };
    // Copy - Ctor for Variant-behaviour
    Add_vec_vec(const Add_vec_vec &old, const std::vector<Expression*>& arguments) : Function(old, arguments) {
        m_arg1val = arguments.at(0)->getValue();
        m_arg2val = arguments.at(1)->getValue();
    };
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(0)->evaluate(x_in, y_in);
        m_arguments.at(1)->evaluate(x_in, y_in);
        std::transform(m_arg1val->begin(), m_arg1val->end(), m_arg2val->begin(), m_value.begin(), std::plus<>());
    };
  private:
    std::vector<double>* m_arg1val;
    std::vector<double>* m_arg2val;
};

class Add_vec_scalar: public Function
{
  public:
    Add_vec_scalar(Expression *a, Expression *b) : Function(a, b, "add_vec_scalar") {
        this->m_arg1val = a->getValue();
        this->m_arg2val = &b->getValue()->at(0);

        this->m_dim = a->getDim();
        this->m_value.resize(m_dim);

        if (a->getSign() == NN && b->getSign() == NN) {
            this->m_sign = NN;
        } else if (a->getSign() == NP && b->getSign() == NP) {
            this->m_sign = NP;
        } else {
            this->m_sign = unknown;
        }
    };
    Add_vec_scalar(const Add_vec_scalar &old, const std::vector<Expression*>& arguments) : Function(old, arguments) {
        m_arg1val = arguments.at(0)->getValue();
        m_arg2val = &arguments.at(1)->getValue()->at(0);
    };
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(0)->evaluate(x_in, y_in);
        m_arguments.at(1)->evaluate(x_in, y_in);
        std::transform(m_arg1val->begin(), m_arg1val->end(), m_value.begin(), [this](const double & c){return c+*m_arg2val;});
    };
  private:
    std::vector<double>* m_arg1val;
    double* m_arg2val;
};

class Mult_vec_vec: public Function
{
  public:
    Mult_vec_vec(Expression *a, Expression *b) : Function(a, b, "mult_vec_vec") {
        this->m_arg1val = a->getValue();
        this->m_arg2val = b->getValue();

        this->m_dim = a->getDim();
        this->m_value.resize(m_dim);
    };
    Mult_vec_vec(const Mult_vec_vec &old, const std::vector<Expression*>& arguments) : Function(old, arguments) {
        m_arg1val = arguments.at(0)->getValue();
        m_arg2val = arguments.at(1)->getValue();
    };
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override = 0;

  protected:
    std::vector<double>* m_arg1val;
    std::vector<double>* m_arg2val;
};

class Mult_vecNN_vecNN: public Mult_vec_vec
{
  public:
    Mult_vecNN_vecNN(Expression *a, Expression *b) : Mult_vec_vec(a, b) {
        this->m_sign = NN;
    };
    Mult_vecNN_vecNN(const Mult_vecNN_vecNN &old, const std::vector<Expression*>& arguments) : Mult_vec_vec(old, arguments) {};
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(0)->evaluate(x_in, y_in);
        m_arguments.at(1)->evaluate(x_in, y_in);
        std::transform(m_arg1val->begin(), m_arg1val->end(), m_arg2val->begin(), m_value.begin(), std::multiplies<>());
    };
};
class Mult_vecNP_vecNP: public Mult_vec_vec
{
  public:
    Mult_vecNP_vecNP(Expression *a, Expression *b) : Mult_vec_vec(a, b) {
        this->m_sign = NN;
    };
    Mult_vecNP_vecNP(const Mult_vecNP_vecNP &old, const std::vector<Expression*>& arguments) : Mult_vec_vec(old, arguments) {};
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(0)->evaluate(y_in, x_in);
        m_arguments.at(1)->evaluate(y_in, x_in);
        std::transform(m_arg1val->begin(), m_arg1val->end(), m_arg2val->begin(), m_value.begin(), std::multiplies<>());
    };
};
class Mult_vecNN_vecNP: public Mult_vec_vec
{
  public:
    Mult_vecNN_vecNP(Expression *a, Expression *b) : Mult_vec_vec(a, b) {
        this->m_sign = NP;
    };
    Mult_vecNN_vecNP(const Mult_vecNN_vecNP &old, const std::vector<Expression*>& arguments) : Mult_vec_vec(old, arguments) {};
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(0)->evaluate(y_in, x_in);
        m_arguments.at(1)->evaluate(x_in, y_in);
        std::transform(m_arg1val->begin(), m_arg1val->end(), m_arg2val->begin(), m_value.begin(), std::multiplies<>());
    };
};
class Mult_vecU_vecNN: public Mult_vec_vec
{
  public:
    Mult_vecU_vecNN(Expression *a, Expression *b) : Mult_vec_vec(a, b) {
        this->m_sign = unknown;
        a_greaterequalthan0.resize(a->getDim());
        a_lessthan0.resize(a->getDim());
        addend1.resize(m_dim);
        addend2.resize(m_dim);
    };
    Mult_vecU_vecNN(const Mult_vecU_vecNN &old, const std::vector<Expression*>& arguments) : Mult_vec_vec(old, arguments) {
        addend1 = old.addend1;
        addend2 = old.addend2;
        a_greaterequalthan0 = old.a_greaterequalthan0;
        a_lessthan0 = old.a_lessthan0;
    };
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(0)->evaluate(x_in, y_in);
        for (size_t i=0;i<m_dim;++i) {
            if (m_arg1val->at(i) >= 0) {
                a_greaterequalthan0.at(i) = m_arg1val->at(i);
                a_lessthan0.at(i) = 0;
            } else {
                a_greaterequalthan0.at(i) = 0;
                a_lessthan0.at(i) = m_arg1val->at(i);
            }
        }
        m_arguments.at(1)->evaluate(y_in, x_in);
        b_swap = *m_arguments.at(1)->getValue();
        m_arguments.at(1)->evaluate(x_in, y_in);
        std::transform(a_greaterequalthan0.begin(), a_greaterequalthan0.end(), m_arg2val->begin(), addend1.begin(), std::multiplies<>());
        std::transform(a_lessthan0.begin(), a_lessthan0.end(), b_swap.begin(), addend2.begin(), std::multiplies<>());
        std::transform(addend1.begin(), addend1.end(), addend2.begin(), m_value.begin(), std::plus<>());
    };
  private:
    std::vector<double> addend1;
    std::vector<double> addend2;
    std::vector<double> b_swap;
    std::vector<double> a_greaterequalthan0;
    std::vector<double> a_lessthan0;
};
class Mult_vecU_vecNP: public Mult_vec_vec
{
  public:
    Mult_vecU_vecNP(Expression *a, Expression *b) : Mult_vec_vec(a, b) {
        this->m_sign = unknown;
        a_greaterequalthan0.resize(a->getDim());
        a_lessthan0.resize(a->getDim());
        addend1.resize(m_dim);
        addend2.resize(m_dim);
    };
    Mult_vecU_vecNP(const Mult_vecU_vecNP &old, const std::vector<Expression*>& arguments) : Mult_vec_vec(old, arguments) {
        addend1 = old.addend1;
        addend2 = old.addend2;
        a_greaterequalthan0 = old.a_greaterequalthan0;
        a_lessthan0 = old.a_lessthan0;
    };
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(0)->evaluate(y_in, x_in);
        for (size_t i=0;i<m_dim;++i) {
            if (m_arg1val->at(i) >= 0) {
                a_greaterequalthan0.at(i) = m_arg1val->at(i);
                a_lessthan0.at(i) = 0;
            } else {
                a_greaterequalthan0.at(i) = 0;
                a_lessthan0.at(i) = m_arg1val->at(i);
            }
        }
        m_arguments.at(1)->evaluate(y_in, x_in);
        b_swap = *m_arguments.at(1)->getValue();
        m_arguments.at(1)->evaluate(x_in, y_in);
        std::transform(a_greaterequalthan0.begin(), a_greaterequalthan0.end(), m_arg2val->begin(), addend1.begin(), std::multiplies<>());
        std::transform(a_lessthan0.begin(), a_lessthan0.end(), b_swap.begin(), addend2.begin(), std::multiplies<>());
        std::transform(addend1.begin(), addend1.end(), addend2.begin(), m_value.begin(), std::plus<>());
    };
  private:
    std::vector<double> addend1;
    std::vector<double> addend2;
    std::vector<double> b_swap;
    std::vector<double> a_greaterequalthan0;
    std::vector<double> a_lessthan0;
};
class Mult_vecU_vecU: public Mult_vec_vec
{
  public:
    Mult_vecU_vecU(Expression *a, Expression *b) : Mult_vec_vec(a, b) {
        this->m_sign = unknown;
        a_greaterequalthan0.resize(m_dim);
        a_lessthan0.resize(m_dim);
        b_greaterequalthan0.resize(m_dim);
        b_lessthan0.resize(m_dim);
        a_swap_greaterequalthan0.resize(m_dim);
        a_swap_lessthan0.resize(m_dim);
        b_swap_greaterequalthan0.resize(m_dim);
        b_swap_lessthan0.resize(m_dim);
        addend1.resize(m_dim);
        addend2.resize(m_dim);
        addend3.resize(m_dim);
        addend4.resize(m_dim);
    };
    Mult_vecU_vecU(const Mult_vecU_vecU &old, const std::vector<Expression*>& arguments) : Mult_vec_vec(old, arguments) {
        addend1 = old.addend1;
        addend2 = old.addend2;
        addend3 = old.addend3;
        addend4 = old.addend4;
        a_greaterequalthan0 = old.a_greaterequalthan0;
        a_lessthan0 = old.a_lessthan0;
        a_swap_greaterequalthan0 = old.a_swap_greaterequalthan0;
        a_swap_lessthan0 = old.a_swap_lessthan0;
        b_greaterequalthan0 = old.b_greaterequalthan0;
        b_lessthan0 = old.b_lessthan0;
        b_swap_greaterequalthan0 = old.b_swap_greaterequalthan0;
        b_swap_lessthan0 = old.b_swap_lessthan0;
    };
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(0)->evaluate(x_in, y_in);
        m_arguments.at(1)->evaluate(x_in, y_in);
        for (size_t i=0;i<m_dim;++i) {
            if (m_arg1val->at(i) >= 0) {
                a_greaterequalthan0.at(i) = m_arg1val->at(i);
                a_lessthan0.at(i) = 0;
                b_greaterequalthan0.at(i) = m_arg2val->at(i);
                b_lessthan0.at(i) = 0;
            } else {
                a_greaterequalthan0.at(i) = 0;
                a_lessthan0.at(i) = m_arg1val->at(i);
                b_greaterequalthan0.at(i) = 0;
                b_lessthan0.at(i) = m_arg2val->at(i);
            }
        }
        m_arguments.at(0)->evaluate(y_in, x_in);
        m_arguments.at(1)->evaluate(y_in, x_in);
        for (size_t i=0;i<m_dim;++i) {
            if (m_arg1val->at(i) >= 0) {
                a_swap_greaterequalthan0.at(i) = m_arg1val->at(i);
                a_swap_lessthan0.at(i) = 0;
                b_swap_greaterequalthan0.at(i) = m_arg2val->at(i);
                b_swap_lessthan0.at(i) = 0;
            } else {
                a_swap_greaterequalthan0.at(i) = 0;
                a_swap_lessthan0.at(i) = m_arg1val->at(i);
                b_swap_greaterequalthan0.at(i) = 0;
                b_swap_lessthan0.at(i) = m_arg2val->at(i);
            }
        }

        std::transform(a_greaterequalthan0.begin(), a_greaterequalthan0.end(), b_greaterequalthan0.begin(), addend1.begin(), std::multiplies<>());
        std::transform(a_swap_greaterequalthan0.begin(), a_swap_greaterequalthan0.end(), b_lessthan0.begin(), addend2.begin(), std::multiplies<>());
        std::transform(a_lessthan0.begin(), a_lessthan0.end(), b_swap_greaterequalthan0.begin(), addend3.begin(), std::multiplies<>());
        std::transform(a_swap_lessthan0.begin(), a_swap_lessthan0.end(), b_lessthan0.begin(), addend4.begin(), std::multiplies<>());
        std::transform(addend1.begin(), addend1.end(), addend2.begin(), m_value.begin(), std::plus<>());
        std::transform(m_value.begin(), m_value.end(), addend3.begin(), m_value.begin(), std::plus<>());
        std::transform(m_value.begin(), m_value.end(), addend4.begin(), m_value.begin(), std::plus<>());
    };
  private:
    std::vector<double> addend1;
    std::vector<double> addend2;
    std::vector<double> addend3;
    std::vector<double> addend4;
    std::vector<double> a_greaterequalthan0;
    std::vector<double> a_lessthan0;
    std::vector<double> b_greaterequalthan0;
    std::vector<double> b_lessthan0;
    std::vector<double> a_swap_greaterequalthan0;
    std::vector<double> a_swap_lessthan0;
    std::vector<double> b_swap_greaterequalthan0;
    std::vector<double> b_swap_lessthan0;
};

class Mult_vec_scalar: public Function
{
  public:
    Mult_vec_scalar(Expression *a, Expression *b) : Function(a, b, "Mult_vec_scalar") {
        this->m_arg1val = a->getValue();
        this->m_arg2val = &b->getValue()->at(0);

        this->m_dim = a->getDim();
        this->m_value.resize(m_dim);
    };
    Mult_vec_scalar(const Mult_vec_scalar &old, const std::vector<Expression*>& arguments) : Function(old, arguments) {
        m_arg1val = arguments.at(0)->getValue();
        m_arg2val = &arguments.at(1)->getValue()->at(0);
    };
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override = 0;
  protected:
    std::vector<double>* m_arg1val;
    double* m_arg2val;
};
class Mult_vecNN_scalarNN: public Mult_vec_scalar
{
  public:
    Mult_vecNN_scalarNN(Expression *a, Expression *b) : Mult_vec_scalar(a, b) {
        this->m_sign = NN;
    };
    Mult_vecNN_scalarNN(const Mult_vecNN_scalarNN &old, const std::vector<Expression*>& arguments) : Mult_vec_scalar(old, arguments) {};
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(0)->evaluate(x_in, y_in);
        m_arguments.at(1)->evaluate(x_in, y_in);
        std::transform(m_arg1val->begin(), m_arg1val->end(), m_value.begin(), [this](const double & c){return c * *m_arg2val;});
    };
};
class Mult_vecNP_scalarNP: public Mult_vec_scalar
{
  public:
    Mult_vecNP_scalarNP(Expression *a, Expression *b) : Mult_vec_scalar(a, b) {
        this->m_sign = NN;
    };
    Mult_vecNP_scalarNP(const Mult_vecNP_scalarNP &old, const std::vector<Expression*>& arguments) : Mult_vec_scalar(old, arguments) {};
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(0)->evaluate(y_in, x_in);
        m_arguments.at(1)->evaluate(y_in, x_in);
        std::transform(m_arg1val->begin(), m_arg1val->end(), m_value.begin(), [this](const double & c){return c * *m_arg2val;});
    };
};
class Mult_vecNN_scalarNP: public Mult_vec_scalar
{
  public:
    Mult_vecNN_scalarNP(Expression *a, Expression *b) : Mult_vec_scalar(a, b) {
        this->m_sign = NP;
    };
    Mult_vecNN_scalarNP(const Mult_vecNN_scalarNP &old, const std::vector<Expression*>& arguments) : Mult_vec_scalar(old, arguments) {};
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(0)->evaluate(y_in, x_in);
        m_arguments.at(1)->evaluate(x_in, y_in);
        std::transform(m_arg1val->begin(), m_arg1val->end(), m_value.begin(), [this](const double & c){return c * *m_arg2val;});
    };
};
class Mult_vecNP_scalarNN: public Mult_vec_scalar
{
  public:
    Mult_vecNP_scalarNN(Expression *a, Expression *b) : Mult_vec_scalar(a, b) {
        this->m_sign = NP;
    };
    Mult_vecNP_scalarNN(const Mult_vecNP_scalarNN &old, const std::vector<Expression*>& arguments) : Mult_vec_scalar(old, arguments) {};
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(0)->evaluate(x_in, y_in);
        m_arguments.at(1)->evaluate(y_in, x_in);
        std::transform(m_arg1val->begin(), m_arg1val->end(), m_value.begin(), [this](const double & c){return c * *m_arg2val;});
    };
};
class Mult_vecU_scalarNN: public Mult_vec_scalar
{
  public:
    Mult_vecU_scalarNN(Expression *a, Expression *b) : Mult_vec_scalar(a, b) {
        this->m_sign = unknown;
        a_greaterequalthan0.resize(a->getDim());
        a_lessthan0.resize(a->getDim());
        addend1.resize(m_dim);
        addend2.resize(m_dim);
    };
    Mult_vecU_scalarNN(const Mult_vecU_scalarNN &old, const std::vector<Expression*>& arguments) : Mult_vec_scalar(old, arguments) {
        addend1 = old.addend1;
        addend2 = old.addend2;
        a_greaterequalthan0 = old.a_greaterequalthan0;
        a_lessthan0 = old.a_lessthan0;
    };
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(0)->evaluate(x_in, y_in);
        for (size_t i=0;i<m_dim;++i) {
            if (m_arg1val->at(i) >= 0) {
                a_greaterequalthan0.at(i) = m_arg1val->at(i);
                a_lessthan0.at(i) = 0;
            } else {
                a_greaterequalthan0.at(i) = 0;
                a_lessthan0.at(i) = m_arg1val->at(i);
            }
        }
        m_arguments.at(1)->evaluate(y_in, x_in);
        b_swap = m_arguments.at(1)->getValue()->at(0);
        m_arguments.at(1)->evaluate(x_in, y_in);
        std::transform(a_greaterequalthan0.begin(), a_greaterequalthan0.end(), addend1.begin(), [this](const double & c){return c * *m_arg2val;});
        std::transform(a_lessthan0.begin(), a_lessthan0.end(), addend2.begin(), [this](const double & c){return c * b_swap;});
        std::transform(addend1.begin(), addend1.end(), addend2.begin(), m_value.begin(), std::plus<>());
    };
  private:
    std::vector<double> addend1;
    std::vector<double> addend2;
    double b_swap;
    std::vector<double> a_greaterequalthan0;
    std::vector<double> a_lessthan0;
};
class Mult_vecNN_scalarU: public Mult_vec_scalar
{
  public:
    Mult_vecNN_scalarU(Expression *a, Expression *b) : Mult_vec_scalar(a, b) {
        this->m_sign = unknown;
    };
    Mult_vecNN_scalarU(const Mult_vecNN_scalarU &old, const std::vector<Expression*>& arguments) : Mult_vec_scalar(old, arguments) {};
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(1)->evaluate(x_in, y_in);
        if (*m_arg2val >= 0) {
            m_arguments.at(0)->evaluate(x_in, y_in);
        } else {
            m_arguments.at(0)->evaluate(y_in, x_in);
        }
        std::transform(m_arg1val->begin(), m_arg1val->end(), m_value.begin(), [this](const double & c){return c * *m_arg2val;});
    };
};
class Mult_vecU_scalarNP: public Mult_vec_scalar
{
  public:
    Mult_vecU_scalarNP(Expression *a, Expression *b) : Mult_vec_scalar(a, b) {
        this->m_sign = unknown;
        a_greaterequalthan0.resize(a->getDim());
        a_lessthan0.resize(a->getDim());
        addend1.resize(m_dim);
        addend2.resize(m_dim);
    };
    Mult_vecU_scalarNP(const Mult_vecU_scalarNP &old, const std::vector<Expression*>& arguments) : Mult_vec_scalar(old, arguments) {
        addend1 = old.addend1;
        addend2 = old.addend2;
        a_greaterequalthan0 = old.a_greaterequalthan0;
        a_lessthan0 = old.a_lessthan0;
    };
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(0)->evaluate(y_in, x_in);
        for (size_t i=0;i<m_dim;++i) {
            if (m_arg1val->at(i) >= 0) {
                a_greaterequalthan0.at(i) = m_arg1val->at(i);
                a_lessthan0.at(i) = 0;
            } else {
                a_greaterequalthan0.at(i) = 0;
                a_lessthan0.at(i) = m_arg1val->at(i);
            }
        }
        m_arguments.at(1)->evaluate(y_in, x_in);
        b_swap = m_arguments.at(1)->getValue()->at(0);
        m_arguments.at(1)->evaluate(x_in, y_in);
        std::transform(a_greaterequalthan0.begin(), a_greaterequalthan0.end(), addend1.begin(), [this](const double & c){return c * *m_arg2val;});
        std::transform(a_lessthan0.begin(), a_lessthan0.end(), addend2.begin(), [this](const double & c){return c * b_swap;});
        std::transform(addend1.begin(), addend1.end(), addend2.begin(), m_value.begin(), std::plus<>());
    };
  private:
    std::vector<double> addend1;
    std::vector<double> addend2;
    double b_swap;
    std::vector<double> a_greaterequalthan0;
    std::vector<double> a_lessthan0;
};
class Mult_vecNP_scalarU: public Mult_vec_scalar
{
  public:
    Mult_vecNP_scalarU(Expression *a, Expression *b) : Mult_vec_scalar(a, b) {
        this->m_sign = unknown;
    };
    Mult_vecNP_scalarU(const Mult_vecNP_scalarU &old, const std::vector<Expression*>& arguments) : Mult_vec_scalar(old, arguments) {};
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(1)->evaluate(y_in, x_in);
        if (*m_arg2val >= 0) {
            m_arguments.at(0)->evaluate(x_in, y_in);
        } else {
            m_arguments.at(0)->evaluate(y_in, x_in);
        }
        std::transform(m_arg1val->begin(), m_arg1val->end(), m_value.begin(), [this](const double & c){return c * *m_arg2val;});
    };
};
class Mult_vecU_scalarU: public Mult_vec_scalar
{
  public:
    Mult_vecU_scalarU(Expression *a, Expression *b) : Mult_vec_scalar(a, b) {
        this->m_sign = unknown;
        a_greaterequalthan0.resize(m_dim);
        a_lessthan0.resize(m_dim);
        a_swap_greaterequalthan0.resize(m_dim);
        a_swap_lessthan0.resize(m_dim);
        addend1_2.resize(m_dim);
        addend3_4.resize(m_dim);
    };
    Mult_vecU_scalarU(const Mult_vecU_scalarU &old, const std::vector<Expression*>& arguments) : Mult_vec_scalar(old, arguments) {
        addend1_2 = old.addend1_2;
        addend3_4 = old.addend3_4;
        a_greaterequalthan0 = old.a_greaterequalthan0;
        a_lessthan0 = old.a_lessthan0;
        a_swap_greaterequalthan0 = old.a_swap_greaterequalthan0;
        a_swap_lessthan0 = old.a_swap_lessthan0;
    };
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(1)->evaluate(x_in, y_in);
        b = *m_arg2val;
        m_arguments.at(0)->evaluate(x_in, y_in);
        for (size_t i = 0; i < m_dim; ++i) {
            if (m_arg1val->at(i) >= 0) {
                a_greaterequalthan0.at(i) = m_arg1val->at(i);
                a_lessthan0.at(i) = 0;
            } else {
                a_greaterequalthan0.at(i) = 0;
                a_lessthan0.at(i) = m_arg1val->at(i);
            }
        }
        m_arguments.at(1)->evaluate(y_in, x_in);
        m_arguments.at(0)->evaluate(y_in, x_in);
        for (size_t i = 0; i < m_dim; ++i) {
            if (m_arg1val->at(i) >= 0) {
                a_swap_greaterequalthan0.at(i) = m_arg1val->at(i);
                a_swap_lessthan0.at(i) = 0;
            } else {
                a_swap_greaterequalthan0.at(i) = 0;
                a_swap_lessthan0.at(i) = m_arg1val->at(i);
            }
        }

        if (b >= 0) {
            std::transform(a_greaterequalthan0.begin(), a_greaterequalthan0.end(), addend1_2.begin(), [this](const double & c){return c* b;});
        } else {
            std::transform(a_swap_greaterequalthan0.begin(), a_swap_greaterequalthan0.end(), addend1_2.begin(), [this](const double & c){return c* b;});
        }
        if (*m_arg2val >= 0) {
            std::transform(a_lessthan0.begin(), a_lessthan0.end(), addend3_4.begin(), [this](const double & c){return c* *m_arg2val;});
        } else {
            std::transform(a_swap_lessthan0.begin(), a_swap_lessthan0.end(), addend3_4.begin(), [this](const double & c){return c* *m_arg2val;});
        }
        std::transform(addend1_2.begin(), addend1_2.end(), addend3_4.begin(), m_value.begin(), std::plus<>());
    };
  private:
    std::vector<double> addend1_2;
    std::vector<double> addend3_4;
    double b;
    std::vector<double> a_greaterequalthan0;
    std::vector<double> a_lessthan0;
    std::vector<double> a_swap_greaterequalthan0;
    std::vector<double> a_swap_lessthan0;
};

class Div_Vec_Scalar: public Function
{
  public:
    Div_Vec_Scalar(Expression *a, Expression *b) : Function(a, b, "div_vec_scalar") {
        this->m_arg1val = a->getValue();
        this->m_arg2val = &b->getValue()->at(0);

        this->m_dim = a->getDim();
        this->m_value.resize(m_dim);
    };
    Div_Vec_Scalar(const Div_Vec_Scalar &old, const std::vector<Expression*>& arguments) : Function(old, arguments) {
        m_arg1val = arguments.at(0)->getValue();
        m_arg2val = &arguments.at(0)->getValue()->at(0);
    };
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override = 0;
  protected:
    std::vector<double>* m_arg1val;
    double* m_arg2val;
};
class Div_VecNN_ScalarNN: public Div_Vec_Scalar
{
  public:
    Div_VecNN_ScalarNN(Expression *a, Expression *b) : Div_Vec_Scalar(a, b) {
        this->m_sign = NN;
    };
    Div_VecNN_ScalarNN(const Div_VecNN_ScalarNN &old, const std::vector<Expression*>& arguments) : Div_Vec_Scalar(old, arguments) {};
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(0)->evaluate(x_in, y_in);
        m_arguments.at(1)->evaluate(y_in, x_in);
        std::transform(m_arg1val->begin(), m_arg1val->end(), m_value.begin(), [this](const double & c){return c / *m_arg2val;});
    };
};
class Div_VecNP_ScalarNP: public Div_Vec_Scalar
{
  public:
    Div_VecNP_ScalarNP(Expression *a, Expression *b) : Div_Vec_Scalar(a, b) {
        this->m_sign = NN;
    };
    Div_VecNP_ScalarNP(const Div_VecNP_ScalarNP &old, const std::vector<Expression*>& arguments) : Div_Vec_Scalar(old, arguments) {};
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(0)->evaluate(y_in, x_in);
        m_arguments.at(1)->evaluate(x_in, y_in);
        std::transform(m_arg1val->begin(), m_arg1val->end(), m_value.begin(), [this](const double & c){return c / *m_arg2val;});
    };
};
class Div_VecNN_ScalarNP: public Div_Vec_Scalar
{
  public:
    Div_VecNN_ScalarNP(Expression *a, Expression *b) : Div_Vec_Scalar(a, b) {
        this->m_sign = NP;
    };
    Div_VecNN_ScalarNP(const Div_VecNN_ScalarNP &old, const std::vector<Expression*>& arguments) : Div_Vec_Scalar(old, arguments) {};
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(0)->evaluate(y_in, x_in);
        m_arguments.at(1)->evaluate(y_in, x_in);
        std::transform(m_arg1val->begin(), m_arg1val->end(), m_value.begin(), [this](const double & c){return c / *m_arg2val;});
    };
};
class Div_VecNP_ScalarNN: public Div_Vec_Scalar
{
  public:
    Div_VecNP_ScalarNN(Expression *a, Expression *b) : Div_Vec_Scalar(a, b) {
        this->m_sign = NP;
    };
    Div_VecNP_ScalarNN(const Div_VecNP_ScalarNN &old, const std::vector<Expression*>& arguments) : Div_Vec_Scalar(old, arguments) {};
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(0)->evaluate(x_in, y_in);
        m_arguments.at(1)->evaluate(x_in, y_in);
        std::transform(m_arg1val->begin(), m_arg1val->end(), m_value.begin(), [this](const double & c){return c / *m_arg2val;});
    };
};
class Div_VecU_ScalarNN: public Div_Vec_Scalar
{
  public:
    Div_VecU_ScalarNN(Expression *a, Expression *b) : Div_Vec_Scalar(a, b) {
        this->m_sign = unknown;
        a_greaterequalthan0.resize(a->getDim());
        a_lessthan0.resize(a->getDim());
        addend1.resize(m_dim);
        addend2.resize(m_dim);
    };
    Div_VecU_ScalarNN(const Div_VecU_ScalarNN &old, const std::vector<Expression*>& arguments) : Div_Vec_Scalar(old, arguments) {
        addend1 = old.addend1;
        addend2 = old.addend2;
        a_greaterequalthan0 = old.a_greaterequalthan0;
        a_lessthan0 = old.a_lessthan0;
    };
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(0)->evaluate(x_in, y_in);
        for (size_t i=0;i<m_dim;++i) {
            if (m_arg1val->at(i) >= 0) {
                a_greaterequalthan0.at(i) = m_arg1val->at(i);
                a_lessthan0.at(i) = 0;
            } else {
                a_greaterequalthan0.at(i) = 0;
                a_lessthan0.at(i) = m_arg1val->at(i);
            }
        }
        m_arguments.at(1)->evaluate(y_in, x_in);
        b_swap = *m_arg2val;
        m_arguments.at(1)->evaluate(x_in, y_in);
        std::transform(a_greaterequalthan0.begin(), a_greaterequalthan0.end(), addend1.begin(), [this](const double & c){return c / b_swap;});
        std::transform(a_lessthan0.begin(), a_lessthan0.end(), addend2.begin(), [this](const double & c){return c / *m_arg2val;});
        std::transform(addend1.begin(), addend1.end(), addend2.begin(), m_value.begin(), std::plus<>());
    };
  private:
    std::vector<double> addend1;
    std::vector<double> addend2;
    double b_swap;
    std::vector<double> a_greaterequalthan0;
    std::vector<double> a_lessthan0;
};
class Div_VecU_ScalarNP: public Div_Vec_Scalar
{
  public:
    Div_VecU_ScalarNP(Expression *a, Expression *b) : Div_Vec_Scalar(a, b) {
        this->m_sign = unknown;
        a_greaterequalthan0.resize(a->getDim());
        a_lessthan0.resize(a->getDim());
        addend1.resize(m_dim);
        addend2.resize(m_dim);
    };
    Div_VecU_ScalarNP(const Div_VecU_ScalarNP &old, const std::vector<Expression*>& arguments) : Div_Vec_Scalar(old, arguments) {
        addend1 = old.addend1;
        addend2 = old.addend2;
        a_greaterequalthan0 = old.a_greaterequalthan0;
        a_lessthan0 = old.a_lessthan0;
    };
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(0)->evaluate(y_in, x_in);
        for (size_t i=0;i<m_dim;++i) {
            if (m_arg1val->at(i) >= 0) {
                a_greaterequalthan0.at(i) = m_arg1val->at(i);
                a_lessthan0.at(i) = 0;
            } else {
                a_greaterequalthan0.at(i) = 0;
                a_lessthan0.at(i) = m_arg1val->at(i);
            }
        }
        m_arguments.at(1)->evaluate(y_in, x_in);
        b_swap = *m_arg2val;
        m_arguments.at(1)->evaluate(x_in, y_in);
        std::transform(a_greaterequalthan0.begin(), a_greaterequalthan0.end(), addend1.begin(), [this](const double & c){return c / b_swap;});
        std::transform(a_lessthan0.begin(), a_lessthan0.end(), addend2.begin(), [this](const double & c){return c / *m_arg2val;});
        std::transform(addend1.begin(), addend1.end(), addend2.begin(), m_value.begin(), std::plus<>());
    };
  private:
    std::vector<double> addend1;
    std::vector<double> addend2;
    double b_swap;
    std::vector<double> a_greaterequalthan0;
    std::vector<double> a_lessthan0;
};
class Div_VecNN_ScalarU: public Div_Vec_Scalar
{
  public:
    Div_VecNN_ScalarU(Expression *a, Expression *b) : Div_Vec_Scalar(a, b) {
        this->m_sign = unknown;
    };
    Div_VecNN_ScalarU(const Div_VecNN_ScalarU &old, const std::vector<Expression*>& arguments) : Div_Vec_Scalar(old, arguments) {};
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(1)->evaluate(y_in, x_in);
        if (*m_arg2val >= 0) {
            m_arguments.at(0)->evaluate(x_in, y_in);
        } else {
            m_arguments.at(0)->evaluate(y_in, x_in);
        }
        std::transform(m_arg1val->begin(), m_arg1val->end(), m_value.begin(), [this](const double & c){return c / *m_arg2val;});
    };
};
class Div_VecNP_ScalarU: public Div_Vec_Scalar
{
  public:
    Div_VecNP_ScalarU(Expression *a, Expression *b) : Div_Vec_Scalar(a, b) {
        this->m_sign = unknown;
    };
    Div_VecNP_ScalarU(const Div_VecNP_ScalarU &old, const std::vector<Expression*>& arguments) : Div_Vec_Scalar(old, arguments) {};
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(1)->evaluate(x_in, y_in);
        if (*m_arg2val >= 0) {
            m_arguments.at(0)->evaluate(x_in, y_in);
        } else {
            m_arguments.at(0)->evaluate(y_in, x_in);
        }
        std::transform(m_arg1val->begin(), m_arg1val->end(), m_value.begin(), [this](const double & c){return c / *m_arg2val;});
    };
};
class Div_VecU_ScalarU: public Div_Vec_Scalar
{
  public:
    Div_VecU_ScalarU(Expression *a, Expression *b) : Div_Vec_Scalar(a, b) {
        this->m_sign = unknown;
        addend1_2.resize(m_dim);
        addend3_4.resize(m_dim);
        a_greaterequalthan0.resize(m_dim);
        a_lessthan0.resize(m_dim);
        a_swap_greaterequalthan0.resize(m_dim);
        a_swap_lessthan0.resize(m_dim);
    };
    Div_VecU_ScalarU(const Div_VecU_ScalarU &old, const std::vector<Expression*>& arguments) : Div_Vec_Scalar(old, arguments) {
        addend1_2 = old.addend1_2;
        addend3_4 = old.addend3_4;
        a_greaterequalthan0 = old.a_greaterequalthan0;
        a_lessthan0 = old.a_lessthan0;
        a_swap_greaterequalthan0 = old.a_swap_greaterequalthan0;
        a_swap_lessthan0 = old.a_swap_lessthan0;
    };
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(1)->evaluate(x_in, y_in);
        b = *m_arg2val;
        m_arguments.at(1)->evaluate(y_in, x_in);
        m_arguments.at(0)->evaluate(x_in, y_in);
        for (size_t i=0;i<m_dim;++i) {
            if (m_arg1val->at(i) >= 0) {
                a_greaterequalthan0.at(i) = m_arg1val->at(i);
                a_lessthan0.at(i) = 0;
            } else {
                a_greaterequalthan0.at(i) = 0;
                a_lessthan0.at(i) = m_arg1val->at(i);
            }
        }
        m_arguments.at(0)->evaluate(y_in, x_in);
        for (size_t i=0;i<m_dim;++i) {
            if (m_arg1val->at(i) >= 0) {
                a_swap_greaterequalthan0.at(i) = m_arg1val->at(i);
                a_swap_lessthan0.at(i) = 0;
            } else {
                a_swap_greaterequalthan0.at(i) = 0;
                a_swap_lessthan0.at(i) = m_arg1val->at(i);
            }
        }
        if (b >= 0) {
            std::transform(a_lessthan0.begin(), a_lessthan0.end(), addend3_4.begin(), [this](const double & c){return c / b;});
        } else {
            std::transform(a_swap_lessthan0.begin(), a_swap_lessthan0.end(), addend3_4.begin(), [this](const double & c){return c / b;});
        }
        if (*m_arg2val >= 0) {
            std::transform(a_greaterequalthan0.begin(), a_greaterequalthan0.end(), addend1_2.begin(), [this](const double & c){return c / *m_arg2val;});
        } else {
            std::transform(a_swap_greaterequalthan0.begin(), a_swap_greaterequalthan0.end(), addend1_2.begin(), [this](const double & c){return c / *m_arg2val;});
        }
        std::transform(addend1_2.begin(), addend1_2.end(), addend3_4.begin(), m_value.begin(), std::plus<>());
    };
  private:
    std::vector<double> addend1_2;
    std::vector<double> addend3_4;
    double b;
    std::vector<double> a_greaterequalthan0;
    std::vector<double> a_lessthan0;
    std::vector<double> a_swap_greaterequalthan0;
    std::vector<double> a_swap_lessthan0;
};

class Div_Vec_Vec: public Function
{
  public:
    Div_Vec_Vec(Expression *a, Expression *b) : Function(a, b, "div_Vec_Vec") {
        this->m_arg1val = a->getValue();
        this->m_arg2val = b->getValue();

        this->m_dim = a->getDim();
        this->m_value.resize(m_dim);
    };
    Div_Vec_Vec(const Div_Vec_Vec &old, const std::vector<Expression*>& arguments) : Function(old, arguments) {
        m_arg1val = arguments.at(0)->getValue();
        m_arg2val = arguments.at(1)->getValue();
    };
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override = 0;
  protected:
    std::vector<double>* m_arg1val;
    std::vector<double>* m_arg2val;
};
class Div_VecNN_VecNN: public Div_Vec_Vec
{
  public:
    Div_VecNN_VecNN(Expression *a, Expression *b) : Div_Vec_Vec(a, b) {
        this->m_sign = NN;
    };
    Div_VecNN_VecNN(const Div_VecNN_VecNN &old, const std::vector<Expression*>& arguments) : Div_Vec_Vec(old, arguments) {};
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(0)->evaluate(x_in, y_in);
        m_arguments.at(1)->evaluate(y_in, x_in);
        std::transform(m_arg1val->begin(), m_arg1val->end(), m_arg2val->begin(), m_value.begin(), std::divides<>());
    };
};
class Div_VecNP_VecNP: public Div_Vec_Vec
{
  public:
    Div_VecNP_VecNP(Expression *a, Expression *b) : Div_Vec_Vec(a, b) {
        this->m_sign = NN;
    };
    Div_VecNP_VecNP(const Div_VecNP_VecNP &old, const std::vector<Expression*>& arguments) : Div_Vec_Vec(old, arguments) {};
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(0)->evaluate(y_in, x_in);
        m_arguments.at(1)->evaluate(x_in, y_in);
        std::transform(m_arg1val->begin(), m_arg1val->end(), m_arg2val->begin(), m_value.begin(), std::divides<>());
    };
};
class Div_VecNN_VecNP: public Div_Vec_Vec
{
  public:
    Div_VecNN_VecNP(Expression *a, Expression *b) : Div_Vec_Vec(a, b) {
        this->m_sign = NP;
    };
    Div_VecNN_VecNP(const Div_VecNN_VecNP &old, const std::vector<Expression*>& arguments) : Div_Vec_Vec(old, arguments) {};
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(0)->evaluate(y_in, x_in);
        m_arguments.at(1)->evaluate(y_in, x_in);
        std::transform(m_arg1val->begin(), m_arg1val->end(), m_arg2val->begin(), m_value.begin(), std::divides<>());
    };
};
class Div_VecNP_VecNN: public Div_Vec_Vec
{
  public:
    Div_VecNP_VecNN(Expression *a, Expression *b) : Div_Vec_Vec(a, b) {
        this->m_sign = NP;
    };
    Div_VecNP_VecNN(const Div_VecNP_VecNN &old, const std::vector<Expression*>& arguments) : Div_Vec_Vec(old, arguments) {};
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(0)->evaluate(x_in, y_in);
        m_arguments.at(1)->evaluate(x_in, y_in);
        std::transform(m_arg1val->begin(), m_arg1val->end(), m_arg2val->begin(), m_value.begin(), std::divides<>());
    };
};
class Div_VecU_VecNN: public Div_Vec_Vec
{
  public:
    Div_VecU_VecNN(Expression *a, Expression *b) : Div_Vec_Vec(a, b) {
        this->m_sign = unknown;
        a_greaterequalthan0.resize(a->getDim());
        a_lessthan0.resize(a->getDim());
        b_swap.resize(b->getDim());
        addend1.resize(m_dim);
        addend2.resize(m_dim);
    };
    Div_VecU_VecNN(const Div_VecU_VecNN &old, const std::vector<Expression*>& arguments) : Div_Vec_Vec(old, arguments) {
        addend1 = old.addend1;
        addend2 = old.addend2;
        a_greaterequalthan0 = old.a_greaterequalthan0;
        a_lessthan0 = old.a_lessthan0;
    };
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(0)->evaluate(x_in, y_in);
        for (size_t i=0;i<m_dim;++i) {
            if (m_arg1val->at(i) >= 0) {
                a_greaterequalthan0.at(i) = m_arg1val->at(i);
                a_lessthan0.at(i) = 0;
            } else {
                a_greaterequalthan0.at(i) = 0;
                a_lessthan0.at(i) = m_arg1val->at(i);
            }
        }
        m_arguments.at(1)->evaluate(y_in, x_in);
        b_swap = *m_arg2val;
        m_arguments.at(1)->evaluate(x_in, y_in);
        std::transform(a_greaterequalthan0.begin(), a_greaterequalthan0.end(), b_swap.begin(),addend1.begin(), std::divides<>());
        std::transform(a_lessthan0.begin(), a_lessthan0.end(), m_arg2val->begin(), addend2.begin(),std::divides<>());
        std::transform(addend1.begin(), addend1.end(), addend2.begin(), m_value.begin(), std::plus<>());
    };
  private:
    std::vector<double> addend1;
    std::vector<double> addend2;
    std::vector<double> b_swap;
    std::vector<double> a_greaterequalthan0;
    std::vector<double> a_lessthan0;
};
class Div_VecU_VecNP: public Div_Vec_Vec
{
  public:
    Div_VecU_VecNP(Expression *a, Expression *b) : Div_Vec_Vec(a, b) {
        this->m_sign = unknown;
        a_greaterequalthan0.resize(a->getDim());
        a_lessthan0.resize(a->getDim());
        b_swap.resize(b->getDim());
        addend1.resize(m_dim);
        addend2.resize(m_dim);
    };
    Div_VecU_VecNP(const Div_VecU_VecNP &old, const std::vector<Expression*>& arguments) : Div_Vec_Vec(old, arguments) {
        addend1 = old.addend1;
        addend2 = old.addend2;
        a_greaterequalthan0 = old.a_greaterequalthan0;
        a_lessthan0 = old.a_lessthan0;
    };
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(0)->evaluate(y_in, x_in);
        for (size_t i=0;i<m_dim;++i) {
            if (m_arg1val->at(i) >= 0) {
                a_greaterequalthan0.at(i) = m_arg1val->at(i);
                a_lessthan0.at(i) = 0;
            } else {
                a_greaterequalthan0.at(i) = 0;
                a_lessthan0.at(i) = m_arg1val->at(i);
            }
        }
        m_arguments.at(1)->evaluate(y_in, x_in);
        b_swap = *m_arg2val;
        m_arguments.at(1)->evaluate(x_in, y_in);
        std::transform(a_greaterequalthan0.begin(), a_greaterequalthan0.end(), b_swap.begin(),addend1.begin(), std::divides<>());
        std::transform(a_lessthan0.begin(), a_lessthan0.end(), m_arg2val->begin(), addend2.begin(),std::divides<>());
        std::transform(addend1.begin(), addend1.end(), addend2.begin(), m_value.begin(), std::plus<>());
    };
  private:
    std::vector<double> addend1;
    std::vector<double> addend2;
    std::vector<double> b_swap;
    std::vector<double> a_greaterequalthan0;
    std::vector<double> a_lessthan0;
};
class Div_VecNN_VecU: public Div_Vec_Vec
{
  public:
    Div_VecNN_VecU(Expression *a, Expression *b) : Div_Vec_Vec(a, b) {
        this->m_sign = unknown;
        b_greaterequalthan0.resize(b->getDim());
        b_lessthan0.resize(b->getDim());
        a_swap.resize(a->getDim());
        addend1.resize(m_dim);
        addend2.resize(m_dim);
    };
    Div_VecNN_VecU(const Div_VecNN_VecU &old, const std::vector<Expression*>& arguments) : Div_Vec_Vec(old, arguments) {
        addend1 = old.addend1;
        addend2 = old.addend2;
        b_greaterequalthan0 = old.b_greaterequalthan0;
        b_lessthan0 = old.b_lessthan0;
    };
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(1)->evaluate(y_in, x_in);
        for (size_t i=0;i<m_dim;++i) {
            if (m_arg2val->at(i) >= 0) {
                b_greaterequalthan0.at(i) = m_arg2val->at(i);
                b_lessthan0.at(i) = 0;
            } else {
                b_greaterequalthan0.at(i) = 0;
                b_lessthan0.at(i) = m_arg2val->at(i);
            }
        }
        m_arguments.at(0)->evaluate(y_in, x_in);
        a_swap = *m_arg2val;
        m_arguments.at(0)->evaluate(x_in, y_in);
        std::transform(b_greaterequalthan0.begin(), b_greaterequalthan0.end(), m_arg2val->begin(),addend1.begin(), std::divides<>());
        std::transform(b_lessthan0.begin(), b_lessthan0.end(), a_swap.begin(), addend2.begin(),std::divides<>());
        std::transform(addend1.begin(), addend1.end(), addend2.begin(), m_value.begin(), std::plus<>());
    };
  private:
    std::vector<double> addend1;
    std::vector<double> addend2;
    std::vector<double> a_swap;
    std::vector<double> b_greaterequalthan0;
    std::vector<double> b_lessthan0;
};
class Div_VecNP_VecU: public Div_Vec_Vec
{
  public:
    Div_VecNP_VecU(Expression *a, Expression *b) : Div_Vec_Vec(a, b) {
        this->m_sign = unknown;
        b_greaterequalthan0.resize(b->getDim());
        b_lessthan0.resize(b->getDim());
        a_swap.resize(a->getDim());
        addend1.resize(m_dim);
        addend2.resize(m_dim);
    };
    Div_VecNP_VecU(const Div_VecNP_VecU &old, const std::vector<Expression*>& arguments) : Div_Vec_Vec(old, arguments) {
        addend1 = old.addend1;
        addend2 = old.addend2;
        b_greaterequalthan0 = old.b_greaterequalthan0;
        b_lessthan0 = old.b_lessthan0;
    };
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(1)->evaluate(x_in, y_in);
        for (size_t i=0;i<m_dim;++i) {
            if (m_arg2val->at(i) >= 0) {
                b_greaterequalthan0.at(i) = m_arg2val->at(i);
                b_lessthan0.at(i) = 0;
            } else {
                b_greaterequalthan0.at(i) = 0;
                b_lessthan0.at(i) = m_arg2val->at(i);
            }
        }
        m_arguments.at(0)->evaluate(y_in, x_in);
        a_swap = *m_arg2val;
        m_arguments.at(0)->evaluate(x_in, y_in);
        std::transform(b_greaterequalthan0.begin(), b_greaterequalthan0.end(), m_arg2val->begin(),addend1.begin(), std::divides<>());
        std::transform(b_lessthan0.begin(), b_lessthan0.end(), a_swap.begin(), addend2.begin(),std::divides<>());
        std::transform(addend1.begin(), addend1.end(), addend2.begin(), m_value.begin(), std::plus<>());
    };
  private:
    std::vector<double> addend1;
    std::vector<double> addend2;
    std::vector<double> a_swap;
    std::vector<double> b_greaterequalthan0;
    std::vector<double> b_lessthan0;
};
class Div_VecU_VecU: public Div_Vec_Vec
{
  public:
    Div_VecU_VecU(Expression *a, Expression *b) : Div_Vec_Vec(a, b) {
        this->m_sign = unknown;
        a_greaterequalthan0.resize(m_dim);
        a_lessthan0.resize(m_dim);
        a_swap_greaterequalthan0.resize(m_dim);
        a_swap_lessthan0.resize(m_dim);
        b_greaterequalthan0.resize(m_dim);
        b_lessthan0.resize(m_dim);
        b_swap_greaterequalthan0.resize(m_dim);
        b_swap_lessthan0.resize(m_dim);
    };
    Div_VecU_VecU(const Div_VecU_VecU &old, const std::vector<Expression*>& arguments) : Div_Vec_Vec(old, arguments) {
        a_greaterequalthan0 = old.a_greaterequalthan0;
        a_lessthan0 = old.a_lessthan0;
        a_swap_greaterequalthan0 = old.a_swap_greaterequalthan0;
        a_swap_lessthan0 = old.a_swap_lessthan0;
        b_greaterequalthan0 = old.b_greaterequalthan0;
        b_lessthan0 = old.b_lessthan0;
        b_swap_greaterequalthan0 = old.b_swap_greaterequalthan0;
        b_swap_lessthan0 = old.b_swap_lessthan0;
    };
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        // Construction of cases
        m_arguments.at(0)->evaluate(x_in, y_in);
        m_arguments.at(1)->evaluate(x_in, y_in);
        for (size_t i=0;i<m_dim;++i) {
            if (m_arg1val->at(i) >= 0) {
                a_greaterequalthan0.at(i) = m_arg1val->at(i);
                a_lessthan0.at(i) = 0;
            } else {
                a_greaterequalthan0.at(i) = 0;
                a_lessthan0.at(i) = m_arg1val->at(i);
            }
            if (m_arg2val->at(i) >= 0) {
                b_greaterequalthan0.at(i) = m_arg2val->at(i);
                b_lessthan0.at(i) = 0;
            } else {
                b_greaterequalthan0.at(i) = 0;
                b_lessthan0.at(i) = m_arg2val->at(i);
            }
        }
        m_arguments.at(0)->evaluate(y_in, x_in);
        m_arguments.at(1)->evaluate(y_in, x_in);
        for (size_t i=0;i<m_dim;++i) {
            if (m_arg1val->at(i) >= 0) {
                a_swap_greaterequalthan0.at(i) = m_arg1val->at(i);
                a_swap_lessthan0.at(i) = 0;
            } else {
                a_swap_greaterequalthan0.at(i) = 0;
                a_swap_lessthan0.at(i) = m_arg1val->at(i);
            }
            if (m_arg2val->at(i) >= 0) {
                b_swap_greaterequalthan0.at(i) = m_arg2val->at(i);
                b_swap_lessthan0.at(i) = 0;
            } else {
                b_swap_greaterequalthan0.at(i) = 0;
                b_swap_lessthan0.at(i) = m_arg2val->at(i);
            }
        }
        double tmp1;
        double tmp2;
        double tmp3;
        double tmp4;
        // Evaluation with zeros being ignored
        for (size_t i=0;i<m_dim;++i) {
            if (b_swap_greaterequalthan0.at(i) == 0) {
                tmp1 = 0;
            } else {
                tmp1 = a_greaterequalthan0.at(i) / b_swap_greaterequalthan0.at(i);
            }
            if (b_swap_lessthan0.at(i) == 0) {
                tmp2 = 0;
            } else {
                tmp2 = a_swap_greaterequalthan0.at(i) / b_swap_lessthan0.at(i);
            }
            if (b_greaterequalthan0.at(i) == 0) {
                tmp3 = 0;
            } else {
                tmp3 = a_lessthan0.at(i) / b_greaterequalthan0.at(i);
            }
            if (b_lessthan0.at(i) == 0) {
                tmp4 = 0;
            } else {
                tmp4 = a_swap_lessthan0.at(i) / b_lessthan0.at(i);
            }
            m_value.at(i) = tmp1 + tmp2 + tmp3 + tmp4;
        }
    };
  private:
    std::vector<double> a_greaterequalthan0;
    std::vector<double> a_lessthan0;
    std::vector<double> a_swap_greaterequalthan0;
    std::vector<double> a_swap_lessthan0;
    std::vector<double> b_greaterequalthan0;
    std::vector<double> b_lessthan0;
    std::vector<double> b_swap_greaterequalthan0;
    std::vector<double> b_swap_lessthan0;
};

class Div_Scalar_Vec: public Function
{
  public:
    Div_Scalar_Vec(Expression *a, Expression *b) : Function(a, b, "div_Scalar_Vec") {
        this->m_arg1val = &a->getValue()->at(0);
        this->m_arg2val = b->getValue();

        this->m_dim = b->getDim();
        this->m_value.resize(m_dim);
    };
    Div_Scalar_Vec(const Div_Scalar_Vec &old, const std::vector<Expression*>& arguments) : Function(old, arguments) {
        m_arg1val = &arguments.at(0)->getValue()->at(0);
        m_arg2val = arguments.at(1)->getValue();
    };
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {};
  protected:
    double* m_arg1val;
    std::vector<double>* m_arg2val;
};
class Div_ScalarNN_VecNN: public Div_Scalar_Vec
{
  public:
    Div_ScalarNN_VecNN(Expression *a, Expression *b) : Div_Scalar_Vec(a, b) {
        this->m_sign = NN;
    };
    Div_ScalarNN_VecNN(const Div_ScalarNN_VecNN &old, const std::vector<Expression*>& arguments) : Div_Scalar_Vec(old, arguments) {};
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(0)->evaluate(x_in, y_in);
        m_arguments.at(1)->evaluate(y_in, x_in);
        std::transform(m_arg2val->begin(), m_arg2val->end(), m_value.begin(), [this](const double & c){return *m_arg1val / c;});
    };
};
class Div_ScalarNP_VecNP: public Div_Scalar_Vec
{
  public:
    Div_ScalarNP_VecNP(Expression *a, Expression *b) : Div_Scalar_Vec(a, b) {
        this->m_sign = NN;
    };
    Div_ScalarNP_VecNP(const Div_ScalarNP_VecNP &old, const std::vector<Expression*>& arguments) : Div_Scalar_Vec(old, arguments) {};
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(0)->evaluate(y_in, x_in);
        m_arguments.at(1)->evaluate(x_in, y_in);
        std::transform(m_arg2val->begin(), m_arg2val->end(), m_value.begin(), [this](const double & c){return *m_arg1val / c;});
    };
};
class Div_ScalarNN_VecNP: public Div_Scalar_Vec
{
  public:
    Div_ScalarNN_VecNP(Expression *a, Expression *b) : Div_Scalar_Vec(a, b) {
        this->m_sign = NP;
    };
    Div_ScalarNN_VecNP(const Div_ScalarNN_VecNP &old, const std::vector<Expression*>& arguments) : Div_Scalar_Vec(old, arguments) {};
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(0)->evaluate(y_in, x_in);
        m_arguments.at(1)->evaluate(y_in, x_in);
        std::transform(m_arg2val->begin(), m_arg2val->end(), m_value.begin(), [this](const double & c){return *m_arg1val / c;});
    };
};
class Div_ScalarNP_VecNN: public Div_Scalar_Vec
{
  public:
    Div_ScalarNP_VecNN(Expression *a, Expression *b) : Div_Scalar_Vec(a, b) {
        this->m_sign = NP;
    };
    Div_ScalarNP_VecNN(const Div_ScalarNP_VecNN &old, const std::vector<Expression*>& arguments) : Div_Scalar_Vec(old, arguments) {};
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(0)->evaluate(x_in, y_in);
        m_arguments.at(1)->evaluate(x_in, y_in);
        std::transform(m_arg2val->begin(), m_arg2val->end(), m_value.begin(), [this](const double & c){return *m_arg1val / c;});
    };
};
class Div_ScalarU_VecNN: public Div_Scalar_Vec
{
  public:
    Div_ScalarU_VecNN(Expression *a, Expression *b) : Div_Scalar_Vec(a, b) {
        this->m_sign = unknown;
    };
    Div_ScalarU_VecNN(const Div_ScalarU_VecNN &old, const std::vector<Expression*>& arguments) : Div_Scalar_Vec(old, arguments) {};
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(0)->evaluate(x_in, y_in);
        if (*m_arg1val >= 0) {
            m_arguments.at(0)->evaluate(y_in, x_in);
        } else {
            m_arguments.at(0)->evaluate(x_in, y_in);
        }
        std::transform(m_arg2val->begin(), m_arg2val->end(), m_value.begin(), [this](const double & c){return *m_arg1val / c;});
    };
};
class Div_ScalarU_VecNP: public Div_Scalar_Vec
{
  public:
    Div_ScalarU_VecNP(Expression *a, Expression *b) : Div_Scalar_Vec(a, b) {
        this->m_sign = unknown;
    };
    Div_ScalarU_VecNP(const Div_ScalarU_VecNP &old, const std::vector<Expression*>& arguments) : Div_Scalar_Vec(old, arguments) {};
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(0)->evaluate(y_in, x_in);
        if (*m_arg1val >= 0) {
            m_arguments.at(0)->evaluate(y_in, x_in);
        } else {
            m_arguments.at(0)->evaluate(x_in, y_in);
        }
        std::transform(m_arg2val->begin(), m_arg2val->end(), m_value.begin(), [this](const double & c){return *m_arg1val / c;});
    };
};
class Div_ScalarNN_VecU: public Div_Scalar_Vec
{
  public:
    Div_ScalarNN_VecU(Expression *a, Expression *b) : Div_Scalar_Vec(a, b) {
        this->m_sign = unknown;
        b_greaterequalthan0.resize(b->getDim());
        b_lessthan0.resize(b->getDim());
        addend1.resize(m_dim);
        addend2.resize(m_dim);
    };
    Div_ScalarNN_VecU(const Div_ScalarNN_VecU &old, const std::vector<Expression*>& arguments) : Div_Scalar_Vec(old, arguments) {
        addend1 = old.addend1;
        addend2 = old.addend2;
        b_greaterequalthan0 = old.b_greaterequalthan0;
        b_lessthan0 = old.b_lessthan0;
    };
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(1)->evaluate(y_in, x_in);
        for (size_t i=0;i<m_dim;++i) {
            if (m_arg2val->at(i) >= 0) {
                b_greaterequalthan0.at(i) = m_arg2val->at(i);
                b_lessthan0.at(i) = 0;
            } else {
                b_greaterequalthan0.at(i) = 0;
                b_lessthan0.at(i) = m_arg2val->at(i);
            }
        }
        m_arguments.at(0)->evaluate(y_in, x_in);
        a_swap = *m_arg1val;
        m_arguments.at(0)->evaluate(x_in, y_in);
        std::transform(b_greaterequalthan0.begin(), b_greaterequalthan0.end(), addend1.begin(), [this](const double & c){return *m_arg1val / c;});
        std::transform(b_lessthan0.begin(), b_lessthan0.end(), addend2.begin(),[this](const double & c){return a_swap / c;});
        std::transform(addend1.begin(), addend1.end(), addend2.begin(), m_value.begin(), std::plus<>());
    };
  private:
    std::vector<double> addend1;
    std::vector<double> addend2;
    double a_swap;
    std::vector<double> b_greaterequalthan0;
    std::vector<double> b_lessthan0;
};
class Div_ScalarNP_VecU: public Div_Scalar_Vec
{
  public:
    Div_ScalarNP_VecU(Expression *a, Expression *b) : Div_Scalar_Vec(a, b) {
        this->m_sign = unknown;
        b_greaterequalthan0.resize(b->getDim());
        b_lessthan0.resize(b->getDim());
        addend1.resize(m_dim);
        addend2.resize(m_dim);
    };
    Div_ScalarNP_VecU(const Div_ScalarNP_VecU &old, const std::vector<Expression*>& arguments) : Div_Scalar_Vec(old, arguments) {
        addend1 = old.addend1;
        addend2 = old.addend2;
        b_greaterequalthan0 = old.b_greaterequalthan0;
        b_lessthan0 = old.b_lessthan0;
    };
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(1)->evaluate(x_in, y_in);
        for (size_t i=0;i<m_dim;++i) {
            if (m_arg2val->at(i) >= 0) {
                b_greaterequalthan0.at(i) = m_arg2val->at(i);
                b_lessthan0.at(i) = 0;
            } else {
                b_greaterequalthan0.at(i) = 0;
                b_lessthan0.at(i) = m_arg2val->at(i);
            }
        }
        m_arguments.at(0)->evaluate(y_in, x_in);
        a_swap = *m_arg1val;
        m_arguments.at(0)->evaluate(x_in, y_in);
        std::transform(b_greaterequalthan0.begin(), b_greaterequalthan0.end(), addend1.begin(), [this](const double & c){return *m_arg1val / c;});
        std::transform(b_lessthan0.begin(), b_lessthan0.end(), addend2.begin(),[this](const double & c){return a_swap / c;});
        std::transform(addend1.begin(), addend1.end(), addend2.begin(), m_value.begin(), std::plus<>());
    };
  private:
    std::vector<double> addend1;
    std::vector<double> addend2;
    double a_swap;
    std::vector<double> b_greaterequalthan0;
    std::vector<double> b_lessthan0;
};
class Div_ScalarU_VecU: public Div_Scalar_Vec
{
  public:
    Div_ScalarU_VecU(Expression *a, Expression *b) : Div_Scalar_Vec(a, b) {
        this->m_sign = unknown;
        b_greaterequalthan0.resize(m_dim);
        b_lessthan0.resize(m_dim);
        b_swap_greaterequalthan0.resize(m_dim);
        b_swap_lessthan0.resize(m_dim);
    };
    Div_ScalarU_VecU(const Div_ScalarU_VecU &old, const std::vector<Expression*>& arguments) : Div_Scalar_Vec(old, arguments) {
        b_greaterequalthan0 = old.b_greaterequalthan0;
        b_lessthan0 = old.b_lessthan0;
        b_swap_greaterequalthan0 = old.b_swap_greaterequalthan0;
        b_swap_lessthan0 = old.b_swap_lessthan0;
    };
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        // Construction of cases
        m_arguments.at(0)->evaluate(x_in, y_in);
        a = *m_arg1val;
        m_arguments.at(0)->evaluate(y_in, x_in);
        m_arguments.at(1)->evaluate(x_in, y_in);
        for (size_t i=0;i<m_dim;++i) {
            if (m_arg2val->at(i) >= 0) {
                b_greaterequalthan0.at(i) = m_arg2val->at(i);
                b_lessthan0.at(i) = 0;
            } else {
                b_greaterequalthan0.at(i) = 0;
                b_lessthan0.at(i) = m_arg2val->at(i);
            }
        }
        m_arguments.at(1)->evaluate(y_in, x_in);
        for (size_t i=0;i<m_dim;++i) {
            if (m_arg2val->at(i) >= 0) {
                b_swap_greaterequalthan0.at(i) = m_arg2val->at(i);
                b_swap_lessthan0.at(i) = 0;
            } else {
                b_swap_greaterequalthan0.at(i) = 0;
                b_swap_lessthan0.at(i) = m_arg2val->at(i);
            }
        }
        double tmp1_3;
        double tmp2_4;
        // Evaluation with zeros being ignored
        for (size_t i=0;i<m_dim;++i) {
            if (a >= 0) {
                if (b_swap_greaterequalthan0.at(i) == 0) {
                    tmp1_3 = 0;
                } else {
                    tmp1_3 = a / b_swap_greaterequalthan0.at(i);
                }
            } else {
                if (b_greaterequalthan0.at(i) == 0) {
                    tmp1_3 = 0;
                } else {
                    tmp1_3 = a / b_greaterequalthan0.at(i);
                }
            }
            if (*m_arg1val >= 0) {
                if (b_swap_lessthan0.at(i) == 0) {
                    tmp2_4 = 0;
                } else {
                    tmp2_4 = *m_arg1val / b_swap_lessthan0.at(i);
                }
            } else {
                if (b_lessthan0.at(i) == 0) {
                    tmp2_4 = 0;
                } else {
                    tmp2_4 = *m_arg1val / b_lessthan0.at(i);
                }
            }
            m_value.at(i) = tmp1_3 + tmp2_4;
        }
    };
  private:
    double a;
    std::vector<double> b_greaterequalthan0;
    std::vector<double> b_lessthan0;
    std::vector<double> b_swap_greaterequalthan0;
    std::vector<double> b_swap_lessthan0;
};

class Dot: public Function
{
  public:
    Dot(Expression *a, Expression *b) : Function(a, b, "dot") {
        this->m_arg1val = a->getValue();
        this->m_arg2val = b->getValue();

        this->m_dim = 1;
        this->m_value.resize(m_dim);
    };
    Dot(const Dot &old, const std::vector<Expression*>& arguments) : Function(old, arguments) {
        m_arg1val = arguments.at(0)->getValue();
        m_arg2val = arguments.at(1)->getValue();
    };
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override = 0;
  protected:
    std::vector<double>* m_arg1val;
    std::vector<double>* m_arg2val;
};
class Dot_NN_NN: public Dot
{
  public:
    Dot_NN_NN(Expression *a, Expression *b) : Dot(a, b) {
        this->m_sign = NN;
    };
    Dot_NN_NN(const Dot_NN_NN &old, const std::vector<Expression*>& arguments) : Dot(old, arguments) {};
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(0)->evaluate(x_in, y_in);
        m_arguments.at(1)->evaluate(x_in, y_in);
        m_value.at(0) = std::inner_product(m_arg1val->begin(), m_arg1val->end(), m_arg2val->begin(), (double)0);
    };
};
class Dot_NP_NP: public Dot
{
  public:
    Dot_NP_NP(Expression *a, Expression *b) : Dot(a, b) {
        this->m_sign = NN;
    };
    Dot_NP_NP(const Dot_NP_NP &old, const std::vector<Expression*>& arguments) : Dot(old, arguments) {};
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(0)->evaluate(y_in, x_in);
        m_arguments.at(1)->evaluate(y_in, x_in);
        m_value.at(0) = std::inner_product(m_arg1val->begin(), m_arg1val->end(), m_arg2val->begin(), (double)0);
    };
};
class Dot_NN_NP: public Dot
{
  public:
    Dot_NN_NP(Expression *a, Expression *b) : Dot(a, b) {
        this->m_sign = NN;
    };
    Dot_NN_NP(const Dot_NN_NP &old, const std::vector<Expression*>& arguments) : Dot(old, arguments) {};
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(0)->evaluate(y_in, x_in);
        m_arguments.at(1)->evaluate(x_in, y_in);
        m_value.at(0) = std::inner_product(m_arg1val->begin(), m_arg1val->end(), m_arg2val->begin(), (double)0);
    };
};
class Dot_U_NN: public Dot
{
  public:
    Dot_U_NN(Expression *a, Expression *b) : Dot(a, b) {
        this->m_sign = unknown;
        a_greaterequalthan0.resize(a->getDim());
        a_lessthan0.resize(a->getDim());
    };
    Dot_U_NN(const Dot_U_NN &old, const std::vector<Expression*>& arguments) : Dot(old, arguments) {
        a_greaterequalthan0 = old.a_greaterequalthan0;
        a_lessthan0 = old.a_lessthan0;
    };
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(0)->evaluate(x_in, y_in);
        for (size_t i=0;i<m_arg1val->size();++i) {
            if (m_arg1val->at(i) >= 0) {
                a_greaterequalthan0.at(i) = m_arg1val->at(i);
                a_lessthan0.at(i) = 0;
            } else {
                a_greaterequalthan0.at(i) = 0;
                a_lessthan0.at(i) = m_arg1val->at(i);
            }
        }
        m_arguments.at(1)->evaluate(y_in, x_in);
        b_swap = *m_arguments.at(1)->getValue();
        m_arguments.at(1)->evaluate(x_in, y_in);
        m_value.at(0) = std::inner_product(a_greaterequalthan0.begin(), a_greaterequalthan0.end(), m_arg2val->begin(), (double)0);
        m_value.at(0) += std::inner_product(a_lessthan0.begin(), a_lessthan0.end(), b_swap.begin(), (double)0);
    };
  private:
    std::vector<double> b_swap;
    std::vector<double> a_greaterequalthan0;
    std::vector<double> a_lessthan0;
};
class Dot_U_NP: public Dot
{
  public:
    Dot_U_NP(Expression *a, Expression *b) : Dot(a, b) {
        this->m_sign = unknown;
        a_greaterequalthan0.resize(a->getDim());
        a_lessthan0.resize(a->getDim());
    };
    Dot_U_NP(const Dot_U_NP &old, const std::vector<Expression*>& arguments) : Dot(old, arguments) {
        a_greaterequalthan0 = old.a_greaterequalthan0;
        a_lessthan0 = old.a_lessthan0;
    };
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(0)->evaluate(y_in, x_in);
        for (size_t i=0;i<m_dim;++i) {
            if (m_arg1val->at(i) >= 0) {
                a_greaterequalthan0.at(i) = m_arg1val->at(i);
                a_lessthan0.at(i) = 0;
            } else {
                a_greaterequalthan0.at(i) = 0;
                a_lessthan0.at(i) = m_arg1val->at(i);
            }
        }
        m_arguments.at(1)->evaluate(y_in, x_in);
        b_swap = *m_arguments.at(1)->getValue();
        m_arguments.at(1)->evaluate(x_in, y_in);
        m_value.at(0) = std::inner_product(a_greaterequalthan0.begin(), a_greaterequalthan0.end(), m_arg2val->begin(), (double)0);
        m_value.at(0) = std::inner_product(a_lessthan0.begin(), a_lessthan0.end(), b_swap.begin(), m_value.at(0));
    };
  private:
    std::vector<double> b_swap;
    std::vector<double> a_greaterequalthan0;
    std::vector<double> a_lessthan0;
};
class Dot_U_U: public Dot
{
  public:
    Dot_U_U(Expression *a, Expression *b) : Dot(a, b) {
        this->m_sign = unknown;
        a_greaterequalthan0.resize(m_dim);
        a_lessthan0.resize(m_dim);
        b_greaterequalthan0.resize(m_dim);
        b_lessthan0.resize(m_dim);
        a_swap_greaterequalthan0.resize(m_dim);
        a_swap_lessthan0.resize(m_dim);
        b_swap_greaterequalthan0.resize(m_dim);
        b_swap_lessthan0.resize(m_dim);
        addend1.resize(m_dim);
        addend2.resize(m_dim);
        addend3.resize(m_dim);
        addend4.resize(m_dim);
    };
    Dot_U_U(const Dot_U_U &old, const std::vector<Expression*>& arguments) : Dot(old, arguments) {
        addend1 = old.addend1;
        addend2 = old.addend2;
        addend3 = old.addend3;
        addend4 = old.addend4;
        a_greaterequalthan0 = old.a_greaterequalthan0;
        a_lessthan0 = old.a_lessthan0;
        a_swap_greaterequalthan0 = old.a_swap_greaterequalthan0;
        a_swap_lessthan0 = old.a_swap_lessthan0;
        b_greaterequalthan0 = old.b_greaterequalthan0;
        b_lessthan0 = old.b_lessthan0;
        b_swap_greaterequalthan0 = old.b_swap_greaterequalthan0;
        b_swap_lessthan0 = old.b_swap_lessthan0;
    };
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(0)->evaluate(x_in, y_in);
        m_arguments.at(1)->evaluate(x_in, y_in);
        for (size_t i=0;i<m_dim;++i) {
            if (m_arg1val->at(i) >= 0) {
                a_greaterequalthan0.at(i) = m_arg1val->at(i);
                a_lessthan0.at(i) = 0;
                b_greaterequalthan0.at(i) = m_arg2val->at(i);
                b_lessthan0.at(i) = 0;
            } else {
                a_greaterequalthan0.at(i) = 0;
                a_lessthan0.at(i) = m_arg1val->at(i);
                b_greaterequalthan0.at(i) = 0;
                b_lessthan0.at(i) = m_arg2val->at(i);
            }
        }
        m_arguments.at(0)->evaluate(y_in, x_in);
        m_arguments.at(1)->evaluate(y_in, x_in);
        for (size_t i=0;i<m_dim;++i) {
            if (m_arg1val->at(i) >= 0) {
                a_swap_greaterequalthan0.at(i) = m_arg1val->at(i);
                a_swap_lessthan0.at(i) = 0;
                b_swap_greaterequalthan0.at(i) = m_arg2val->at(i);
                b_swap_lessthan0.at(i) = 0;
            } else {
                a_swap_greaterequalthan0.at(i) = 0;
                a_swap_lessthan0.at(i) = m_arg1val->at(i);
                b_swap_greaterequalthan0.at(i) = 0;
                b_swap_lessthan0.at(i) = m_arg2val->at(i);
            }
        }
        m_value.at(0) = std::inner_product(a_greaterequalthan0.begin(), a_greaterequalthan0.end(), b_greaterequalthan0.begin(), (double)0);
        m_value.at(0) = std::inner_product(a_swap_greaterequalthan0.begin(), a_swap_greaterequalthan0.end(), b_lessthan0.begin(), m_value.at(0));
        m_value.at(0) = std::inner_product(a_lessthan0.begin(), a_lessthan0.end(), b_swap_greaterequalthan0.begin(), m_value.at(0));
        m_value.at(0) = std::inner_product(a_swap_lessthan0.begin(), a_swap_lessthan0.end(), b_swap_lessthan0.begin(), m_value.at(0));
    };
  private:
    std::vector<double> addend1;
    std::vector<double> addend2;
    std::vector<double> addend3;
    std::vector<double> addend4;
    std::vector<double> a_greaterequalthan0;
    std::vector<double> a_lessthan0;
    std::vector<double> b_greaterequalthan0;
    std::vector<double> b_lessthan0;
    std::vector<double> a_swap_greaterequalthan0;
    std::vector<double> a_swap_lessthan0;
    std::vector<double> b_swap_greaterequalthan0;
    std::vector<double> b_swap_lessthan0;
};

class Mult_Mat_Vec: public Function
{
  public:
    Mult_Mat_Vec(MultiDimParameter *mat, Expression *a) : Function(mat, a, "dot_mat") {
        this->m_dim = mat->getValue()->getDim().at(0);
        this->m_value.resize(m_dim);

        this->m_vecval = a->getValue();
        this->m_matval = mat->getValue();
    };
    Mult_Mat_Vec(const Mult_Mat_Vec &old, const std::vector<Expression*>& arguments) : Function(old, arguments) {
        m_matval = dynamic_cast<MultiDimParameter*>(arguments.at(0))->getValue();
        m_vecval = arguments.at(1)->getValue();
    };
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override = 0;
  protected:
    std::vector<double>* m_vecval;
    Matrix* m_matval;
};
class Mult_MatNN_VecNN: public Mult_Mat_Vec
{
  public:
    Mult_MatNN_VecNN(MultiDimParameter *mat, Expression *a) : Mult_Mat_Vec(mat, a) {
        this->m_sign = NN;
    };
    Mult_MatNN_VecNN(const Mult_MatNN_VecNN &old, const std::vector<Expression*>& arguments) : Mult_Mat_Vec(old, arguments) {};
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(0)->evaluate(x_in, y_in);
        m_arguments.at(1)->evaluate(x_in, y_in);
        for (int row=0; row < m_dim; ++row) {
            m_value.at(row) = std::inner_product(m_matval->getRowIt(row), m_matval->getRowIt(row) + m_matval->getDim().at(1), m_vecval->begin(), (double)0);
        }
    };
};
class Mult_MatNP_VecNP: public Mult_Mat_Vec
{
  public:
    Mult_MatNP_VecNP(MultiDimParameter *mat, Expression *a) : Mult_Mat_Vec(mat, a) {
        this->m_sign = NN;
    };
    Mult_MatNP_VecNP(const Mult_MatNP_VecNP &old, const std::vector<Expression*>& arguments) : Mult_Mat_Vec(old, arguments) {};
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(0)->evaluate(y_in, x_in);
        m_arguments.at(1)->evaluate(y_in, x_in);
        for (int row=0; row < m_dim; ++row) {
            m_value.at(row) = std::inner_product(m_matval->getRowIt(row), m_matval->getRowIt(row) + m_matval->getDim().at(1), m_vecval->begin(), (double)0);
        }
    };
};
class Mult_MatNN_VecNP: public Mult_Mat_Vec
{
  public:
    Mult_MatNN_VecNP(MultiDimParameter *mat, Expression *a) : Mult_Mat_Vec(mat, a) {
        this->m_sign = NP;
    };
    Mult_MatNN_VecNP(const Mult_MatNN_VecNP &old, const std::vector<Expression*>& arguments) : Mult_Mat_Vec(old, arguments) {};
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(0)->evaluate(y_in, x_in);
        m_arguments.at(1)->evaluate(x_in, y_in);
        for (int row=0; row < m_dim; ++row) {
            m_value.at(row) = std::inner_product(m_matval->getRowIt(row), m_matval->getRowIt(row) + m_matval->getDim().at(1), m_vecval->begin(), (double)0);
        }
    };
};
class Mult_MatNP_VecNN: public Mult_Mat_Vec
{
  public:
    Mult_MatNP_VecNN(MultiDimParameter *mat, Expression *a) : Mult_Mat_Vec(mat, a) {
        this->m_sign = NP;
    };
    Mult_MatNP_VecNN(const Mult_MatNP_VecNN &old, const std::vector<Expression*>& arguments) : Mult_Mat_Vec(old, arguments) {};
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(0)->evaluate(x_in, y_in);
        m_arguments.at(1)->evaluate(y_in, x_in);
        for (int row=0; row < m_dim; ++row) {
            m_value.at(row) = std::inner_product(m_matval->getRowIt(row), m_matval->getRowIt(row) + m_matval->getDim().at(1), m_vecval->begin(), (double)0);
        }
    };
};
class Mult_MatU_VecNN: public Mult_Mat_Vec
{
  public:
    Mult_MatU_VecNN(MultiDimParameter *mat, Expression *a) : Mult_Mat_Vec(mat, a) {
        this->m_sign = unknown;
        a_greaterequalthan0 = Matrix(*mat->getValue());
        a_lessthan0 = Matrix(*mat->getValue());
    };
    Mult_MatU_VecNN(const Mult_MatU_VecNN &old, const std::vector<Expression*>& arguments) : Mult_Mat_Vec(old, arguments) {
        a_greaterequalthan0 = old.a_greaterequalthan0;
        a_lessthan0 = old.a_lessthan0;
    };
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(0)->evaluate(x_in, y_in);
        a_greaterequalthan0 = m_matval->getGreaterEqualThan0();
        a_lessthan0 = m_matval->getLessThan0();

        m_arguments.at(1)->evaluate(x_in, y_in);
        b = *m_vecval;
        m_arguments.at(1)->evaluate(y_in, x_in);
        for (int row=0; row < m_dim; ++row) {
            m_value.at(row) = std::inner_product(a_greaterequalthan0.getRowIt(row), a_greaterequalthan0.getRowIt(row) + m_matval->getDim().at(1), b.begin(), (double)0);
            m_value.at(row) = std::inner_product(a_lessthan0.getRowIt(row), a_lessthan0.getRowIt(row) + m_matval->getDim().at(1), m_vecval->begin(), m_value.at(row));
        }
    };
  private:
    Matrix a_greaterequalthan0;
    Matrix a_lessthan0;
    std::vector<double> b;
};
class Mult_MatU_VecNP: public Mult_Mat_Vec
{
  public:
    Mult_MatU_VecNP(MultiDimParameter *mat, Expression *a) : Mult_Mat_Vec(mat, a) {
        this->m_sign = unknown;
        a_greaterequalthan0 = Matrix(*mat->getValue());
        a_lessthan0 = Matrix(*mat->getValue());
    };
    Mult_MatU_VecNP(const Mult_MatU_VecNP &old, const std::vector<Expression*>& arguments) : Mult_Mat_Vec(old, arguments) {
        a_greaterequalthan0 = old.a_greaterequalthan0;
        a_lessthan0 = old.a_lessthan0;
    };
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(0)->evaluate(y_in, x_in);
        a_greaterequalthan0 = m_matval->getGreaterEqualThan0();
        a_lessthan0 = m_matval->getLessThan0();

        m_arguments.at(1)->evaluate(x_in, y_in);
        b = *m_vecval;
        m_arguments.at(1)->evaluate(y_in, x_in);
        for (int row=0; row < m_dim; ++row) {
            m_value.at(row) = std::inner_product(a_greaterequalthan0.getRowIt(row), a_greaterequalthan0.getRowIt(row) + m_matval->getDim().at(1), b.begin(), (double)0);
            m_value.at(row) = std::inner_product(a_lessthan0.getRowIt(row), a_lessthan0.getRowIt(row) + m_matval->getDim().at(1), m_vecval->begin(), m_value.at(row));
        }
    };
  private:
    Matrix a_greaterequalthan0;
    Matrix a_lessthan0;
    std::vector<double> b;
};
class Mult_MatNN_VecU: public Mult_Mat_Vec
{
  public:
    Mult_MatNN_VecU(MultiDimParameter *mat, Expression *a) : Mult_Mat_Vec(mat, a) {
        this->m_sign = unknown;
        b_greaterequalthan0.resize(m_vecval->size());
        b_lessthan0.resize(m_vecval->size());
    };
    Mult_MatNN_VecU(const Mult_MatNN_VecU &old, const std::vector<Expression*>& arguments) : Mult_Mat_Vec(old, arguments) {
        b_greaterequalthan0 = old.b_greaterequalthan0;
        b_lessthan0 = old.b_lessthan0;
    };
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(0)->evaluate(x_in, y_in);
        a = *m_matval;
        m_arguments.at(0)->evaluate(y_in, x_in);

        m_arguments.at(1)->evaluate(x_in, y_in);
        for (size_t i=0;i<m_dim;++i) {
            if (m_vecval->at(i) >= 0) {
                b_greaterequalthan0.at(i) = m_vecval->at(i);
                b_lessthan0.at(i) = 0;
            } else {
                b_greaterequalthan0.at(i) = 0;
                b_lessthan0.at(i) = m_vecval->at(i);
            }
        }
        for (int row=0; row < m_dim; ++row) {
            m_value.at(row) = std::inner_product(a.getRowIt(row), a.getRowIt(row) + m_matval->getDim().at(1), b_greaterequalthan0.begin(), (double)0);
            m_value.at(row) = std::inner_product(m_matval->getRowIt(row), m_matval->getRowIt(row) + m_matval->getDim().at(1), b_lessthan0.begin(), m_value.at(row));
        }
    };
  private:
    Matrix a;
    std::vector<double> b_greaterequalthan0;
    std::vector<double> b_lessthan0;
};
class Mult_MatNP_VecU: public Mult_Mat_Vec
{
  public:
    Mult_MatNP_VecU(MultiDimParameter *mat, Expression *a) : Mult_Mat_Vec(mat, a) {
        this->m_sign = unknown;
        b_greaterequalthan0.resize(m_vecval->size());
        b_lessthan0.resize(m_vecval->size());
    };
    Mult_MatNP_VecU(const Mult_MatNP_VecU &old, const std::vector<Expression*>& arguments) : Mult_Mat_Vec(old, arguments) {
        b_greaterequalthan0 = old.b_greaterequalthan0;
        b_lessthan0 = old.b_lessthan0;
    };
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(0)->evaluate(x_in, y_in);
        a = *m_matval;
        m_arguments.at(0)->evaluate(y_in, x_in);

        m_arguments.at(1)->evaluate(y_in, x_in);
        for (size_t i=0;i<m_dim;++i) {
            if (m_vecval->at(i) >= 0) {
                b_greaterequalthan0.at(i) = m_vecval->at(i);
                b_lessthan0.at(i) = 0;
            } else {
                b_greaterequalthan0.at(i) = 0;
                b_lessthan0.at(i) = m_vecval->at(i);
            }
        }
        for (int row=0; row < m_dim; ++row) {
            m_value.at(row) = std::inner_product(a.getRowIt(row), a.getRowIt(row) + m_matval->getDim().at(1), b_greaterequalthan0.begin(), (double)0);
            m_value.at(row) = std::inner_product(m_matval->getRowIt(row), m_matval->getRowIt(row) + m_matval->getDim().at(1), b_lessthan0.begin(), m_value.at(row));
        }
    };
  private:
    Matrix a;
    std::vector<double> b_greaterequalthan0;
    std::vector<double> b_lessthan0;
};
class Mult_MatU_VecU: public Mult_Mat_Vec
{
  public:
    Mult_MatU_VecU(MultiDimParameter *mat, Expression *a) : Mult_Mat_Vec(mat, a) {
        this->m_sign = unknown;
        a_greaterequalthan0 = Matrix(*mat->getValue());
        a_lessthan0 = Matrix(*mat->getValue());
        a_swap_greaterequalthan0 = Matrix(*mat->getValue());
        a_swap_lessthan0 = Matrix(*mat->getValue());
        b_greaterequalthan0.resize(m_vecval->size());
        b_lessthan0.resize(m_vecval->size());
        b_swap_greaterequalthan0.resize(m_vecval->size());
        b_swap_lessthan0.resize(m_vecval->size());
    };
    Mult_MatU_VecU(const Mult_MatU_VecU &old, const std::vector<Expression*>& arguments) : Mult_Mat_Vec(old, arguments) {
        a_greaterequalthan0 = old.a_greaterequalthan0;
        a_lessthan0 = old.a_lessthan0;
        a_swap_greaterequalthan0 = old.a_swap_greaterequalthan0;
        a_swap_lessthan0 = old.a_swap_lessthan0;
        b_greaterequalthan0 = old.b_greaterequalthan0;
        b_lessthan0 = old.b_lessthan0;
        b_swap_greaterequalthan0 = old.b_swap_greaterequalthan0;
        b_swap_lessthan0 = old.b_swap_lessthan0;
    };
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(0)->evaluate(x_in, y_in);
        a_greaterequalthan0 = m_matval->getGreaterEqualThan0();
        a_lessthan0 = m_matval->getLessThan0();
        m_arguments.at(1)->evaluate(x_in, y_in);
        for (size_t i=0;i<m_dim;++i) {
            if (m_vecval->at(i) >= 0) {
                b_greaterequalthan0.at(i) = m_vecval->at(i);
                b_lessthan0.at(i) = 0;
            } else {
                b_greaterequalthan0.at(i) = 0;
                b_lessthan0.at(i) = m_vecval->at(i);
            }
        }
        m_arguments.at(0)->evaluate(y_in, x_in);
        a_swap_greaterequalthan0 = m_matval->getGreaterEqualThan0();
        a_swap_lessthan0 = m_matval->getLessThan0();
        m_arguments.at(1)->evaluate(y_in, x_in);
        for (size_t i=0;i<m_dim;++i) {
            if (m_vecval->at(i) >= 0) {
                b_swap_greaterequalthan0.at(i) = m_vecval->at(i);
                b_swap_lessthan0.at(i) = 0;
            } else {
                b_swap_greaterequalthan0.at(i) = 0;
                b_swap_lessthan0.at(i) = m_vecval->at(i);
            }
        }
        for (int row=0; row < m_dim; ++row) {
            m_value.at(row) = std::inner_product(a_greaterequalthan0.getRowIt(row), a_greaterequalthan0.getRowIt(row) + m_matval->getDim().at(1), b_greaterequalthan0.begin(), (double)0);
            m_value.at(row) = std::inner_product(a_swap_greaterequalthan0.getRowIt(row), a_swap_greaterequalthan0.getRowIt(row) + m_matval->getDim().at(1), b_lessthan0.begin(), m_value.at(row));
            m_value.at(row) = std::inner_product(a_lessthan0.getRowIt(row), a_lessthan0.getRowIt(row) + m_matval->getDim().at(1), b_swap_greaterequalthan0.begin(), m_value.at(row));
            m_value.at(row) = std::inner_product(a_swap_lessthan0.getRowIt(row), a_swap_lessthan0.getRowIt(row) + m_matval->getDim().at(1), b_swap_lessthan0.begin(), m_value.at(row));
        }
    };
  private:
    Matrix a_greaterequalthan0;
    Matrix a_lessthan0;
    Matrix a_swap_greaterequalthan0;
    Matrix a_swap_lessthan0;
    std::vector<double> b_greaterequalthan0;
    std::vector<double> b_lessthan0;
    std::vector<double> b_swap_greaterequalthan0;
    std::vector<double> b_swap_lessthan0;
};

class Inv: public Function
{
  public:
    explicit Inv(Expression *a) : Function(a, "inverse") {
        this->m_argval = a->getValue();

        this->m_dim = a->getDim();
        this->m_value.resize(m_dim);

        if (a->getSign() == NN) {
            this->m_sign = NN;
        } else if (a->getSign() == NP) {
            this->m_sign = NP;
        } else {
            this->m_sign = unknown;
        }

    };
    Inv(const Inv &old, const std::vector<Expression*>& arguments) : Function(old, arguments) {
        m_argval = arguments.at(0)->getValue();
    };
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(0)->evaluate(y_in, x_in);
        std::transform(m_argval->begin(), m_argval->end(), m_value.begin(), [](const double & c){return std::log2(c);});
    };
  private:
    std::vector<double>* m_argval;
};

class Log2: public Function
{
  public:
    explicit Log2(Expression *a) : Function(a, "log2") {
        this->m_argval = a->getValue();

        this->m_dim = a->getDim();
        this->m_value.resize(m_dim);
        this->m_sign = unknown;
    };
    Log2(const Log2 &old, const std::vector<Expression*>& arguments) : Function(old, arguments) {
        m_argval = arguments.at(0)->getValue();
    };
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(0)->evaluate(x_in, y_in);
        std::transform(m_argval->begin(), m_argval->end(), m_value.begin(), [](const double & c){return std::log2(c);});
    };
  private:
    std::vector<double>* m_argval;
};

class LogN: public Function
{
  public:
    explicit LogN(Expression *a) : Function(a, "ln") {
        this->m_argval = a->getValue();

        this->m_dim = a->getDim();
        this->m_value.resize(m_dim);
        this->m_sign = unknown;
    };
    LogN(const LogN &old, const std::vector<Expression*>& arguments) : Function(old, arguments) {
        m_argval = arguments.at(0)->getValue();
    };
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(0)->evaluate(x_in, y_in);
        std::transform(m_argval->begin(), m_argval->end(), m_value.begin(), [](const double & c){return std::log(c);});
    };
  private:
    std::vector<double>* m_argval;
};

class Neg: public Function
{
  public:
    explicit Neg(Expression *a) : Function(a, "negation") {
        this->m_argval = a->getValue();

        this->m_dim = a->getDim();
        this->m_value.resize(m_dim);

        if (a->getSign() == NN) {
            this->m_sign = NP;
        } else if (a->getSign() == NP) {
            this->m_sign = NN;
        } else {
            this->m_sign = unknown;
        }
    };
    Neg(const Neg &old, const std::vector<Expression*>& arguments) : Function(old, arguments) {
        m_argval = arguments.at(0)->getValue();
    };
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        // In a negated Expression, the nonincreasing and nondecreasing variables change!
        m_arguments.at(0)->evaluate(y_in, x_in);
        std::transform(m_argval->begin(), m_argval->end(), m_value.begin(), [](const double & c){return -c;});
    };
  private:
    std::vector<double>* m_argval;
};


class Prod: public Function
{
  public:
    explicit Prod(Expression *a) : Function(a, "product") {
        this->m_argval = a->getValue();

        this->m_dim = 1;
        this->m_value.resize(m_dim);
        this->m_sign = NN;
    };
    Prod(const Prod &old, const std::vector<Expression*>& arguments) : Function(old, arguments) {
        m_argval = arguments.at(0)->getValue();
    };
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(0)->evaluate(x_in, y_in);
        m_value.at(0) = std::accumulate(m_argval->begin(), m_argval->end(), static_cast<double>(1), std::multiplies<>());
    };
  private:
    std::vector<double>* m_argval;
};


class Sum: public Function
{
  public:
    explicit Sum(Expression *a) : Function(a, "sum") {
        this->m_argval = a->getValue();

        this->m_dim = 1;
        this->m_value.resize(m_dim);

        this->m_sign = a->getSign();
    };
    Sum(const Sum &old, const std::vector<Expression*>& arguments) : Function(old, arguments) {
        m_argval = arguments.at(0)->getValue();
    };
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(0)->evaluate(x_in, y_in);
        m_value.at(0) = std::accumulate(m_argval->begin(), m_argval->end(), static_cast<double>(0), std::plus<>());
    };
  private:
    std::vector<double>* m_argval;
};

class List : public Function
{
  public:
    explicit List(std::vector<Expression*> a) : Function(a, "list_of_expressions") {
        for (auto arg : a) {
            m_valuepointer.push_back(arg->getValue());
        }

        this->m_dim = a.size();
        this->m_value.resize(m_dim);

        int nncount = 0;
        int npcount = 0;
        for (int i=0; i<m_dim; ++i) {
            if (m_arguments.at(i)->getSign() == NN) {
                nncount++;
            } else if (m_arguments.at(i)->getSign() == NP) {
                npcount++;
            }
            if ((npcount > 0 && nncount > 0) || m_arguments.at(i)->getSign() == unknown) {
                this->m_sign = unknown;
                break;
            }
        }
    };
    List(const List &old, const std::vector<Expression*>& arguments) : Function(old, arguments) {
        for (auto arg : arguments) {
            m_valuepointer.push_back(arg->getValue());
        }
    };
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        for (int i=0; i<m_dim; ++i) {
            m_arguments.at(i)->evaluate(x_in, y_in);
            m_value.at(i) = m_valuepointer.at(i)->at(0);
        }
    };
  private:
     std::vector<std::vector<double>*> m_valuepointer;
};

class Sliced : public Function
{
  public:
    explicit Sliced(Expression *a, int index) : Function(a, "sliced_expressions") {
        this->m_index = index;
        this->m_valuepointer = &a->getValue()->at(index);

        this->m_dim = 1;
        this->m_value.resize(m_dim);

        this->m_sign = a->getSign();
    };
    Sliced(const Sliced &old, const std::vector<Expression*>& arguments) : Function(old, arguments) {
            this->m_index = old.m_index;
            this->m_valuepointer = &arguments.at(0)->getValue()->at(m_index);
    };
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_arguments.at(0)->evaluate(x_in, y_in);
        m_value.at(0) = *m_valuepointer;
    };
  private:
    int m_index;
    double* m_valuepointer;
};

/*  Down below are Methods for creating a contiguous storage container for all Functions in an objective
 * In order to do so, a std::variant container is utilized. Therefore, ALL concrete expression classes
 * are potential elements.
 *  ----------------------------------------------------------------------------------------------------
 */

typedef std::variant<
        Add_vec_scalar,
        Add_vec_vec,

        Mult_vecNN_scalarNN,
        Mult_vecNP_scalarNP,
        Mult_vecNN_scalarNP,
        Mult_vecNP_scalarNN,
        Mult_vecU_scalarNN,
        Mult_vecU_scalarNP,
        Mult_vecNN_scalarU,
        Mult_vecNP_scalarU,
        Mult_vecU_scalarU,

        Mult_MatNN_VecNN,
        Mult_MatNP_VecNP,
        Mult_MatNN_VecNP,
        Mult_MatNP_VecNN,
        Mult_MatU_VecNN,
        Mult_MatU_VecNP,
        Mult_MatNN_VecU,
        Mult_MatNP_VecU,
        Mult_MatU_VecU,

        Mult_vecNN_vecNN,
        Mult_vecNP_vecNP,
        Mult_vecNN_vecNP,
        Mult_vecU_vecNN,
        Mult_vecU_vecNP,
        Mult_vecU_vecU,

        Dot_NN_NN,
        Dot_NP_NP,
        Dot_NN_NP,
        Dot_U_NN,
        Dot_U_NP,
        Dot_U_U,

        Div_VecNN_VecNN,
        Div_VecNP_VecNP,
        Div_VecNN_VecNP,
        Div_VecNP_VecNN,
        Div_VecU_VecNN,
        Div_VecU_VecNP,
        Div_VecNN_VecU,
        Div_VecNP_VecU,
        Div_VecU_VecU,

        Div_VecNN_ScalarNN,
        Div_VecNP_ScalarNP,
        Div_VecNN_ScalarNP,
        Div_VecNP_ScalarNN,
        Div_VecU_ScalarNN,
        Div_VecU_ScalarNP,
        Div_VecNN_ScalarU,
        Div_VecNP_ScalarU,
        Div_VecU_ScalarU,

        Div_ScalarNN_VecNN,
        Div_ScalarNP_VecNP,
        Div_ScalarNN_VecNP,
        Div_ScalarNP_VecNN,
        Div_ScalarU_VecNN,
        Div_ScalarU_VecNP,
        Div_ScalarNN_VecU,
        Div_ScalarNP_VecU,
        Div_ScalarU_VecU,

        Log2, LogN,
        Inv,
        Neg,
        Prod, Sum,
        List, Sliced,

        Parameter, MultiDimParameter, Variable> variant_expression;

Expression* getBasePointer(variant_expression& variant) {
    if (variant.index() == 0) {return &std::get<Add_vec_scalar>(variant);}
    else if (variant.index() == 1) {return &std::get<Add_vec_vec>(variant);}

    else if (variant.index() == 2) {return &std::get<Mult_vecNN_scalarNN>(variant);}
    else if (variant.index() == 3) {return &std::get<Mult_vecNP_scalarNP>(variant);}
    else if (variant.index() == 4) {return &std::get<Mult_vecNN_scalarNP>(variant);}
    else if (variant.index() == 5) {return &std::get<Mult_vecNP_scalarNN>(variant);}
    else if (variant.index() == 6) {return &std::get<Mult_vecU_scalarNN>(variant);}
    else if (variant.index() == 7) {return &std::get<Mult_vecU_scalarNP>(variant);}
    else if (variant.index() == 8) {return &std::get<Mult_vecNN_scalarU>(variant);}
    else if (variant.index() == 9) {return &std::get<Mult_vecNP_scalarU>(variant);}
    else if (variant.index() == 10) {return &std::get<Mult_vecU_scalarU>(variant);}

    else if (variant.index() == 11) {return &std::get<Mult_MatNN_VecNN>(variant);}
    else if (variant.index() == 12) {return &std::get<Mult_MatNP_VecNP>(variant);}
    else if (variant.index() == 13) {return &std::get<Mult_MatNN_VecNP>(variant);}
    else if (variant.index() == 14) {return &std::get<Mult_MatNP_VecNN>(variant);}
    else if (variant.index() == 15) {return &std::get<Mult_MatU_VecNN>(variant);}
    else if (variant.index() == 16) {return &std::get<Mult_MatU_VecNP>(variant);}
    else if (variant.index() == 17) {return &std::get<Mult_MatNN_VecU>(variant);}
    else if (variant.index() == 18) {return &std::get<Mult_MatNP_VecU>(variant);}
    else if (variant.index() == 19) {return &std::get<Mult_MatU_VecU>(variant);}

    else if (variant.index() == 20) {return &std::get<Mult_vecNN_vecNN>(variant);}
    else if (variant.index() == 21) {return &std::get<Mult_vecNP_vecNP>(variant);}
    else if (variant.index() == 22) {return &std::get<Mult_vecNN_vecNP>(variant);}
    else if (variant.index() == 23) {return &std::get<Mult_vecU_vecNN>(variant);}
    else if (variant.index() == 24) {return &std::get<Mult_vecU_vecNP>(variant);}
    else if (variant.index() == 25) {return &std::get<Mult_vecU_vecU>(variant);}

    else if (variant.index() == 26) {return &std::get<Dot_NN_NN>(variant);}
    else if (variant.index() == 27) {return &std::get<Dot_NP_NP>(variant);}
    else if (variant.index() == 28) {return &std::get<Dot_NN_NP>(variant);}
    else if (variant.index() == 29) {return &std::get<Dot_U_NN>(variant);}
    else if (variant.index() == 30) {return &std::get<Dot_U_NP>(variant);}
    else if (variant.index() == 31) {return &std::get<Dot_U_U>(variant);}

    else if (variant.index() == 32) {return &std::get<Div_VecNN_VecNN>(variant);}
    else if (variant.index() == 33) {return &std::get<Div_VecNP_VecNP>(variant);}
    else if (variant.index() == 34) {return &std::get<Div_VecNN_VecNP>(variant);}
    else if (variant.index() == 35) {return &std::get<Div_VecNP_VecNN>(variant);}
    else if (variant.index() == 36) {return &std::get<Div_VecU_VecNN>(variant);}
    else if (variant.index() == 37) {return &std::get<Div_VecU_VecNP>(variant);}
    else if (variant.index() == 38) {return &std::get<Div_VecNN_VecU>(variant);}
    else if (variant.index() == 39) {return &std::get<Div_VecNP_VecU>(variant);}
    else if (variant.index() == 40) {return &std::get<Div_VecU_VecU>(variant);}

    else if (variant.index() == 41) {return &std::get<Div_VecNN_ScalarNN>(variant);}
    else if (variant.index() == 42) {return &std::get<Div_VecNP_ScalarNP>(variant);}
    else if (variant.index() == 43) {return &std::get<Div_VecNN_ScalarNP>(variant);}
    else if (variant.index() == 44) {return &std::get<Div_VecNP_ScalarNN>(variant);}
    else if (variant.index() == 45) {return &std::get<Div_VecU_ScalarNN>(variant);}
    else if (variant.index() == 46) {return &std::get<Div_VecU_ScalarNP>(variant);}
    else if (variant.index() == 47) {return &std::get<Div_VecNN_ScalarU>(variant);}
    else if (variant.index() == 48) {return &std::get<Div_VecNP_ScalarU>(variant);}
    else if (variant.index() == 49) {return &std::get<Div_VecU_ScalarU>(variant);}

    else if (variant.index() == 50) {return &std::get<Div_ScalarNN_VecNN>(variant);}
    else if (variant.index() == 51) {return &std::get<Div_ScalarNP_VecNP>(variant);}
    else if (variant.index() == 52) {return &std::get<Div_ScalarNN_VecNP>(variant);}
    else if (variant.index() == 53) {return &std::get<Div_ScalarNP_VecNN>(variant);}
    else if (variant.index() == 54) {return &std::get<Div_ScalarU_VecNN>(variant);}
    else if (variant.index() == 55) {return &std::get<Div_ScalarU_VecNP>(variant);}
    else if (variant.index() == 56) {return &std::get<Div_ScalarNN_VecU>(variant);}
    else if (variant.index() == 57) {return &std::get<Div_ScalarNP_VecU>(variant);}
    else if (variant.index() == 58) {return &std::get<Div_ScalarU_VecU>(variant);}

    else if (variant.index() == 59) {return &std::get<Log2>(variant);}
    else if (variant.index() == 60) {return &std::get<LogN>(variant);}
    else if (variant.index() == 61) {return &std::get<Inv>(variant);}
    else if (variant.index() == 62) {return &std::get<Neg>(variant);}
    else if (variant.index() == 63) {return &std::get<Prod>(variant);}
    else if (variant.index() == 64) {return &std::get<Sum>(variant);}
    else if (variant.index() == 65) {return &std::get<List>(variant);}
    else if (variant.index() == 66) {return &std::get<Sliced>(variant);}

    else if (variant.index() == 67) {return &std::get<Parameter>(variant);}
    else if (variant.index() == 68) {return &std::get<MultiDimParameter>(variant);}
    else if (variant.index() == 69) {return &std::get<Variable>(variant);}
    return nullptr;
}

int findElementGetIndex(const std::vector<variant_expression>& vec_of_different_types, int id_to_find) {
    size_t length = vec_of_different_types.size();
    for (size_t i=0; i<length; ++i) {
        if (std::visit([](auto&& v) {
            return v.getID();
        }, vec_of_different_types.at(i)) == id_to_find) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

std::vector<variant_expression> get_variant_vector() {

    // The return vector
    std::vector<variant_expression> contiguousStorageContainer;

    // Reserve Space, so pointers do not misbehave
    contiguousStorageContainer.reserve(Expression::getExpressionList().size());

    // Für JEDE Expression des gegebenen Objectives
    for (auto expr: Expression::getExpressionList()) {

        // 1. Falls Parameter:
        if (auto param = dynamic_cast<Parameter *>(expr)) {
            // Füge Kopie zur zusammenhängenden Container hinzu
            contiguousStorageContainer.push_back(Parameter{*param});
        }
        // 2. Falls MultiDimParameter:
        if (auto param = dynamic_cast<MultiDimParameter *>(expr)) {
            // Füge Kopie zur zusammenhängenden Container hinzu
            contiguousStorageContainer.push_back(MultiDimParameter{*param});
        }
        // 3. Falls Variable:
        if (auto var = dynamic_cast<Variable *>(expr)) {
            // Füge Kopie zur zusammenhängenden Container hinzu
            contiguousStorageContainer.push_back(Variable{*var});
        }

        // 4. Falls Funktion:
        if (auto func = dynamic_cast<Function *>(expr)) {
            // Erstelle temporären argument-Vektor:
            std::vector<Expression *> tmp;

            // für JEDES Argument:
            for (auto arg: func->getArgs()) {
                // Suche in dem neuen variant_container nach der passenden Expression
                int idToFind = arg->getID();
                int vectorIndexOfArgument = findElementGetIndex(contiguousStorageContainer, idToFind);
                Expression *parsedBasePointerCorrect = getBasePointer(
                        contiguousStorageContainer.at(vectorIndexOfArgument));

                // Und speichere diesen gefunden Pointer/Adresse zu dem Argument.
                tmp.push_back(parsedBasePointerCorrect);
            }

            // Schließlich kopiere diese Funktion mit den gefunden Argumenten --> Hier Fallunterscheidung, je nach Funktionstyp (wg. unt. copy-Konstruktor)
            if (auto old = dynamic_cast<Add_vec_scalar *>(func)) {
                Add_vec_scalar copy_of_old = Add_vec_scalar{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Add_vec_vec *>(func)) {
                Add_vec_vec copy_of_old = Add_vec_vec{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            }

            else if (auto old = dynamic_cast<Mult_vecNN_scalarNN *>(func)) {
                Mult_vecNN_scalarNN copy_of_old = Mult_vecNN_scalarNN{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Mult_vecNP_scalarNP *>(func)) {
                Mult_vecNP_scalarNP copy_of_old = Mult_vecNP_scalarNP{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Mult_vecNN_scalarNP *>(func)) {
                Mult_vecNN_scalarNP copy_of_old = Mult_vecNN_scalarNP{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Mult_vecNP_scalarNN *>(func)) {
                Mult_vecNP_scalarNN copy_of_old = Mult_vecNP_scalarNN{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Mult_vecU_scalarNN *>(func)) {
                Mult_vecU_scalarNN copy_of_old = Mult_vecU_scalarNN{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Mult_vecNN_scalarU *>(func)) {
                Mult_vecNN_scalarU copy_of_old = Mult_vecNN_scalarU{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Mult_vecU_scalarNP *>(func)) {
                Mult_vecU_scalarNP copy_of_old = Mult_vecU_scalarNP{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Mult_vecNP_scalarU *>(func)) {
                Mult_vecNP_scalarU copy_of_old = Mult_vecNP_scalarU{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Mult_vecU_scalarU *>(func)) {
                Mult_vecU_scalarU copy_of_old = Mult_vecU_scalarU{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            }

            else if (auto old = dynamic_cast<Mult_vecNN_vecNN *>(func)) {
                Mult_vecNN_vecNN copy_of_old = Mult_vecNN_vecNN{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Mult_vecNP_vecNP *>(func)) {
                Mult_vecNP_vecNP copy_of_old = Mult_vecNP_vecNP{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Mult_vecNN_vecNP *>(func)) {
                Mult_vecNN_vecNP copy_of_old = Mult_vecNN_vecNP{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Mult_vecU_vecNN *>(func)) {
                Mult_vecU_vecNN copy_of_old = Mult_vecU_vecNN{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Mult_vecU_vecNP *>(func)) {
                Mult_vecU_vecNP copy_of_old = Mult_vecU_vecNP{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Mult_vecU_vecU *>(func)) {
                Mult_vecU_vecU copy_of_old = Mult_vecU_vecU{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            }

            else if (auto old = dynamic_cast<Mult_MatNN_VecNN *>(func)) {
                Mult_MatNN_VecNN copy_of_old = Mult_MatNN_VecNN{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Mult_MatNP_VecNP *>(func)) {
                Mult_MatNP_VecNP copy_of_old = Mult_MatNP_VecNP{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Mult_MatNN_VecNP *>(func)) {
                Mult_MatNN_VecNP copy_of_old = Mult_MatNN_VecNP{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Mult_MatNP_VecNN *>(func)) {
                Mult_MatNP_VecNN copy_of_old = Mult_MatNP_VecNN{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Mult_MatU_VecNN *>(func)) {
                Mult_MatU_VecNN copy_of_old = Mult_MatU_VecNN{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Mult_MatU_VecNP *>(func)) {
                Mult_MatU_VecNP copy_of_old = Mult_MatU_VecNP{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Mult_MatNN_VecU *>(func)) {
                Mult_MatNN_VecU copy_of_old = Mult_MatNN_VecU{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Mult_MatNP_VecU *>(func)) {
                Mult_MatNP_VecU copy_of_old = Mult_MatNP_VecU{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Mult_MatU_VecU *>(func)) {
                Mult_MatU_VecU copy_of_old = Mult_MatU_VecU{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            }

            else if (auto old = dynamic_cast<Dot_NN_NN *>(func)) {
                Dot_NN_NN copy_of_old = Dot_NN_NN{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Dot_NP_NP *>(func)) {
                Dot_NP_NP copy_of_old = Dot_NP_NP{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Dot_NN_NP *>(func)) {
                Dot_NN_NP copy_of_old = Dot_NN_NP{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Dot_U_NN *>(func)) {
                Dot_U_NN copy_of_old = Dot_U_NN{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Dot_U_NP *>(func)) {
                Dot_U_NP copy_of_old = Dot_U_NP{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Dot_U_U *>(func)) {
                Dot_U_U copy_of_old = Dot_U_U{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            }

            else if (auto old = dynamic_cast<Div_VecNN_VecNN *>(func)) {
                Div_VecNN_VecNN copy_of_old = Div_VecNN_VecNN{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Div_VecNP_VecNP *>(func)) {
                Div_VecNP_VecNP copy_of_old = Div_VecNP_VecNP{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Div_VecNN_VecNP *>(func)) {
                Div_VecNN_VecNP copy_of_old = Div_VecNN_VecNP{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Div_VecNP_VecNN *>(func)) {
                Div_VecNP_VecNN copy_of_old = Div_VecNP_VecNN{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Div_VecU_VecNN *>(func)) {
                Div_VecU_VecNN copy_of_old = Div_VecU_VecNN{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Div_VecU_VecNP *>(func)) {
                Div_VecU_VecNP copy_of_old = Div_VecU_VecNP{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Div_VecNN_VecU *>(func)) {
                Div_VecNN_VecU copy_of_old = Div_VecNN_VecU{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Div_VecNP_VecU *>(func)) {
                Div_VecNP_VecU copy_of_old = Div_VecNP_VecU{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Div_VecU_VecU *>(func)) {
                Div_VecU_VecU copy_of_old = Div_VecU_VecU{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            }

            else if (auto old = dynamic_cast<Div_VecNN_ScalarNN *>(func)) {
                Div_VecNN_ScalarNN copy_of_old = Div_VecNN_ScalarNN{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Div_VecNP_ScalarNP *>(func)) {
                Div_VecNP_ScalarNP copy_of_old = Div_VecNP_ScalarNP{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Div_VecNN_ScalarNP *>(func)) {
                Div_VecNN_ScalarNP copy_of_old = Div_VecNN_ScalarNP{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Div_VecNP_ScalarNN *>(func)) {
                Div_VecNP_ScalarNN copy_of_old = Div_VecNP_ScalarNN{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Div_VecU_ScalarNN *>(func)) {
                Div_VecU_ScalarNN copy_of_old = Div_VecU_ScalarNN{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Div_VecU_ScalarNP *>(func)) {
                Div_VecU_ScalarNP copy_of_old = Div_VecU_ScalarNP{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Div_VecNN_ScalarU *>(func)) {
                Div_VecNN_ScalarU copy_of_old = Div_VecNN_ScalarU{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Div_VecNP_ScalarU *>(func)) {
                Div_VecNP_ScalarU copy_of_old = Div_VecNP_ScalarU{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Div_VecU_ScalarU *>(func)) {
                Div_VecU_ScalarU copy_of_old = Div_VecU_ScalarU{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            }

            else if (auto old = dynamic_cast<Div_ScalarNN_VecNN *>(func)) {
                Div_ScalarNN_VecNN copy_of_old = Div_ScalarNN_VecNN{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Div_ScalarNP_VecNP *>(func)) {
                Div_ScalarNP_VecNP copy_of_old = Div_ScalarNP_VecNP{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Div_ScalarNN_VecNP *>(func)) {
                Div_ScalarNN_VecNP copy_of_old = Div_ScalarNN_VecNP{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Div_ScalarNP_VecNN *>(func)) {
                Div_ScalarNP_VecNN copy_of_old = Div_ScalarNP_VecNN{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Div_ScalarU_VecNN *>(func)) {
                Div_ScalarU_VecNN copy_of_old = Div_ScalarU_VecNN{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Div_ScalarU_VecNP *>(func)) {
                Div_ScalarU_VecNP copy_of_old = Div_ScalarU_VecNP{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Div_ScalarNN_VecU *>(func)) {
                Div_ScalarNN_VecU copy_of_old = Div_ScalarNN_VecU{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Div_ScalarNP_VecU *>(func)) {
                Div_ScalarNP_VecU copy_of_old = Div_ScalarNP_VecU{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Div_ScalarU_VecU *>(func)) {
                Div_ScalarU_VecU copy_of_old = Div_ScalarU_VecU{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            }

            else if (auto old = dynamic_cast<Log2 *>(func)) {
                Log2 copy_of_old = Log2{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<LogN *>(func)) {
                LogN copy_of_old = LogN{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Inv *>(func)) {
                Inv copy_of_old = Inv{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Neg *>(func)) {
                Neg copy_of_old = Neg{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Prod *>(func)) {
                Prod copy_of_old = Prod{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Sum *>(func)) {
                Sum copy_of_old = Sum{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<List *>(func)) {
                List copy_of_old = List{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Sliced *>(func)) {
                Sliced copy_of_old = Sliced{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            }
        }
    }
    return contiguousStorageContainer;
}


std::vector<variant_expression> get_variant_vector(Expression* objective) {

    // Get a vector with all expressions from the objective
    std::vector<Expression*> list_of_expressions_in_objective = objective->get_expression_list();

    // The return vector
    std::vector<variant_expression> contiguousStorageContainer;

    // Reserve Space, so pointers do not misbehave
    contiguousStorageContainer.reserve(list_of_expressions_in_objective.size());

    // Für JEDE Expression des gegebenen Objectives
    for (auto expr : list_of_expressions_in_objective) {

        // 1. Falls Parameter:
        if (auto param = dynamic_cast<Parameter*>(expr)) {
            // Füge Kopie zur zusammenhängenden Container hinzu
            contiguousStorageContainer.push_back(Parameter{*param});
        }
        // 2. Falls MultiDimParameter:
        if (auto param = dynamic_cast<MultiDimParameter*>(expr)) {
            // Füge Kopie zur zusammenhängenden Container hinzu
            contiguousStorageContainer.push_back(MultiDimParameter{*param});
        }
        // 3. Falls Variable:
        if (auto var = dynamic_cast<Variable*>(expr)) {
            // Füge Kopie zur zusammenhängenden Container hinzu
            contiguousStorageContainer.push_back(Variable{*var});
        }

        // 4. Falls Funktion:
        if (auto func = dynamic_cast<Function*>(expr)) {
            // Erstelle temporären argument-Vektor:
            std::vector<Expression*> tmp;

            // für JEDES Argument:
            for (auto arg : func->getArgs()) {
                // Suche in dem neuen variant_container nach der passenden Expression
                int idToFind = arg->getID();
                int vectorIndexOfArgument = findElementGetIndex(contiguousStorageContainer, idToFind);
                Expression* parsedBasePointerCorrect = getBasePointer(contiguousStorageContainer.at(vectorIndexOfArgument));

                // Und speichere diesen gefunden Pointer/Adresse zu dem Argument.
                tmp.push_back(parsedBasePointerCorrect);
            }

            // Schließlich kopiere diese Funktion mit den gefunden Argumenten --> Hier Fallunterscheidung, je nach Funktionstyp (wg. unt. copy-Konstruktor)
            if (auto old = dynamic_cast<Add_vec_scalar *>(func)) {
                Add_vec_scalar copy_of_old = Add_vec_scalar{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Add_vec_vec *>(func)) {
                Add_vec_vec copy_of_old = Add_vec_vec{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            }

            else if (auto old = dynamic_cast<Mult_vecNN_scalarNN *>(func)) {
                Mult_vecNN_scalarNN copy_of_old = Mult_vecNN_scalarNN{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Mult_vecNP_scalarNP *>(func)) {
                Mult_vecNP_scalarNP copy_of_old = Mult_vecNP_scalarNP{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Mult_vecNN_scalarNP *>(func)) {
                Mult_vecNN_scalarNP copy_of_old = Mult_vecNN_scalarNP{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Mult_vecNP_scalarNN *>(func)) {
                Mult_vecNP_scalarNN copy_of_old = Mult_vecNP_scalarNN{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Mult_vecU_scalarNN *>(func)) {
                Mult_vecU_scalarNN copy_of_old = Mult_vecU_scalarNN{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Mult_vecNN_scalarU *>(func)) {
                Mult_vecNN_scalarU copy_of_old = Mult_vecNN_scalarU{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Mult_vecU_scalarNP *>(func)) {
                Mult_vecU_scalarNP copy_of_old = Mult_vecU_scalarNP{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Mult_vecNP_scalarU *>(func)) {
                Mult_vecNP_scalarU copy_of_old = Mult_vecNP_scalarU{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Mult_vecU_scalarU *>(func)) {
                Mult_vecU_scalarU copy_of_old = Mult_vecU_scalarU{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            }

            else if (auto old = dynamic_cast<Mult_vecNN_vecNN *>(func)) {
                Mult_vecNN_vecNN copy_of_old = Mult_vecNN_vecNN{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Mult_vecNP_vecNP *>(func)) {
                Mult_vecNP_vecNP copy_of_old = Mult_vecNP_vecNP{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Mult_vecNN_vecNP *>(func)) {
                Mult_vecNN_vecNP copy_of_old = Mult_vecNN_vecNP{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Mult_vecU_vecNN *>(func)) {
                Mult_vecU_vecNN copy_of_old = Mult_vecU_vecNN{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Mult_vecU_vecNP *>(func)) {
                Mult_vecU_vecNP copy_of_old = Mult_vecU_vecNP{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Mult_vecU_vecU *>(func)) {
                Mult_vecU_vecU copy_of_old = Mult_vecU_vecU{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            }

            else if (auto old = dynamic_cast<Mult_MatNN_VecNN *>(func)) {
                Mult_MatNN_VecNN copy_of_old = Mult_MatNN_VecNN{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Mult_MatNP_VecNP *>(func)) {
                Mult_MatNP_VecNP copy_of_old = Mult_MatNP_VecNP{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Mult_MatNN_VecNP *>(func)) {
                Mult_MatNN_VecNP copy_of_old = Mult_MatNN_VecNP{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Mult_MatNP_VecNN *>(func)) {
                Mult_MatNP_VecNN copy_of_old = Mult_MatNP_VecNN{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Mult_MatU_VecNN *>(func)) {
                Mult_MatU_VecNN copy_of_old = Mult_MatU_VecNN{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Mult_MatU_VecNP *>(func)) {
                Mult_MatU_VecNP copy_of_old = Mult_MatU_VecNP{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Mult_MatNN_VecU *>(func)) {
                Mult_MatNN_VecU copy_of_old = Mult_MatNN_VecU{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Mult_MatNP_VecU *>(func)) {
                Mult_MatNP_VecU copy_of_old = Mult_MatNP_VecU{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Mult_MatU_VecU *>(func)) {
                Mult_MatU_VecU copy_of_old = Mult_MatU_VecU{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            }

            else if (auto old = dynamic_cast<Dot_NN_NN *>(func)) {
                Dot_NN_NN copy_of_old = Dot_NN_NN{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Dot_NP_NP *>(func)) {
                Dot_NP_NP copy_of_old = Dot_NP_NP{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Dot_NN_NP *>(func)) {
                Dot_NN_NP copy_of_old = Dot_NN_NP{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Dot_U_NN *>(func)) {
                Dot_U_NN copy_of_old = Dot_U_NN{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Dot_U_NP *>(func)) {
                Dot_U_NP copy_of_old = Dot_U_NP{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Dot_U_U *>(func)) {
                Dot_U_U copy_of_old = Dot_U_U{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            }

            else if (auto old = dynamic_cast<Div_VecNN_VecNN *>(func)) {
                Div_VecNN_VecNN copy_of_old = Div_VecNN_VecNN{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Div_VecNP_VecNP *>(func)) {
                Div_VecNP_VecNP copy_of_old = Div_VecNP_VecNP{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Div_VecNN_VecNP *>(func)) {
                Div_VecNN_VecNP copy_of_old = Div_VecNN_VecNP{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Div_VecNP_VecNN *>(func)) {
                Div_VecNP_VecNN copy_of_old = Div_VecNP_VecNN{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Div_VecU_VecNN *>(func)) {
                Div_VecU_VecNN copy_of_old = Div_VecU_VecNN{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Div_VecU_VecNP *>(func)) {
                Div_VecU_VecNP copy_of_old = Div_VecU_VecNP{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Div_VecNN_VecU *>(func)) {
                Div_VecNN_VecU copy_of_old = Div_VecNN_VecU{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Div_VecNP_VecU *>(func)) {
                Div_VecNP_VecU copy_of_old = Div_VecNP_VecU{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Div_VecU_VecU *>(func)) {
                Div_VecU_VecU copy_of_old = Div_VecU_VecU{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            }

            else if (auto old = dynamic_cast<Div_VecNN_ScalarNN *>(func)) {
                Div_VecNN_ScalarNN copy_of_old = Div_VecNN_ScalarNN{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Div_VecNP_ScalarNP *>(func)) {
                Div_VecNP_ScalarNP copy_of_old = Div_VecNP_ScalarNP{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Div_VecNN_ScalarNP *>(func)) {
                Div_VecNN_ScalarNP copy_of_old = Div_VecNN_ScalarNP{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Div_VecNP_ScalarNN *>(func)) {
                Div_VecNP_ScalarNN copy_of_old = Div_VecNP_ScalarNN{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Div_VecU_ScalarNN *>(func)) {
                Div_VecU_ScalarNN copy_of_old = Div_VecU_ScalarNN{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Div_VecU_ScalarNP *>(func)) {
                Div_VecU_ScalarNP copy_of_old = Div_VecU_ScalarNP{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Div_VecNN_ScalarU *>(func)) {
                Div_VecNN_ScalarU copy_of_old = Div_VecNN_ScalarU{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Div_VecNP_ScalarU *>(func)) {
                Div_VecNP_ScalarU copy_of_old = Div_VecNP_ScalarU{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Div_VecU_ScalarU *>(func)) {
                Div_VecU_ScalarU copy_of_old = Div_VecU_ScalarU{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            }

            else if (auto old = dynamic_cast<Div_ScalarNN_VecNN *>(func)) {
                Div_ScalarNN_VecNN copy_of_old = Div_ScalarNN_VecNN{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Div_ScalarNP_VecNP *>(func)) {
                Div_ScalarNP_VecNP copy_of_old = Div_ScalarNP_VecNP{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Div_ScalarNN_VecNP *>(func)) {
                Div_ScalarNN_VecNP copy_of_old = Div_ScalarNN_VecNP{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Div_ScalarNP_VecNN *>(func)) {
                Div_ScalarNP_VecNN copy_of_old = Div_ScalarNP_VecNN{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Div_ScalarU_VecNN *>(func)) {
                Div_ScalarU_VecNN copy_of_old = Div_ScalarU_VecNN{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Div_ScalarU_VecNP *>(func)) {
                Div_ScalarU_VecNP copy_of_old = Div_ScalarU_VecNP{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Div_ScalarNN_VecU *>(func)) {
                Div_ScalarNN_VecU copy_of_old = Div_ScalarNN_VecU{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Div_ScalarNP_VecU *>(func)) {
                Div_ScalarNP_VecU copy_of_old = Div_ScalarNP_VecU{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Div_ScalarU_VecU *>(func)) {
                Div_ScalarU_VecU copy_of_old = Div_ScalarU_VecU{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            }

            else if (auto old = dynamic_cast<Log2 *>(func)) {
                Log2 copy_of_old = Log2{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<LogN *>(func)) {
                LogN copy_of_old = LogN{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Inv *>(func)) {
                Inv copy_of_old = Inv{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Neg *>(func)) {
                Neg copy_of_old = Neg{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Prod *>(func)) {
                Prod copy_of_old = Prod{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Sum *>(func)) {
                Sum copy_of_old = Sum{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<List *>(func)) {
                List copy_of_old = List{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            } else if (auto old = dynamic_cast<Sliced *>(func)) {
                Sliced copy_of_old = Sliced{*old, tmp};
                contiguousStorageContainer.push_back(copy_of_old);
            }
        }
    }
    return contiguousStorageContainer;
}

#endif //EXPRTREE_FUNCTION_H