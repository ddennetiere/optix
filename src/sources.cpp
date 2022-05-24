////////////////////////////////////////////////////////////////////////////////
/**
*      \file        sources.cpp
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
#include "sources.h"
# include <random>

//   ----------------   XYGridSource implementation   --------------------------

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

/** \todo if needed iterate the generation of upstream sources. */
int XYGridSource::generate(const double wavelength, const char polar)
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

    RayType::ComplexType Samp, Pamp;
    if(polar=='S')
        Samp=1., Pamp=0;
    else if(polar=='P')
        Samp=0, Pamp=1.;
    else if(polar=='R')
        Samp=sqrt(2.), Pamp=complex<double>(0,sqrt(2.));
    else if(polar=='L')
        Samp=sqrt(2.), Pamp=complex<double>(0,-sqrt(2.));
    else
        throw ParameterException("invalid polarization (S, P, R or L only are  allowed)", __FILE__, __func__, __LINE__);


    VectorXd vXprim=VectorXd::LinSpaced(ntXprim, -Xprim, nXprim==1? 0 : Xprim);  // When LinSpaced size parameter is set to 1, it returns a vector of length 1 containing 'high' is returned.
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
              m_impacts.emplace_back(RayBaseType(org,dir),wavelength,Samp,Pamp); // amplitude set to 1 and S polar by default
          }
      }

    m_OPDvalid=false;
    return nRays;
}


//   ----------------   RadialGridSource implementation   --------------------------


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

int RadialGridSource::generate(const double wavelength, const char polar)
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

    RayType::ComplexType Samp, Pamp;
    if(polar=='S')
        Samp=1., Pamp=0;
    else if(polar=='P')
        Samp=0, Pamp=1.;
    else if(polar=='R')
        Samp=sqrt(2.), Pamp=complex<double>(0,sqrt(2.));
    else if(polar=='L')
        Samp=sqrt(2.), Pamp=complex<double>(0,-sqrt(2.));
    else
        throw ParameterException("invalid polarization (S, P, R or L only are  allowed)", __FILE__, __func__, __LINE__);


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
              m_impacts.emplace_back(RayBaseType(org,dir),wavelength,Samp,Pamp); // amplitude set to 1 and S polar by default
          }
        }
       }
      }

    m_OPDvalid=false;
    return nRays;
}



//   ----------------   GaussianSource implementation   --------------------------


GaussianSource::GaussianSource(string name ,Surface * previous):Surface(true,name, previous)
{
   // cout << "creating Gaussian source " << name << endl;
    Parameter param;
    param.type=Dimensionless;
    param.group=SourceGroup;
    param.value=1000.;
    param.flags=NotOptimizable;
    defineParameter("nRays", param);  // 1000 points par défaut

    param.type=Distance;
    param.value=0;
    param.flags=0;
    defineParameter("sigmaX", param);  //
    defineParameter("sigmaY", param);  // default sigma source = 0 (source ponctuelle)


    param.type=Angle ;
    param.value=0.35e-3;
    defineParameter("sigmaXdiv", param); //
    defineParameter("sigmaYdiv", param); // default round source dsigma div = 500 µrad


    setHelpstring("nRays", " number of rays to be generated");  // complete la liste de infobulles de la classe Surface
    setHelpstring("sigmaX", "RMS source size in X direction");
    setHelpstring("sigmaY", "RMS source size in Y direction");
    setHelpstring("sigmaXdiv", "RMS source divergence in X direction");
    setHelpstring("sigmaYdiv", "RMS source divergence in y direction");

}

int GaussianSource::generate(const double wavelength, const char polar)
{
    int nRays;
    FloatType sigmaX, sigmaY, sigmaXprim, sigmaYprim;

    Parameter param;
    getParameter("nRays",param);
    nRays=lround(param.value);
    if(nRays<1)
        nRays=1;
    getParameter("sigmaX", param);
    sigmaX=param.value;
    getParameter("sigmaY", param);
    sigmaY=param.value;
    getParameter("sigmaXdiv", param);
    sigmaXprim=param.value;
    getParameter("sigmaYdiv", param);
    sigmaYprim=param.value;

    RayType::ComplexType Samp, Pamp;
    if(polar=='S')
        Samp=1., Pamp=0;
    else if(polar=='P')
        Samp=0, Pamp=1.;
    else if(polar=='R')
        Samp=sqrt(2.), Pamp=complex<double>(0,sqrt(2.));
    else if(polar=='L')
        Samp=sqrt(2.), Pamp=complex<double>(0,-sqrt(2.));
    else
        throw ParameterException("invalid polarization (S, P, R or L only are  allowed)", __FILE__, __func__, __LINE__);


    normal_distribution<FloatType> gaussX(0.,sigmaX);

    normal_distribution<FloatType> gaussY(0., sigmaY);

    normal_distribution<FloatType> gaussXprim(0., sigmaXprim);

    normal_distribution<FloatType> gaussYprim(0., sigmaYprim);

    reserveImpacts(m_impacts.size() + nRays);
    VectorType org=VectorType::Zero(),dir=VectorType::Zero();

    if(nRays==1) // retourne le rayon axial
    {
        org <<0, 0, 0;
        dir << 0, 0, 1.L;
        m_impacts.push_back(RayType(RayBaseType(org,dir),wavelength,Samp,Pamp)); // amplitude set to 1 and S polar by default
        return nRays;
    }
    random_device rd;
    // if not clean enough use a Mersenne twister as mt19937 gen{rd()};
    for(int i=0; i<nRays; ++ i)
    {
        org <<0, 0, 0;
        if(sigmaX > 0)
            org(0)=gaussX(rd);
        if(sigmaY >0)
            org(1)=gaussY(rd);
        dir << 0, 0, 1.L;
        if(sigmaXprim > 0)
            dir(0)=gaussXprim(rd);
        if(sigmaYprim > 0)
            dir(1)=gaussYprim(rd);
        dir.normalize();
        m_impacts.push_back(RayType(RayBaseType(org,dir),wavelength,Samp,Pamp)); // amplitude set to 1 and S polar by default
    }
    m_OPDvalid=false;
    return nRays;
}





//   ----------------   AstigmaticGaussianSource implementation   --------------------------


AstigmaticGaussianSource::AstigmaticGaussianSource(string name ,Surface * previous):Surface(true,name, previous)
{
    cout << "creating Astigmatic gaussian source " << name << endl;
    Parameter param;
    param.type=Dimensionless;
    param.group=SourceGroup;
    param.value=1000.;
    param.flags=NotOptimizable;
    defineParameter("nRays", param);  // 1000 points par défaut

    param.type=Distance;
    param.value=0;
    param.flags=0;
    defineParameter("sigmaX", param);  //
    defineParameter("sigmaY", param);  // default sigma source = 0 (source ponctuelle)
    defineParameter("waistX", param);  //
    defineParameter("waistY", param);  // default sigma source = 0 (source ponctuelle)


    param.type=Angle ;
    param.value=0.35e-3;
    defineParameter("sigmaXdiv", param); //
    defineParameter("sigmaYdiv", param); // default round source dsigma div = 500 µrad


    setHelpstring("nRays", " number of rays to be generated");  // complete la liste de infobulles de la classe Surface
    setHelpstring("sigmaX", "RMS source size in X direction");
    setHelpstring("sigmaY", "RMS source size in Y direction");
    setHelpstring("waistX", "distance of X waist to the source plane");
    setHelpstring("waistY", "distance of Y waist to the source plane");
    setHelpstring("sigmaXdiv", "RMS source divergence in X direction");
    setHelpstring("sigmaYdiv", "RMS source divergence in y direction");

}

int AstigmaticGaussianSource::generate(const double wavelength, const char polar)
{
    int nRays;
    FloatType sigmaX, sigmaY, sigmaXprim, sigmaYprim, waistX, waistY;

    Parameter param;
    getParameter("nRays",param);
    nRays=lround(param.value);
    if(nRays<1)
        nRays=1;
    getParameter("sigmaX", param);
    sigmaX=param.value;
    getParameter("sigmaY", param);
    sigmaY=param.value;
    getParameter("sigmaXdiv", param);
    sigmaXprim=param.value;
    getParameter("sigmaYdiv", param);
    sigmaYprim=param.value;
    getParameter("waistX", param);
    waistX=param.value;
    getParameter("waistY", param);
    waistY=param.value;

    RayType::ComplexType Samp, Pamp;
    if(polar=='S')
        Samp=1., Pamp=0;
    else if(polar=='P')
        Samp=0, Pamp=1.;
    else if(polar=='R')
        Samp=sqrt(2.), Pamp=complex<double>(0,sqrt(2.));
    else if(polar=='L')
        Samp=sqrt(2.), Pamp=complex<double>(0,-sqrt(2.));
    else
        throw ParameterException("invalid polarization (S, P, R or L only are  allowed)", __FILE__, __func__, __LINE__);


    normal_distribution<FloatType> gaussX(0.,sigmaX);

    normal_distribution<FloatType> gaussY(0., sigmaY);

    normal_distribution<FloatType> gaussXprim(0., sigmaXprim);

    normal_distribution<FloatType> gaussYprim(0., sigmaYprim);

    reserveImpacts(m_impacts.size() + nRays);
    VectorType org=VectorType::Zero(),dir=VectorType::Zero();

    if(nRays==1) // retourne le rayon axial
    {
        org <<0, 0, 0;
        dir << 0, 0, 1.L;
        m_impacts.push_back(RayType(RayBaseType(org,dir),wavelength)); // amplitude set to 1 and S polar
        return nRays;
    }
    random_device rd;
    // if not clean enough use a Mersenne twister as mt19937 gen{rd()};
    for(int i=0; i<nRays; ++ i)
    {
        dir << 0, 0, 1.L;
        if(sigmaXprim > 0)
            dir(0)=gaussXprim(rd);
        if(sigmaYprim > 0)
            dir(1)=gaussYprim(rd);
        dir.normalize();

        org <<0, 0, 0;
        if(sigmaX > 0)
            org(0)=gaussX(rd)-waistX*dir(0);
        if(sigmaY >0)
            org(1)=gaussY(rd)-waistY*dir(1);

        m_impacts.push_back(RayType(RayBaseType(org,dir),wavelength,Samp,Pamp)); // amplitude set to 1 and S polar by default
    }

    m_OPDvalid=false;

    return nRays;
}

