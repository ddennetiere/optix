////////////////////////////////////////////////////////////////////////////////
/**
*      \file           mirrors.cpp
*
*      \brief         Mirror template instanciation
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
#include "mirrors.h"

template class Mirror<Plane>;
template class Mirror<Sphere>;
template class Mirror<Cylinder>;
