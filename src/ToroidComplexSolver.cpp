////////////////////////////////////////////////////////////////////////////////
/**
*      \file        ToroidComplexSolver.cpp
*
*      \brief       Computational part of the intercept function of toroid class when degenerated conics have imaginary 2 intercepts instead of 4 .
*
*                   Computations are placed in two separate cpp files for linkage performance issues mainly in debug modes
*                   \relates Toroid::intercept() and ToroidSolver.cpp
*
*      \author         François Polack <francois.polack@synchroton-soleil.fr>
*      \date        2020-12-23  Creation
*      \date        Last update
*
*/
///////////////////////////////////////////////////////////////////////////////////
//
//             REVISIONS
//
////////////////////////////////////////////////////////////////////////////////////
#include "types.h"
#include <iostream>
#include <limits>
using namespace std;

int ComplexVpSolver(Matrix<FloatType,2,Dynamic> &solutions,Matrix<complex<FloatType>,3,3> &matSys)
{
    static  Matrix<complex<FloatType>,2,2> SumDif;
    SumDif << 1.L, 1.L, 1.L, -1.L;


    ComplexEigenSolver<Matrix<complex<FloatType>,3,3> > ces(3);
    ces.compute(matSys);
    if(ces.info()!=Success)
    {
        cout << "Eigen solver NO Convergence\n";
        return -1;
    }
//    Matrix<complex<FloatType>,3,1> EigenVals=ces.eigenvalues();
//    Matrix<complex<FloatType>,3,3> EigenVect=ces.eigenvectors();  //  .colwise().normalized();

   if ( abs(ces.eigenvalues()[0] )  >  numeric_limits<FloatType>::epsilon() )
   {
       cout << "Cas complexe : La valeur propre 0 est non nulle \n";  // Todo gérer cette erreur
       cout << "eigen value:" << ces.eigenvalues().transpose() << endl;
       return  -1;
   }

//    Matrix<complex<FloatType>,3,1> Mu,Nu;
//
//    Mu=(EigenVect.transpose()*EigenVect).diagonal();
//    cout << "Mu vect \n" << Mu <<endl;
//
//    Nu.array()=EigenVals.array()* Mu.array().inverse();
//    cout << "Nu vect\n"<< Nu << endl;


    Matrix<complex<FloatType>,3,2> scaledVP;
    Matrix<FloatType,3,1> Inter;
    complex<FloatType> Mu;
    int i,j;
    FloatType s=1.L;

    for(i=1,j=0 ; i <3; ++i)
    {
        Mu=ces.eigenvectors().col(i).transpose()*ces.eigenvectors().col(i);
     //   Nu0=ces.eigenvalues()[i]/Mu0;
//        cout << i << "Mu=" << Mu0 <<"  Nu=" << Nu0 << endl;
//        scaledVP.col(j++)=sqrt(s*Nu[i])*ces.eigenvectors().col(i);
        scaledVP.col(j++)=sqrt(s*ces.eigenvalues()[i]/Mu)*ces.eigenvectors().col(i);
        s=-s;
    }
//    cout << " scaled VPs\n" << scaledVP  << endl;
    scaledVP*=SumDif;
//    cout << " vecteur solutions sum et dif\n" << scaledVP  << endl;
    solutions=Matrix<FloatType,2,Dynamic>::Zero(2,2);
    for(i=0; i<2;++i)
    {
        Inter=scaledVP.col(i).cross(scaledVP.col(i).conjugate()).imag();
//        cout << i << "    " << Inter.transpose()  << endl;
        solutions.col(i)=Inter.hnormalized();
    }
    return 2;

//    cout << " points de base du faisceau \n"  <<  solutions << endl;
}

