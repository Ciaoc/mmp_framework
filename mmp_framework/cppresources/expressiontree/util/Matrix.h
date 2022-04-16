#ifndef EXPRTREE_MATRIX_H
#define EXPRTREE_MATRIX_H

#include <vector>

class Matrix {
public:
    Matrix();
    Matrix(size_t rows, size_t cols);
    Matrix(size_t rows, size_t cols, std::vector<double> mat);
    Matrix(const Matrix &old) = default;
    double& operator()(size_t i, size_t j);
    double operator()(size_t i, size_t j) const;
    void print() const;

    Matrix getGreaterEqualThan0();
    Matrix getLessThan0();
    std::vector<size_t> getDim() const;
    std::vector<double> getRow(size_t row);
    std::vector<double>::iterator getRowIt(size_t row);
    std::vector<double> getColumn(size_t col);

private:
    size_t mRows;
    size_t mCols;
    std::vector<double> mData;
};

Matrix::Matrix():mRows(0),mCols(0),mData(0) {}
Matrix::Matrix(size_t rows, size_t cols)
:   mRows(rows),
    mCols(cols),
    mData(rows * cols)
{
}

Matrix::Matrix(size_t rows, size_t cols, std::vector<double> mat)
:   mRows(rows),
    mCols(cols),
    mData(rows * cols)
{
    mData = mat;
}

Matrix Matrix::getGreaterEqualThan0() {
    Matrix ret = Matrix(*this);
    for (double value : ret.mData) {
        if (value < 0) {
            value = 0;
        }
    }
    return ret;
}

Matrix Matrix::getLessThan0() {
    Matrix ret = Matrix(*this);
    for (double value : ret.mData) {
        if (value > 0) {
            value = 0;
        }
    }
    return ret;
}

// Matrix element-access operators
double& Matrix::operator()(size_t i, size_t j) {return mData[i * mCols + j];}
double Matrix::operator()(size_t i, size_t j) const {return mData[i * mCols + j];}
void Matrix::print() const {
    printf("\n       [");
    for (int i=0; i<mRows; ++i) {
        for (int j=0; j<mCols; ++j) {
            if (j != mCols-1) {
                std::cout << (*this)(i,j) << ", ";
            } else {
                std::cout << (*this)(i,j);
            }
        }
        if (i < mRows-1) {
            std::cout << std::endl << "        ";
        } else {
            std::cout << "]" << std::endl;
        }

    }
}

std::vector<size_t> Matrix::getDim() const {
    std::vector<size_t> ret(2);
    ret.at(0) = mRows;
    ret.at(1) = mCols;
    return ret;
}
std::vector<double> Matrix::getRow(size_t row) {
    std::vector<double> ret(mCols);
    for (int k=0; k<mCols; k++) {
        ret.at(k) = (*this)(row, k);
    }
    return ret;
}
std::vector<double>::iterator Matrix::getRowIt(size_t row) {
    return mData.begin() + row * mCols;
}
std::vector<double> Matrix::getColumn(size_t col) {
    std::vector<double> ret(mRows);
    for (int k=0; k<mRows; k++) {
        ret.at(k) = (*this)(k, col);
    }
    return ret;
}

#endif //EXPRTREE_MATRIX_H
