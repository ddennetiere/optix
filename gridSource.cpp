////////////////////////////////////////////////////////////////////////////////
/**
*      \file        gridSource.cpp
*
*      \brief         implements the base mechanisms for sources
*
*      \author         François Polack <francois.polack@synchroton-soleil.fr>
*      \date        2020-10-30  Creation
*      \date        Last update
*
*/
///////////////////////////////////////////////////////////////////////////////////
//
//             REVISIONS
//
////////////////////////////////////////////////////////////////////////////////////
#include "gridSource.h"

XYGridSource::XYGridSource(string name ,Surface * previous):Surface(true,name, previous)  // surface non réfléchissante
{
    Parameter param;
    param.type=Angle;
    param.group=SourceGroup;
    param.value=5.e-4;
    defineParameter("divX", param);  // 1/2 divergence Y par défaut 500 µrad
    defineParameter("divY", param);   // 1/2 divergence Z par défaut 500 µrad

    param.type=Distance;
    param.value=0;
    defineParameter("sizeX", param);  // 1/2 field Y  Defaut 0 (source ponctuelle)
    defineParameter("sizeY", param);   // 1/2 field Z

    param.type=Dimensionless;
    param.value=1;
    defineParameter("nXsize", param); //1 seul point source
    defineParameter("nYsize", param);
    param.value=5;
    defineParameter("nXdiv", param); // soit 9 x 9 points
    defineParameter("nYdiv", param);

    setHelpstring("divX", "1/2 divergence in the horiz. plane ");  // complete la liste de infobulles de la classe Surface
    setHelpstring("divY", "1/2 divergence in the vertical plane");
    setHelpstring("nXdiv", "Number of steps in horiz. 1/2 divergence ");
    setHelpstring("nYdiv", "Number of steps in vertical 1/2 divergence ");
    setHelpstring("sizeX", "1/2 source size in the Horiz. plane ");
    setHelpstring("sizeY", "1/2 source size in the vertical plane");
    setHelpstring("nXsize", "Number of steps in horiz. 1/2 size");
    setHelpstring("nYsize", "Number of steps in vertical 1/2 size");
}

/**< \todo if needed iterate the generation of upstream sources. Upstream direction is require since the impacts are cleared before generation*/
int XYGridSource::generate(double wavelength)
{
    Parameter param;
    getParameter("nXdiv",param);
    nXprim=lround(param.value);
    if(nXprim <1) nXprim=1;
    getParameter("nYdiv",param);
    nYprim=lround(param.value);
    if(nYprim <1) nYprim=1;
    getParameter("nXsize",param);
    nX=lround(param.value);
    if(nX <1) nX=1;
    getParameter("nYsize",param);
    nY=lround(param.value);
    if(nY <1) nY=1;

    getParameter("divX", param);
    double Xprim=param.value;
    getParameter("divY",param);
    double Yprim=param.value;
    getParameter("sizeX",param);
    double Xsize=param.value;
    getParameter("sizeY", param);
    double Ysize=param.value;
    int ntXprim=2*nXprim-1;
    int ntYprim=2*nYprim-1;
    int ntX=2*nX-1;
    int ntY=2*nY-1;

    VectorXd vXprim=VectorXd::LinSpaced(ntXprim, -Xprim, nXprim==1? 0 : Xprim);
    VectorXd vYprim=VectorXd::LinSpaced(ntYprim, -Yprim, nYprim==1? 0 : Yprim);
    VectorXd vX=VectorXd::LinSpaced(ntX, -Xsize, nX==1? 0 : Xsize);
    VectorXd vY=VectorXd::LinSpaced(ntY, -Ysize, nY==1? 0 : Ysize);


    int nRays=vXprim.size()*vYprim.size()*vX.size()*vY.size();
    reserveImpacts(m_impacts.size() + nRays);
    VectorType org=VectorType::Zero(),dir=VectorType::Zero();

    for(Index iY=0; iY <ntY; ++iY)
      for(Index iX=0; iX< ntX; ++iX)
      {
        org << vX(iX), vY(iY), 0 ;
        for(Index iYprim=0; iYprim<ntYprim; ++iYprim)
          for(Index iXprim=0; iXprim<ntXprim; ++iXprim)
          {
              dir << vXprim(iXprim), vYprim(iYprim), 1.L;
              m_impacts.emplace_back(RayBaseType(org,dir),wavelength); // amplitude set to 1 and S polar
          }
      }

    return nRays;
}


// Implementation of  RadialGridSource



RadialGridSource::RadialGridSource(string name ,Surface * previous):Surface(true,name, previous)
{
    Parameter param;
    param.type=Angle;
    param.group=SourceGroup;
    param.value=5.e-4;
    defineParameter("divR", param);  // 1/2 divergence Y par défaut 500 µrad

    param.type=Distance;
    param.value=0;
    defineParameter("sizeR", param);  // 1/2 field Y  Defaut 0 (source ponctuelle)


    param.type=Dimensionless;
    param.value=1;
    defineParameter("nRsize", param); //1 seul point source
    param.value=10;
    defineParameter("nRdiv", param);
    defineParameter("nTheta_div", param);

    setHelpstring("divR", "1/2 divergence in the radial direction");  // complete la liste de infobulles de la classe Surface
    setHelpstring("nRdiv", "Number of radial steps in 1/2 divergence");
    setHelpstring("nTheta_div", "Number of azimuth steps in 2 Pi");
    setHelpstring("sizeR", "Radius of the  source");
    setHelpstring("nRsize", "Number of radial steps in source radius ");
    setHelpstring("nTheta_size", "Number of azimuth steps in 2 Pi ");
}

int RadialGridSource::generate(double wavelength)
{
    Parameter param;
    getParameter("nRdiv",param);
    nRprim=lround(param.value)-1;
    if(nRprim <0) nRprim=0;
    getParameter("nTheta_div",param);
    nTprim=lround(param.value);
    if(nTprim <1) nTprim=1;
    getParameter("nRsize",param);
    nR=lround(param.value)-1;
    if(nR <0) nR=0;
    getParameter("nTheta_size",param);
    nT=lround(param.value);
    if(nT <1) nT=1;

    getParameter("divR", param);
    double Rprim=param.value;
    getParameter("sizeR",param);
    double Rsize=param.value;

    ArrayXd tempV=ArrayXd::LinSpaced(nT+1, -M_PI,M_PI );
    ArrayXd SinSize=(tempV.segment(1,nT)).sin();
    ArrayXd CosSize=(tempV.segment(1,nT)).cos();
    tempV=ArrayXd::LinSpaced(nTprim+1, -M_PI,M_PI );
    ArrayXd SinDiv=(tempV.segment(1,nTprim)).sin();
    ArrayXd CosDiv=(tempV.segment(1,nTprim)).cos();

    int nRays=(1+nR*nT)*(1+nRprim*nTprim) ;
    reserveImpacts(m_impacts.size() + nRays);
    VectorType org=VectorType::Zero(),dir=VectorType::Zero();

 //   m_impacts.emplace_back(RayBaseType(org,dir),wavelength); // on axis ray

    for(Index iR=0; iR <=nR; ++iR)
    {
      double Ro=Rsize*sqrt(double(iR)/nR);
      for(Index iT=0; iT< (iR==0) ? 1: nT; ++iT)
      {
        org <<Ro*CosSize(iT), Ro*SinSize(iT), 0;
        for(Index iRprim=0; iRprim<=nRprim; ++iRprim)
        {
          double RoPrim=Rprim*sqrt(double(iRprim)/nRprim);
          for(Index iTprim=0; iTprim< (iRprim==0) ? 1:nTprim; ++iTprim)
          {
              dir << RoPrim*CosDiv(iTprim), RoPrim*SinDiv(iTprim), 1.L;
              m_impacts.emplace_back(RayBaseType(org,dir),wavelength); // amplitude set to 1 and S polar
          }
        }
       }
      }

    return nRays;
}


