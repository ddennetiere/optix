#ifndef OPTICALELEMENTS_H_INCLUDED
#define OPTICALELEMENTS_H_INCLUDED

////////////////////////////////////////////////////////////////////////////////
/**
*      \file           opticalelements.h
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

template<class Shape>
class Film: public Shape
{
public:
    Film(string name="" ,Surface * previous=NULL):Surface(true,name, previous) {}
    virtual ~Film(){}
    virtual inline string  getRuntimeClass(){return string("Film<")+Shape::getRuntimeClass()+">"; }
    inline int align(double wavelength)
    {
        int rcode= Surface::align(wavelength);  // this call defines the space transforms
        if(!rcode)
          return Shape::align(wavelength); // this call transforms the surface equation
        else return rcode;
    }
} ;

typedef Film<Plane> PlaneFilm;
typedef Film<Sphere> SphericalFilm ;
typedef Film<Cylinder> CylindricalFilm;


#endif // OPTICALELEMENTS_H_INCLUDED
