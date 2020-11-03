#ifndef MIRRORS_H_INCLUDED
#define MIRRORS_H_INCLUDED

////////////////////////////////////////////////////////////////////////////////
/**
*      \file           mirrors.h
*
*      \brief         Definition of the mirror elements of the library
*
*      \author         François Polack <francois.polack@synchroton-soleil.fr>
*      \date        2020-10-30  Creation
*      \date         Last update
*

*/
///////////////////////////////////////////////////////////////////////////////////
//
//             REVISIONS
//
////////////////////////////////////////////////////////////////////////////////////

#include "plane.h"
#include "sphere.h"
#include "cylinder.h"


/** \brief  Mirror template class
 */
template<class Shape>
class Mirror: public Shape
{
public:
    Mirror(string name="" ,Surface * previous=NULL):Surface(false,name, previous) {}
    virtual ~Mirror(){}
    virtual inline string  getRuntimeClass(){return string("Mirror<")+Shape::getRuntimeClass()+">"; }
    inline int align(double wavelength)
    {
        int rcode= Surface::align(wavelength);  // this call defines the space transforms
        if(!rcode)
          return Shape::align(wavelength); // this call transforms the surface equation
        else return rcode;
    }
} ;

typedef Mirror<Plane> PlaneMirror;
typedef Mirror<Sphere> SphericalMirror ;
typedef Mirror<Cylinder> CylindricalMirror;

#endif // MIRRORS_H_INCLUDED
