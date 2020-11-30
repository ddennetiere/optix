////////////////////////////////////////////////////////////////////////////////
/**
*      \file           wavefront.cpp
*
*      \brief         TODO  fill in file purpose
*
*      \author         François Polack <francois.polack@synchroton-soleil.fr>
*      \date        2020-11-29  Creation
*      \date        Last update
*
*/
///////////////////////////////////////////////////////////////////////////////////
//
//             REVISIONS
//
////////////////////////////////////////////////////////////////////////////////////
#include "wavefront.h"
#include <exception>

using namespace std;

EIGEN_DEVICE_FUNC ArrayXXd Legendre(int Norder, const Ref<ArrayXd>& Xpos, Ref<ArrayXXd> derivative )
{
    if (Xpos.maxCoeff() >1. || Xpos.minCoeff() <-1. )
        throw runtime_error("Values in Xpos vector should be in the [-1, +1] range");

    int Xsize=Xpos.size();
    ArrayXXd fvalue(Xsize, Norder);
    double c1,c2;

    fvalue.col(0).setOnes();
    fvalue.col(1)= Xpos;
    derivative.col(0).setZero();
    derivative.col(1).setOnes();
    for(int icol=2; icol <Norder; icol++)
    {
        c1=(2.*icol-1.)/icol;
        c2=double(icol-1)/icol;
        fvalue.col(icol)=c1* Xpos*fvalue.col(icol-1)-c2*fvalue.col(icol-2);
        derivative.col(icol)=c1*(Xpos*derivative.col(icol-1)+fvalue.col(icol-1))-c2*derivative.col(icol-2);
  //      second.col(icol)=c1*(Xpos*second.col(icol-1)+ 2.*derivative.col(icol-1))-c2*second.col(icol-2);  // si dérivée seconde utile (col 0 et 1 == zero)
    }
    return fvalue;
}


EIGEN_DEVICE_FUNC ArrayXXd LegendreIntegrateSlopes(int Nx, int Ny, const Ref<ArrayX4d>& WFdata, const Ref<Array2d>& Xaperture,
                                                   const Ref<Array2d>& Yaperture)
{
    Index numData=WFdata.rows(), nvars=Nx*Ny-1;
    Index nlines=2*numData, i=0,j=0, k=0;
    double Kx=2./(Xaperture(1)-Xaperture(0)), Ky=2/(Yaperture(1)-Yaperture(0));
    double X0=(Xaperture(1)+Xaperture(0))/(Xaperture(1)-Xaperture(0));
    double Y0=(Yaperture(1)+Yaperture(0))/(Yaperture(1)-Yaperture(0));
    ArrayXXd Zcoefs(Nx,Ny);
    ArrayXXd Lx, Ly, LPx, LPy;
    ArrayXd Xnormed=Kx*WFdata.col(0)-X0, Ynormed=Ky*WFdata.col(1)-Y0;

    MatrixXd Mat(nlines, Nx*Ny), A;
    VectorXd Vprim(nlines), Rhs;

    Lx=Legendre(Nx,Xnormed, LPx);
    Ly=Legendre(Ny,Ynormed, LPy);

    for(j=0, k=0; j < Ny; ++j, ++k)
        for(i=0; i < Nx; ++i )
        {
            Mat.block(0, k, numData, 1)=LPx*Ly;
            Mat.block(numData, k, numData, 1)=Lx*LPy;
        }
    Vprim.segment(0, numData)=WFdata.col(2);
    Vprim.segment(numData, numData)=WFdata.col(3);

    A=Mat.block(0,1,nlines,nvars).transpose()*Mat.block(0,1,nlines,nvars);
    Rhs=Mat.block(0,1,nlines,nvars).transpose()*Vprim;

    Map<VectorXd>(Zcoefs.data()+1, nvars)=A.lu().solve(Rhs);
    Zcoefs(0,0)=0;
    return Zcoefs;
}
