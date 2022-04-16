#ifndef EXPRTREE_LEAF_H
#define EXPRTREE_LEAF_H

#include <utility>

#include "Expression.h"

class Leaf: public Expression
{
  public:
    explicit Leaf(const std::string& name) : Expression(name){};
    Leaf(const Leaf & old) = default;
    void evaluate(const std::vector<double>& x_in, const std::vector<double>& y_in) override {};
};

#endif //EXPRTREE_LEAF_H
