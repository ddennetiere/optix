#ifndef _ELEMENT_H
#define   _ELEMENT_H

/**
*************************************************************************
*   \file       elementbase.h
*
*   \brief    definition file of Element class
*
*
*
*
*   \author             François Polack  <francois.polack@synchroton-soleil.fr>
*   \date               Creation : 2021-02-02
*   \date               Last update:
*

 ***************************************************************************/

#include <string>
#include <vector>
#include <map>

#include "types.h"
#include "files.h"


using namespace std;

//class SourceBase;

inline double oneMinusSqrt1MinusX(double x)
{
    if(::fabs(x) > 0.01) return 1.-sqrt(1. - x);
    return  x*(0.5 + x*(0.125 + x*(0.0625 + x*(0.0390625 + x*(0.02734375 + x*(0.0205078125 + x*0.01611328125))))));
}


/** \brief Abstract base class of all optical elements, surfaces and groups
*
*
*
*     Optical element classes might be derived from an agregate with shape and behaviour classes, but they MUST refer to the same Element object.
*      Hence these derived objects must declare the base Element member as virtual
*
*     General parameters common to all optical elements
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
class ElementBase {
protected:
    typedef map<string,Parameter>::iterator ParamRef;
    typedef map<string, string>::iterator HelpstrRef;
public:
   // typedef Matrix<FloatType,3,1> VectorType;
    typedef RayBaseType::VectorType VectorType;
    typedef Matrix<FloatType,3,3> RotationType;  // peut-être à déplacer dans le namespace Element (public)
    typedef Transform<FloatType,3,Affine> IsometryType;  // c'est quandmeme une isometrie

    ElementBase(bool transparent=true, string name="", ElementBase* previous=NULL); /**< \brief default  constructor (Film) with explicit chaining to previous */

    virtual ~ElementBase(){}    /**< \brief virtual destructor */


    virtual inline string getRuntimeClass(){return "Surface";}/**< \brief return the derived class name of this object */

    /** \brief Align this surface with respect to the main incident ray according to the parameters,
    *
    *       \param wavelength the alignment wavelength (used by chromatic elements only)
    *       \return  0 if alignment  is OK ; -1 if a grating can't be aligned
    *
    *    Align defines the actual surface equation in the computation frame. It is implemented in derived clsses
    */
    virtual int align(double wavelength)=0;

/** \brief  call transmit or reflect according to surface type ray and iterate to the next surface*/
    virtual void propagate(RayType& ray)=0 ;/**< \param ray the propagated ray */

    inline void setName(string&& name ){m_name=name;} /**< \brief sets the element name for scripting */
    inline string getName(){return m_name;}    /**<  \brief retrieves the element name for scripting */

    /** \brief defines the preceeding element in the active chain
    *
    *  \param previous pointer to the previous element; if NULL  the element should be a  source otherwise the chain will not be activable */
     inline void setPrevious(ElementBase* previous)  { // virtual qualifier withdrawn //  sources need a special treatment
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

      inline ElementBase* getPrevious()  { return m_previous;}


    /** \brief defines the next element in the active chain
    *
    *  \param next pointer to the next element; if  left NULL  the element will be the last of the active chain*/
    inline void setNext(ElementBase* next)
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

      inline ElementBase* getNext(){ return m_next;}


    /** \brief removes this element- from the active chain
    */
    inline void remove()
    {
        if(m_previous) m_previous->m_next=m_next;
        if(m_next) m_next->m_previous=m_previous;
        m_previous=m_next=NULL;
    }

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

    static void setHelpstrings();  /**< \brief sets help strings all alignment parameters */

    /** \brief setFrameTransforms defines the geometric space transforms related to the position of an element in the various frames
    *
    *       \param wavelength the alignment wavelength (used by chromatic elements only)
    *       \return  0 if alignment  is OK ; -1 if a grating can't be aligned
    *
    *       alignement defines the transformations matrices needed for ray propagation computations
    *       This is a default implementation which will be overridden for gratings and maybe other elements
    *       \n If the surface is reflective \b theta is the half-deflection angle of the chief ray,
    *       if it is transmissive, the chief ray direction is not modified and theta represents the surface tilt angle of normal to chief ray)
    *       \ In all cases \b Phi defines the frame rotation auround the incident chief ray from entry to exit.
    *       When the surface is reflective the incidence plane is the YZ plane of the rotated frame and XZ is tangent to the surface
    *       \n \b Psi is always the rotation around the surface normal (in case of transmissive a
    */
    virtual int setFrameTransforms(double wavelength)  ;

    int alignFromHere(double wavelength);/**< \brief Align the whole surface chain;  stops at first error and return the error code
                            *    \return  0 if OK; the error code which stopped the alignment */

    bool isAligned();/**< \brief  returns true if all elements of the surface chain starting from here are aligned, false otherwise */

    EIGEN_DEVICE_FUNC inline IsometryType& exitFrame(){return m_exitFrame;}  /**< \brief returns a reference to the space transform from laboratory oriented frame to exit space of this element */


    friend TextFile& operator<<(TextFile& file,  ElementBase& elem);  /**< \brief Dump this Element object to a TextFile, in a human readable format  */

    friend TextFile& operator>>(TextFile& file,  ElementBase* elem);  /**< \brief Retrieves a Element object from a TextFile  \todo call interface::createElement(type) */

protected:

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

    static FloatType m_FlipSurfCoefs[];
    static map<string, string> m_helpstrings;  /**< \brief  parameter description help strings  */
    static int m_nameIndex;  /**< \brief Index for automatic naming of surfeces created without a name */
 //   vector<RayType> m_impacts; /**<  \brief the ray impacts on the surfaces in forward or backward element space */



//  surface definition primary parameters
    string m_name;   /**< \brief  an identification name for calling this surface from scripts  */
    ElementBase* m_previous; /**<  \brief the previous element in the surface chain */
    ElementBase* m_next; /**<  \brief the next element in the surface chain */
    map<string,Parameter> m_parameters;   /**<  \brief tagged parameters of the surface.  */

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

    bool m_transmissive;   /**< \brief  flag defining whether ray propagation should call the surface transmit() or reflect()  function */
    bool m_isaligned;  /**< \brief flag tracking if the surfacee need to be realigned unset when a parameter is changed */


};



#endif // header guard

