////////////////////////////////////////////////////////////////////////////////
/**
*      \file           plane.cpp
*
*      \brief         Plane surface class implementation file
*
*      \author         François Polack <francois.polack@synchroton-soleil.fr>
*      \date        2020-10-13  Creation
*      \date        Last update
*
*/
///////////////////////////////////////////////////////////////////////////////////
//
//             REVISIONS
//
////////////////////////////////////////////////////////////////////////////////////
#include "plane.h"


Plane::Plane(string name, Surface *previous):Surface(true,name, previous)//{Surface::m_transmissive=false;}
{
    m_hyperplane=RayType::PlaneType(VectorType::UnitZ(),0); // plan par origine normale OZ
}

Plane::~Plane(){}
