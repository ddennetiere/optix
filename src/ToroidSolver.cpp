////////////////////////////////////////////////////////////////////////////////
/**
*      \file         ToroidSolver.cpp
*
*      \brief         Computational part of the intercept function of toroid class.
*
*                   Computations are placed in two separate cpp files for linkage performance issues mainly in debug modes
*                   \relates Toroid::intercept() and ToroidComplexSolver.cpp
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
//using namespace std; no longer usable in recet C++ releases

using std::complex;
using std::cout, std::endl;

/** \brief Solution of conic intersection equation  when degenerated conics have imaginary 2 intercepts instead of 4 .
 *  \ingroup GlobalCpp
 * \param solutions The return complex Eigen vectors
 * \param matSys The complex Matrix the Eigen vectors of which are requested
 * \return 2 if computation succeeds, -1 if it fails
 *
 *  This is part of  the intercept function of toroid class.*/
int ComplexVpSolver(Matrix<FloatType,2,Dynamic> &solutions,Matrix<complex<FloatType>,3,3> &matSys);
// in a separate  ToroidComplexSolver.cpp file for compilation performance issues

int GetZeroVal(Array<FloatType,3,1> &vect, FloatType tol);



int ToroidSolver(Matrix<FloatType,2,Dynamic> &solutions, Matrix<FloatType,3,3> & Mat1, Matrix<FloatType,3,3> & Mat2)
{

    Matrix<FloatType,3,3> MatS=Mat1*Mat2.inverse();
//    cout <<"MatS\n" << MatS <<endl;
    Matrix<complex<FloatType>,3,1> Ev;

    EigenSolver<Matrix<FloatType,3,3> > es;
    es.compute(MatS,false); // we don't need the eigen vectors at thsi  step
    Ev=es.eigenvalues();
//    cout << "Intercept eigen Values\n" << Ev.transpose() << endl;

    int i;
    for(i=0; i<2; ++i)
    {
        if(Ev(i).imag()!=0)  // VP complexe
        {
 //         Migré dans ToroidComplexSoler.cpp  pour éviter le flag -Wa,-mbig-obj

            Matrix<complex<FloatType>,3,3> MatCS=Mat1-Ev(i)*Mat2 ;
            return ComplexVpSolver(solutions,MatCS);
        }
    }
    if (i==2)  // toutes les valeurs propres sont réelles
    {
//        cout << "all real eigen values\n";
        static Matrix<FloatType,2,2> SumDif ;
        SumDif << 1.L, 1.L, 1.L, -1.L;
        int i,j,k,s, iv0;
        Matrix<FloatType,3,4> Eigenvects;  // Il y a 3 paires de  vecteurs propres significatifs  mais 2 paires suffisent à obtenir tous les points d'insersection. Pas trouvé de différence significative entre paires
        SelfAdjointEigenSolver<Matrix<FloatType,3,3> > saes;  // ces matrices sont réelles et symétriques. Les vecteurs propres sont assy réels même si les types sont complexes

        Array<FloatType,3,1> absVp;

        for(i=0,k=0; i<2; ++i) // i<3 pour avoir toutes les paires
        {
             MatS= Mat1 - Ev[i].real() *Mat2; // le Vps sont réelles  Une de VPs de S doit être nulle
          //  saes.computeDirect(MatS);
            saes.compute(MatS);
            absVp = saes.eigenvalues().array().abs();
            iv0=GetZeroVal(absVp, 1e-12);  // iv0 est la VP nulle sauf sit le ratio zeroVP/ next Vp est hors tolérance
            if(iv0==-1) // hors tolérance
            {
                cout << "Toroid: zero eigen value tolerance not satisfied\n :" << saes.eigenvalues().transpose() << endl;
                exit(-1);
            }
 //           FloatType ZeroTest=1000* numeric_limits<FloatType>::epsilon();
//           FloatType ZeroTest=5*saes.eigenvalues().maxCoeff()* numeric_limits<FloatType>::epsilon();
    //        cout << "System " << i << "  eigen values  :" << saes.eigenvalues().transpose() << " // " <<  ZeroTest << endl;
    //        cout << "\t\t eigen vectors\n" << saes.eigenvectors() <<endl;
            // verification des signes pour factoriser la conique dégénerée en 2 droites réelles les val  propres doivent être de signes contraires
            //et stockage des vecteurs propres significataifs * par la racine due la valeur propre associée (en val abs)
            for(j=0,s=1;j<3;++j)
            {
//                if(abs((saes.eigenvalues()[j])) > ZeroTest)
                if(j != iv0)  // VP0 exclue
                {
    //            cout <<"i j k "<<i <<"  " << j << " " << k  <<" EV(j) " <<saes.eigenvalues()[j]<<endl;


    //                if(signbit(saes.eigenvalues()[j]))
                    if(saes.eigenvalues()[j] < 0)
                    {
                        s=-s;
                        try {
                            Eigenvects.col(k++)=sqrt(-saes.eigenvalues()[j])*saes.eigenvectors().col(j);
                        } catch (EigenException & excpt)
                        {
                            cout << "Eigen exception case < 0 k=" << k << " j=" << j << endl << excpt.what() <<endl;
  //                          cout << "Zero test value " << ZeroTest << endl;
                            cout << " Zero VP is " << iv0 <<endl;
                            cout << "Eigen values :" << saes.eigenvalues().transpose()<< endl;
                            cout << "Eigen vectors:\n" << saes.eigenvectors() <<endl;
                            exit(-1);
                        }
                    }
                    else
                    {
                        try{
                            Eigenvects.col(k++)=sqrt(saes.eigenvalues()[j])*saes.eigenvectors().col(j);
                        } catch (EigenException & excpt)
                        {
                            cout << "Eigen exception case  > 0 k=" << k << " j=" << j << endl << excpt.what() <<endl;
//                            cout << "Zero test value " << ZeroTest << endl;
                            cout << " Zero VP is " << iv0 <<endl;
                            cout << "Eigen values :" << saes.eigenvalues().transpose()<< endl;
                            cout << "Eigen vectors:\n" << saes.eigenvectors() <<endl;
                            exit(-1);
                        }
                    }
                }
            }

            // cout << "product sign " << s << endl;  // 1 signe  >0 pas de solution réelle
            if (s==1)
            {
//                cout << "the problem has no real solution\n";
    //                throw RayException("No intercept found", __FILE__, __func__, __LINE__);

                return  0;
            }
//            cout << "blocking sumdif i="<< i <<endl;
            Eigenvects.block(0,2*i,3,2)*= SumDif; // compute sum and difference of the scaled eigenvectors
    //            cout << "Normalization\n" << saes.eigenvectors().real().transpose()*saes.eigenvectors().real()<<endl;
//            cout << "blocked sumdif\n";
        }
    //    cout << "scaled useful VPs\n" << Eigenvects <<endl;
    //    for(i=0; i<3; ++i)

    //    cout << "sum & dif scaled VPs\n" << Eigenvects <<endl;

//        cout << "the problem has 4 solutions\n";
      //  Matrix<FloatType,3,1> inter;
        solutions=Matrix<FloatType,2,Dynamic>::Zero(2,4);
        for(i=0, k=0; i<2; ++i)
            //for(j=2*(1+i/2); j<6; ++j,++k)
            for(j=2; j<4; ++j,++k)
                solutions.col(k)=(Eigenvects.col(i).cross(Eigenvects.col(j))).hnormalized();



        return 4;

    }
    else
    {
        cout << "VPw of S : " << Ev.transpose() << endl;
        return -1; // code d'erreur
    }
}
