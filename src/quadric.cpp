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


RayBaseType::VectorType Quadric::intercept(RayBaseType& ray, VectorType * normal)
{
    // ray-=m_translationFromPrevious; // change since 19/06/2024 DO NOT ref frame from previous to this surface

    if(ray.m_alive)
    {
        ray.moveTo(-ray.direction().dot(ray.origin()) ).rebase();   // move and rebase close to the nearest of quadric apex  (0 point)

        // la 4° composante de U doit être nulle !!
        RayBaseType::HomogeneousType X=ray.origin().homogeneous();
        RayBaseType::HomogeneousType U=ray.direction().homogeneous();
        U[3]=0;

        RayBaseType::HomogeneousType MX=m_alignedQuadric*X;
        RayBaseType::HomogeneousType MU=m_alignedQuadric*U;

        FloatType a=U.dot(MU);
        if(a==0)
            throw InterceptException("Quadric: U.dot(Mu)==0",__FILE__,__func__, __LINE__);
        FloatType r=U.dot(MX) / a ;
        if(r==0)
            throw InterceptException("Quadric: U.dot(Mx)==0",__FILE__,__func__, __LINE__);
        FloatType q=X.dot(MX) /(a*r*r);

        if(q>1)
             ray.m_alive=false;
        else
        {
            FloatType distance= -r* oneMinusSqrt1MinusX(q);
            if(normal)
                *normal=-(MX+ distance*MU).segment(0,3).normalized();  // extraction de la direction de l'equation du plan tangent
            ray.moveTo(distance).rebase();
        }
    }

    return ray.position();
}
