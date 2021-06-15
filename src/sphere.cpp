////////////////////////////////////////////////////////////////////////////////
/**
*      \file           sphere.cpp
*
*      \brief         Spherical surface class implementation
*
*      \author         François Polack <francois.polack@synchroton-soleil.fr>
*      \date        2020-10-28  Creation
*      \date        Last update
*
*/
///////////////////////////////////////////////////////////////////////////////////
//
//             REVISIONS
//
////////////////////////////////////////////////////////////////////////////////////
#include "sphere.h"

Sphere::Sphere()
{
    Parameter param;
    param.type=InverseDistance;
    param.group=ShapeGroup;
    defineParameter("curvature", param);  // courbure par défaut 0
    setHelpstring("curvature", "Curvature (1/Rc) of the sphere");  // complete la liste de infobulles de la classe Surface
}

void Sphere::createSurface()
{
    Parameter cv;
    if(!getParameter("curvature", cv) )
       throw ParameterException("Curvature parameter not found", __FILE__, __func__, __LINE__);
    if(cv.value==0)  //sphère degénérée en plan Z=0
    {
        m_localQuadric.setZero();
        m_localQuadric(2,2)=1.L;
    }
    else        /**< \todo avec cette nouvelle définition cv==0 n'est plus un cas particulier voir aussi cylindre */
    {
        m_localQuadric=RayType::QuadricType::Identity()*cv.value ;//(cv.value*cv.value);
        m_localQuadric(3,3)=0; //passe par 0
        m_localQuadric(3,2)=m_localQuadric(2,3)=-1.L; //cv.value;
    }
}

