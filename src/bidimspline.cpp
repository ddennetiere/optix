/**
*************************************************************************
*   \file           bidimspline.cpp
*
*   \brief             implementation file
*
*
*
*   \author             François Polack  <francois.polack@synchroton-soleil.fr>
*   \date               Creation: 2023-06-30
*   \date               Last update: 2023-06-30
****************************************************************************/

#include "bidimspline.h"
#include "wavefront.h"

bool SplineSolve(SparseMatrix<double,ColMajor> &A,const Ref<VectorXd> &B, Ref<VectorXd> C)
{
    A.makeCompressed();  // pas sûr que ce soit utile

    SparseLU<SparseMatrix<double, ColMajor>, COLAMDOrdering<int > >  solver;
    // Compute the ordering permutation vector from the structural pattern of A
    solver.analyzePattern(A);
    // Compute the numerical factorization
    solver.factorize(A);
    if(solver.info()!=Success)
    {
        std::cout <<solver.info() <<std::endl;
        if(solver.info()==NumericalIssue )
            throw std::runtime_error(solver.lastErrorMessage());

    }

    //Use the factors to solve the linear system

    C = solver.solve(B);

     return B.isApprox(A*C);//,1E-6);

}

int BidimSpline::basisFunctions(Axis axe, double u, MatrixXd& C, bool deriv)
{
    const int& K=degree;
    int K1=K+1 ; // ordre de la fonction +1   ie dimension des vecteurs et matrices
    int K1s=K1*K1;
    ArrayXd &mu=(axe==X) ? muX :muY;
    int i=span(axe,u),j,n,p;

    double * N= new double[K1s];
    double *psrc, *pdest, *pc, *pnm;
    double v0,q0;

    memset((void*)N, 0, K1s*sizeof(double));
    N[0]=1.;

    if(deriv)
        C.setZero(K1,2); // C est column major on veut des coefs consécutifs pour chaque function
    else
       C.setZero(K1,1);

    for (p=1, pnm=N ; p<K1; p++, pnm+=K1)
    {
        for (n=p ; n>0; n--)
        {
            j=i+n;
            v0=mu[j]-mu[j-p];
            if(!(v0==0.))
            {
                pdest=psrc=pnm+n-1;
                pdest+=K1;

                *(pdest++)+=(mu[j]-u)/v0 * *psrc;
                *pdest += (u-mu[j-p])/v0 * *psrc;

                if (deriv && p==K ) // assignation de la dérivée dans C
                {
                    q0=p/v0;

                    pc=C.data()+K1+n;
                    *(pc--) +=q0 * *psrc;
                    *pc -=q0 * *psrc;
                }
            }
        }
    }

//	Assignation de la fonction dans C
    for(psrc=N+K*K1,pc=C.data(), n=K1; n>0 ; psrc++,pc++,n-- )
        *pc=*psrc;
    delete[] N;


    return i;
}



ArrayXXd BidimSpline::basisFunctions(Axis axe, double u, Index* interval, bool deriv)
{
    const int& K=degree;
    int K1=K+1 ; // ordre de la fonction +1   ie dimension des vecteurs et matrices
    int K1s=K1*K1;
    ArrayXd &mu=(axe==X) ? muX :muY;
    int i=span(axe,u),j,n,p;

    double * N= new double[K1s];
    double *psrc, *pdest, *pc, *pnm;
    double v0,q0;

    memset((void*)N, 0, K1s*sizeof(double));
    N[0]=1.;
    ArrayXXd C;
    if(deriv)
        C.setZero(K1,2); // C est column major on veut des coefs consécutifs pour chaque function
    else
       C.setZero(K1,1);

    for (p=1, pnm=N ; p<K1; p++, pnm+=K1)
    {
        for (n=p ; n>0; n--)
        {
            j=i+n;
            v0=mu[j]-mu[j-p];
            if(!(v0==0.))
            {
                pdest=psrc=pnm+n-1;
                pdest+=K1;

                *(pdest++)+=(mu[j]-u)/v0 * *psrc;
                *pdest += (u-mu[j-p])/v0 * *psrc;

                if (deriv && p==K ) // assignation de la dérivée dans C
                {
                    q0=p/v0;

                    pc=C.data()+K1+n;
                    *(pc--) +=q0 * *psrc;
                    *pc -=q0 * *psrc;
                }
            }
        }
    }

//	Assignation de la fonction dans C
    for(psrc=N+K*K1,pc=C.data(), n=K1; n>0 ; psrc++,pc++,n-- )
        *pc=*psrc;
    delete[] N;

    *interval=i;
    return C;
}


MatrixXd BidimSpline::interpolator(Axis axe, const Ref< ArrayXd> & uval, MatrixXd & deriv)
{
    Index colsize=axe ==X? m_controlValues.rows() : m_controlValues.cols();
    if( colsize < 4)
        throw runtime_error("bidimspline initialization error");
    Index K1= degree+1;

    MatrixXd value=MatrixXd::Zero(colsize, uval.size());
    deriv.setZero(colsize, uval.size());
    MatrixXd C;

    for(Index i=0; i < uval.size(); ++i)
    {
        Index p=basisFunctions(axe,uval(i), C, true)-degree;
        value.col(i).segment(p,K1)=C.col(0);
        deriv.col(i).segment(p,K1)=C.col(1);
    }
    return value;
}

int   BidimSpline::basisFunctionDerivatives(Axis axe, double u, Ref<ArrayXXd> C)
{
    const int& K=degree;
    int K1=K+1 ; // ordre de la fonction +1   ie dimension des vecteurs et matrices
    int K1s=K1*K1, kk1=K*K1;
    ArrayXd &mu=(axe==X) ? muX :muY;
    int i=span(axe,u), j,l,n,p,cpsz;

    double * N= new double[K1s], *A= new double [K1s];
    double *pn, *pc, *pa,  *pnsrc, *pndest, *pnm;
    double v0,q0;
//    vector col;
    cpsz=K1s*sizeof(double);
    memset((void*)N, 0, cpsz);
    N[0]=1.;

    C.Zero(K1,K1);
    C.data()[kk1]=1.;

    for (p=1, pnm=N ; p<K1; p++, pnm+=K1)   // p : K  valeurs  [1, K]  croissant
    {
        memcpy((void*)A, (void*)C.data(),cpsz);
        memset((void*)C.data(), 0, cpsz);

        for (n=p; n>0; n--)   // n : p  valeurs  [1, p],  décroissant , n-p >=0
        {
            j=i+n;
            v0=mu[j]-mu[j-p]; // segment [i+n-p, i+n]    inclus dans [i, i+K]
            if(!(v0==0.))
            {
                pndest=pnsrc=pnm+n-1;
                pndest+=K1;

                *(pndest++)+=(mu[j]-u)/v0 * *pnsrc;
                *pndest+=(u-mu[j-p])/v0 * *pnsrc;

                q0=p/v0;

            //  assignation des dérivées dans C
                for (l=0, pa=A+kk1+n-1, pc=C.data()+kk1+n; l<p; l++, pa-=K1, pc-=K1)
                {
                    *(pc--)+=q0 * *pa;
                    *(pc++)-=q0 * *pa;
                }
            }
        }
//      Assignation de la fonction dans c

        for(pn=N+p*K1,pc=C.data()+(K-p)*K1, n=0; n< K1; pn++,pc++,n++ )
            *pc=*pn;

    }
    delete[] A;
    delete[] N;
    return i;
}




void BidimSpline::buildControlPoints(const Ref<ArrayXd> &Xvalues, const Ref<ArrayXd> &Yvalues,
                        const Ref<ArrayXXd> &Zvalues)
{
    typedef Triplet<double> Tdbl;
    // il faut construire des bases Uniformes  Z.rows) -1 intervalles en X et Z.cols()-1 intervalles en Y
    if (Zvalues.rows() != Xvalues.size() || Zvalues.cols() != Yvalues.size() )
        throw std::runtime_error("input spline fitting point arrays incorrectly matched");
    Index n=Zvalues.size(),nx=Zvalues.rows(), ny=Zvalues.cols(), mx=muX.size()-degree, my=muY.size()-degree,
            bXrange=muX.size()-2*degree-1, bYrange=muY.size()-2*degree-1, nsp=degree+1, nsp2=nsp*nsp;
    if (n < mx*my )
        throw std::runtime_error("Too few data points. The linear system is under defined");
    Index i, ix,iy, p, q, j, k, r,s;
    VectorXd splineX, splineY;
    SparseMatrix<double,ColMajor> A(n,mx*my);
    std::vector< Tdbl > vcoefs;  // non zero coefs
    vcoefs.reserve(n*nsp2);
    for(iy=0, i=0; iy<ny; ++iy)
    {
        VectorXd splineY=basisFunctions(Y,Yvalues(iy),&q, false);
        q-=degree;
        if( q<0 || q>bYrange )
            throw std::runtime_error("indexing error");
        for(ix=0; ix<nx; ++ix, ++i)
        {
            splineX=basisFunctions(X,Xvalues(ix),&p, false);  // splineX(Y)est un vecteur colonne
            p-=degree;
            if(p<0 || p>bXrange )
                throw std::runtime_error("indexing error");
            for(k=0, s=q*mx; k <nsp; ++k,s+=mx)
                for(j=0,r=s+p;j <nsp; ++j,++r)
                    vcoefs.push_back(Tdbl(i,r,splineX(j)*splineY(k)));
        }
    }
    A.setFromTriplets(vcoefs.begin(), vcoefs.end());
    VectorXd C(n);
    VectorXd B=Zvalues.reshaped().matrix();// this flatten the matrices but copy seems needed
    if(!SplineSolve(A,B,C) )
        throw std::runtime_error("Resolution of spline fitting system failed");
    m_controlValues=C.reshaped(nx,ny);


}


 ArrayXXd BidimSpline::uniformSpaced1DSolve(const Ref<ArrayXXd>& input)
 {
     int N= input.rows()-1;  // this is the number of spline intervals
     // define auxiliary vectors
     ArrayXd D(input.rows()); // the new first diagonal after substitution
     ArrayXXd Phi(input.rows(), input.cols());  //The new right hand side  after substitution
     ArrayXXd Coeffs(N+3, input.cols()); // the array of computed spline coefficients
     Block<ArrayXXd> C=Coeffs.middleRows(1, N+1);
     // Iteration loop to compute the 1st diagonal
     D(0)=-1./3.;
     Phi.row(0)=2./3.*input.row(0);
     D(1)=0.25;
     Phi.row(1)=1.5*(input.row(1) -0.25*Phi.row(0));

     for(int i=2; i <N-1; ++i)
     {
         D(i)=1/(4.-D(i-1));
         Phi.row(i)=D(i)*(6.*input.row(i)-Phi.row(i-1));
     }
     double cc=7.-2*D(N-2);
     D(N-1)=3./cc;
     Phi.row(N-1)=2./cc*(6.*input.row(N-1)-Phi.row(N-2));

     C.row(N)=(Phi.row(N-1)+2.*input.row(N))/(3.+D(N-1));
     C.row(N-1)=3.*C.row(N)-2.*input.row(N);

     for(int i=N-2; i >=0; --i)
        C.row(i)=Phi.row(i)-D(i)*C.row(i+1);

     // in Coefs we have to set the rows 0 and N+2 respectively with rows 0 and N of the input
     Coeffs.row(0)= input.row(0);
     Coeffs.row(N+2)=input.row(N);

     return Coeffs;

 }

 ArrayXXd BidimSpline::uniformSpaced1DSolveT(const Ref<ArrayXXd>& input)
 {
     int N= input.cols()-1;  // this is the number of spline intervals
     // define auxiliary vectors
     ArrayXd D(input.cols()); // the new first diagonal after substitution
     ArrayXXd Phi(input.rows(), input.cols());  //The new right hand side  after substitution
     ArrayXXd Coeffs(N+3, input.rows()); // the array of computed spline coefficients (transposed)
     Block<ArrayXXd> C=Coeffs.middleRows(1, N+1);

     // Iteration loop to compute the 1st diagonal
     D(0)=-1./3.;
     Phi.col(0)=2./3.*input.col(0);
     D(1)=0.25;
     Phi.col(1)=1.5*(input.col(1) -0.25*Phi.col(0));

     for(int i=2; i <N-1; ++i)
     {
         D(i)=1/(4.-D(i-1));
         Phi.col(i)=D(i)*(6.*input.col(i)-Phi.col(i-1));
     }
     double cc=7.-2*D(N-2);
     D(N-1)=3./cc;
     Phi.col(N-1)=2./cc*(6.*input.col(N-1)-Phi.col(N-2));

     C.row(N)=(Phi.col(N-1)+2.*input.col(N))/(3.+D(N-1));
    // beware Eigen doen't like algebra on 1D arrays of different orientation
     C.row(N-1)=3.*C.row(N)-2.*input.col(N).transpose();


     for(int i=N-2; i >=0; --i)
        C.row(i)=Phi.col(i).transpose()-D(i)*C.row(i+1);

     // in Coefs we have to set the rows 0 and N+2 respectively with rows 0 and N of the input
     Coeffs.row(0)= input.col(0);
     Coeffs.row(N+2)=input.col(N);

     return Coeffs;

 }


void BidimSpline::setFromGridData(Array22d& limits, const Ref<ArrayXXd>& gridData )
{
    if(degree!=3)
    throw ParameterException("This function is only available for cubic B-spline interpolation. Degree MUST BE 3 ", __FILE__, __func__, __LINE__);
    setUniformKnotBase(X,gridData.rows()-1,limits(0,0), limits(1,0));

    setUniformKnotBase(Y,gridData.cols()-1,limits(0,1), limits(1,1));

    ArrayXXd temp=uniformSpaced1DSolveT(gridData);
    m_controlValues=uniformSpaced1DSolveT(temp);

}

Array22d BidimSpline::getLimits()
{
    Array22d limits;
    limits(0,0)=muX(0);
    limits(0,1)=muY(0);
    limits(1,0)=muX(muX.size()-1);
    limits(1,1)=muY(muY.size()-1);
    return limits;
}


ArrayXXd BidimSpline::getLegendreFit(int Nx, int Ny, SurfaceStats* pStats)
{
     // grid interpolated surface is not saved we need to recover it.
    ArrayXd xval=ArrayXd::LinSpaced(muX.size()-2*degree, muX(0),muX(muX.size()-1));
    ArrayXd yval=ArrayXd::LinSpaced(muY.size()-2*degree, muY(0),muY(muY.size()-1));

    MatrixXd derivx, interx=interpolator(X, xval, derivx);
    MatrixXd derivy, intery=interpolator(Y, yval, derivy );

    MatrixXd surface= interx.transpose()* m_controlValues*intery;

    ArrayXXd coeffs=LegendreFitGrid(Nx,Ny, surface);
    if (pStats)
    {
        pStats->sigma=sqrt(surface.squaredNorm());
        pStats->sigmaPrimX=sqrt((derivx.transpose()* m_controlValues*intery).squaredNorm());
        pStats->sigmaPrimY=sqrt((interx.transpose()* m_controlValues*derivy).squaredNorm());
    }

    return LegendreNormalize(coeffs);
}
