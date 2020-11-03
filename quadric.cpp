////////////////////////////////////////////////////////////////////////////////
/**
*      \file           quadric.cpp
*
*      \brief         Quadric surface class implementation file
*
*      \author         François Polack <francois.polack@synchroton-soleil.fr>
*      \date        2020-10-14  Creation
*      \date        Last update
*
*/
///////////////////////////////////////////////////////////////////////////////////
//
//             REVISIONS
//
////////////////////////////////////////////////////////////////////////////////////

#include "quadric.h"


Quadric::Quadric():Surface(true, "Quadric"){} //{Surface::m_transmissive=false;}


Quadric::~Quadric(){}


EIGEN_DEVICE_FUNC  RayBaseType::VectorType Quadric::intercept(RayType& ray, VectorType * normal)
{
    ray-=m_translationFromPrevious; // change ref frame from previous to this surface

    ray.moveTo(-ray.direction().dot(ray.origin()) ).rebase();   // move and rebase close to the nearest of quadric apex  (0 point)

    RayBaseType::HomogeneousType MX=m_alignedQuadric*ray.origin().homogeneous();
    RayBaseType::HomogeneousType MU=m_alignedQuadric*ray.direction().homogeneous();
    FloatType b=ray.direction().homogeneous().dot(MX);
    FloatType delta=b*b - (ray.direction().homogeneous().transpose()*MU).dot((ray.origin().homogeneous().transpose()*MX));
    if(delta < 0)
        throw RayException("No intercept found", __FILE__, __func__, __LINE__);
    FloatType distance= b>0 ? b-sqrt(delta) : b+sqrt(delta); // return the closest to 0
    *normal=(MX+ distance*MU).segment(0,3).normalized();

    return ray.moveTo(distance).position();
}
