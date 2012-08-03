/*
 *    This file was auto-generated by ACADO Toolkit.
 *
 *    ACADO Toolkit -- A Toolkit for Automatic Control and Dynamic Optimization.
 *    Copyright (C) 2008-2011 by Boris Houska, Hans Joachim Ferreau et al., K.U.Leuven.
 *    Developed within the Optimization in Engineering Center (OPTEC) under
 *    supervision of Moritz Diehl. All rights reserved.
 *
 */


extern "C"{
	#include "solver.hpp"
}
#include "INCLUDE/QProblem.hpp"
#include "INCLUDE/SolutionAnalysis.hpp"
#include "INCLUDE/Utils.hpp"


int logNWSR;


long solve( void )
{
	int nWSR = QPOASES_NWSRMAX;
	
	QProblem example(4, 15);
	returnValue retVal = example.init( params.H,params.g,params.A,params.lb,params.ub,params.lbA,params.ubA, nWSR,vars.y );
	
	example.getPrimalSolution( vars.x );
	example.getDualSolution( vars.y );
	
	SolutionAnalysis sa;
	
	// retVal = sa.getHessianInverse(&example, params.iH);
	
	real_t seed[KKT_DIM * KKT_DIM];
	real_t iKKT[KKT_DIM * KKT_DIM];
	for (int i = 0; i < KKT_DIM * KKT_DIM; ++i)
		seed[ i ] = 0.0;
	
	seed[0 * KKT_DIM + 0] = 1.0;
	seed[1 * KKT_DIM + 1] = 1.0;
	seed[2 * KKT_DIM + 2] = 1.0;
	seed[3 * KKT_DIM + 3] = 1.0;
	
	sa.getVarianceCovariance(&example, seed, iKKT);
	
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
			params.Sigma[i * 4 + j] = iKKT[i * KKT_DIM + j];
	}
	print(params.Sigma, 4, 4);
	
	logNWSR = nWSR;
	
	return (long) retVal;
}