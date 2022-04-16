#ifndef EXPRTREE_VARIABLE_H
#define EXPRTREE_VARIABLE_H

#include "Leaf.h"

class Variable: public Leaf
{
  public:
    explicit Variable(int dimension, sign sign) : Leaf("variable") {
        this->m_dim = dimension;
        this->m_value.resize(dimension);
        this->m_sign = sign;
    };
    Variable(const Variable & old) = default;
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {
        m_value = x_in;
    };
};

#endif //EXPRTREE_VARIABLE_H
