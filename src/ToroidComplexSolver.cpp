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

int GetZeroVal(Array<FloatType,3,1> &vect, FloatType tol)
{
    int m;
    FloatType minV=vect.minCoeff(&m);
    for(int i=0; i<vect.size(); ++i)
    {
        if(i!=m && minV/vect(i) >tol)
        {
            m=-1;
            break;
        }
    }
    return m;
}


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
    Array<FloatType,3,1> absVp=ces.eigenvalues().array().abs() ;
    int iv0=GetZeroVal(absVp, 1e-12);  // iv0 est la VP nulle sauf sit le ratio zeroVP/ next Vp est hors tolérance
    if(iv0==-1) // hors tolérance
    {
        cout << "Toroid complex solver: zero eigen value tolerance not satisfied\n :" << ces.eigenvalues().transpose() << endl;
        exit(-1);
    }
// Eigen ne garantit pas  que les VPs soient triées
//   if ( abs(ces.eigenvalues()[0] )  > 1000.* numeric_limits<FloatType>::epsilon() )
//    if(iv0!=0)
//       cout << "Toroid complex solver:  La valeur propre 0 est non nulle \n";  // Todo gérer cette erreur
//       cout << "eigen value:" << ces.eigenvalues().transpose() << endl;
//       exit (-1);
//   }

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

    for(i=0,j=0 ; i <3; ++i)
    {
        if(i!=iv0)
        {
            Mu=ces.eigenvectors().col(i).transpose()*ces.eigenvectors().col(i);
         //   Nu0=ces.eigenvalues()[i]/Mu0;
    //        cout << i << "Mu=" << Mu0 <<"  Nu=" << Nu0 << endl;
    //        scaledVP.col(j++)=sqrt(s*Nu[i])*ces.eigenvectors().col(i);
            scaledVP.col(j++)=sqrt(s*ces.eigenvalues()[i]/Mu)*ces.eigenvectors().col(i);
            s=-s;
        }
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

