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

#include "types.h"

using namespace std;

   /** \defgroup globalc  Interface C functions
    *
    *   \brief Functions exported by the C interface
    * \{
    */

extern "C"
{
    /** \brief Create an optical element (or group)
     *
     * \param type  runtime class of the new element
     * \param name name of the new element
     * \return if success  a unique ID identifying the newly created element. Otherwise returns 0
     */
    DLL_EXPORT size_t CreateElement(const char* type, const char* name);

    /** \brief enumerates the element list of the current system
     *

     * \param[in,out] pHandle address of a location containing: \n on input, a handle to the current enumerator or 0 to get the first element of the system;
     *  \n on output, a handle to underlying enumerator of the parameter list or 0 if the retrieved element is the last one
     * \param[out] elemID a pointer to a longlong interger which will  receive the element ID
     * \param[out] nameBuffer char* a character buffer to receive the retrieved element name.
     * \param[in] bufSize The size of the tag buffer
     * \return true if a new parameter was found and returned; false if the parameter was the last one
     * Memory leeks will occur if the handle is dropped before it is returned as 0. If needed clear a non zero hande with ReleaseElementHandle()
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
     * \param[in] bufSize the size of the buffer in bytes
     * the buffer will be empty if element ID doesn't exists
     */
    DLL_EXPORT void GetElementName(size_t elementID, char* strBuffer, int bufSize);

    /** \brief  removes an element by its name
     *
     * \param name name of the element to delete
     * \return the Id of the removed element; 0 if the element was not found
     */
    DLL_EXPORT size_t RemoveElement_byName(const char* name);

    /** \brief removes an element by its  ID
     *
     * \param elementID the ID of the element to delete
     * \return the Id of the removed element; 0 if the element was not found
     */
    DLL_EXPORT size_t RemoveElement_byID(size_t elementID);

    /** \brief Chain two elements by their names
     *
     *  Links to element together. The upstream link of previous and downstream link of next will remain unchanged
     *  Set one link to 0 to cut the link chain
     * \param previous name of the element to place upstream
     * \param next name of the element to place downstream
     */
    DLL_EXPORT void ChainElement_byName(const char* previous, const char* next);

    /** \brief Chain two elements by their IDs
     *
     *  Links to element together. The upstream link of previous and downstream link of next will remain unchanged
     *  Set one link to 0 to cut the link chain
     * \param prevID  ID of the element to place upstream
     * \param nextID  ID of the element to place downstream
     */
    DLL_EXPORT void ChainElement_byID(size_t prevID, size_t nextID);

    /** \brief Gets the element immediately upstream of the given one in the link chain
     *
     * \param elementID the ID of the given element
     * \return the ID of the previous element
     */
    DLL_EXPORT size_t GetPreviousElement(size_t elementID);

    /** \brief Gets the element immediately downstream of the given one in the link chain
     *
     * \param elementID the ID of the given element
     * \return the ID of the next element
     */
    DLL_EXPORT size_t GetNextElement(size_t elementID);

    /** \brief Modifies an element parameter
     *
     * \param elementID The ID of the element to modify
     * \param paramTag the name of the parameter to change
     * \param[in] paramData the new parameter data as a Parameter struct
     * \return true if the parameter was found and successfully changed;  \n false if the parameter was not found or cannot be modified
     */
    DLL_EXPORT bool SetParameter(size_t elementID,const char* paramTag, Parameter paramData);

    /** \brief retrieves an element parameter
     *
     * \param elementID  The ID of the element to query
     * \param paramTag the name of the parameter to get
     * \param paramData a pointer to a Parameter struct to receive the parameter data
     * \return true if the parameter was found and successfully returned;  \n false if the parameter was not found
     */
    DLL_EXPORT bool GetParameter(size_t elementID,const char* paramTag, Parameter* paramData);

    /** \brief enumerates the parameter list of a given optical element
     *
     * \param[in] elementID The ID of the element to query
     * \param[in,out] pHandle address of a location containing: \n on input, a handle to the current enumerator or 0 to get the first parameter;
     *  \n on output, a handle to underlying enumerator of the parameter list or 0 if the retrieved element is the last one
     * \param[out] tagBuffer char* a character buffer to receive the retrieved parameter name.
     * \param[in] bufSize The size of the tag buffer
     * \param[out] paramData a pointer to a Parameter struct to receive the parameter data
     * \return true if a new parameter was found and returned; false if the parameter was the last one
     * Memory leeks will occur if the handle is dropped before it is returned as 0. If needed clear a non zero hande with ReleaseParameterHandle()
     */
    DLL_EXPORT bool EnumerateParameters(size_t elementID, size_t * pHandle, char* tagBuffer, const int bufSize , Parameter* paramData);

    /** \brief release a parameter enumeration handle returned by EnumerateParameter()
     *
     * \param handle a non null handle returned by EnumerateParameter()
     *
     */
    DLL_EXPORT void ReleaseParameterEnumHandle(size_t handle);

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

}
/** \} */  //end of globalc group

#endif // INTERFACE_H_INCLUDED
