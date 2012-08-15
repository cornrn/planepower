/*
 *    This file was auto-generated by ACADO Toolkit.
 *
 *    ACADO Toolkit -- A Toolkit for Automatic Control and Dynamic Optimization.
 *    Copyright (C) 2008-2011 by Boris Houska, Hans Joachim Ferreau et al., K.U.Leuven.
 *    Developed within the Optimization in Engineering Center (OPTEC) under
 *    supervision of Moritz Diehl. All rights reserved.
 *
 */


real_t* getAcadoVariablesX(  ){
return acadoVariables.x;
}

real_t* getAcadoVariablesU(  ){
return acadoVariables.u;
}

real_t* getAcadoVariablesXRef(  ){
return acadoVariables.xRef;
}

real_t* getAcadoVariablesURef(  ){
return acadoVariables.uRef;
}

void printStates(  ){
int run01,run02;
printf( "acadoVariables.x = \n" );
for( run01=0; run01<22; ++run01 ){
  for( run02=0; run02<11; ++run02 )
    printf( "%e \t", acadoVariables.x[run02*22+run01] );
  printf( "\n" );
}
}

void printControls(  ){
int run01,run02;
printf( "acadoVariables.u = \n" );
for( run01=0; run01<3; ++run01 ){
  for( run02=0; run02<10; ++run02 )
    printf( "%e \t", acadoVariables.u[run02*3+run01] );
  printf( "\n" );
}
}

real_t getTime(  ){
real_t current_time = 0.000000e+00;
struct timeval theclock;
gettimeofday( &theclock,0 );
current_time = 1.0*theclock.tv_sec + 1.0e-6*theclock.tv_usec;
return current_time;
}

void printHeader(  ){
    printf("\nACADO Toolkit -- A Toolkit for Automatic Control and Dynamic Optimization.\nCopyright (C) 2008-2011 by Boris Houska and Hans Joachim Ferreau, K.U.Leuven.\nDeveloped within the Optimization in Engineering Center (OPTEC) under\nsupervision of Moritz Diehl. All rights reserved.\n\nACADO Toolkit is distributed under the terms of the GNU Lesser\nGeneral Public License 3 in the hope that it will be useful,\nbut WITHOUT ANY WARRANTY; without even the implied warranty of\nMERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\nGNU Lesser General Public License for more details.\n\n" );
}
