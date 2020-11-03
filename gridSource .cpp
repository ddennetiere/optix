////////////////////////////////////////////////////////////////////////////////
/**
*      \file           sourcebase.cpp
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

XYGridSource::XYGridSource(string name ,Surface * previous):Surface(false,name, previous)
{
    Parameter param;
    param.type=Angle;
    param.group=SourceGroup;
    param.value=5.e-4;
    defineParameter("divY", param);  // 1/2 divergence Y par défaut 500 µrad
    defineParameter("divZ", param);   // 1/2 divergence Z par défaut 500 µrad

    param.type=Distance;
    param.value=0;
    defineParameter("sizeY", param);  // 1/2 field Y  Defaut 0 (source ponctuelle)
    defineParameter("sizeZ", param);   // 1/2 field Z

    param.type=Dimensionless;
    param.value=1;
    defineParameter("nYsize", param); //1 seul point source
    defineParameter("nZsize", param);
    param.value=10;
    defineParameter("nYdiv", param); // soit 21x21 points
    defineParameter("nZdiv", param);

    setHelpstring("divY", "1/2 divergence in the horiz. plane ");  // complete la liste de infobulles de la classe Surface
    setHelpstring("divZ", "1/2 divergence in the vertical plane");
    setHelpstring("nYdiv", "Number of steps in horiz. 1/2 divergence ");
    setHelpstring("nZdiv", "Number of steps in vertical 1/2 divergence ");
    setHelpstring("sizeY", "1/2 source size in the Horiz. plane ");
    setHelpstring("sizeZ", "1/2 source size in the vertical plane");
    setHelpstring("nYsize", "Number of steps in horiz. 1/2 size");
    setHelpstring("nZsize", "Number of steps in horiz. 1/2 size");
}

/**< \todo if needed iterate the generation of upstream sources. Upstream direction is require since the impacts are cleared before generation*/
int XYGridSource::generate(double wavelength)
{
    Parameter param;
    getParameter("nYdiv",param);
    nYprim=lround(param.value);
    if(nYprim <1) nYprim=1;
    getParameter("nZdiv",param);
    nZprim=lround(param.value);
    if(nZprim <1) nZprim=1;
    getParameter("nYsize",param);
    nY=lround(param.value);
    if(nY <1) nY=1;
    getParameter("nZsize",param);
    nZ=lround(param.value);
    if(nZ <1) nZ=1;

    getParameter("divY", param);
    double Yprim=param.value;
    getParameter("divZ",param);
    double Zprim=param.value;
    getParameter("sizeX",param);
    double Ysize=param.value;
    getParameter("sizeZ", param);
    double Zsize=param.value;

    VectorXd vYprim=VectorXd::LinSpaced(2*nYprim-1, -Yprim, nYprim==1? 0 : Yprim);
    VectorXd vZprim=VectorXd::LinSpaced(2*nZprim-1, -Zprim, nZprim==1? 0 : Zprim);
    VectorXd vY=VectorXd::LinSpaced(2*nY-1, -Ysize, nY==1? 0 : Ysize);
    VectorXd vZ=VectorXd::LinSpaced(2*nZ-1, -Zsize, nZ==1? 0 : Zsize);


    int nRays=vYprim.size()*vZprim.size()*vY.size()*vZ.size();
    reserveImpacts(m_impacts.size() + nRays);
    VectorType org=VectorType::Zero(),dir=VectorType::Zero();

    for(Index iZ=0; iZ <nZ; ++iZ)
      for(Index iY=0; iY< nY; ++iY)
      {
        org.segment(1,2) <<vY(iY), vZ(iZ);
        for(Index iZprim=0; iZprim<nZprim; ++iZprim)
          for(Index iYprim=0; iYprim<nYprim; ++iYprim)
          {
              dir.segment(1,2) << vYprim(iYprim), vZprim(iZprim);
              m_impacts.emplace_back(RayBaseType(org,dir),wavelength); // amplitude set to 1 and S polar
          }
      }

    return nRays;
}


// Implementation of  RadialGridSource



RadialGridSource::RadialGridSource(string name ,Surface * previous):Surface(false,name, previous)
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
    setHelpstring("nRdiv", "Number of steps in the radial 1/2divergence");
    setHelpstring("nTheta_div", "Number of azimut steps on divegence  ");
    setHelpstring("sizeR", "Radius of the  source size ");
    setHelpstring("nRsize", "Number of azimut steps ");
    setHelpstring("nTheta_size", "Number of azimut steps on size ");
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
        org.segment(1,2) <<Ro*CosSize(iT), Ro*SinSize(iT);
        for(Index iRprim=0; iRprim<=nRprim; ++iRprim)
        {
          double RoPrim=Rprim*sqrt(double(iRprim)/nRprim);
          for(Index iTprim=0; iTprim< (iRprim==0) ? 1:nTprim; ++iTprim)
          {
              dir.segment(1,2) << RoPrim*CosDiv(iTprim), RoPrim*SinDiv(iTprim);
              m_impacts.emplace_back(RayBaseType(org,dir),wavelength); // amplitude set to 1 and S polar
          }
        }
       }
      }

    return nRays;
}


