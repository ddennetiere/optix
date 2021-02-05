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
#include "toroid.h"


/** \brief  Mirror template class
 */
template<class SShape>   // pour ne pas confobndre avec Eigen::Shape
class Mirror: public SShape
{
public:
    Mirror(string name="" ,Surface * previous=NULL):Surface(false,name, previous) {}
    virtual ~Mirror(){}
    virtual inline string  getRuntimeClass(){return string("Mirror<")+SShape::getRuntimeClass()+">"; }
    inline int align(double wavelength)
    {
        int rcode= SShape::setFrameTransforms(wavelength);  // this call defines the space transforms
        if(!rcode)
          return SShape::align(wavelength); // this call transforms the surface equation
        else return rcode;
    }
} ;

typedef Mirror<Plane> PlaneMirror;
typedef Mirror<Sphere> SphericalMirror ;
typedef Mirror<Cylinder> CylindricalMirror;
typedef Mirror<Toroid> ToroidalMirror;

template<class SShape>
class Film: public SShape
{
public:
    Film(string name="" ,Surface * previous=NULL):Surface(true,name, previous){SShape::setRecording(RecordInput);}
    virtual ~Film(){}
    virtual inline string  getRuntimeClass(){return string("Film<")+SShape::getRuntimeClass()+">"; }
    inline int align(double wavelength)
    {
        int rcode= SShape::setFrameTransforms(wavelength);  // this call defines the space transforms
        if(!rcode)
          return SShape::align(wavelength); // this call transforms the surface equation
        else return rcode;
    }
} ;

typedef Film<Plane> PlaneFilm;
typedef Film<Sphere> SphericalFilm ;
typedef Film<Cylinder> CylindricalFilm;
typedef Film<Toroid> ToroidalFilm;


#endif // OPTICALELEMENTS_H_INCLUDED
