#ifndef INTERFACE_H_INCLUDED
#define INTERFACE_H_INCLUDED

/**
*      \file           interface.h
*
*      \brief         Interface definition file. Defines a collection of the pure C functions enabling to call the Optix.dll library from any language
*
*      \author         François Polack <francois.polack@synchroton-soleil.fr>
*      \date        2020-11-12  Creation
*      \date         Last update
*
*/



///////////////////////////////////////////////////////////////////////////////////
//
//             REVISIONS
//
////////////////////////////////////////////////////////////////////////////////////


/**
*      \mainpage   OptiX
*       An X-ray optics library
*       this is an &alpha; version of the library
*      \date               Last update: 2022-02-09
*
*      * For Main Interface C functions  see \ref mainAPI
*      * For aperture utility functions  see \ref apertureAPI
*      * Enumerated values used in the interface: \ref enums
*      * Element classes which can be implemented in OptiX: \ref elemClasses
*      * OptiX internal Global C++ function: \ref GlobalCpp
*
*      \defgroup globalc  Interface C functions
*      \brief Functions exported by the C interface
*
*
*      \defgroup elemClasses Instantiable optical elements classes
*      \brief  The limited list of class names which can be used to create optical elements in OptiX
*
*       The CreateElement() function of the C interface and the C++ internal function CreateElementObject() recognize two names for each instantiable optical element class.
*       A template style name, Name<class1[,class2]>, or a capitalized name with internal capital letters (camel case).
*       The GetElementType() C function calls ElementBase::getOptixClass(), and  always returns the template style name.
*
*       In the special case of polynomial surfaces based on polynomial surface the typedef  \ref NaturalPolynomialSurface and \ref LegendrePolynomialSurface
*       are used instead of the templates \ref PolynomialSurface< \ref NaturalPolynomial>  and \ref PolynomialSurface< \ref LegendrePolynomial>
*
*       The actual underlying class is not always templated. When it is a template the template style name is the internal class name, and the simple name is a C++ typedef.
*       When the underlying class is not templated, e.g. case of sources, the templated style name is an alias only recognized by the CreateElement() / CreateElementObject() functions.
*
*      \defgroup enums  Enumeration list
*      \brief  enumerated values used in the library
*
*      \defgroup GlobalCpp  Global internal variables and functions
*      \brief  Variables and functions defined at internal level and not exported by the C API
*
*      \defgroup InternalVar Internal global variables
*      \brief internally defined global variables not visible in the C API
*      \ingroup GlobalCpp
*
*      \defgroup mainAPI  main Interface C functions
*      \brief Functions defining optical systems and running computations
*
*      \ingroup globalc
*
*      declared in interface.h
*      \see see also \ref apertureAPI "C interface to Aperture defining functions"
*
*      \defgroup apertureAPI  C functions of the aperture API
*      \brief  interface C Functions for aperture handling exported by the OptiX library\n **Aperture API is currently modified, please check the status of each function**
*        \n beware that region indices that were previously defined as 64 bit integers (size_t) are now defined as 32 bit integers (int)
*      \ingroup globalc
*
*       Note that opaque aperture region set the ray intensities to 0, but do not stop the propagation (ray is maintained alive).
*       This allow to compute continuous wavefronts despite the obscurations, so long it can be numerically done
*       (even though it might be unphysical).
*
*      \warning The prototype of Aperture API functions were all modified to return a bolean error flag as the main API functions
*           The up-to-date functions are signalled in the documentation with the date of modification
*      declared in apertureAPI.h
*      \see see also \ref mainAPI "Main Interface C functions"
*
*      <H1>  General information on the Aperture API </H1>
*
*   Any Element based on a Surface class has an ApertureStop member, which is composed of an array of superimposed stopping \ref Region Regions or Stops;\n
*   * Each Region is a simply connected domain, which can be either a Polygon or an Ellipse.\n
*       \e Mind that no check is made on the simply connected character of a polygon when created
*   * Each region defines the opacity of its inside area and imposes it over all underlying regions.
*   * It doesn't define the opacity of its outside area unless it is the bottom element of the stack.
*       - if the region Opacity=true, the optical transmittance  is 0 inside the region
*       - if the region Opacity=false, the optical transmittance is 1. inside the region
*   * The opacity of the intersection of all outside areas of all regions, is the opposite of the opacity of the bottom region
*       (region 0,first defined in the stack)
*
*       In other words,  the first stop (region 0) defines an opacity in all space.\n
*       Adding an opaque Region obstructs the transmission under the region area, but does not change the outside \n
*       while adding a transparent Region opens an aperture though all the stacked stops and still does not change the outside \n
*   *   The logical stack should define region of smaller and smaller sizes from bottom to top. This is intended to allow rounding of squared apertures and stops.
*
*       *Functions of the Aperture API used to return a int64_t value;  A negative value meant that an error occurred.*
*       *The functions are presently modified to return a boolean result true for succes , false for failure, in which can GetOptiXError will give the reason.*
*
*      \defgroup surferrorAPI  C functions of the surface error API
*      \brief  interface C Functions exported by the OptiX library to incorporate surface errors into ray tracing
*      \ingroup globalc
*
*      declared in interface.h
*      \see see also \ref mainAPI "Main Interface C functions"
*
*      \defgroup reflectivityAPI  C functions of the reflectivity API
*      \brief  interface C Functions exported by the OptiX library to handle objects defined in the C++ RefleX library
*      \ingroup globalc
*
*      declared in ReflectivityAPI.h
*      \see see also \ref mainAPI "Main Interface C functions"
*
*       \defgroup NonStandard API functions having a non standard return type
*       \brief The functions recorded in this group do not return an error flag, return value not bolean or having another meaning
*       \ingroup globalc
*       \warning *24/06/2023*:   FUNCTION  ***GetElementID(const char* elementName)*** was removed from the interface
*       \see replacement function FindElementID()
*
****************************************************************************/


////////////////////////////////////////////////////////////////////////////////


#ifdef BUILD_DLL
    #define DLL_EXPORT __declspec(dllexport)
#else
    #ifdef NO_DLL
        #define DLL_EXPORT
    #else
        #define DLL_EXPORT __declspec(dllimport)
    #endif
#endif





#include "ctypes.h"

#ifdef __cplusplus

extern "C"
{
#else
#include <stdbool.h>
#endif

/** \ingroup mainAPI
*   \{
*/



     /** \brief  Check and reset the error status
     *
     * \param errstring_ptr a pointer to the error string. **This is a read only location, dont try to modify this string**
     * \return the error flag. Error status is reset to no error on exit.
     */
    DLL_EXPORT bool GetOptiXError( char** errstring_ptr);

    /** \brief  **Deprecated** - Check and reset the error status
     * \deprecated This function is deprecated and replaced by  GetOptiXError(), which can handle any message length.
     *
     * \param buffer a char buffer to receive the error string. Can be NULL if not needed
     * \param bufferSize  the size in bytes of the buffer (256 is safe)
     * \return the error flag. Error status is reset to no error on exit.
     */
    DLL_EXPORT bool GetOptiXLastError(char* buffer, int bufferSize);


    /** \brief  gets or print OptiX version info
     *
     * \param version  a pointer to a READ-ONLY location where the version string is stored.
     * if this pointer is NULL, the version string is printed to stdout
     * \return this function returns always true
     *
     *  *Return type changed 24/06/2023 (commit 9a16050)*
     */
    DLL_EXPORT bool Version(char** version
#ifdef __cplusplus
                            =0);
#else
                            );
#endif // __cplusplus



    /** \brief Align the element chain starting from the given element
     *
     * \param elementID first element of the chain to align
     * \param wavelength alignment wavelength (for gratings). It can be 0.
     * \return true if alignment was successful, false otherwise and OptiXLastError is set
     *
     */
    DLL_EXPORT bool Align(size_t elementID, double wavelength);

    /** \brief Defines the deviation angle so that the Cff ratio is satisfied at the given wavelength
     *
     * \param elementID ID of the element which must be of source type
     * \param Cff The Cff ratio (i.e. output/input sine of grazing angle)  which should be achieved at the given xavelength
     * \param wavelength the wavelength (m) at which alignment is sought
     * \return true if elementID is a grating and was aligned ; false otherwise and OptiXLastError is set
     */
    DLL_EXPORT bool AlignGrating4Cff(size_t elementID, double Cff, double wavelength);

    /** \brief Chain two elements by their IDs
     *
     *  Links to element together. The upstream link of previous and downstream link of next will remain unchanged
     *  Set one link to 0 to cut the link chain
     * \param prevID  ID of the element to place upstream
     * \param nextID  ID of the element to place downstream
     * \return false if both IDs are 0  or one is invalid , true otherwise
     */
    DLL_EXPORT bool ChainElement_byID(size_t prevID, size_t nextID);

    /** \brief Chain two elements by their names
     *
     *  Links to element together. The upstream link of previous and downstream link of next will remain unchanged
     *  Set one link to "" to cut the link chain
     * \param previous name of the element to place upstream
     * \param next name of the element to place downstream
     * \return false if both names are empty("") or one is invalid , true otherwise
     */
    DLL_EXPORT bool ChainElement_byName(const char* previous, const char* next);

     /** \brief Clear impacts on this elements and all recording elements which follows in the link chain
     *
     *  Note that Radiate use the source impact list to initiate the ray tracing. If the ray initialization is unchaged,
     *  Do not call generate but clear the impact from the element following the source
     * \param elementID the starting element ID
     * \return false if elementId is not valid, true otherwise
     */
    DLL_EXPORT bool ClearImpacts(size_t elementID);

    /** \brief Create an optical element (or group)
     *
     * Th function calls the internal function CreateElementObject(type, name)
     * \param[in] type  runtime class of the new element must be one of the types accepted by the CreateElementObject function
     * \param[in] name name of the new element
     * \param[out]  elementID A size_t location to return the unique ID identifying the newly created element, if creation succesds.
     *  The returned value will be 0 if the function fails.
     * \return true if the new element was created; false if creation failed
     *
     *   *Return type changed 25/06/2023 (Commit 43dc952d13f7b)*
     */
    DLL_EXPORT bool CreateElement(const char* type, const char* name, size_t* elementID);

    /** \brief delete an element by its  ID and remove its reference from the system
     *
     * \param elementID the ID of the element to delete
     * \return false if the element was not found; true otherwise
     */
    DLL_EXPORT bool DeleteElement_byID(size_t elementID);

    /** \brief  delete an element by its name and remove its reference from the system
     *
     * \param name name of the element to delete
     * \return false if the element was not found; true otherwise
     */
    DLL_EXPORT bool DeleteElement_byName(const char* name);

    /** \brief Write a C_DiagramStruct to a file in binary format <i>(it is questionnable whether it should be maintained as is,in the interface)</i>
     *
     *  The format follows closely the structure pattern. 4 int32 with the nthe dimension m_dim of the arrays, maximum capacity, spot count, and lost rays,
     *  4 arrays of  m_dim doubles with the statistics (min, max, mean, sigma), then the array of m_dim * m_count  doubles
     * \param filename name of the file
     * \param cdiagram a pointer to the C_DiagramStruct to be written on file
     * \return an error flag: true if the daigram was successfully written, false otherwise
     */
    DLL_EXPORT bool DiagramToFile(const char* filename, C_DiagramStruct* cdiagram);

    DLL_EXPORT void DumpArgParameter(Parameter* param); /**< debugging function */

    /** \brief dump and compare given parameter with stored data
     *
     * \param elementID Handle to the inquired element
     * \param paramTag The name pf the paameter to dump
     * \return error flag true if no error false otherwise
     *
     */
    DLL_EXPORT bool DumpParameter(size_t elementID, const char* paramTag);

    /** \brief Dump the content of an XML system file to stdout (not up to date)
     *
     * \param filename Name of the input file
     * \return false in case of error and the OptiXLastError will be set.  true otherwise
     *  \warning This function was not updated since 068/06/2021
     */
    DLL_EXPORT bool DumpXML(const char* filename);

    /** \brief Defines the size, divergence and waists position of a an astigmatic gaussian source emulating undulator radiation at the given wavelength
     *
     * \param elementID The sourcede be defined . It should be an astigmatic gaussian source, but a simple gaussian source may be used if the source is round or SD_UndulatorDistance == 0
     * \param sigmaX The electron beam size (in m) in the X direction
     * \param sigmaY The electron beam size (in m) in the Y direction
     * \param sigmaprimX The electron beam divergence in (rad) in the X direction
     * \param sigmaprimY The electron beam divergence in (rad) in the Y direction
     * \param undulatorLength the undulator length (m)
     * \param SD_UndulatorDistance distance of the center of the undulator from the Straight section center (in m)
     * \param wavelength the undulator radiation wavelength (m)
     * \param detuning divergence amplification factor to take detuning into account
     * \return true if elementID is a valid source and waists position and size were properly set; otherwise return false OptiXLastError is set
     */
    DLL_EXPORT bool EmulateUndulator(size_t elementID, double sigmaX, double sigmaY, double sigmaprimX, double sigmaprimY,
                                     double undulatorLength,  double SD_UndulatorDistance, double wavelength, double detuning);

    /** \brief enumerates the element list of the current system
     *
     * \param[in,out] pHandle address of a location containing: \n on input, a handle to the current enumerator or 0 to get the first element of the system;
     *  \n on output, a  handle to underlying enumerator of the parameter list or 0 if the retrieved element is the last one or an error occurs
     * \param[out] elemID a pointer to a longlong interger which will  receive the element ID or 0 on error
     * \param[out] nameBuffer char* a character buffer to receive the retrieved element name.
     * \param[in] bufSize The size of the tag buffer
     * \return true if a new element was found and returned; false if  the name buffer was too small and the OptiXLastError will be set
     * in this latter case, the element name is truncated and the enumerator is  invalidated  and set to 0
     *
     * Memory leaks will occur if the handle is dropped before it is returned as 0. If needed clear a non zero handle with ReleaseElementEnumHandle()
     */
    DLL_EXPORT bool EnumerateElements(size_t * pHandle, size_t* elemID, char * nameBuffer, const int bufSize);

    /** \brief release an element enumeration handle returned by EnumerateElement()
     *
     * \param handle a non null handle returned by EnumerateElement()
     *  \return true if handle was deleted; false if handle was invalid
     *
     *   *Return type changed 24/06/2023 (commit 9a16050)*
     */
    DLL_EXPORT bool ReleaseElementEnumHandle(size_t handle);


    /** \brief enumerates the parameter list of a given optical element
     *
     * \param[in] elementID The ID of the element to query
     * \param[in,out] pHandle address of a location containing: \n on input, a handle to the current enumerator or 0 to get the first parameter;
     *  \n on output, a handle to underlying enumerator of the parameter list or 0 if the retrieved element is the last one
     * \param[out] tagBuffer char* a character buffer to receive the retrieved parameter name.
     * \param[in] bufSize The size of the tag buffer
     * \param[out] paramData a pointer to a Parameter struct to receive the parameter data. This parameter must not have an Array type
     *      when called for the first time. Array, if any, will be allocated by C and should be copied. They will be cleared by the next
     *      call to the function
     * \return true if a new parameter was found and returned without error ; false if elementID was invalid or the buffer was too small, and OptixLastError is set
     * in this latter case, the parameter name is truncated and the enumerator is  invalidated  and set to 0
     * Memory leaks will occur if the handle is dropped before it is returned as 0. If needed clear a non zero hande with ReleaseParameterEnumHandle()
     */
    DLL_EXPORT bool EnumerateParameters(size_t elementID, size_t * pHandle, char* tagBuffer, const int bufSize , Parameter* paramData);

    /** \brief Release a parameter enumeration handle returned by EnumerateParameters()
     *
     * \param[in] handle a non null handle returned by EnumerateParameters)
     * \param[in] paramData (optional) pointer to the Parameter struct used to iterate. If given will clear any paramArray struct possibly allocated by the iterator
     * \return true if handle wase released ; false if handle is invalid
     *
     * *Return type changed 24/06/2023 (commit 9a16050)*
     */
    DLL_EXPORT bool ReleaseParameterEnumHandle(size_t handle,
#ifdef __cplusplus
                Parameter *paramData =0);
#else
                Parameter *paramData);
#endif // __cplusplus


    /** \brief  retrieves the unique ID of an element from its name
     *
     * \param[in] elementName name of the searched element
     * \param[out] elemID The address of a location to return the ID
     * \return true if element was found; false if not
     *
     *  *Return type changed 24/06/2023 (commit 9a16050)*
     */
    DLL_EXPORT bool FindElementID(const char* elementName, size_t * elemID);


    /** \brief Gets the element immediately downstream of the given one in the link chain
     *
     * \param elementName the name of the element to inquire
     * \param nextID size_t location to return the ID of the next element, or 0 if the element either is the last of the link chain either is invalid
     *  \return true if function succeeds; false if a parameter is invalid
     *
     *  *Return type changed 24/06/2023  (commit 65b8c5c4)*
     */
    DLL_EXPORT bool FindNextElement(const char * elementName, size_t * nextID);


    /** \brief Gets the element immediately upstream of the given one in the link chain
     *
     * \param elementName the name of the searched element
     * \param previousID a size_t location to return the ID of the previous element, or 0 if the element either is the first of the link chain either is invalid
     *  \return true if function succeeds; false if a parameter is invalid
     *
     * *Return type changed 24/06/2023 (commit 65b8c5c4)*
     */
    DLL_EXPORT bool FindPreviousElement(const char * elementName, size_t * previousID);


    /** \brief This function defines a polynomial surface to fit the given height data (Only apply to polynomial surfaces)
     *
     * The basis polynomial function are separable products of Legendre Polynomials in respectively X and Y.
     * If the surface is known to be a pure cylinder, just set the degree of freedom in the constant direction to N=1
     * \param elementID The Id of the element to define
     * \param Nx The degree of freedom of polynomial in X (polynomial degree - 1)
     * \param Ny The degree of freedom of polynomial in Y (polynomial degree - 1)
     * \param limits The boundaries of the definition domain. The given array must have a size of 4 at least, and contain the values
     *  of xmin, xmax, ymin, ymax. All base polynomials have a value of 1 at definition limits
     * \param heights address of an ArrayParameter struct containing the height data to fit, in a 3 column array X, Y, Z, in this order.
     * The dims array contain the number of data points in dims[0] and dims[1] should be 3. Data are read in column major,
     *    Point number is incremented the fastest.
     * \param sigmah location where the sigma of the fit height residuals will be returned, if this address is valid
     * \return true if the function succeeds; otherwise it will return false and the reason will be given by OptixLastError.
     *
     */
    DLL_EXPORT bool FitSurfaceToHeights(size_t elementID, int64_t Nx, int64_t Ny, const double *limits, const ArrayParameter *heights, double *sigmah);


    /** \brief This function defines a polynomial surface to fit the given height data (Only apply to polynomial surfaces)
     *
     * The basis polynomial function are separable products of Legendre Polynomials in respectively X and Y.
     * If the surface is known to be a pure cylinder, just set the degree of freedom in the constant direction to N=1
     * \param elementID The Id of the element to define
     * \param Nx The degree of freedom of polynomial in X (polynomial degree - 1)
     * \param Ny The degree of freedom of polynomial in Y (polynomial degree - 1)
     * \param limits The boundaries of the definition domain. The given array must have a size of 4 at least, and contain the values
     *  of xmin, xmax, ymin, ymax. All base polynomials have a value of 1 at definition limits
     * \param slopes address of an ArrayParameter struct containing the slope data to fit, in a 4 column array X, Y, dZ/dx, dZ/dY, in this order.
     *   The dims array contain the number of data points in dims[0] and dims[1] should be 4. Data are read in column major,
     *    Point number is incremented the fastest.
     * \param sigmasx  location where the sigma of the X slope fit residuals will be returned, if this address is valid
     * \param sigmasy  location where the sigma of the Y slope fit residuals will be returned, if this address is valid
     * \return true if the function succeeds; otherwise it will return false and the reason will be given by OptixLastError.
     *
     */
    DLL_EXPORT bool FitSurfaceToSlopes(size_t elementID, int64_t Nx, int64_t Ny, const double *limits, const ArrayParameter *slopes, double * sigmasx, double* sigmasy);


    /** \brief Generate source rays of the given wavelength but do not run the ray tracing, (only valid for sources).
     *
     * \param[in] elementID ID of the element which must be of source type
     * \param[in] wavelength  the radiation wavelength (must be  >0)
     * \param[out] numRays an optional int location to return the number of generated rays. Set as 0 or NULL if this value is not requested.
     *          numRays value will be 0 if the function fails.
     *      otherwise GetOptiXLastError will return NoError even if 0 rays were created
     * \return true if the function succeds; false otherwise and sets the OptiXLastError
     *
     * This function initiates the rays which will be propagated from the source by populating the source "impact" list.
     * ClearImpact() is not called before executing Generate(). Therefore ray congruences with different wavelengths can be superimposed.
     * ClearImpacts() should be called previously if a new set of rays must be created.
     * Also note the that Radiate() is not either executed,  but must be called independently.
     *
     *  *Return type changed 24/06/2023 (commit 9a16050)*
     */
    DLL_EXPORT bool Generate(size_t elementID, double wavelength,
#ifdef __cplusplus
                             int *numRays=0);
#else
                            int *numRays);
#endif // __cplusplus


    /** \brief Same as Generate() but allow specifying other polarizations than 'S'
     *
     * \param[in] elementID ID of the element which must be of source type
     * \param[in] wavelength  the radiation wavelength (must be  >0
     * \param[in] polar Polarization of the generated rays which can be 'S', 'P', 'R', 'L'
     * \param[out] numRays an optional int location to return the number of generated rays. Set as 0 or NULL if this value is not requested.
     *          numRays value will be 0 if the function fails.
     * \return true if the function succeeds; false otherwise
     *
     *  *Return type changed 24/06/2023 (commit 9a16050)*
     */
    DLL_EXPORT bool GeneratePol(size_t elementID, double wavelength, char polar,
#ifdef __cplusplus
                                 int * numRays=0);
#else
                                 int * numRays);
#endif // __cplusplus


    /** \brief Get the status of global aperture activity flag
     * \param activityFlag a boolean location to receive the global activity flag of apertures,
     *      i.e. whether or not apertures are taken into account during ray tracing
     * \return error code is always true
     *
     *  *Return type changed 22/06/2023 (commit 9a16050)
     */
    DLL_EXPORT bool GetAperturesActive(bool * activityFlag);

    /** \brief retrieve an array parameter (
     *
     * \param elementID  The ID of the element to query
     * \param paramTag the name of the parameter to get
     * \param paramData pointer to a Parameter struct to receive the parameter data. This structure **must have the ArrayData flag bit
     *  set and theparamArray member must point to an initialized ParameterArray struct  able to store at least maxsize array
     *  elements. This structure is owned by the caller, which is in charge of its creation and destruction.
     * \param maxsize The size of the provided storage the ParameterArray struct pointed by paramArray.
     * \return false if an error occurred in which case OptixLastError is set. Otherwise return true;
     */
    DLL_EXPORT bool GetArrayParameter(size_t elementID, const char* paramTag, Parameter* paramData, size_t maxsize);


    /** \brief retrieves the name of an element from its ID
     *
     * \param[in] elementID unique ID of the searched element
     * \param[in,out] strBuffer a char buffer to be filled with the name upon return
     * \param[in] bufSize the size of the buffer in bytes. if buffer is too small name will be truncated and an error set
     * \return false if ID is invalid or buffer too small and Error will be set ; otherwise return true
     */
    DLL_EXPORT bool GetElementName(size_t elementID, char* strBuffer, int bufSize);


    /** \brief retrieves the type of an element from its ID
     *
     * \param[in] elementID unique ID of the searched element
     * \param[in,out] strBuffer a char buffer to be filled with the type string upon return
     * \param[in] bufSize the size of the buffer in bytes. if buffer is too small name will be truncated and an error set
     * \return false if ID is invalid or buffer too small and Error will be set ; otherwise return true
     */
    DLL_EXPORT bool GetElementType(size_t elementID, char* strBuffer, int bufSize);


    /** \brief return the exit frame orientation and position of the given element in the absolute frame
     *
     *  The function return an array of four vectors of size 3, in the location pointed by frame_vectors.
     *  They are respectively and in this order, the unit vectors in X, Y, and Z,
     *  followed by the position of the frame origin in the absolute frame
     * \param elementID the element ID
     * \param frame_vectors Pointer to a location of 12 doubles at least, where the frame vectors will be returned
     *      CAUTION: no check can be done on the size of this location
     * \return  a boolean value, true for success, false if function fails and  OptixLastError was set.
     */
    DLL_EXPORT bool GetExitFrame(size_t elementID, double* frame_vectors);


    /** \brief computes the polynomial expansion of the line density, and the central line tilt and cuvrature, of an holographic grating
     *
     * \param elementID ID of the element which must be a Holographic Grating
     * \param gratInfo pointer to a  GratingPatternInfo structure which will be filed with the requested info. The caller is responsible of creating and destructing this structure
     * \param halfLength The 1/2 length of the grating. Needed to evaluate the axial polynomial expansion of line density
     * \param halfWidth The 1/2 width of the grating. Needed to evaluate the central line curvature
     * \return false (and sets OptiXLastError) if elementID is invalid or if the object is not an holographic grating; otherwise return true
     */
    DLL_EXPORT bool GetHologramPatternInfo(size_t elementID,  GratingPatternInfo *gratInfo, double halfLength, double halfWidth );

    /** \brief Evaluates the impacts stored in the given element in the chosen reference frame  returns it in a C_DiagramStruct of dimension m_dim >= 6.
     *
     * According to the m_dim value, wavelength and stokes parameter are returned or not. Wavelength is returned before Stokes parameters for compatibility.
     * If m_dim=6 only impact position and direction are returned. if m_dim=7 wavelength is added. if m_dim=8 S0= intensity is added.
     * The polarization parameters s1,S2,S3, are added if m_dim >=11. Components abouve  are left unitialized and corresponding statistics are meaningless
     *
     * \param elementID The ID of the element in the space of which the impacts must be evaluated. Impact recording must be active on this object
     * \param diagram the address of a prefilled C_DiagramStruct to receive the data. The m_dim member <b> must  have the value of 6 at least </b>, and the statistics buffers
     * must be already allocated with enough space for m_dim double values each. The m_reserved field <b> must be set </b> and the  m_spots buffer must be preallocated
     * with enough space to accommodate m_reserved spot vectors of m_dim double values. The calling function is responsible for initializing and deleting this structure
     * in the appropriate manner.
     * \param frame the reference frame in which the impacts must be evaluated
     * \return a boolean value, true for success, false for failure and OptixLastError is set.
     */
    DLL_EXPORT bool GetImpactsData(size_t elementID, C_DiagramStruct * diagram, enum FrameID frame);


    /** \brief Gets the element immediately downstream of the given one in the link chain
     *    previousID
     * \param[in] elementID the ID of the given element
     * \param[out] nextID The address of a location to return the next ID.
     *      nextID will be 0 if elementID is the last element of the chain
     * \return function will only fail and return false if elementID is invalid; otherwise it will return true
     *
     * *Return type changed 24/06/2023 (commit 65b8c5c4)*
     */
    DLL_EXPORT bool GetNextElement(size_t elementID,size_t * nextID);


    /** \brief retrieves an element ordinary parameter (defined by a single double-type value)
     *
     * \param elementID  The ID of the element to query
     * \param paramTag the name of the parameter to get
     * \param paramData a pointer to a Parameter struct to receive the parameter data
     * \return true if the element Id is valid and the parameter was found and successfully returned;
     *  false if the elementId is invalid, the parameter was not found or is an Array. The OptiXlastError is set
     */
    DLL_EXPORT bool GetParameter(size_t elementID,const char* paramTag, Parameter* paramData);


    /** \brief Retrieves the dimensions of the parameter array
     *
     * \param elementID  The ID of the element to query
     * \param paramTag the name of the parameter to get
     * \param dims a pointer to an array of two int64_t numbers in which the dimensions of the parameter array will be copied
     *      ( dims[0] will return the number of elements in the fast varying dimension)
     * \return false if elementID is invalid or if the parameter is not found or is not an array, or the dims pointer is invalid, true otherwise
     */
    DLL_EXPORT bool GetParameterArrayDims(size_t elementID, const char* paramTag,  int64_t (*dims)[2]);


    /** \brief Retrieves the total number of elements of the parameter array
     *
     * \param elementID  The ID of the element to query
     * \param paramTag the name of the parameter to get
     * \param size a size_t location to return the parameter array size (ndim(0)*ndim(1))
     * \return false if elementID is invalid or if the parameter is not found or is not an array, true otherwise
     */
    DLL_EXPORT bool GetParameterArraySize(size_t elementID, const char* paramTag, size_t * size);


    /** \brief retieves the flags field of a prameter
     *
     * \param elementID  The ID of the element to query
     * \param paramTag the name of the parameter to get
     * \param flags uint32_t location to receive the flags field
     * \return false if an error occurred in which case OptixLastError is set. Otherwise return true;
     */
    DLL_EXPORT bool GetParameterFlags(size_t elementID, const char* paramTag, uint32_t *flags);


     /** \brief Gets the element immediately upstream of the given one in the link chain
     *
     *   *changed 24/06/2023 (commit 65b8c5c4)*
     * \param[in] elementID the ID of the given element
     * \param[out] previousID The address of a location to return the previous ID.
     *      previousID will be 0 if elementID is the first element of the chain
     * \return function will only fail and return false if elementID is invalid; otherwise it will return true
     */
    DLL_EXPORT bool GetPreviousElement(size_t elementID, size_t * previousID );


    /** \brief compute the PSF on a plane at given distance in free space from a given element, or a set of planes
     *
     * \param elementID The element on which the wavefront is computed
     * \param wavelength The radiation wavelength (m). It should be the same  wavelength use in the generating WaveRadiate call
     * \param psfParams PSFparameters parameters needed to define the WF interpolation and the PSF resolution and size
     *      note that the pixel size might be redefine to satisfy the prescribed oversampling factor
     * \param psfData C_ndArray* Address of a C_ndArray struct which must be initialized in order to receive two complex tables (S&P) of size xSamples*ySamples*numOffsetplanes
     *      \n the ndims attribute will be set to 4 and the first 4 size_t elements of storage will be filled  by the dimensions values {xSamples,ySamples,numOffsetplanes,2} immediately followed by the PSF data
     *      \n the total memory size of the psfData.storage area must be larger than (xSamples*ySamples*numOffsetplanes*2+1)*8 bytes and documented in the allocatedStorage attribute \see note in C_ndArray
     * \return true if successful; false, in case of an error and OptiX  LastError is set
     */
    DLL_EXPORT bool GetPsf(size_t elementID, double wavelength, PSFparameters *psfParams, C_ndArray * psfData);


    /** \brief Retrieves the impact recording mode of an element
     *
     * \param[in] elementID the Id of the element to inquire of
     * \param[out] recordingMode  an int location to return the recording mode of the element, which is a value of the RecordMode enumeration
     * \return true if the function succeeds, false otherwise
     *
     *  *changed 24/06/2023 (commit 9a16050)*
      */
    DLL_EXPORT bool GetRecording(size_t elementID, int *recordingMode);


    /** \brief Finds the most upstream source in the element chain starting from the given element
     *
     * \param elementID  The element from which the chain is explored
     * \param sourceID a size_t location to return the ID of the most upstream source or NULL if no source is found upstream the given element
     * \return true if the function syucceeds, false if a parameter is invalid
     *
     *    *Return type changed 24/06/2023 (commit 65b8c5c4)*
     */
    DLL_EXPORT bool GetSource(size_t elementID, size_t * sourceID);


    /** \brief Evaluates a spot diagram at a given distance from an element and returns it in a C_DiagramStruct of dimension at least 4
     *
     * According to the m_dim value, wavelength and stokes parameter are returned or not. Wavelength is returned before Stokes parameters for compatibility.
     * If m_dim=4 only impact position and direction are returned. if m_dim=5 wavelength is added. if m_dim=6 S0= intensity is added.
     * The polarization parameters s1,S2,S3, are added if m_dim >=9. Components abouve  are left unitialized and corresponding statistics are meaningless
     *
     * \param elementID The ID of the element in the space of which the spot diagram must be evaluated. Impact recording must be active on this object
     * \param diagram  the address of a prefilled C_DiagramStruct to receive the data. The m_dim member <b> must  have the value of 4 at least </b>, and the statistics buffers
     * must be already allocated with enough space for m_dim  double values each. The m_reserved field <b> must be set </b> and the  m_spots buffer must be preallocated
     * with enough space to accommodate m_reserved spot vectors of m_dim double values. The calling function is responsible for initializing and deleting this structure
     * in the appropriate manner.
     * \param  distance (optional) The distance from the element where the spot diagram is evaluated. Default value is 0
     * \return a boolean value, true for success, false for failure and OptixLastError is set.
     */
    DLL_EXPORT bool GetSpotDiagram(size_t elementID, C_DiagramStruct * diagram, double distance);


     /** \brief  return the surface frame orientation and position of the given element in the absolute frame
     *
     *  The function return an array of four vectors of size 3, in the location pointed by frame_vectors.
     *  They are respectively and in this order, the unit vectors in X, Y, and Z,
     *  followed by the position of the frame origin in the absolute frame
     * \param elementID the element ID
     * \param frame_vectors Pointer to a location of 12 doubles at least, where the frame vectors will be returned
     *      CAUTION: no check can be done on the size of this location
     * \return a boolean value, true for success, false if function fails and  OptixLastError was set.
     */
    DLL_EXPORT bool GetSurfaceFrame(size_t elementID, double* frame_vectors);


    /** \brief Check if element is used in transmission rather than reflexion mode (mainly useful for gratings)
     *
     * \param elementID the Id of the element to inquire of
     * \param transmissionMode a boolean location to receive the transmission status of the element
     * \return true if the elementID is valid  ; false otherwise
     *
     *  *changed 22/06/2023 (commit 9a16050)*
     */
    DLL_EXPORT bool GetTransmissive(size_t elementID, bool * transmissionMode);


    /** \brief Check there is an element with this ID in the current system
     *
     * \param ID  The element ID to check
     * \param valid  a boolean location to return the elementID validity
     * \return true : this function always succeeds
     *
     *   *Return type changed 22/06/2023  (commit 9a16050)*
     */
    DLL_EXPORT bool IsElementValid(size_t  ID, bool *valid);


    /** \brief Load an optical system from a configuration file description (provisional RefleX input)
     *
     * A Configuration file contains description of the beamline and tables used for reflectivity.
     * It is hierarchically organized with the level of indentation (it is better to use spaces than tabs)
     * It might define several section and subsections introduced by keyword. Keywords are always capitalized they are follewd by parameters separated by spaces
     * Section defining keywords:
     *  - DBASEPATH  Path to the optics database directory
     *  - DATABASE  Databases to open
     *  - INDEXTABLE  name : New index table . Should be followed indented by the database and material name to tabulate, each one on a separate line
     *  - COATINGTABLE name: New table of reflectivity. Should be followed indented by coating definitions. Coating are composed of \n
     *   +  a substrate on level 1 indented line: \e coating_name \e substrate_material \e roughness  \n
     *     * additionnal layers on level 2 indented new lines: \e material \e thickness [ \e compactness ]. Material names are composed of   DBase:material (names as defined in DBase)  with a colon separator \n
     *   + sub-keywords: ANGLERANGE and ENERGYRANGE are used to define the tabulation grid
     *  - BEAMLINE Introduce the optical element section (no parameter). Each element is defined on an level 1 new line. \n
     *   + \e class_name  \e element_name;  parameters follow on level 2 separate lines \n
     *    *  \e parameter_name \e value [ \e range ], range is optionnal. Macro as INV() and DEGREE() can be used to modify the values \n
     *    * COATING \e CoatingTable_name \e coating_name define a reflective coating
     *    * APERTURE Keyword, optionnally followed by activity (default= \e active ) starts an aperture stop \n           Aperture Regions definition are on level 4. The can be of 4 types \n
     *        +    RECTANGULAR \n
     *        +    CIRCULAR  \n
     *        +    ELLIPTICAL  \n
     *        +    POLYGONAL  \n
     *  - CHAIN
     *
     * \see Commented configuration file samples
     * \param filename Full path to the configuration file
     * \return True if the system was successfully loaded. False otherwise. The Optix LastError is set when the error comes from the OptiX element implementation.
     * \n It is not always set when the error comes from the config file itself; either wrong indentation or syntax
     */
    DLL_EXPORT bool LoadConfigurationFile(const char* filename);


    /** \brief Load a new system from a Solemio file
     *
     *  All Solemio elements are not exactly converted
     * \param filename the full path of the file to load
     * \return true  if loading was complete, false otherwise and OptixLastError is set
     */
    DLL_EXPORT bool LoadSolemioFile(char * filename);


    /** \brief Loads a datafile created by SaveSystem()
     *
     * \deprecated Use LoadSystemFromXml() instead
     * \param filename const char*
     * \return true if the file was found and read otherwise return false  and  the OptiX_last_error is set
     *   Loads a complete system discarding the old one if any.
     */
    DLL_EXPORT bool LoadSystem(const char* filename);


    /** \brief Clear previous system and load a new one from an XML file <i>(not yet tested)</i>
     *
     * \param filename Name of the input file
     * \return false in case of error and the OptiXLastError will be set.  true otherwise
     */
    DLL_EXPORT bool LoadSystemFromXml(const char * filename);


    DLL_EXPORT void MemoryDump(void* address, uint64_t size); /**< debugging function */


    /** \brief Propagate all rays generated in the source through the element chain
     *
     * \param elementID ID of the element which must be of source type
     * \return false if element is invalid or is not a source; return true otherwise
     *
     *  This function propagates the set of rays defined in the source "impact" list though the elements linked from here.\n
     *  For a new ray tracing ClearImpact should be previously called,\n
     *  either from the next element if the source impacts need not be changed,\n
     *  either from
     */
    DLL_EXPORT bool Radiate(size_t elementID);


    /** \brief Change the wavelength all rays generated in the source, then propagate them through the element chain
     *
     * \param elementID ID of the element which must be of source type
     * \param wavelength  the radiation wavelength (must be  >0)
     * \return false if element is invalid or is not a source, or wavelength <0; return true otherwise
     *
     *  \see Radiate, Generate
     */
    DLL_EXPORT bool RadiateAt(size_t elementID, double wavelength);


    /** \brief **Deprecated** Save all the elements of the current system to a file
     *
     * \deprecated Use SaveSystemAsXml() instead
     * \param filename the full path to the output file
     *  \return true if the file was written, otherwise returns false  and  the OptiX_last_error is set
     *  The datafile is mainly a text file with special characters as separators
     */
    DLL_EXPORT bool SaveSystem(const char* filename);


    /** \brief Save the system in memory to a file in XML format
     *
     * \param filename Name of the output file
     * \return false in case of error and the OptiXLastError will be set.  true otherwise
     */
    DLL_EXPORT bool SaveSystemAsXml(const char * filename);


    /** \brief switch the global aperture activity flag
     * \param activity new value of activity. If true, aperture stops are taken into account for ray intensity
     */
    DLL_EXPORT void SetAperturesActive(const bool activity);


 #ifdef  HAS_REFLEX
    /** \brief Assign a Coating to a reflective surface - **requires RefleX**
     *
     * \param elementID The Identifier of the optical element
     * \param coatingTable The table where the coating is defined
     * \param coatingName Name of the coating in the table
     * \return True if assignation succeeds; False otherwise. OptiXLastError should give a reason
     */
    DLL_EXPORT bool SetCoating(size_t elementID,const char* coatingTable, const char* coatingName);
#endif // HAS_REFLEX


    /** \brief Modifies an element parameter
     *
     * <b>The type, group and flags of a parameter are internally defined and will not be changed no matter how they are defined</b>
     * The function will fail if the element or tag doesn't exist
     * \param elementID The ID of the element to modify
     * \param paramTag the name of the parameter to change
     * \param[in] paramData the new parameter data as a Parameter struct
     * \return true if the element ID is valid and the parameter was found and successfully changed;
     *      \n false if the parameter was not found or cannot be modified; OptiXLast error is set in this case
     */
    DLL_EXPORT bool SetParameter(size_t elementID,const char* paramTag, Parameter paramData);


    /** \brief sets the impact recording mode of an element
     *
     * \param elementID the Id of the element to modify
     * \param recordingMode the  new recording mode which must be a value of the RecordMode enumeration
     * \return true if the element can record and the recording mode is valid; false if the element cannot record impacts (groups) or mode is invalid
     */
    DLL_EXPORT bool SetRecording(size_t elementID, int recordingMode);


   /** \brief Set the transmission or reflexion mode of the element. (only available for gratings)
     *
     * \param elementID the ID of the element to chang
     * \param transmit true value to make the element transmissive, false if the element is reflective
     * \return true if the element is a grating and transmission mode was set, false otherwise and the element is unchanged
     */
    DLL_EXPORT bool SetTransmissive(size_t elementID, bool transmit);


    /** \brief Radiate a "wavefront" emitted from a single point source  with aperture angle distributed on a regular grid
     *
     * Align and radiate a wavefront of given wavelength and aperture angle from the nominal source position
     * \param elementID if the corresponding element is a source, the wavefront will be radiated from the position of this source.
     *  If it is not a source the most upstream source position will be used
     * \param WFemitParams Parameters of emission wavelength, aperture and number of points in this aperture
     * \return true if successfull; false, in case of an error and OptiXLastError is set
     */
    DLL_EXPORT bool WaveRadiate(size_t elementID, WFemission WFemitParams);
/** \} */  //end of mainAPI group


/** \ingroup surferrorAPI
*   \{
*/

    DLL_EXPORT bool setSurfaceErrors(size_t elementID,double xmin, double xmax, double ymin, double ymax,
                                     int64_t xsize, int64_t ysize, const double* heightErrors);


    /** \brief delete the height error map associated to the surface, which hence become perfect again
     *
     * \param elementID size_t
     * \return DLL_EXPORT bool
     *
     */
    DLL_EXPORT bool unsetSurfaceErrors(size_t elementID );


/** \} */  //end of surferrorAPI group
  //  DLL_EXPORT bool AddElementsFromXml(const char * filename);  la gestion des nom en double doit être testée

#ifdef __cplusplus
}
#endif



#endif // INTERFACE_H_INCLUDED
