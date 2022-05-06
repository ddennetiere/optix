#ifndef REFLECTIVITYAPI_H_INCLUDED
#define REFLECTIVITYAPI_H_INCLUDED

/**
*************************************************************************
*   \file       ReflectivityAPI.h

*
*   \brief     C interface to the Reflectivity functions provided by RefleX library
*
*
*
*
*   \author             François Polack  <francois.polack@synchroton-soleil.fr>
*   \date               Creation : 2022-03-22
*
*   \date               Last update: 2022-03-22

*
*
 ***************************************************************************/

#ifdef HAS_REFLEX



#ifdef BUILD_DLL
    #define DLL_EXPORT __declspec(dllexport)
#else
    #ifdef NO_DLL
        #define DLL_EXPORT
    #else
        #define DLL_EXPORT __declspec(dllimport)
    #endif
#endif

#include <CoatingTable.h>

extern map<string, XDatabase> dataBases;   /**< \brief a set of DABAX data bases indexed for fast access  */
extern map<string, MaterialTable> indexTables; /**< \brief a set of tables of material index. Material tables interpolate
                                        *  the index of the registered materials on a common energy grid with minimum number of steps*/
extern CoatingTable coatingTable;  /**< \brief the coating table enables interpolation of the registered coatings on a unique energy and angle grid */





#ifdef __cplusplus
extern "C"
{
#else
#include <stdbool.h>
#endif


/** \ingroup reflectivityAPI
*   \{
*/


/** \brief Opens a DabaX database file
 *
 * \param filepath An absolute or relative path to the database
 * \return the name under which this database will be referred as or an empty string if an error occured, in which cases OptiXLastError is set
 * \see also EnumeratesDatabases
 */
DLL_EXPORT const char* OpenDatabase(const char* filepath);


/** \brief enumerates the names of the DabaX databases open in the application
 *
 * \param[in,out] pHandle address of a location containing: \n on input, a handle to the current enumerator or 0 to get the first open database;
 *  \n on output, a  handle to underlying enumerator of the parameter list or 0 if the retrieved database is the last one or an error occurs
 * \param[out] nameBuffer char* a character buffer to receive the retrieved database name.
 * \param[in] bufSize The size of the name buffer
 * \return true if a new element was found and returned; false if  the name buffer was too small and the OptiXLastError will be set
 * in this latter case, the element name is truncated and the enumerator is  invalidated  and set to 0
 *
 * \attention Memory leaks will occur if the handle is dropped before it is returned as 0. If needed clear a non zero handle with ReleaseElementHandle()
 */
DLL_EXPORT bool EnumerateDatabases(size_t * pHandle, char * nameBuffer, const int bufSize);

/** \brief release an element enumeration handle returned by EnumerateDatabases()
 *
 * \param handle a non null handle returned by EnumerateDatabases()
 */
DLL_EXPORT void ReleaseDatabaseEnumHandle(size_t handle);

/** \brief Enumerates the entry of an open database
 *
 * \param dbaseName Name of the XDatabase the entry of which are requested
 * \param[in,out] pHandle pHandle address of a location to hold a pointer to the enumerator \see enumerateDatabases
 * \param nameBuffer character buffer to receive the names
 * \param bufSize size of the buffer
 * \return false and  OptiXLastError is set in case of an error
 * \attention Call  ReleaseDBEntryEnumHandle before destroying a non 0 returned handle to avoid memory leaks
 */
DLL_EXPORT bool EnumerateDbaseEntries(const char* dbaseName, size_t * pHandle, char * nameBuffer, const int bufSize);


/** \brief Release a non 0 handle
 * \param handle the handle to release
 */
DLL_EXPORT void ReleaseDBentryEnumHandle(size_t handle);

/** \brief Creates a new empty index table
 *
 * It is wise to create different index tables for material and compounds whose parameters are provided by different Databases since the Energy grids of different tables may badly overlap
 * Remind that the energy interpolation range of an index table is the common range of all listed materials
 * \param name The name the table will be given
 * \return true if the table was created, false if a table of same name already exists. The OptiXLastError will be set in this case.
 */
DLL_EXPORT bool CreateIndexTable(const char* name);

/** \brief enumerates the names of the index tables defines in the application
 *
 * \param[in,out] pHandle address of a location containing: \n on input, a handle to the current enumerator or 0 to get the first open database;
 *  \n on output, a  handle to underlying enumerator of the parameter list or 0 if the retrieved database is the last one or an error occurs
 * \param[out] nameBuffer char* a character buffer to receive the retrieved database name.
 * \param[in] bufSize The size of the name buffer
 * \return true if a new element was found and returned; false if  the name buffer was too small and the OptiXLastError will be set
 * in this latter case, the element name is truncated and the enumerator is  invalidated  and set to 0
 *
 * Memory leaks will occur if the handle is dropped before it is returned as 0. If needed clear a non zero handle with ReleaseIndexTableEnumHandle()
 */
DLL_EXPORT bool EnumerateIndexTables(size_t * pHandle, char * nameBuffer, const int bufSize);

/** \brief release an element enumeration handle returned by EnumerateIndexTables()
 * \param handle a non null handle returned by EnumerateIndexTables()
 */
DLL_EXPORT void ReleaseIndexTableEnumHandle(size_t handle);



/** \brief Adds a material from a database  to an IndexTable
 *
 * \param table the name of the IndexTable to be modified
 * \param database The name of the Xdatabase where the optical parameters of the new material will be loaded from
 * \param material The name of the material (in the Xdatabase) to add to the IndexTable
 * \return true id the specified element could be added to the table; false ortherwise, and the OptiXLastError is set
 */
DLL_EXPORT bool AddMaterial(const char* table, char* database, const char* material);

/** \brief Adds a compound material to an IndexTable
 *
 * \param table the name of the IndexTable to be modified
 * \param database The name of the Xdatabase where the optical parameters of the new material will be loaded from
 * \param formula The stoechiometric formula of the compound. This is also the name the compound will be  given
 * \param density the specific mass (g/cm^2)
 * \return true id the compound of specified formula could be added to the table; false ortherwise, and the OptiXLastError is set
 */
DLL_EXPORT bool AddCompound(const char* table, char* database, const char* formula,double density);

/** \brief enumerate the elements and compounds available in an IndexTable
 *
 * \param[in] indexTable const Name of the IndexTable to search in
 * \param[in,out] pHandle address of a location containing: \n on input, a handle to the current enumerator or 0 to get the first element;
 *  \n on output, a  handle to underlying enumerator of the parameter list or 0 if the retrieved element is the last one or an error occurs
 * \param[out] nameBuffer a character buffer to receive the retrieved database name.
 * \param[in] bufSize size of the buffer
 * \return true if the function was successful, 0 otherwise and he OptiXLastError is set
 */
DLL_EXPORT bool EnumerateMaterials(const char* indexTable, size_t * pHandle, char * nameBuffer, const int bufSize);

/** \brief Release a non 0 handle
 * \param Handle the handle to release
 */
DLL_EXPORT void ReleaseMaterialEnumHandle(size_t Handle);

/** \} */     // ingroup reflectivityAPI


#ifdef __cplusplus
}
#endif


#endif // HAS_REFLEX


#endif // REFLECTIVITYAPI_H_INCLUDED
