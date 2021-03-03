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
#include "gratingbase.h"
#include "Poly1D.h"
#include "holo.h"


/** \class Mirror
 *  \brief  Mirror template class
 *  \tparam SShape a shape class derived from surface mainly  providing the intercept() function, with  specific définitions of the surface shape
 *
 *  Mirror implements a reflective surface of the given  shape
 */
template<class SShape>   // pour ne pas confondre avec Eigen::Shape
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

typedef Mirror<Plane> PlaneMirror;          /**< implements a reflective plane surface */
typedef Mirror<Sphere> SphericalMirror ;    /**< implements a reflective spherical surface */
typedef Mirror<Cylinder> CylindricalMirror; /**< implements a reflective cylindrical surface */
typedef Mirror<Toroid> ToroidalMirror;      /**< implements a reflective toroidal surface */



/** \class Film
 *  \brief  Film template class
 *  \tparam SShape a shape class derived from surface mainly  providing the intercept() function, with  specific definitions of the surface shape
 *
 *  Film implements a transmissive virtual surface (no optical properties)  of  given  shape, suitable to records ray impacts.
 *  By default impact recording is activated for recording impacts in the entrance frame.
 */
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

typedef Film<Plane> PlaneFilm;           /**< implements a plane film */
typedef Film<Sphere> SphericalFilm ;     /**< implements a spherical film*/
typedef Film<Cylinder> CylindricalFilm;  /**< implements a cylindrical film*/
typedef Film<Toroid> ToroidalFilm;       /**< implements a toroidal film */


/** \class Grating
 *  \brief  Grating template class
 *  \tparam PatternType a class derived from class Pattern providing the grating line density vector at any point of the surface,  together with specific definition parameters
 *  \tparam SShape a shape class derived from class Surface mainly  providing the intercept() function, with  specific définitions of the surface shape
 *
 *  Grating implements a diffractive element depending of 1 parameter (set of lines)
 */
template<class PatternType, class SShape>
class Grating: public GratingBase, public PatternType, public SShape
{
public:
    Grating(string name="" ,Surface * previous=NULL):Surface(false,name, previous){}
    virtual ~Grating(){}
    virtual inline string  getRuntimeClass(){return string("Grating<")+PatternType::getRuntimeClass()+
                        ","+SShape::getRuntimeClass()+">"; }
    inline int align(double wavelength)
    {
        int rcode= GratingBase::setFrameTransforms(wavelength);  // this call defines the space transforms and should call PatternType::GratingVector()
        if(rcode)
          return SShape::align(wavelength); // this call transforms the surface equation
        else return rcode;
    }
    inline bool setParameter(string name, Parameter& param)
    {
        bool result;
        result=SShape::setParameter(name, param);
        if(result)
            result=PatternType::setParameter(name,param);
        if(!result)
            SetOptiXLastError("invalid grating parameter",__FILE__,__func__);
        return result;
    }

} ;

typedef Grating<Holo,Plane> PlaneHoloGrating;
typedef Grating<Holo,Sphere> SphericalHoloGrating;
typedef Grating<Holo,Cylinder> CylindricalHoloGrating;
typedef Grating<Holo,Toroid> ToroidalHoloGrating;
typedef Grating<Poly1D,Plane> PlanePoly1DGrating;
typedef Grating<Poly1D,Sphere> SphericalPoly1DGrating;
typedef Grating<Poly1D,Cylinder> CylindricalPoly1DGrating;
typedef Grating<Poly1D,Toroid> ToroidalPoly1DGrating;

/** \brief  Creates a new element of the given type and returns it as a pointer to the base class  ElementBase
 *  \ingroup GlobalCpp
 * \param s_type The composite type of the element to be created
 * \param name The name of the element
 * \return  pointer to the base class  ElementBase; Throw ElementException id s_type is invalid
 */
 ElementBase* CreateElementObject(string s_type, string name);

/** \brief Change grating type from reflection type transmission type and vice versa
 *  \ingroup GlobalCpp
 *
 * \param pelem ElementBase* a pointer to the grating to modify (will fail if element is not a grating)
 * \return true for success , false for failure
 *
 */
inline  bool MakeGratingTransmissive(ElementBase* pelem)
{
   if(pelem->getRuntimeClass().compare(0,8,"Grating<" )!=0)
        return false;
   pelem->setTransmissive(true);
   return true;
}

#endif // OPTICALELEMENTS_H_INCLUDED
