////////////////////////////////////////////////////////////////////////////////
/**
*      \file           cylinder.cpp
*
*      \brief         Cylinder  surface class implementation
*
*      \author         François Polack <francois.polack@synchroton-soleil.fr>
*      \date        2020-10-29  Creation
*      \date        Last update
*
*/
///////////////////////////////////////////////////////////////////////////////////
//
//             REVISIONS
//
////////////////////////////////////////////////////////////////////////////////////
#include "cylinder.h"

Cylinder::Cylinder()
{
    Parameter param;
    param.type=InverseDistance;
    param.group=ShapeGroup;
    defineParameter("curvature", param);  // courbure par défaut 0
    defineParameter("axis_angle", param); // angle=0  Tangential cylinder
    setHelpstring("curvature", "Curvature (1/Rc) of the sphere");  // complete la liste de infobulles de la classe Surface
    setHelpstring("axis_angle", "Orientation of the axis 0= tangential cylinder ");
}

void Cylinder::createSurface()
{
    Parameter param;
    Matrix<FloatType,2,1> V;
    if(!getParameter("axis_angle", param) )
       throw ParameterException("axis_angle parameter not found", __FILE__, __func__, __LINE__);
    V(0)=cosl(param.value);
    V(1)=sinl(param.value);
    if(!getParameter("curvature", param) )
       throw ParameterException("Curvature parameter not found", __FILE__, __func__, __LINE__);
    if(param.value==0)  //cylindre degénéré en plan Z=0
    {
        m_localQuadric.setZero();
        m_localQuadric(2,2)=1.L;
    }
    else  /**< \todo cas degenéré inutile avec nouvelle definition */
    {
        //V*=param.value;
        m_localQuadric.block(0,0,2,2)=V*V.transpose()*param.value;
        m_localQuadric(3,2) =m_localQuadric(2,3)=-1.L; //  param.value;
        m_localQuadric(3,3)=0; //passe par 0
    }
}
