#ifndef EXPRTREE_PARAMETER_H
#define EXPRTREE_PARAMETER_H

#include "Leaf.h"
#include "util/Matrix.h"

class Parameter: public Leaf
{
  public:
    explicit Parameter(const std::vector<double>& valuein = {} ) : Leaf("Parameter") {
        this->m_dim = valuein.size();
        this->m_value = valuein;
        for (int i=0; i<m_dim; ++i) {
            if (m_value.at(i) >= 0) {
                this->m_sign = NN;
            } else if (m_value.at(i) <= 0) {
                this->m_sign = NP;
            } else {
                this->m_sign = unknown;
            }
        }
    };
    Parameter(const Parameter & old) = default;
    static bool isExisting(std::vector<double>* value){};
};

class MultiDimParameter: public Leaf
{
  public:
    explicit MultiDimParameter(int rows, int cols, const std::vector<double>& valuein) : Leaf("MultDimParameter") {
        this->m_dim = 0;
        this->m_matrix = Matrix(rows, cols, valuein);
        for (int i=0; i<valuein.size(); ++i) {
            if (valuein.at(i) >= 0) {
                this->m_sign = NN;
            } else if (valuein.at(i) <= 0) {
                this->m_sign = NP;
            } else {
                this->m_sign = unknown;
            }
        }
    };
    MultiDimParameter(const MultiDimParameter & old) : Leaf(old) {
        m_matrix = old.m_matrix;
    };
    Matrix* getValue() {return &m_matrix;};
    void printValue() override {m_matrix.print();};
    std::vector<size_t> getDim() const {return m_matrix.getDim();};
  private:
    Matrix m_matrix;
};
#endif //EXPRTREE_PARAMETER_H