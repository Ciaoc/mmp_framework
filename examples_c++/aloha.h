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


#ifndef _ALOHA_H_
#define _ALOHA_H_

#include <array>
#include <cmath>
#include <numeric>
#include <algorithm>

#include "../mmp_framework/cppresources/algorithm/MMP.h"

using std::array;

template <size_t Dim>
class ALOHA : public MMPconstraints<Dim>
{
	using typename MMPconstraints<Dim>::vtypeS;
	using typename MMPconstraints<Dim>::RBox;

	public:
		double ck[Dim];
		bool beta[Dim][Dim];
		double Rmin[Dim];
		
		ALOHA();

        double evaluate(const std::vector<double>& x, const std::vector<double>& y) const; // helper for debug
        bool eval_constr(const std::vector<double>& x, const std::vector<double>& y) const; // helper for debug

		bool isOptimal() { return this->status == BRB<Dim>::Status::Optimal; }; // helper for python

	private:
		double MMPobj(const vtypeS& x, const vtypeS& y) const override;
		bool constraints(const vtypeS& x, const vtypeS& y) const override;
		vtypeS feasiblePoint(const RBox& r) const override
			{ return r.lb(); }
};


template <size_t D>
ALOHA<D>::ALOHA()
	: MMPconstraints<D>()
{
	this->setUB(1);
	this->setLB(0);
}

template <size_t D>
double
ALOHA<D>::evaluate(const std::vector<double>& x, const std::vector<double>& y) const
{
	double ret = 1;

	for (size_t i = 0; i < D; ++i) {
		double tmp = 1.0;
		double tmp2 = 1.0;
		double tmp3 = 0.0;
		std::cout << "\n(k=" << i << ")" << std::endl;
		std::cout << "ck * xk = " << ck[i] * x[i] << std::endl;

		for (size_t j = 0; j < D; ++j) {
			if (beta[i][j])
				tmp *= (1.0 - static_cast<double>(y[j]));
		}
        std::cout << "prod = " << tmp << std::endl;
        std::cout << "1-y = " << (1.0 - static_cast<double>(y[i])) << std::endl;
        std::cout << "ck * xk * prod = " << ck[i] * x[i] * tmp << std::endl;
		ret *= (ck[i] * x[i] * tmp);  // prop fair utility
	}

	return std::log(ret);
}


template <size_t D>
bool ALOHA<D>::eval_constr(const std::vector<double>& x, const std::vector<double>& y) const
{
    std::vector<double> prod;

    std::cout << "\nrk         >=     Rmin" << std::endl;
	for (size_t i = 0; i < D; ++i)
	{
		double tmp = 1.0;

		for (size_t j = 0; j < D; ++j)
			if (beta[i][j])
				tmp *= (1.0 - static_cast<double>(x[j]));
//			prod.push_back(tmp);

        std::cout << ck[i] * x[i] * tmp << "    >=    " << Rmin[i] << std::endl;
        std::cout << "Rmin - rk =  [ " << Rmin[i] - (ck[i] * y[i] * tmp) << "]" << std::endl;
		if (ck[i] * y[i] * tmp < Rmin[i])
			return false;
	}

//	std::cout << "ck: [ " << ck[0] << ", " << ck[1] << ", " << ck[2] << " ]" << std::endl;
//	std::cout << "xk: [ " << y[0] << ", " << y[1] << ", " << y[2] << " ]" << std::endl;
//	std::cout << "prod: [ " << prod.at(0) << ", " << prod.at(1) << ", " << prod.at(2) << " ]" << std::endl;

	return true;
}


template <size_t D>
double
ALOHA<D>::MMPobj(const vtypeS& x, const vtypeS& y) const
{
	double ret = 1;

	for (size_t i = 0; i < D; ++i) {
		double tmp = 1.0;
		for (size_t j = 0; j < D; ++j)
			if (beta[i][j])
				tmp *= (1.0 - static_cast<double>(y[j]));

		ret *= (ck[i] * x[i] * tmp);  // prop fair utility
	}

	return std::log(ret);
}


template <size_t D>
bool ALOHA<D>::constraints(const vtypeS& x, const vtypeS& y) const
{
	for (size_t i = 0; i < D; ++i)
	{
		double tmp = 1.0;

		for (size_t j = 0; j < D; ++j)
			if (beta[i][j])
				tmp *= (1.0 - static_cast<double>(x[j]));

		if (ck[i] * y[i] * tmp < Rmin[i])
			return false;
	}

	return true;
}
#endif
