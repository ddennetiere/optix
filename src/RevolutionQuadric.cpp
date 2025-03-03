/**
 *************************************************************************
*   \file           RevolutionQuadric.cpp
*
*   \brief             implementation file
*
*
*
*   \author             François Polack  <francois.polack@synchroton-soleil.fr>
*   \date               Creation: 2021-06-23
*   \date               Last update: 2021-06-23
 ***************************************************************************/#include "RevolutionQuadric.h"

#include "RevolutionQuadric.h"
#include <limits>

RevolutionQuadric::RevolutionQuadric()
{
    Parameter param;
    param.type=InverseDistance;
    param.group=ShapeGroup;
    defineParameter("invp", param);
    param.value=-1;  // par défaut = foyer "source" à -1m
    setHelpstring("invp", "inverse distance to the \"source\" focal point");  // complete la liste de infobulles de la classe Surface
    defineParameter("invq", param);
    param.value=-1;  // par défaut = foyer "image"à +1m
    setHelpstring("invp", "inverse distance to the \"image\" focal point");  // complete la liste de infobulles de la classe Surface
    param.type=Angle;
    defineParameter("theta0", param);  //0 par défaut = conique dégénérée en plan Z=0
    setHelpstring("theta0", "Angle of focal rays to tangent plane");  // complete la liste de infobulles de la classe Surface
}

void RevolutionQuadric::createSurface()
{
    Parameter invp, invq, theta0;
    if(!getParameter("invp", invp) )
       throw ParameterException("invp parameter not found", __FILE__, __func__, __LINE__);
    if(!getParameter("invq", invq) )
       throw ParameterException("invq parameter not found", __FILE__, __func__, __LINE__);
    if(!getParameter("theta0", theta0) )
       throw ParameterException("theta0 parameter not found", __FILE__, __func__, __LINE__);
    if(abs(invp.value -invq.value) < numeric_limits<double>::epsilon())
       throw ParameterException("invp must not be equal to invq", __FILE__, __func__, __LINE__);
    double phi, sintheta, dpq1, sgnpq;
    sgnpq=copysign(1., invp.value*invq.value);
    dpq1=invp.value - invq.value;
    sintheta=sin(theta0.value);
    phi=atan2((invp.value + invq.value)*sgnpq*sintheta,dpq1*sgnpq*cos(theta0.value));

    IsometryType Rphi(AngleAxis<FloatType>(phi,VectorType::UnitY()));  // c'est bien une rotation de -phi dans le plan XZ
    RayType::QuadricType tempQ;
    tempQ.setZero();
    tempQ(0,0)=4*invp.value*invq.value*sintheta*sintheta/dpq1;
    tempQ(2,2)=tempQ(1,1)=-dpq1;
    tempQ(0,3)=tempQ(3,0)= -2.*sintheta*sin(phi);
    tempQ(2,3)=tempQ(3,2)= -2.*sintheta*cos(phi);
//    cout<< " invp " << invp.value << " invq " << invq.value << " tan theta0 " << tan(theta0.value)<< endl << "angle Phi " << phi << endl;
//    cout << " Matrice Q sur axes " << endl << tempQ << endl;
//    cout << " Matrice rotation " << endl << Rphi.matrix() << endl;
//    cout << "angle axis phi" <<endl << AngleAxis<FloatType>(phi,VectorType::UnitY()).matrix() << endl;
    m_localQuadric=Rphi.matrix().transpose()*tempQ*Rphi.matrix();
}
