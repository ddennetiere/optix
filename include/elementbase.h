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
*   \date               Last update: 2021-08-09
*

 ***************************************************************************/

#include <string>
#include <vector>
#include <map>
#include <iostream>

#include "types.h"
#include "files.h"


//using namespace std; no longer valid
using std::string;
using std::map;
using std::pair;
using std::vector;

class ElementCollection;
//class SourceBase;

//#define  ERROR_MAXSIZE 1024
//extern char LastError[ERROR_MAXSIZE];
extern string LastError;  // changed 07/06/2023
extern  char noerror[];
extern bool OptiXError;

/** \brief  set the global error message  to b retrieved by from the C interface
 *  \ingroup GlobalCpp
 *
 * \param what the error string
 * \param filename file where the error occured (use __FILE__)
 * \param funcname function where the error occured (use __func__)
 * \param line (optional) line where the error occurred (use __LINE__)
 */
inline void SetOptiXLastError(string what, const char* filename, const char* funcname, const int line=0 )
{
    OptiXError=true; // changed 07/06/2023
//    if(line==0)
//        sprintf(LastError, "%s in function %s of file %s", what.c_str(), funcname, filename);
//    else
//        sprintf(LastError, "%s in function %s, line %d of file %s", what.c_str(), funcname, line, filename);
    char infostring[256];
    if(line==0)
        sprintf(infostring, " in function %s of file %s",funcname, filename);
    else
         sprintf(infostring, " in function %s, line %d of file %s", funcname, line, filename);
    LastError=what + infostring;
}

inline void ForwardOptiXLastError(const char* filename, const char* funcname, const int line=0 )
{
    OptiXError=true;    // changed 07/06/2023
//    char * pst= LastError+ strlen(LastError);
//    char buf[256];
//    if(line==0)
//        sprintf(buf, "\ncalled by function %s of file %s", funcname, filename);
//    else
//        sprintf(buf, "\ncalled by function %s, line %d of file %s", funcname, line, filename);
//    if(strlen(LastError)+strlen(buf)>= ERROR_MAXSIZE)
//        throw runtime_error("Error buffer overflow");
//    strcpy(pst,buf);

    char infostring[256];
    if(line==0)
        sprintf(infostring, "\ncalled by function %s of file %s",  funcname, filename);
    else
         sprintf(infostring, "\ncalled by function %s, line %d of file %s",funcname, line, filename);
    LastError+= infostring;
}

/** \brief Reset the internal error
 *  \ingroup GlobalCpp
 */
inline void ClearOptiXError() {OptiXError=false;  LastError.clear();}


void memoryDump(void* address, uint64_t size);

/** \brief Abstract base class of all optical elements, surfaces and groups
*
*
*
*     Optical element classes might be derived from an agregate with shape and behavior classes, but they MUST refer to the same Element object.
*      Hence these derived objects must declare the base Element member as virtual
*
*     General parameters common to all optical elements
*     -----------------------------------------
*
*   Name of parameter | UnitType | Description
*   ----------------- | -------- | --------------
*   \b distance | Distance | Distance to the preceeding element. (see note)
*   \b theta | Angle | Chief ray half-deviation
*   \b phi | Angle | Rotation angle of the surface reference frame around the incident chief ray
*   \b psi | Angle | Rotation angle of the surface reference frame around its normal
*   \b Dtheta | Angle | Correction to the incidence angle theta
*   \b Dphi | Angle | Correction to phi rotation angle
*   \b Dpsi | Angle | Correction to in-plane rotation angle psi
*   \b DX | Distance | X offset of the element in the absolute reference frame
*   \b DY | Distance | Y offset of the element in the absolute reference frame
*   \b DZ | Distance | Z offset of the element in the absolute reference frame
*
*   \note
*   - <em> All parameters are in \b BasicGroup </em>
*   - If element is first (previous==0) the incident chief ray is along Z, with an origin at 0 in absolute frame .
*   The element distance is counted from this absolute origin.
*   \todo Clipping is not yet implemented, but it could be done at the ElementBase level where the transform matrix are defined
 */
class ElementBase {
public:
    // ces typedefs sont peut-être à déplacer dans un namespace Element indépendant(public)
    typedef map<string,Parameter>::iterator ParamIterator; /**< \brief type of iterator used for parameter table lookup */
    typedef map<string, string>::iterator HelpstrIterator;  /**< \brief type of iterator used for helpstring table lookup */

   // typedef Matrix<FloatType,3,1> VectorType;
    typedef RayBaseType::VectorType VectorType; /**< \brief the type of vectors used for point and rays */
    typedef Matrix<FloatType,3,3> RotationType;  /**< \brief the type of rotation matrix used for space coordinate transforms */
    typedef Transform<FloatType,3,Affine> IsometryType; /**< \brief the type of isometry matrix used for space coordinate transforms */

    ElementBase(bool transparent=true, string name="", ElementBase* previous=NULL); /**< \brief default  constructor (Film) with explicit chaining to previous */

    /** \brief virtual destructor
     *
     *  Clears the links of neighboring and parent elements
     */
    virtual ~ElementBase()
    {
     //   cout << "destroying element " << m_name << endl;
        if(m_next)
            chainNext(NULL);
        if(m_previous)
            chainPrevious(NULL);
    }

    virtual inline string getOptixClass() =0;/**< \brief return the derived class name of this object */
        //{return "Surface";}
    /** \brief Align this surface with respect to the main incident ray according to the parameters,
    *       \param wavelength the alignment wavelength (used by chromatic elements only)
    *       \return  0 if alignment  is OK ; -1 if a grating can't be aligned and OptiXLastError is set with the grating name
    *
    *   Surfaces are aligned in two times.
    *   *   definition of transforms between input exit and surface frames
    *   *   definition of the surface equation in the propagation space (input frame)
    *    Align() defines the actual surface equation in the computation frame. it cannot be called before the
    *       frame transforms are defined (see setFrameTransforms())
    *    \n   It must be reimplemented in derived classes
    */
    virtual int align(double wavelength)=0;

/** \brief  call transmit or reflect according to surface type ray and iterate to the next surface*/
    virtual void propagate(RayType& ray)=0 ;/**< \param ray the propagated ray */

    /** \brief  propagate contruction  of surface errors. The function does nothing else if the runtime class is not derived from Surface   */
    virtual void  generateSurfaceErrors()
    {
        if(m_next!=NULL )
            m_next->generateSurfaceErrors();
    }

    inline void setName(string&& name ){m_name=name;} /**< \brief sets the element name for scripting  \param name new name of the element*/
    inline string getName(){ return m_name;}    /**<  \brief retrieves the element name for scripting \return the name of the element as string*/

    /** \brief  set or reset the pointer to the previous element without taking care of the old or new linked elements
     * \param previous a pointer to the new previous element
     */
    inline void setPrevious(ElementBase* previous) {m_previous=previous;}

    /** \brief set or reset the pointer the next element without taking care of the old or new linked elements
     * \param next a pointer to the new next element
     */
    inline void setNext(ElementBase* next){m_next=next;}

    /** \brief set a reference to the group which the element belongs to
     * \param parent the parent group of this element
     */
    inline void setParent(ElementBase* parent){m_parent=parent;}

    /** \brief set or reset the preceeding element in the active chain and update all linked elements (old and new)
    *
    *  \param previous pointer to the previous element; if NULL  the element should be a  source otherwise the chain will not be activable */
     inline void chainPrevious(ElementBase* previous)  { // virtual qualifier withdrawn //  sources need a special treatment
        if(m_previous == previous) return;
        if(m_previous) m_previous->m_next=NULL;
        if(previous)
        {
            if(previous->m_next)
                previous->m_next->m_previous=NULL;
            previous->m_next=this;
        }
        m_previous=previous;
    }

    /** \brief retrieves a pointer to the previous element of the active chain
     * \return a pointer to the previous element
     */
    inline ElementBase* getPrevious()  { return m_previous;}

    /** \brief  set or reset the following element in the active chain and update all linked elements (old and new)
    *
    *  \param next pointer to the next element; if  left NULL  the element will be the last of the active chain*/
    inline void chainNext(ElementBase* next)
    {
        if(m_next == next) return;
        if(m_next)  m_next->m_previous=NULL;
        if(next)
        {
            if(next->m_previous)
                next->m_previous->m_next=NULL;
            next->m_previous=this;
        }
        m_next=next;
    }

    /** \brief Retrieves the next element in the activfe chain
     *
     * \return a pointer toe the element following this one in the chain, or 0 if this is the last one.
     */
    inline ElementBase* getNext(){ return m_next;}

    /** \brief Retrieves the parent element
     *
     * \return a pointer to the parent element which must be a group (not yet implemented, or 0 if this element is not part of a group
     *
     */
    inline ElementBase* getParent(){ return m_parent;}


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
    * The type and group and flags of a parameter are internally defined and cannot be changed. Their actual values will be reflected in param on return
    * if the array flags are not identical OpticsLastError will be set and the function  will reteurn false
    * \param name the name of parameter to set
    * \param param the new parameter  object
    * \return  true if parameters was changed , false if parameter doesn't belong to the object of has a different array flag
    */
    inline virtual  bool setParameter(string name, Parameter& param)
    {
        //std::cout << "setting elementBase parameter " << name << std::endl;
        ParamIterator it=m_parameters.find(name);

        if (it !=m_parameters.end())
        {

            param.type=it->second.type; // type, flags and group of a parameter must not be modified
            param.group=it->second.group;
            if( (param.flags&ArrayData) != (it->second.flags&ArrayData))
            {
                string reason;
                if(it->second.flags&ArrayData)
                    reason=" has an array type but the given parameter has a single value type";
                else
                    reason=" has a single value type but the given parameter has an array type";

                SetOptiXLastError(string("parameter name ")+ name +reason,__FILE__, __func__);
                return false;
            }
            param.flags=it->second.flags;
            it->second=param;
            m_isaligned=false;

            return true;
        }
        else
        {
            SetOptiXLastError(string("parameter name ")+ name + " is invalid",__FILE__, __func__);
            return false;
        }
    }

    inline virtual  bool dumpParameter(string name)
    {
//        std::cout << "New data for parameter " << name << "\n";
//        param.dump();
        ParamIterator it=m_parameters.find(name);
        if (it !=m_parameters.end())
        {
            std::cout << "Data in parameter " << name << "\n";
            it->second.dump();
            return true;
        }
        else
        {
            SetOptiXLastError(string("parameter name ")+ name + " is invalid",__FILE__, __func__);
            return false;
        }
    }

    /** \brief retrieves a numeric parameter by its name
     *
     *  If the parameter is an array le param.paramArray member will point to a ParameterArray struct containing the array dimensions
     *  and data This struct is owned by the parameter object and destroyed with it
     * \param name The parameter name
     * \param param a parameter object Parameter to be modified on return
     * \return true if parameter is valid, false otherwise
     */
    inline bool getParameter(string name, Parameter& param)
    {
        ParamIterator it=m_parameters.find(name);
        if (it !=m_parameters.end())
        {
            param=it->second;
            return true;
        }
        else
        {
            SetOptiXLastError(string("parameter name ")+ name + " is invalid",__FILE__, __func__);
            return false;
        }
    }

    inline bool getParameterRef(string name, Parameter *paramRef)
    {
        ParamIterator it=m_parameters.find(name);
        if (it !=m_parameters.end())
        {
            paramRef=&(it->second);
            return true;
        }
        else
        {
            SetOptiXLastError(string("parameter name ")+ name + " is invalid",__FILE__, __func__);
            return false;
        }
    }

    /** \brief Check if the parameter is an array and retrieves its memory size
     *
     * \param name The name of the parameter to query
     * \param size integer location which will receive the memory size of the array (dim(0)*dim(1)) or 1 if the parameter is a simple value
     * \return true if the parameter is an array, false if it is a single value.
     *
     */
    inline bool getParameterArraySize(string name, size_t * size)
    {
        ParamIterator it=m_parameters.find(name);
        if(!size)
        {
            SetOptiXLastError(string("Invalid size parameter"),__FILE__, __func__);
            return false;
        }
        else if (it !=m_parameters.end())
        {
            if (it->second.flags & ArrayData)
            {
                *size=it->second.paramArray->dims[0]*it->second.paramArray->dims[1];
                return true;
            }
            else
            {
               SetOptiXLastError(string("parameter name ")+ name + " has not an array type",__FILE__, __func__);
                return false;
            }

        }
        else
        {
            SetOptiXLastError(string("parameter name ")+ name + " is invalid",__FILE__, __func__);
            return false;
        }
    }


    /** \brief Check if the parameter is an array and retrieves its memory size
     *
     * \param name The name of the parameter to query
     * \param dims pointer to the array of two int64_t numbers containing the dimensions of the parameter array (dims[0]correspond to the fast varying index)
     * \return  true if the parameter is an array, false if it is a single value.
     *
     */
    inline bool getParameterArrayDims(string name, int64_t (**dims)[2])
    {
        ParamIterator it=m_parameters.find(name);
        if (it !=m_parameters.end())
        {
            if (it->second.flags & ArrayData)
            {
           //     int64_t(&dimref)[2]=it->second.paramArray->dims;
                *dims=&(it->second.paramArray->dims);
                return true;
            }
            else
            {
               SetOptiXLastError(string("parameter name ")+ name + " has not an array type",__FILE__, __func__);
                return false;
            }

        }
        else
        {
            SetOptiXLastError(string("parameter name ")+ name + " is invalid",__FILE__, __func__);
            return false;
        }
    }

    inline bool getParameterFlags(string name, uint32_t &flags)
    {
        ParamIterator it=m_parameters.find(name);
        if (it !=m_parameters.end())
        {
            flags=it->second.flags;
            return true;
        }
        else
        {
            SetOptiXLastError(string("parameter name ")+ name + " is invalid",__FILE__, __func__);
            return false;
        }
    }

    inline ParamIterator parameterBegin(){return m_parameters.begin();}/**< \brief return an iterator positionned on the first element of the parameter list of this surface*/
    inline ParamIterator parameterEnd(){return m_parameters.end();}  /**< \brief return an iterator positionned after the last element of the parameter list of this surface*/

    /** \brief return true if inserted returne false if helpstring is already existing
    */
    static inline bool setHelpstring(string paramName, string helpstring)
    {
        pair<HelpstrIterator,bool> result=m_helpstrings.insert(make_pair(paramName, helpstring));
        return result.second;
    }

    /** \brief return an empty string if entry doesn't exist but doesn't create it
    */
    static inline void getHelpstring(string paramName, string& helpstring)
    {
        HelpstrIterator it=m_helpstrings.find(paramName);
        if(it !=m_helpstrings.end())
            helpstring=it->second;
        else
            helpstring.erase();
    }

    static void setHelpstrings();  /**< \brief sets help strings all alignment parameters */

    inline void setTransmissive(bool trans=true) {m_transmissive=trans;}/**< \brief Change the transmissive flag (most objects are created reflective)
                        *        \param trans new value of the transmissive flag*/
    inline bool getTransmissive(){return m_transmissive;}/**< \brief retrieve the value of the transmissive flag \return the value of the transmissive flag*/

    /** \brief setFrameTransforms defines the geometric space transforms related to the position of an element in the various frames
    *       \param wavelength the alignment wavelength (used by chromatic elements only)
    *       \return  0 if alignment  is OK ; -1 if a grating can't be aligned
    *
    *       setFrameTransforms() defines the transformations matrices needed for ray propagation computations. It doesn't define
    *       the surface equation in computation space. \n This is done by the align() function.
    *       \n This implementation is a default one, which will be overridden for gratings and maybe other elements.
    *       * If the surface is reflective \b theta is the half-deflection angle of the chief ray,
    *       * if it is transmissive, the chief ray direction is not modified and theta represents the surface tilt angle of normal to chief ray)
    *
    *       In all cases \b Phi defines the frame rotation around the incident chief ray from entry to exit.
    *       \n When the surface is reflective the incidence plane is the YZ plane of the rotated frame and XZ is tangent to the surface
    *       \n \b Psi is always the rotation around the surface normal
    */
    virtual int setFrameTransforms(double wavelength)  ;

    EIGEN_DEVICE_FUNC inline IsometryType& exitFrame(){return m_exitFrame;}  /**< \brief returns a reference to the space transform from laboratory oriented frame to exit space of this element */

    inline IsometryType surfaceFrame()
    {
        IsometryType sframe(m_surfaceDirect);
        sframe.translation()=m_exitFrame.translation();
        return sframe;
    }

    int alignFromHere(double wavelength);/**< \brief Align the whole surface chain;  stops at first error and return the error code
                            *    \return  0 if OK; the error code which stopped the alignment */

    bool isAligned();/**< \brief  returns true if all elements of the surface chain starting from here are aligned, false otherwise */



     bool isSource();/**< \brief Checks whether this element is a source (i.e. has a radiate() function )*/
     ElementBase* getSource(); /**< \brief explore the element chain to find the most upstream source from here  \return the most upstream source or NULL if there is none */

    virtual void dumpData();/**< \brief dump internal data to standard output */

    friend TextFile& operator<<(TextFile& file,  ElementBase& elem);  /**< \brief Dump this Element object to a TextFile, in a human readable format  */

    friend TextFile& operator>>(TextFile& file,  ElementBase* elem);  /**< \brief Retrieves a Element object from a TextFile  \todo call interface::createElementObject(type) */

    friend bool SaveElementsAsXml(const char * filename, ElementCollection &system);
protected:

    /** \brief Creates and sets a new named numeric parameter
    */
    inline ParamIterator defineParameter(string name,const Parameter& param)    {
//        if (param.flags & ArrayData)
//            std::cout << "defining parameter  array " << name << " of size " << param.paramArray->dims[0] << " x " << param.paramArray->dims[1] <<
//              " data  address " << param.paramArray  <<std::endl;
        pair<ParamIterator,bool> result= m_parameters.insert(make_pair(name,param));
        if(!result.second)
          result.first->second=param;
//        if (param.flags & ArrayData)
//            std::cout << "Created parameter  array " << name << " of size " << result.first->second.paramArray->dims[0] << " x " << result.first->second.paramArray->dims[1] <<
//            " data  address " << result.first->second.paramArray <<std::endl;

        m_isaligned=false;  //maybe some changes do not require this
        return result.first;
    }
    inline void removeParameter(string name){ m_parameters.erase(name);}/**< \brief removes a tagged parameter */

    static FloatType m_FlipSurfCoefs[]; /**< \brief list of coefficient of the  matrix transforming surface frame coordinates into propagation frame  coordinates  */
    static map<string, string> m_helpstrings;  /**< \brief  parameter description help strings  */
    static int m_nameIndex;  /**< \brief Index for automatic naming of surfaces created without a name */
 //   vector<RayType> m_impacts; /**<  \brief the ray impacts on the surfaces in forward or backward element space */



//  surface definition primary parameters
    string m_name;   /**< \brief  an identification name for calling this surface from scripts  */
    ElementBase* m_previous; /**< \brief the previous element in the surface chain */
    ElementBase* m_next; /**< \brief the next element in the surface chain */
    ElementBase* m_parent; /**< \brief The containing group - unused while group concept is not implemented  -  */
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
    VectorType m_translationFromPrevious;   /**< \brief Translation  from previous local frame to this local frame */
    RotationType m_frameDirect;  /**<  \brief rotation part of the transform expressing absolute frame coordinates from local frame ccordinates
    *
    *     == m_exitFrame.linear()   le doublon est-il utile ?*/
    RotationType m_frameInverse; /**<  \brief rotation part of transform  absolute --> outFrame      */

    bool m_transmissive;   /**< \brief  flag defining whether ray propagation should call the surface transmit() or reflect()  function */
    bool m_isaligned;  /**< \brief flag tracking if the surfacee need to be realigned unset when a parameter is changed */


};



#endif // header guard

