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


#include "tin.h"


TIN<4,false>
get_tin(void)
{
	TIN<4,false> tin;

	const double Pmax = 1;
	const double N = 1e-2;

	const double alpha[] = { 8.3401758e+02, 3.9182301e+01, 8.7457578e+03, 1.2471862e+02 };
	const double beta[][4] = { 
		{0, 5.9968562e+00, 9.5184622e+00, 6.0737956e-01},
		{1.3587096e+00, 0, 2.0014184e-02, 1.6249435e+00},
		{3.8521406e-01, 4.6761915e-01, 0, 1.8704400e+00},
		{1.2729254e-01, 2.1447293e-02, 3.1017335e-02, 0}
	};

	tin.setUB(Pmax);
	tin.setLB(0);
	tin.setPrecision(1e-2);
	tin.useRelTol = false;
	tin.disableReduction = true;
	tin.outputEvery = 1e6;
	//tin.enablePruning = false;

	for (size_t i = 0; i < tin.dim(); ++i)
	{
		tin.alpha[i] = alpha[i];
		for (size_t j = 0; j < tin.dim(); ++j)
		{
			if (i == j)
				tin.beta[i][i] = 0.0;
			else
				tin.beta[i][j] = beta[i][j];
		}
		tin.sigma[i] = N;
	}
    return tin;
	//tin.optimize();
}

//void testtest2(TIN<20,false> tin, const std::vector<double>& x, const std::vector<double>& y)
//{
//    printf("Result of TIN: ");
//    std::cout << tin.evaluate(x, y) << std::endl;
//}

TIN<4,false> optimize_tin(TIN<4,false> tin)
{
    tin.optimize();
    return tin;
}
