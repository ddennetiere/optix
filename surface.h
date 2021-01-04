/**
*************************************************************************
*   \file       surface.h
*
*   \brief     Surface base class and surface parameter class definition file
*
*
*
*
*   \author             François Polack  <francois.polack@synchroton-soleil.fr>
*   \date               Creation : 2020-10-05
*   \date               Last update:
*
*   \mainpage   OptiX
*       An X-ray optics librray
*       this is an &alpha; version of the library
*
*   \defgroup enums  Enumeration list
*      \brief  enumerated values used in the library
*
*
 ***************************************************************************/

#ifndef _SURFACE_H
#define   _SURFACE_H

#include <string>
#include <vector>
#include <map>

#include "types.h"
#include "files.h"


using namespace std;

class SourceBase;

inline double oneMinusSqrt1MinusX(double x)
{
    if(::fabs(x) > 0.01) return 1.-sqrt(1. - x);
    return  x*(0.5 + x*(0.125 + x*(0.0625 + x*(0.0390625 + x*(0.02734375 + x*(0.0205078125 + x*0.01611328125))))));
}


/** \brief Abstract base class of all optical surfaces
*
*       Surface is the base class of both shape definition classes like Plane, Quadric, Toroid
*       and behaviour type classes like Transparency, Mirror, Grating.
*     \n Optical element classes might be derived from an agregates of these classes, but they MUST refer to the same surface object.
*      Hence these derived objects must declare the base Surface ùmember as virtual
*
*     General parameters common to all surfaces
*     -----------------------------------------
*
*   Name of parameter | Description
*   -------------- | --------------
*   \b distance | Distance to the preceeding surface
*   \b theta | Chief ray half-deviation
*   \b phi | Roatation angle of the surface reference frame around the incident chief ray
*   \b psi | Roatation angle of the surface reference frame around its normal
*   \b Dtheta | Correction to the incidence angle theta
*   \b Dphi | Correction to phi rotation angle
*   \b Dpsi | Correction to in-plane rotation angle psi
*   \b DX | X offset in surface reference frame
*   \b DY | Y offset in surface reference frame
*   \b DZ | Z offset in surface reference frame
 */
class Surface {
protected:
    typedef map<string,Parameter>::iterator ParamRef;
    typedef map<string, string>::iterator HelpstrRef;
public:
   // typedef Matrix<FloatType,3,1> VectorType;
    typedef RayBaseType::VectorType VectorType;
    typedef Matrix<FloatType,3,3> RotationType;  // peut-être à déplacer dans le namespace Surface (public)
    typedef Transform<FloatType,3,Affine> IsometryType;  // c'est quandmeme une isometrie

//    Surface();/**<  \brief default creator make the surface as a film  */
    virtual ~Surface(){}    /**< \brief virtual destructor */

    Surface(bool transparent=true, string name="", Surface* previous=NULL); /**< \brief default  constructor (Film) with explicit chaining to previous */

    virtual inline string getRuntimeClass(){return "Surface";}/**< \brief return the derived class name of this object */

    EIGEN_DEVICE_FUNC virtual VectorType intercept(RayType& ray, VectorType * normal=NULL )=0; /**< \brief Pure virtual function
    *
    *   Implemented in shape classes (Plane , Quadric, Toroid)
    *   \n All mplementations <b> must move and rebase </b> the ray at its intersection with the surface and return this position
    */

    virtual RayType& transmit(RayType& ray);       /**<  \brief  ray transmission and impact storage. \n To be reimplemented in derived class
            *   \param ray the input ray in previous element exit space \return the transmitted ray in exit space of this element
            *
            *  this implementation moves the ray to the surface and rebase it there
            *   All implementations <b> must take care of recording impacts </b> in entrance or exit plane as required */

    virtual RayType& reflect(RayType& ray);    /**< \brief Ray reflection and impact storage. \n To be reimplemented in derived class
            *    \param ray the input ray inprevious element exit space  \return the reflected ray in exit space of this element
            *
            *    This implementation simply reflect the ray on the tangent plane
            *   All implementations <b> must take care of recording impacts </b> in entrance or exit plane as required */

    inline void setRecording(RecordMode rflag){m_recording=rflag;} /**< \brief Sets the impact recording mode for the surface */
    inline RecordMode getRecording(){return m_recording;} /**< \brief Gets the impact recording mode of the surface */

/** \brief  call transmit or reflect according to surface type ray and iterate to the next surface*/
    inline void propagate(RayType& ray) /**< the propagated ray */
    {
        if(m_transmissive)
            transmit(ray);
        else
            reflect(ray);
        if(m_next!=NULL)
            m_next->propagate(ray);
    }

    void clearImpacts();    /**< \brief  clear the impact vector of all elements of the surface chain starting from this element*/

    void reserveImpacts(int n); /**< \brief  reserve space for élements in the impact vector of all elements of the surface chain starting from this element to avoid reallocations
                                *    \param  n the  number of elements to reserve t*/

    inline void setName(string&& name ){m_name=name;} /**< \brief sets the element name for scripting */
    inline string getName(){return m_name;}    /**<  \brief retrieves the element name for scripting */

    /** \brief defines the preceeding element in the active chain
    *
    *  \param previous pointer to the previous element; if NULL  the element should be a  source otherwise the chain will not be activable */
    virtual inline void setPrevious(Surface* previous)  { // virtual because sources need a special treatment
        if(m_previous == previous) return;
        if(m_previous) m_previous->m_next=NULL;
        if(previous)
        {
            if(previous->m_next)
                previous->m_next->m_previous=NULL;
            m_previous->m_next=this;
        }
        m_previous=previous;
    }

    /** \brief defines the next element in the active chain
    *
    *  \param next pointer to the next element; if  left NULL  the element will be the last of the active chain*/
    inline void setNext(Surface* next)
    {
        if(m_next == next) return;
        if(m_next)  m_next->m_previous=NULL;
        if(next)
        {
            if(next->m_previous)
                next->m_previous->m_next=NULL;
            m_next->m_previous=this;
        }
        m_next=next;
    }

    /** \brief removes this element- from the active chain
    */
    inline void remove()
    {
        if(m_previous) m_previous->m_next=m_next;
        if(m_next) m_next->m_previous=m_previous;
        m_previous=m_next=NULL;
    }

    /** \brief align the element accordg to the parameters and return 0 if OK otherwise a return code
    *
    *       \param wavelength the alinment wavelength (used by chromatic elements only)

    *       alignement defines the transformations matrices needed for ray propagation computations
    *       This is a default implementation which will be overridden for gratings and maybe other elements
    *       \n If the surface is reflective \b theta is the half-deflection angle of the chief ray,
    *       if it is transmissive, the chief ray direction is not modified and theta represents the surface tilt angle of normal to chief ray)
    *       \n In all cases \b Phi defines the frame rotation auround the incident chief ray from entry to exit.
    *       When the surface is reflective the incidence plane is the YZ plane of the rotated frame and XZ is tangent to the surface
    *       \n \b Psi is always the rotation around the surface normal (in case of transmissive a
    */
    virtual int align(double wavelength);/**< \brief Align this surface with respect to the main incident ray and defines the related geometric space transforms
                            *   \return  0 if alignment  is OK ; -1 if a grating can't be aligned */
    int alignFromHere(double wavelength);/**< \brief Align the whole surface chain;  stops at first error and return the error code
                            *    \return  0 if OK; the error code which stopped the alignment */



    bool isAligned();/**< \brief  returns true if all elements of the surface chain starting from here are aligned, false otherwise */

     bool isSource();/**< \brief Checks whether this element is a source (i.e. has a radiate() function )*/
     Surface* getSource(); /**< \brief explore the element string to find the most upstream source from here  \return the most upstream source or NULL if there is none */


    /** \brief Modifies an existing  named numeric parameter
    *
    * The type and group of a parameter are internally defined and cannot be changed. Their actual values will be reflected in param on returning
    * \param name the name of parameter to set
    * \param param the new parameter  object
    * \return  true if parameters was changed , false if parameter doesn't belong to the object
    */
    inline bool setParameter(string name, Parameter& param)
    {
        ParamRef it=m_parameters.find(name);
        if (it !=m_parameters.end())
        {
            param.type=it->second.type; // type and group of a parameter must not be modified
            param.group=it->second.group;
            it->second=param;
            m_isaligned=false;
            return true;
        }
        else
            return false;
    }

    /** \brief retrieves a numeric parameter by its name
    */
    inline bool getParameter(string name, Parameter& param)
    {
        ParamRef it=m_parameters.find(name);
        if (it !=m_parameters.end())
        {
            param=it->second;
            return true;
        }
        else
            return false;
    }

    inline map<string,Parameter>::iterator parameterBegin(){return m_parameters.begin();}/**< \brief return an iterator positionned on the first element of the parameter list of this surface*/
    inline map<string,Parameter>::iterator parameterEnd(){return m_parameters.end();}  /**< \brief return an iterator positionned after the last element of the parameter list of this surface*/

    /** \brief return true if inserted returne false if helpstring is already existing
    */
    static inline bool setHelpstring(string paramName, string helpstring)
    {
        pair<HelpstrRef,bool> result=m_helpstrings.insert(make_pair(paramName, helpstring));
        return result.second;
    }

    /** \brief return an empty string if entry doesn't exist but doesn't create it
    */
    static inline void getHelpstring(string paramName, string& helpstring)
    {
        HelpstrRef it=m_helpstrings.find(paramName);
        if(it !=m_helpstrings.end())
            helpstring=it->second;
        else
            helpstring.erase();
    }

    static void setHelpstrings();  /**< \brief sets help strings of all parameters used by this object */

    EIGEN_DEVICE_FUNC inline IsometryType& exitFrame(){return m_exitFrame;}  /**< \brief returns a reference to the space transform from laboratory oriented frame to exit space of this element */

    /** \brief get the impacts and directions of the set of of rays propagated from the source
     *
     * \param impacts a vector of  propagated rays rays in this object recording space. Ray position is the ray intercept with the surface
     * \param frame  The type of frame where impacts must be referred to. Can be *AlignedLocalFrame, *SurfaceFrame, *GeneralFrame or *LocalAbsoluteFrame
     * \return int  The number of lost rays in propagation from source
     */
    int getImpacts(vector<RayType> &impacts, FrameID frame);

    /** \brief Computes and fills-up a SpotDiagram object from the internally stored impact collection
     * \param spotDiagram a SpotDiagram object reference which wiill be uptated on return
     * \param distance the distance from this surface along the alignment ray where the observation plane is located
     * \return the number of stored impacts
     */
    int getSpotDiagram(SpotDiagram& spotDiagram, double distance=0);

    /** \brief Computes and fills-up a CausticDiagram object from the internally stored impact collection
     *
     * The CausticDiagram consists in the point of each ray wich is the closest to the central alignment ray
     * \param causticData a CausticDiagram object reference which wiill be uptated on return
     * \return the number of stored impacts
     */    int getCaustic(CausticDiagram& causticData);

     int getWavefrontData(SpotDiagram& WFdata, double distance=0);
     EIGEN_DEVICE_FUNC MatrixXd getWavefontExpansion(double distance, Index Nx, Index Ny, Array22d& XYbounds);

    friend TextFile& operator<<(TextFile& file,  Surface& surface);  /**< \brief Duf this Surface object to a TextFile, in a human readable format  */

    friend TextFile& operator >>(TextFile& file,  Surface& surface);  /**< \brief Retrieves a Surface object from a TextFile  */

protected:
    static FloatType m_FlipSurfCoefs[];
    static map<string, string> m_helpstrings;  /**< \brief  parameter description help strings  */
    static int m_nameIndex;
    vector<RayType> m_impacts; /**<  \brief the ray impacts on the surfaces in forward or backward element space */


    /** \brief Creates and sets a new named numeric parameter
    */
    inline ParamRef defineParameter(string name, Parameter& param)    {
        pair<ParamRef,bool> result= m_parameters.insert(make_pair(name,param));
        if(!result.second)
          result.first->second=param;
        m_isaligned=false;  //maybe some changes do not require this
        return result.first;
    }
    inline void removeParameter(string name){ m_parameters.erase(name);}/**< \brief removes a tagged parameter */

//  surface definition primary parameters
    string m_name;   /**< \brief  an identification name for calling this surface from scripts  */
    map<string,Parameter> m_parameters;   /**<  \brief tagged parameters of the surface.  */
    bool m_transmissive;   /**< \brief  flag defining whether ray propagation should call the surface transmit() or reflect()  function */
    RecordMode m_recording; /**<  \brief flag defining wether or not the ray impacts on this surace are recorded and in forward or backward space   */
    Surface* m_previous; /**<  \brief the previous element in the surface chain */
    Surface* m_next; /**<  \brief the next element in the surface chain */


// surface secondary parameters build during alignment

    IsometryType m_exitFrame; /**< \brief    the local ref frame  expressed in absolute coordinates as a space transformation.
    *
    *     Linear part is the  pure rotation matrix of the base vectors  (scaling is == 1, To get it, use linear() not rotation(), since computation of eigen values is useless);
    *     \n Translation  part is the frame shift from source.
    * \n To avoid small differences of large numbers, computations are done
    *   in an local straighten frame ie. non rotated with respect to lab frame, with origin at the alignment rays intersection.
    * \n The transformation from previous to this, is a pure translation of chief ray direction and distanceToPrevious  */

    IsometryType m_surfaceDirect; /**<  \brief fait passer la surface du repère surface (= de définition ie.normale selonZ ) au repère local redressé */
    IsometryType m_surfaceInverse;  /**<  \brief ramène  du repère local redressé  au  repère surface ( normale à S selon Z) */
    VectorType m_translationFromPrevious;   /**< \brief Translation  from previous local fraee to this local frame */
    RotationType m_frameDirect;  /**<  \brief rotation part of the transform expressing absolute frame coordinates from local frame ccordinates
    *
    *     == m_exitFrame.linear()   le doublon est-il utile ?*/
    RotationType m_frameInverse; /**<  \brief rotation part of transform  absolute --> outFrame      */

    bool m_isaligned;  /**< \brief flag tracking if the surfacee need to be realigned unset when a parameter is changed */


//	rayon pole_norm;
//	rayon aux;
//	FLOAT sigmapentelong;
//	FLOAT sigmapentetransv;


};

#endif











