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
*   \defgroup elemClasses Instantiable optical elements classes
*      \brief  The limited list of class names which can be used to create optical elements in OptiX
*
*   The CreateElement() function of the C interface and the C++ internal function CreateElementObject() recognize two names for each instantiable optical element class.
*   A template style name, Name<class1[,class2]>, or a capitalized name with internal capital letters (camel case).
*   The GetElementType() C function calls ElementBase::getOptixClass(), and  always returns the template style name.
*
*   The actual underlying class is not always templated. When it is a template the template style name is the internal class name, and the simple name is a C++ typedef.
*   When the underlying class is not templated, e.g. case of sources, the templated style name is an alias only recognized by the CreateElement() / CreateElementObject() functions.
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
#include "ConicBaseCylinder.h"
#include "RevolutionQuadric.h"
#include "gratingbase.h"
#include "Poly1D.h"
#include "holo.h"



typedef map<ElementBase *, ElementBase*> ElementCopyMap;

/** \brief Structure maintaining an equivalence between a duplicated element chain and the original one.
 *  \n This structure is intended for computing propagation in parallel through different alignment conditions while optimizing
 */
struct ChainCopy
{
    ElementBase * First=NULL; /**< the first element of the chain */
    ElementCopyMap copyMap;/**< mapping  original element pointers to copied element pointers */
    ~ChainCopy();   /**<  structure destructor: deletes all the copied elements */
};


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
    /** \brief Constructor and default constructor
     * \param name name of the new element (empty name by default)
     * \param previous pointer to the previous element (NULL by default)
     */
    Mirror(string name="" ,Surface * previous=NULL):Surface(false,name, previous) {}
    virtual ~Mirror(){}  /**< \brief destructor */

    /** \brief \brief reimplemented from ElementBase
     *  retrieves the class of the template implementation namely MirrorM<SShape>
    */
    virtual inline string  getOptixClass(){return string("Mirror<")+SShape::getOptixClass()+">"; }

    /** \brief reimplemented from ElementBase
     *
     *  align the element by setting the associated space transforms, then defining the appropriate surface equation of SShape class
     * \param wavelength the alignment wavelength
     * \return int 0 if successful; an error code otherwise
     */
    inline int align(double wavelength)
    {
        int rcode= SShape::setFrameTransforms(wavelength);  // this call defines the space transforms
        if(!rcode)
          return SShape::align(wavelength); // this call transforms the surface equation
        else return rcode;
    }
} ;

/** \ingroup elemClasses
* \{
*/

typedef Mirror<Plane> PlaneMirror;          /**<  \brief Implements a Mirror, the Surface of which has a Plane shape */
typedef Mirror<Sphere> SphericalMirror ;    /**<  \brief Implements a Mirror, the Surface of which has a Sphere shape */
typedef Mirror<Cylinder> CylindricalMirror; /**<  \brief Implements a Mirror, the Surface of which has a Cylinder shape \n The unrotated surface is curved in the tangential plane */
typedef Mirror<Toroid> ToroidalMirror;      /**<  \brief Implements a Mirror, the Surface of which has a Toroid shape \n The Major curvatur of the unrotated surface is in the tangential plane*/
typedef Mirror<ConicBaseCylinder> ConicBaseCylindricalMirror; /**< \brief Implements a Mirror, the Surface of which has a ConicBaseCylinder shape \n The directrix the unrotated surface is a conic in the tangential plane */
typedef Mirror<RevolutionQuadric> RevolutionQuadricMirror; /**< \brief Implements a Mirror, the Surface of which has a RevolutionQuadric shape \n The foci of the unrotated surface ly in the sagittal plane*/


/** \} */  //end of ingroup bloc


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
    /** \brief Constructor and default constructor
     * \param name name of the new element (empty name by default)
     * \param previous pointer to the previous element (NULL by default)
     */
    Film(string name="" ,Surface * previous=NULL):Surface(true,name, previous){SShape::setRecording(RecordInput);}
    virtual ~Film(){}   /**< \brief destructor */
    virtual inline string  getOptixClass(){return string("Film<")+SShape::getOptixClass()+">"; }/**< \brief reimplemented from ElementBase
            * retrieves the class of the template implementation namely Film<SShape>  */

    /** \brief reimplemented from ElementBase
     *
     *  align the element by setting the associated space transforms, then defining the appropriate surface equation of SShape class
     * \param wavelength the alignment wavelength
     * \return int 0 if successful; an error code otherwise
     */
    inline int align(double wavelength)
    {
        int rcode= SShape::setFrameTransforms(wavelength);  // this call defines the space transforms
        if(!rcode)
          return SShape::align(wavelength); // this call transforms the surface equation
        else return rcode;
    }
} ;


/** \ingroup elemClasses
* \{
*/

typedef Film<Plane> PlaneFilm;           /**<  \brief Implements a Film, the Surface of which has a Plane shape */
typedef Film<Sphere> SphericalFilm ;     /**<  \brief Implements a Film, the Surface of which has a Sphere shape */
typedef Film<Cylinder> CylindricalFilm;  /**<  \brief Implements a Film, the Surface of which has a Cylinder shape with generatrix in the Y direction*/
typedef Film<Toroid> ToroidalFilm;       /**<  \brief Implements a Film, the Surface of which has a Toroid shape and main curvature in X plane*/
typedef Film<ConicBaseCylinder> ConicBaseCylindricalFilm; /**< \brief Implements a Film, the Surface of which has a ConicBaseCylinder shape \n The directrix the unrotated surface is a conic in the tangential plane */
typedef Film<RevolutionQuadric> RevolutionQuadricFilm; /**< \brief Implements a Film, the Surface of which has a RevolutionQuadric shape \n The foci of the unrotated surface ly in the sagittal plane*/

/** \} */  //end of group

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
    virtual inline string  getOptixClass(){return string("Grating<")+PatternType::getOptixClass()+
                        ","+SShape::getOptixClass()+">"; }
    inline int align(double wavelength)
    {
        int rcode= GratingBase::setFrameTransforms(wavelength);  // this call defines the space transforms and should call PatternType::GratingVector()
        if(rcode==0)
          return SShape::align(wavelength); // this call transforms the surface equation
        else return rcode;
    }
    /** \brief Change the parameter and modifies the SShape or Pattern internal parameters if needed
     *
     * \param name parameter name
     * \param param Parameter data
     * \return true if success; false if failure; Last error is set in this case
     * \todo catch parameter error
     */
    inline bool setParameter(string name, Parameter& param)
    {
        bool result;
        //Modif FP 2021-08-09
        // la classe ElementBase qui contient la liste (seule et unique) des paramètres est commune à SShape et PatternType
        //Mais les fonctions setParameter de chaque classe et non de ZlementBasee  doivent être appelées pour  définir correctement les variables membres
     //   if(!result)  // le paramètre n'appartient pas à la classe PatternType

     //   Indispensable de mettre à jour la classe surface (Shape) avant la classe pattern
        result=SShape::setParameter(name,param);  // thow Parameter error

        result|=PatternType::setParameter(name, param);
        if(!result)
        {
            cout <<  "Parameter "<< name << " was not set \n";
            SetOptiXLastError("invalid grating parameter",__FILE__,__func__);
        }
        return result;
    }

} ;


/** \ingroup elemClasses
* \{
*/

typedef Grating<Holo,Plane> PlaneHoloGrating;           /**< \brief Implements a \ref Holo "holographic"  Grating drawn on a Plane surface*/
typedef Grating<Holo,Sphere> SphericalHoloGrating;      /**< \brief implements a \ref Holo "holographic"  Grating drawn on a \ref Sphere "spherical" surface */
typedef Grating<Holo,Cylinder> CylindricalHoloGrating;  /**< \brief implements a \ref Holo "holographic"  Grating drawn on a \ref  Cylinder "cylindrical" surface */
typedef Grating<Holo,Toroid> ToroidalHoloGrating;       /**< \brief implements a \ref Holo "holographic"  Grating drawn on a \ref Toroid "toroidal" surface */
typedef Grating<Holo,ConicBaseCylinder> ConicBaseCylinderHoloGrating;   /**< \brief implements a \ref Holo "holographic"  Grating drawn on a \ref ConicBaseCylinder "ConicBaseCylindrical" surface */
typedef Grating<Holo, RevolutionQuadric> RevolutionQuadricHoloGrating;  /**< \brief implements a \ref Holo "holographic"  Grating drawn on a \ref RevolutionQuadric "RevolutionQuadric" surface */
typedef Grating<Poly1D,Plane> PlanePoly1DGrating;       /**< \brief implements a \ref Poly1D "1D polynomial"  Grating drawn on a Plane surface */
typedef Grating<Poly1D,Sphere> SphericalPoly1DGrating;  /**< \brief implements a \ref Poly1D "1D polynomial"  Grating drawn on a \ref Sphere "spherical" surface */
typedef Grating<Poly1D,Cylinder> CylindricalPoly1DGrating; /**< \brief implements a \ref Poly1D "1D polynomial"  Grating drawn on a \ref  Cylinder "cylindrical" surface */
typedef Grating<Poly1D,Toroid> ToroidalPoly1DGrating;      /**< \brief implements a \ref Poly1D "1D polynomial"  Grating drawn on a \ref Toroid "toroidal" surface */
typedef Grating<Poly1D,ConicBaseCylinder> ConicBaseCylinderPoly1DGrating;  /**< \brief implements a \ref Poly1D "1D polynomial" Grating drawn on a \ref ConicBaseCylinder "ConicBaseCylindrical" surface */
typedef Grating<Poly1D, RevolutionQuadric> RevolutionQuadricPoly1DGrating; /**< \brief implements a \ref Poly1D "1D polynomial" Grating drawn on a \ref RevolutionQuadric "RevolutionQuadric" surface */

/** \} */

/** \brief  Creates a new element of the given type and returns it as a pointer to the base class  ElementBase
 *  \ingroup GlobalCpp
 * \param s_type The composite type of the element to be created
 * \param name The name of the element
 * \return  pointer to the base class  ElementBase; Throw ElementException if s_type is invalid
 */
 ElementBase* CreateElementObject(string s_type, string name);

/** \brief Change grating type from reflection type transmission type and vice versa
 *  \ingroup GlobalCpp
 *
 * \param pelem ElementBase* a pointer to the grating to modify (will fail if element is not a grating)
 * \param trans = true to define a transmission grating and false for a reflection grating
 * \return true for success , false for failure
 *
 * by default gratings are created reflective
 */
inline  bool MakeGratingTransmissive(ElementBase* pelem, bool trans=true )
{
   if(pelem->getOptixClass().compare(0,8,"Grating<" )!=0)
        return false;
   pelem->setTransmissive(trans);
   return true;
}

/** \brief makes a deep copy of the source element
 *  \ingroup GlobalCpp
 * \param source a pointer to the element to copy
 * \return a pointer to the new created  copy
 *
 *   The new element is created on the heap. Links to previous, next and parent elements are set to NULL. The original source element is left unchanged.
 */
ElementBase* ElementCopy(ElementBase* source);

/** \brief Duplicate all the elements of a chain with proper linkage
 *  \ingroup GlobalCpp
 * \param source a pointer to he first element of the chain to duplicate
 * \param newChain reference to a ChainCopy structure
 * \return true if copy was successful; false if an error occurred before all elements were copied.
 *
 * Duplicate creates an image of the chained optical system, which can be used for parallel computation  during optimization for instance.
 * \n This image is deleted, with all elements,  when the returned ChainCopy structure is deleted. If creation fails, some element copies may have been created.
 *  They  will be recorded into the  newChain ChainCopy object,and deleted with it.
 */
bool DuplicateChain(ElementBase * source, ChainCopy& newChain);

/** \brief Replace an element of an optical system chain by an element of another optical type
 *  \ingroup GlobalCpp
 * \param elem a pointer to the original element to be replaced
 * \param newType the runtime class of the replacing element.
 * \return a pointer to a new element of the requested type, created on the heap (must be explicitly deleted).
 *
 * A new element is created and replace the original one in the element chain to which it belonged. The new element will have the name of the original one.
 * The original element is destroyed.
 * \n This operation is only possible on element created on the heap (new operator or CreateElementObject())
 */
ElementBase* ChangeElementType(ElementBase* elem, string newType);

#endif // OPTICALELEMENTS_H_INCLUDED
