/**
 *************************************************************************
*   \file           cone.cpp
*
*   \brief             implementation file
*
*
*
*   \author             François Polack  <francois.polack@synchroton-soleil.fr>
*   \date               Creation: 2023-05-10
*   \date               Last update: 2023-05-10
 ***************************************************************************/#include "cone.h"

Cone::Cone()
{
    Parameter param;
    param.type=InverseDistance;
    param.group=ShapeGroup;
    defineParameter("curvature", param);  // courbure par défaut 0
    defineParameter("directrix_angle", param); // angle=0  Tangential cylinder
    defineParameter("apex_distance", param);
    setHelpstring("curvature", "Curvature (1/Rc) of the directrix circle");  // complete la liste de infobulles de la classe Surface
    setHelpstring("directrix_angle", "inclination angle of the directrix plane wit respect to the YOZ plane ");
    setHelpstring("apex_distance", "Distance from the cone apex to the origin point");
}


void Cone::createSurface()
{
    Parameter param;

    if(!getParameter("apex_distance", param) )
       throw ParameterException("apex_distance parameter not found", __FILE__, __func__, __LINE__);
    double x0=param.value;

    if(!getParameter("directrix_angle", param) )
       throw ParameterException("directrix_angle parameter not found", __FILE__, __func__, __LINE__);
    double costheta=cos(param.value);
    double tantheta=tan(param.value);

    if(!getParameter("curvature", param) )
       throw ParameterException("Curvature parameter not found", __FILE__, __func__, __LINE__);
    double kinv=x0*param.value;

    m_localQuadric.setZero();
    m_localQuadric(1,1)=kinv;
    m_localQuadric(2,2)=kinv/(costheta*costheta)+2*tantheta;
    m_localQuadric(0,2)=m_localQuadric(2,0)=1.;
    m_localQuadric(2,3)=m_localQuadric(3,2)=-x0;
}
