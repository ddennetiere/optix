////////////////////////////////////////////////////////////////////////////////
/**
*      \file           toroid.cpp
*
*      \brief         Spherical surface class implementation
*
*      \author         François Polack <francois.polack@synchroton-soleil.fr>
*      \date        2020-12-11  Creation
*      \date        Last update
*
*/
///////////////////////////////////////////////////////////////////////////////////
//
//             REVISIONS
//
////////////////////////////////////////////////////////////////////////////////////
#include "toroid.h"
//#include <chrono>
//using namespace chrono;
// #define DIAG_OUTPUT
//#include <windows.h>  cette instruction est inutile et fait planter la compilation dans les versions récentes de C++




//Matrix<FloatType,2,2>  ComplexVpSolver(Matrix<complex<FloatType>,3,3> &matSys);
//int RealVpSolver(Matrix<FloatType,2,Dynamic> &solutions, Matrix<FloatType,3,3> & Mat1, Matrix<FloatType,3,3> & Mat2, Matrix<FloatType,3,1> Ev);

/** \brief Computes the intersections of two conics
 *  \ingroup GlobalCpp
 * \param[out] solutions  The returned intersections
 * \param[in] Mat1 the matrix of the first conic
 * \param[in] Mat2 the matrix of the second conic
 * \return The number of intersection points
 *
 *  This is part of  the intercept function of toroid class. The problem of intersection of a  toroid by a straight line can be
 *     translated into the intersection of two conics  */
int ToroidSolver(Matrix<FloatType,2,Dynamic> &solutions, Matrix<FloatType,3,3> & Mat1, Matrix<FloatType,3,3> & Mat2);



Toroid::Toroid(string name, Surface *previous):Surface(false,name, previous)  //{Surface::m_transmissive=false;}
{
    Parameter param;
    param.type=InverseDistance;
    param.group=ShapeGroup;
    defineParameter("minor_curvature", param);  // courbure par défaut 0
    setHelpstring("minor_curvature", "Curvature (1/rc) of the generator circle");  // complete la liste de infobulles de la classe Surface
    defineParameter("major_curvature", param);  // courbure par défaut 0
    setHelpstring("major_curvature", "Curvature (1/Rc) of the generated circle at apex");  // complete la liste de infobulles de la classe Surface

}

void Toroid::createSurface()
{
    Parameter param;

    if(!getParameter("minor_curvature", param) )
       throw ParameterException("minor_curvature parameter not found", __FILE__, __func__, __LINE__);
    double minCurv=param.value;
    if(!getParameter("major_curvature", param) )
       throw ParameterException("major_curvature parameter not found", __FILE__, __func__, __LINE__);
    double majCurv=param.value;

    m_toreMat1.setZero();
    m_toreMat2.setZero();
    m_toreMat1(1,1)=m_toreMat1(3,3)=minCurv;
    m_toreMat2(0,0)=m_toreMat2(2,2)=majCurv;
    m_toreMat2(3,3)=-majCurv;
    m_toreMat1(3,4)=m_toreMat1(4,3)=m_toreMat2(2,4)=m_toreMat2(4,2)=-1.L;
    m_toreMat2(3,4)=m_toreMat2(4,3)=1.L;

}

RayBaseType::VectorType Toroid::intercept(RayBaseType& ray, VectorType * normal)
{
    try
    {
    //    high_resolution_clock clock;
    //    high_resolution_clock::time_point start(clock.now());
        ray-=m_translationFromPrevious; // change ref frame from previous to this surface

        if(!ray.m_alive)
        {
            cout << m_name <<" dead ray received\n";
            return ray.position();
        }
    #ifdef DIAG_OUTPUT
        LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds;
        LARGE_INTEGER Frequency;

        QueryPerformanceFrequency(&Frequency);
        QueryPerformanceCounter(&StartingTime);

    #endif // DIAG_OUTPUT

        ray.moveTo(-ray.direction().dot(ray.origin()) ).rebase();   // move and rebase close to the nearest of toroid apex  (0 point)
//        cout << m_name  <<": input point (" << ray.position().transpose() << ") (" << ray.direction().transpose() <<  ")\n";
    //    cout << "input direction " << ray.direction() << endl;
    //    Matrix<FloatType,5,3> rayMat=Matrix<FloatType,5,3>::Zero(); //  matrice
        Matrix<FloatType,5,3> rayMat;
        rayMat.setZero(); //  matrice
//        cout << "set direction block\n";
        rayMat.block(0,0,3,1)=ray.direction();
//        cout << "set position block\n";
        rayMat.block(0,2,3,1) =ray.position();
//        cout << "passed\n";
        rayMat(3,1)=rayMat(4,2)=1.L;
    //    cout << " Ray transform to thc\n" << rayMat << endl;
        // definitions des 2 // equations quadratique en t et h  à résoudre

        Matrix<FloatType,3,3> Mat1= rayMat.transpose()* m_alignedMat1 *rayMat;
        Matrix<FloatType,3,3> Mat2= rayMat.transpose()* m_alignedMat2 *rayMat;
//        cout << "calling toroid solver\n";
        int nsols;
        int degenerate=0;
        if (abs(Mat1.determinant()) < 1.e-12 )
            degenerate=1;
        if (abs(Mat2.determinant()) < 1.e-12 )
            degenerate=2;
        if(degenerate)
        {

            VectorType U, T0, sol;
            FloatType a,b,c;
            switch(degenerate)
            {
            case 1:
                // le rayon  mineur est quasi nul donc h=0;

                // l'intesection avec l'autre conique conduit à l'équation quadratique a x^2 +2bx +c avec
                a=Mat2(0,0);
                b=Mat2(0,2);
                c=Mat2(2,2);
                break;
            case 2:
                // la conique 2 dégénère en la droite parametrique T0 + U x
                U << Mat2(2,1), -Mat2(2,0), 0;
                T0 << 0, -Mat2(2,2)/2./Mat2(2,1), 1.;
                // l'intesection avec l'autre conique conduit à l'équation quadratique a x^2 +2bx +c avec
                a=U.transpose()*Mat1*U;
                b=U.transpose()*Mat1*T0;
                c=T0.transpose()*Mat1*T0;
                break;
            }
            FloatType delta=b*b-a*c;
            if(delta < 0)
            {
//                cout << "in " << m_name <<": delta=" << delta <<" no sol degen=" << degenerate <<endl;
//                if(degenerate==1)
//                    cout <<  Mat2 <<endl;

                ray.m_alive=false;
                return ray.position();
            }

            if(degenerate==1)
//                cout << "in " << m_name <<": delta OK\n";
                sol << (-b +sqrt(delta))/a, 0,1.;
            else
                sol=T0+ U*(-b +sqrt(delta))/a;

            Matrix<FloatType,5,1> N1,N2,V= rayMat*sol;  // V=rayMat* {t,h,1}
            N1 = m_alignedMat1 * V;
            N2 = m_alignedMat2 * V;
//            cout << m_name <<"  "<< N1.transpose() <<endl << N2.transpose() <<endl<<endl;
            if(normal)
            {
                *normal= (N2.head(3)*N1(3)-N1.head(3)*N2(3)).normalized();
                //cout << "normal:" << normal->transpose() << endl;
            }
            ray.moveTo(sol(0)).rebase();
            return ray.position();
        }

        Matrix<FloatType,2,Dynamic> sols;

        try {
            nsols= ToroidSolver(sols, Mat1, Mat2);
        }catch(RayException & excpt) {
            throw RayException(excpt.what()+"\nin "+m_name+": ToroidSolver called from ", __FILE__, __func__, __LINE__);
        }

      //  cout <<"solver has " << nsols << " solution\n";
        int minNormIndex;
        switch (nsols)
        {
        case 0 :
            //cout << "no solution found\n";
            ray.m_alive=false;
            return ray.position();
            break;
        case 2:
    //        cout << "The intersection problem has 2 solutions:\n" <<  sols << endl;
    //        cout << "squared norm"  <<sols.colwise().squaredNorm() <<endl;
    //        break;
        case 4:
    //        cout << "The intersection problem has 4 solutions:\n" <<  sols << endl;
    //        cout << "squared norm"  <<sols.colwise().squaredNorm() <<endl;
            break;
        default:
            cout << "Abnormal number of solutions " << nsols <<endl;
            throw RayException("Abnormal intercept number", __FILE__, __func__, __LINE__);
        }

        // ici je dois avoir retrouvé toutes les valeurs possibles de t et h  // le choix doit il porter sur la norme ou sur t ?
        sols.colwise().squaredNorm().minCoeff(&minNormIndex);
        ray.moveTo(sols(0,minNormIndex)).rebase();

      //  cout << "toroid intercept OK\n";

        // calcul de la normale
        if(normal)
        {
            Matrix<FloatType,5,1> N1,N2,V= rayMat*sols.col(minNormIndex).homogeneous();  // V={t,h,1}

            N1 = m_alignedMat1 * V;
            N2 = m_alignedMat2 * V;
            Matrix<FloatType,3,1> NN= (N2*N1(3)-N1*N2(3)).segment(0,3);
            *normal=NN.normalized();


        }
     //   cout << "toroid normal OK\n";
    //    cout << "intercept computation time :" << duration_cast<microseconds>(clock.now()-start).count() << " usec\n" ;

    #ifdef DIAG_OUTPUT
        QueryPerformanceCounter(&EndingTime);
        ElapsedMicroseconds.QuadPart = (EndingTime.QuadPart - StartingTime.QuadPart)*1000000;

        cout << "intercept computation time :" << ElapsedMicroseconds.QuadPart/Frequency.QuadPart << " usec\n" ;

        cout << "Intercept " <<  ray.position().transpose() << endl;
        if(normal)
            cout << "Normal    " <<  normal->transpose() << endl;
    #endif // DIAG_OUTPUT

        return ray.position();
    }
    catch( EigenException & excpt)
    {
        cout<< "Eigen exception in toroid::intercept line "  << __LINE__ <<endl << excpt.what() <<endl;
        exit(-1);
    }
}
