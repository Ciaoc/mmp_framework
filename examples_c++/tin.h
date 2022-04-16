/* Copyright (C) 2018-2019 Bho Matthiesen, Christoph Hellings
 * 
 * This program is used in the article:
 *
 * Bho Matthiesen, Christoph Hellings, Eduard A. Jorswieck, and Wolfgang
 * Utschick, "Mixed Monotonic Programming for Fast Global Optimization,"
 * submitted to IEEE  Transactions on Signal Processing.
 * 
 * 
 * License:
 * This program is licensed under the GPLv2 license. If you in any way use this
 * code for research that results in publications, please cite our original
 * article listed above.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details. */


#ifndef _TIN_H_
#define _TIN_H_

#include <array>
#include <cmath>
#include <numeric>
#include <algorithm>

#include <iostream>

#include "../mmp_framework/cppresources/algorithm/MMP.h"

template <size_t Dim, bool propFair = false>
class TIN : public MMP<Dim>
{
	using typename MMP<Dim>::vtypeS;

	public:

        // Parameter für die Objective-Function
		double alpha[Dim];
		std::array<double, Dim> beta[Dim];
		double sigma[Dim];

		TIN() : MMP<Dim>() {}

        double evaluate(const std::vector<double>& x, const std::vector<double>& y) const;

	private:
		double MMPobj(const vtypeS& x, const vtypeS& y) const override;
};

template <size_t D, bool propFair>
double TIN<D,propFair>::evaluate(const std::vector<double>& x, const std::vector<double>& y) const
{
	double ret = 1;

	for (size_t i = 0; i < D; ++i) {
	    std::cout << std::endl;
		double tmp = 1 + alpha[i] * x[i] / (std::inner_product(y.begin(), y.end(), beta[i].begin(), sigma[i]) + beta[i][i]*(static_cast<double>(x[i])-y[i]));
//        std::cout << "\ntmp(k=" << i << ") = " << tmp;

//        std::cout << "alpha * x(k=" << i << ") = " << alpha[i] * x[i] << std::endl;
        std::cout << "beta(k=" << i << ") = [";
        std::for_each(beta[i].begin(), beta[i].end(), [](const double & c){std::cout << c << ", ";});
        std::cout << "]" << std::endl << "y   (k=" << i << ") = [";
        std::for_each(y.begin(), y.end(), [](const double & c){std::cout << c << ", ";});
        std::cout  << "]" << std::endl;
        std::cout << "DOT_var_mat: beta * y(k=" << i << ") = " << std::inner_product(y.begin(), y.end(), beta[i].begin(), (double)0) << std::endl;
//        std::cout << "Division term(k=" << i << ") = " << alpha[i] * x[i] / std::inner_product(y.begin(), y.end(), beta[i].begin(), sigma[i]) << std::endl;
//        std::cout << "1 + Division term(k=" << i << ") = " << 1 + alpha[i] * x[i] / std::inner_product(y.begin(), y.end(), beta[i].begin(), sigma[i]) << std::endl;

		if constexpr(propFair)
			ret *= std::log2(tmp);
		else
			ret *= tmp;
//			std::cout << "cumulative return value ret = ret * tmp(k=" << i << ") = " << ret << std::endl;
	}

	if constexpr(propFair)
		return ret;
	else
//	    std::cout << "ret is now: " << ret << std::endl;
//	    std::cout << "log2 of ret is: " << std::log2(ret) << std::endl;
		return std::log2(ret);
}

template <size_t D, bool propFair>
double TIN<D,propFair>::MMPobj(const vtypeS& x, const vtypeS& y) const
// overridden function MMPobj()
{
	double ret = 1;

	for (size_t i = 0; i < D; ++i) {
		double tmp = 1 + alpha[i] * x[i] / (std::inner_product(y.begin(), y.end(), beta[i].begin(), sigma[i]) + beta[i][i]*(static_cast<double>(x[i])-y[i]));

		if constexpr(propFair)
			ret *= std::log2(tmp);
		else
			ret *= tmp;
	}

	if constexpr(propFair)
		return ret;
	else
		return std::log2(ret);
}

#endif
