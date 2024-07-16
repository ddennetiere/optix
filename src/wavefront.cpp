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
#include <algorithm>

//using namespace std; no longer usable in recet C++ releases

using std::cout, std::endl;


ArrayXXd Legendre(int Norder, const Ref<ArrayXd>& Xpos, ArrayXXd& derivative )
{
//    cout << "bounds in Legendre  " << Xpos.minCoeff() <<"  " << Xpos.maxCoeff() << endl;
    if ((Xpos.maxCoeff() > 1.) || (Xpos.minCoeff() < -1. ))
        throw std::runtime_error("Values in Xpos vector should be in the [-1, +1] range");

    int Xsize=Xpos.size();
    ArrayXXd fvalue(Xsize, Norder);
    derivative.resize(Xsize, Norder);

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


ArrayXXd LegendreIntegrateSlopes(int Nx, int Ny, const Ref<ArrayX4d>& WFdata, const Ref<Array2d>& Xaperture,
                                                   const Ref<Array2d>& Yaperture)
{
//  WFdata Transversa aberration array  stored in aperture points order. Columns are respectively
//      X component of transverse aberration, Y component of transverse aberration.
//      X aperture angle, Y aperture angle

    Index numData=WFdata.rows(), nvars=Nx*Ny-1;
    Index nlines=2*numData, i=0,j=0, k=0;
    double Kx=2./(Xaperture(1)-Xaperture(0));
    double Ky=2./(Yaperture(1)-Yaperture(0));
    double X0=(Xaperture(1)+Xaperture(0))/2.;
    double Y0=(Yaperture(1)+Yaperture(0))/2.;
    ArrayXXd Zcoefs=ArrayXXd::Zero(Nx,Ny);
    ArrayXXd Lx, Ly, LPx, LPy;
    ArrayXd Xnormed=Kx*(WFdata.col(2)-X0), Ynormed=Ky*(WFdata.col(3)-Y0);  // ccordonnées normalisés

    MatrixXd Mat(nlines, Nx*Ny), A;
    VectorXd Vprim(nlines), Rhs;

    Lx=Legendre(Nx,Xnormed, LPx);
    Ly=Legendre(Ny,Ynormed, LPy);

    for(j=0, k=0; j < Ny; ++j)
        for(i=0; i < Nx; ++i, ++k)
        {               //  Derivation normalization factor added 19/03/2023
            Mat.block(0, k, numData, 1)=Kx*LPx.col(i)*Ly.col(j);
            Mat.block(numData, k, numData, 1)=Ky*Lx.col(i)*LPy.col(j);
        }
    Vprim.segment(0, numData)=WFdata.col(0);
    Vprim.segment(numData, numData)=WFdata.col(1);
    // On construit la matrice en supprimant la colonne 0 (facteur constant qui n'a pas d'incidence sur les aberrations transverses à fitter)
    A=Mat.block(0,1,nlines,nvars).transpose()*Mat.block(0,1,nlines,nvars);
    Rhs=Mat.block(0,1,nlines,nvars).transpose()*Vprim;


// la ligne suivante ne compile pas sur ORD01022
    Map<VectorXd>(Zcoefs.data()+1, nvars)=A.lu().solve(Rhs);
//    VectorXd vtemp=A.lu().solve(Rhs);
//    Map<VectorXd> mtemp(Zcoefs.data()+1, nvars);
//    mtemp=vtemp;
    Zcoefs(0,0)=0;
    return Zcoefs;
}


ArrayXXd LegendreFitXYZ(int Nx, int Ny, const Ref<ArrayX3d>& XYZdata,
                                const Ref<Array2d>& Xaperture, const Ref<Array2d>& Yaperture)
{
    Index numData=XYZdata.rows(), nvars=Nx*Ny;
    Index  i=0,j=0, k=0;
    double Kx=2./(Xaperture(1)-Xaperture(0));
    double Ky=2./(Yaperture(1)-Yaperture(0));
    double X0=(Xaperture(1)+Xaperture(0))/2.;
    double Y0=(Yaperture(1)+Yaperture(0))/2.;
    ArrayXXd Zcoefs=ArrayXXd::Zero(Nx,Ny);
    ArrayXXd Lx, Ly, LPx, LPy;
    ArrayXd Xnormed=Kx*(XYZdata.col(0)-X0), Ynormed=Ky*(XYZdata.col(1)-Y0);  // ccordonnées normalisés

    MatrixXd Mat(numData, nvars), A;
    VectorXd Vprim(numData), Rhs;

    Lx=Legendre(Nx,Xnormed, LPx);
    Ly=Legendre(Ny,Ynormed, LPy);

    for(j=0, k=0; j < Ny; ++j)
        for(i=0; i < Nx; ++i, ++k)
            Mat.col(k)=Lx.col(i)*Ly.col(j);

    A=Mat.transpose()*Mat;
    Rhs=Mat.transpose()*XYZdata.matrix().col(2);

    Map<VectorXd>(Zcoefs.data(), nvars)=A.lu().solve(Rhs);

    return Zcoefs;
}

ArrayXXd LegendreFitGrid(int Nx, int Ny, const Ref<ArrayXXd>& griddata)
{
    Index numData=griddata.rows()*griddata.cols(), nvars=Nx*Ny;
    Index  i=0,j=0, k=0;

    ArrayXXd Zcoefs=ArrayXXd::Zero(Nx,Ny);
    ArrayXXd Lx, Ly, LPx, LPy, matLx(griddata.rows(),griddata.cols()), matLy(griddata.rows(),griddata.cols());
    ArrayXd Xnormed=ArrayXd::LinSpaced(griddata.rows(),-1.,1.);
    ArrayXd Ynormed=ArrayXd::LinSpaced(griddata.cols(),-1.,1.);

    MatrixXd Mat(numData, nvars), A;
    VectorXd Vprim(numData), Rhs;
    Lx=Legendre(Nx,Xnormed, LPx);
    Ly=Legendre(Ny,Ynormed, LPy);

    Map<ArrayXd> colLx(matLx.data(),numData,1);
    Map<ArrayXd> colLy(matLy.data(),numData,1);

    for(j=0, k=0; j < Ny; ++j)
        for(i=0; i < Nx; ++i, ++k)
        {
            matLx.colwise()=Lx.col(i);
            matLy.rowwise()=Ly.col(j).transpose();
            Mat.col(k)=colLx*colLy;
        }
    A=Mat.transpose()*Mat;
    Rhs=Mat.transpose()*griddata.matrix().reshaped();

    Map<VectorXd>(Zcoefs.data(), nvars)=A.lu().solve(Rhs);

    return Zcoefs;
}




ArrayXXd  LegendreNormalize(const Ref<ArrayXXd>& coefs)
{
    ArrayXd xnorm= ArrayXd::LinSpaced(coefs.rows(),1., 2.*coefs.rows()-1. );
    ArrayXd ynorm= ArrayXd::LinSpaced(coefs.cols(),1., 2.*coefs.cols()-1. );

    ArrayXXd normCoef=coefs.colwise()/xnorm.sqrt();
    normCoef.rowwise()/=ynorm.sqrt().transpose();
    return normCoef;
}

ArrayXXd  LegendreFromNormal(const Ref<ArrayXXd>& coefs)
{
    ArrayXd xnorm= ArrayXd::LinSpaced(coefs.rows(),1., 2.*coefs.rows()-1. );
    ArrayXd ynorm= ArrayXd::LinSpaced(coefs.cols(),1., 2.*coefs.cols()-1. );

    ArrayXXd natCoef=coefs.colwise()*xnorm.sqrt();
    natCoef.rowwise()*=ynorm.sqrt().transpose();
    return natCoef;
}

ArrayXXd LegendreSurface(const Ref<ArrayXd>& Xpos, const Ref<ArrayXd>& Ypos, const Ref<Array22d>& bounds, const Ref<MatrixXd>& legendreCoefs )
{
    ArrayXXd surface;
    MatrixXd Lx, Ly;
    ArrayXXd deriv;
    double Kx=2./(bounds(1,0)-bounds(0,0));
    double Ky=2./(bounds(1,1)-bounds(0,1));
    double X0=(bounds(1,0)+bounds(0,0))/2.;
    double Y0=(bounds(1,1)+bounds(0,1))/2.;
    ArrayXd Xnormed=Kx*(Xpos-X0), Ynormed=Ky*(Ypos-Y0);


    Lx=Legendre(legendreCoefs.rows(),Xnormed, deriv);
    Ly=Legendre(legendreCoefs.cols(),Ynormed, deriv);
    surface=Lx*legendreCoefs*Ly.transpose();
    return surface;
}

ArrayXXd LegendreSurfaceGrid(int Nx, int Ny, const Ref<MatrixXd>& legendreCoefs )
{
    ArrayXXd surface;
    MatrixXd Lx, Ly;
    ArrayXXd deriv;
    ArrayXd Xgrid=ArrayXd::LinSpaced(Nx,-1., 1.);
    ArrayXd Ygrid=ArrayXd::LinSpaced(Ny,-1., 1.);

    Lx=Legendre(legendreCoefs.rows(),Xgrid, deriv);
    Ly=Legendre(legendreCoefs.cols(),Ygrid, deriv);
    surface=Lx*legendreCoefs*Ly.transpose();
    return surface;
}

ArrayXd Legendre2DInterpolate(const Ref<ArrayXd>& Xpos, const Ref<ArrayXd>& Ypos, const Ref<Array22d>& bounds, const Ref<MatrixXd>& legendreCoefs )
{
    ArrayXXd Zvalues;
    MatrixXd Lx, Ly;
    ArrayXXd deriv;
    double Kx=2./(bounds(1,0)-bounds(0,0));
    double Ky=2./(bounds(1,1)-bounds(0,1));
    double X0=(bounds(1,0)+bounds(0,0))/2.;
    double Y0=(bounds(1,1)+bounds(0,1))/2.;
    ArrayXd Xnormed=Kx*(Xpos-X0), Ynormed=Ky*(Ypos-Y0);


    Lx=Legendre(legendreCoefs.rows(),Xnormed, deriv);
    Ly=Legendre(legendreCoefs.cols(),Ynormed, deriv);
    Zvalues=(Lx*legendreCoefs*Ly.transpose()).diagonal(); // Lazy evaluation enforced  see https://stackoverflow.com/questions/37863668/eigen-use-of-diagonal-matrix/37868577#37868577
    return Zvalues ;

}
