#ifndef INTERFACE_H_INCLUDED
#define INTERFACE_H_INCLUDED

////////////////////////////////////////////////////////////////////////////////
/**
*      \file           interface.h
*
*      \brief         TODO  fill in file purpose
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



   /** \defgroup globalc  Interface C functions
    *
    *   \brief Functions exported by the C interface
    * \{
    */
#ifdef __cplusplus
extern "C"
{
#else
#include <stdbool.h>
#endif

    /** \brief Dumps the version number and compilation date of the library to the console
     */
    DLL_EXPORT void Version();

    /** \brief Create an optical element (or group)
     *
     * \param type  runtime class of the new element
     * \param name name of the new element
     * \return if success  a unique ID identifying the newly created element. Otherwise returns 0 and the OptiXError is set
     */
    DLL_EXPORT size_t CreateElement(const char* type, const char* name);

    /** \brief Check there is an element with this ID in the cirrunt system
     *
     * \param ID  The element ID to check
     * \return true if the element exists , false otherwise
     *
     */
    DLL_EXPORT bool IsElementValid(size_t  ID);

    /** \brief check and reset the error status
     *
     * \param buffer a char buffer to receive the error string. Can be NULL if not needed
     * \param bufferSize  the size in bytes of the buffer (256 is safe)
     * \return the error flag. Error status is reset to no error on exit.
     */
    DLL_EXPORT bool GetOptiXLastError(char* buffer, int bufferSize);

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
     * Memory leeks will occur if the handle is dropped before it is returned as 0. If needed clear a non zero handle with ReleaseElementHandle()
     */
    DLL_EXPORT bool EnumerateElements(size_t * pHandle, size_t* elemID, char * nameBuffer, const int bufSize);

    /** \brief release an element enumeration handle returned by EnumerateElement()
     *
     * \param handle a non null handle returned by EnumerateElement()
     *
     */
    DLL_EXPORT void ReleaseElementEnumHandle(size_t handle);


    /** \brief retrieves the unique ID of an element from its name
     *
     * \param elementName name of the searched element
     * \return the unique ID of the element having this name if it exists, 0 otherwise
     */
    DLL_EXPORT size_t GetElementID(const char* elementName);

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

    /** \brief  delete an element by its name and remove its reference from the system
     *
     * \param name name of the element to delete
     * \return false if the element was not found; true otherwise
     */
    DLL_EXPORT bool DeleteElement_byName(const char* name);

    /** \brief delete an element by its  ID and remove its reference from the system
     *
     * \param elementID the ID of the element to delete
     * \return false if the element was not found; true otherwise
     */
    DLL_EXPORT bool DeleteElement_byID(size_t elementID);

    /** \brief Chain two elements by their names
     *
     *  Links to element together. The upstream link of previous and downstream link of next will remain unchanged
     *  Set one link to "" to cut the link chain
     * \param previous name of the element to place upstream
     * \param next name of the element to place downstream
     * \return false if both names are empty("") or one is invalid , true otherwise
     */
    DLL_EXPORT bool ChainElement_byName(const char* previous, const char* next);

    /** \brief Chain two elements by their IDs
     *
     *  Links to element together. The upstream link of previous and downstream link of next will remain unchanged
     *  Set one link to 0 to cut the link chain
     * \param prevID  ID of the element to place upstream
     * \param nextID  ID of the element to place downstream
     * \return false if both IDs are 0  or one is invalid , true otherwise
     */
    DLL_EXPORT bool ChainElement_byID(size_t prevID, size_t nextID);

    /** \brief Gets the element immediately upstream of the given one in the link chain
     *
     * \param elementID the ID of the given element
     * \return the ID of the previous element, or 0 if the element either is the first of the link chain either is invalid
     */
    DLL_EXPORT size_t GetPreviousElement(size_t elementID);

    /** \brief Gets the element immediately downstream of the given one in the link chain
     *
     * \param elementID the ID of the given element
     * \return the ID of the next element, or 0 if the element either is the last of the link chain either is invalid
     */
    DLL_EXPORT size_t GetNextElement(size_t elementID);

    /** \brief Check if element is used in transmission rather than reflexion mode (mainly useful for gratings)
     *
     * \param elementID the Id of the element to inquire of
     * \return true if the element is transmissive ; false otherwise
     */
    DLL_EXPORT bool GetTransmissive(size_t elementID);

    /** \brief Set the transmission or reflexion mode of the element. (only available for gratings)
     *
     * \param elementID the ID of the element to chang
     * \param transmit true value to make the element transmissive, false if the element is reflective
     * \return true if the element is a grating and transmission mode was set, false otherwise and the element is unchanged
     */
    DLL_EXPORT bool SetTransmissive(size_t elementID, bool transmit);

    /** \brief retrieves the impact recording mode of an element
     *
     * \param elementID the Id of the element to inquire of
     * \return the recording mode which is a value of the RecordMode enumeration
     */
    DLL_EXPORT int GetRecording(size_t elementID);

    /** \brief sets the impact recording mode of an element
     *
     * \param elementID the Id of the element to modify
     * \param recordingMode the  new recording mode which must be a value of the RecordMode enumeration
     * \return true if the element can record and the recording mode is valid; false if the element cannot record impacts (groups) or mode is invalid
     */
    DLL_EXPORT bool SetRecording(size_t elementID, int recordingMode);

    /** \brief Modifies an element parameter
     *
     * \param elementID The ID of the element to modify
     * \param paramTag the name of the parameter to change
     * \param[in] paramData the new parameter data as a Parameter struct
     * \return true if the element ID is valid and the parameter was found and successfully changed;  \n false if the parameter was not found or cannot be modified
     */
    DLL_EXPORT bool SetParameter(size_t elementID,const char* paramTag, struct Parameter paramData);

    /** \brief retrieves an element parameter
     *
     * \param elementID  The ID of the element to query
     * \param paramTag the name of the parameter to get
     * \param paramData a pointer to a Parameter struct to receive the parameter data
     * \return true if the element Id is valid and the parameter was found and successfully returned;  \n false if the parameter was not found or elementId is invalid
     */
    DLL_EXPORT bool GetParameter(size_t elementID,const char* paramTag, struct Parameter* paramData);

    /** \brief enumerates the parameter list of a given optical element
     *
     * \param[in] elementID The ID of the element to query
     * \param[in,out] pHandle address of a location containing: \n on input, a handle to the current enumerator or 0 to get the first parameter;
     *  \n on output, a handle to underlying enumerator of the parameter list or 0 if the retrieved element is the last one
     * \param[out] tagBuffer char* a character buffer to receive the retrieved parameter name.
     * \param[in] bufSize The size of the tag buffer
     * \param[out] paramData a pointer to a Parameter struct to receive the parameter data
     * \return true if a new parameter was found and returned without error ; false if elementID was invalid or the buffer was too small, and OptixLastError is set
     * in this latter case, the parameter name is truncated and the enumerator is  invalidated  and set to 0
     * Memory leeks will occur if the handle is dropped before it is returned as 0. If needed clear a non zero hande with ReleaseParameterHandle()
     */
    DLL_EXPORT bool EnumerateParameters(size_t elementID, size_t * pHandle, char* tagBuffer, const int bufSize , struct Parameter* paramData);

    /** \brief release a parameter enumeration handle returned by EnumerateParameter()
     *
     * \param handle a non null handle returned by EnumerateParameter()
     *
     */
    DLL_EXPORT void ReleaseParameterEnumHandle(size_t handle);

    /** \brief Align the element chain sarting from the given element
     *
     * \param elementID first element of the chain to align
     * \param wavelength alignment wavelength (for gratings). It can be 0.
     * \return true if alignment was successful, false otherwise and OptiXLastError is et
     *
     */
    DLL_EXPORT bool Align(size_t elementID, double wavelength);

    /** \brief generate source rays of the given wavelength but do not run the ray tracing, (only valid for sources).
     *
     * \param elementID ID of the element which must be of source type
     * \param wavelength  the radiation wavelength (must be  >0)
     * \return the number of generated rays. It will be 0 if elementID is invalid or is not a source, or wavelength <0 and OptiXLastError is set,
     *      otherwise GetOptiXLastError will return NoError even if 0 rays were created
     *
     * ClearImpact() is not called before executing Generate(). Therefore ray congruences with different wavelengths can be superimposed.
     * Also note the that Radiate() is not either executed,  but must be called independantly.
     */
    DLL_EXPORT int Generate(size_t elementID, double wavelength);

    /** \brief Propagate all rays generated in the source through the element chain
     *
     * \param elementID ID of the element which must be of source type
     * \return false if element is invalid or is not a source; return true otherwise
     */
    DLL_EXPORT bool Radiate(size_t elementID);

    /** \brief Change the wavelength all rays generated in the source, then propagate them through the element chain
     *
     * \param elementID ID of the element which must be of source type
     * \param wavelength  the radiation wavelength (must be  >0)
     * \return false if element is invalid or is not a source, or wavelength <0; return true otherwise
     */
    DLL_EXPORT bool RadiateAt(size_t elementID, double wavelength);

    /** \brief Clear impacts on this elements and all recording elements which follows in the link chain
     *
     * \param elementID the starting element ID
     * \return false if elementId is not valid, true otherwise
     */
    DLL_EXPORT bool ClearImpacts(size_t elementID);

    /** \brief Save all the elements of the current system to a file
     *
     * \param filename the full path to the output file
     *  \return true if the file was written, otherwise returns false  and  the OptiX_last_error is set
     *  The datafile is mainly a text file with special characters as separators
     */
    DLL_EXPORT bool SaveSystem(const char* filename);

    /** \brief Loads a datafile created by
     *
     * \param filename const char*
     * \return true if the file was found and read otherwise return false  and  the OptiX_last_error is set
     *   Loads a complete system discarding the old one if any.
     */
    DLL_EXPORT bool LoadSystem(const char* filename);

    /** \brief Load a new system from a Solemio file
     *
     *  All Solemio elements are not exactly converted
     * \param filename the full path of the file to load
     * \return true  if loading was complete, false otherwise and OptixLastError is set
     */
    DLL_EXPORT bool LoadSolemioFile(char * filename);

    /** \brief Evaluates a spot diagram at a given distance from an element and returns it in a C_DiagramStruct
     *
     * \param elemId The ID of the element in the space of which the spot diagram must be evaluated. Impact recording must be active on this object
     * \param diagram  the address of a prefilled C_DiagramStruct to receive the data. The m_dim member <b> must  have the value of 5 </b>, and the statistics buffers
     * must be already allocated with enough space for 5 int int values each. The m_reserved field <b> must be set </b> and the  m_spots buffer must be preallocated
     * with enough space to accommodate m_reserved spot vectors of 5 double values. The calling function is responsible for initializing and deleting this structure
     * in the appropriate manner.
     * \param  distance (optional) The distance from the element where the spot diagram is evaluated. Default value is 0
     * \return a boolean value, true for success, false for failure and OptixLastError is set.
     */
    DLL_EXPORT bool GetSpotDiagram(size_t elemId, struct C_DiagramStruct * diagram, double distance);

    /** \brief Write a C_DiagramStruct to a file in binary format <i>(it is questionnable whether it should be maintained as is,in the interface)</i>
     *
     *  The format follows closely the structure pattern. 4 int32 with the nthe dimension m_dim of the arrays, maximum capacity, spot count, and lost rays,
     *  4 arrays of  m_dim doubles with the statistics (min, max, mean, sigma), then the array of m_dim * m_count  doubles
     * \param filename name of the file
     * \param cdiagram a pointer to the C_DiagramStruct to be written on file
     * \return DLL_EXPORT bool
     */
    DLL_EXPORT bool DiagramToFile(const char* filename, struct C_DiagramStruct* cdiagram);

    /** \brief Save the system in memory to a file in XML format
     *
     * \param filename Name of the output file
     * \return false in case of error and the OptiXLastError will be set.  true otherwise
     */
    DLL_EXPORT bool SaveSystemAsXml(const char * filename);

    /** \brief Clear previous system and load a new one from an XML file <i>(not yet tested)</i>
     *
     * \param filename Name of the input file
     * \return false in case of error and the OptiXLastError will be set.  true otherwise
     */
    DLL_EXPORT bool LoadSystemFromXml(const char * filename);

  //  DLL_EXPORT bool AddElementsFromXml(const char * filename);  la gestion des nom en double doit être testée

    /** \brief Dump the content of an XML system file to stdout (given for convenience)
     *
     * \param filename Name of the input file
     * \return false in case of error and the OptiXLastError will be set.  true otherwise
     */
    DLL_EXPORT bool DumpXML(const char* filename);


#ifdef __cplusplus
}
#endif

/** \} */  //end of globalc group

#endif // INTERFACE_H_INCLUDED
