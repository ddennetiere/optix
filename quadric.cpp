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


Quadric::Quadric():Surface(false, "Quadric"){} //{Surface::m_transmissive=false;}


Quadric::~Quadric(){}


EIGEN_DEVICE_FUNC  RayBaseType::VectorType Quadric::intercept(RayType& ray, VectorType * normal)
{
    ray-=m_translationFromPrevious; // change ref frame from previous to this surface

    if(ray.m_alive)
    {
        ray.moveTo(-ray.direction().dot(ray.origin()) ).rebase();   // move and rebase close to the nearest of quadric apex  (0 point)

        RayBaseType::HomogeneousType MX=m_alignedQuadric*ray.origin().homogeneous();
        RayBaseType::HomogeneousType MU=m_alignedQuadric*ray.direction().homogeneous();

       FloatType a=ray.direction().homogeneous().dot(MU);

        FloatType r=ray.direction().homogeneous().dot(MX) / a ;
        FloatType q=ray.origin().homogeneous().dot(MX) /(a*r*r);
        if(q>1)
             ray.m_alive=false;
        else
        {
            FloatType distance= -r* oneMinusSqrt1MinusX(q);

            *normal=-(MX+ distance*MU).segment(0,3).normalized();
            ray.moveTo(distance).rebase();
        }
    }
    return ray.position();
}
