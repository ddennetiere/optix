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
extern map<string,CoatingTable> coatingTables;  /**< \brief the coating table enables interpolation of the registered coatings on a unique energy and angle grid */





#ifdef __cplusplus
extern "C"
{
#else
#include <stdbool.h>
#endif


/** \ingroup reflectivityAPI
*   \{
*/


/* ******************************************************
*         Databases interface functions
*********************************************************
*/


/** \brief Opens a DabaX database file and add it to the list of accessible databases
 *
 * \param filepath An absolute or relative path to the database
 * \return the name under which this database will be referred as or an empty string if an error occured, in which cases OptiXLastError is set
 * \see also EnumeratesDatabases
 */
DLL_EXPORT const char* OpenDatabase(const char* filepath);

/** \brief close the named DabaX database  and removes it from the list of accessible databases <b> if it is no longer referred to by other objects </b>
 *
 * \param database the name of the database to close
 * \return true if the database was found, is no longer used and removed from the list.
 *  It will return false if the database  was not found in the list or is still in use; OptiXLastError is set accordingly
 */
DLL_EXPORT bool CloseDatabase(const char* database);

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

/** \brief release an enumeration handle returned by EnumerateDatabases()
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


/* ******************************************************
*         Index tables interface functions
*********************************************************
*/

/** \brief Creates a new empty index table
 *
 * It is wise to create different index tables for material and compounds whose parameters are provided by different Databases since the Energy grids of different tables may badly overlap
 * Remind that the energy interpolation range of an index table is the common range of all listed materials
 * \param name The name the table will be given
 * \return true if the table was created, false if a table of same name already exists. The OptiXLastError will be set in this case.
 */
DLL_EXPORT bool CreateIndexTable(const char* name);

/** \brief Delete an index table if it is no longer used and remove it of the list of index tables
 *
 * \param name The name of the index table to suppress
 * \return true if the index table was removed; false if the table name was not found or the table is still in use. The OptiXLastEror is set accordingly
*/
DLL_EXPORT bool DeleteIndexTable(const char* name);

/** \brief enumerates the names of the index tables defines in the application
 *
 * \param[in,out] pHandle address of a location containing: \n on input, a handle to the current enumerator or 0 to get the first indexTable;
 *  \n on output, a  handle to underlying enumerator of the parameter list or 0 if the retrieved indexTable is the last one or an error occurs
 * \param[out] nameBuffer char* a character buffer to receive the retrieved indexTable name.
 * \param[in] bufSize The size of the name buffer
 * \return true if a new item was found and returned; false if  the name buffer was too small and the OptiXLastError will be set
 * in this latter case, the item name is truncated and the enumerator is  invalidated  and set to 0
 *
 * Memory leaks will occur if the handle is dropped before it is returned as 0. If needed clear a non zero handle with ReleaseIndexTableEnumHandle()
 */
DLL_EXPORT bool EnumerateIndexTables(size_t * pHandle, char * nameBuffer, const int bufSize);

/** \brief release an non 0 enumeration handle returned by EnumerateIndexTables()
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
DLL_EXPORT bool AddCompound(const char* table,const char* database, const char* formula,double density);

/** \brief removes a material entry in a MaterialTable if it is unused
 *
 * \param indexTable The name of the index Table to update
 * \param materialName The name of the material entry to suppress
 * \return true if the entry was removed. false if the material is not found in the table or is used by a coating. OptiXLastError will report the reason of failure
 */
DLL_EXPORT bool RemoveMaterial(const char* indexTable, const char* materialName);

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


/* ******************************************************
*         Coatings interface functions
*********************************************************
*/


/** \brief Creates a new empty coating  table
 *
 * Each table is given a unique energy and angle grid on which the reflectivity is computed in advance allowing fast interpolation during ray tracing
 *
 * \param name The name the table will be given
 * \return true if the table was created, false if a table of same name already exists. The OptiXLastError will be set in this case.
 */
DLL_EXPORT bool CreateCoatingTable(const char* name);


/** \brief enumerate the defined  coatingTables
 *
 * \param[in,out] pHandle address of a location containing: \n on input, a handle to the current enumerator or 0 to get the first coatingTable;
 *  \n on output, a  handle to underlying enumerator of the parameter list or 0 if the retrieved coatingTable is the last one or an error occurs
 * \param[out] nameBuffer a character buffer to receive the retrieved coatingTable name.
 * \param[in] bufSize size of the buffer
 * \return true if the function was successful, 0 otherwise and he OptiXLastError is set
 *
 * Memory leaks will occur if the handle is dropped before it is returned as 0. If needed clear a non zero handle with ReleaseCoatingTableEnumHandle()
 */
DLL_EXPORT bool EnumerateCoatingTables(size_t * pHandle, char * nameBuffer, const int bufSize);



/** \brief release an non 0 enumeration handle returned by EnumerateCoatingTables()
 * \param handle a non null handle returned by EnumerateCoatingTables()
 */
DLL_EXPORT void ReleaseCoatingTableEnumHandle(size_t handle);


/** \brief enumerate the coatings available in an coatingTable
 *
 * \param[in] coatingTable const Name of the coatingTable to search in
 * \param[in,out] pHandle address of a location containing: \n on input, a handle to the current enumerator or 0 to get the first coating;
 *  \n on output, a  handle to underlying enumerator of the parameter list or 0 if the retrieved coating is the last one or an error occurs
 * \param[out] nameBuffer a character buffer to receive the retrieved coating name.
 * \param[in] bufSize size of the buffer
 * \return true if the function was successful, 0 otherwise and he OptiXLastError is set
 *
 * Memory leaks will occur if the handle is dropped before it is returned as 0. If needed clear a non zero handle with ReleaseCoatingEnumHandle()
 */
DLL_EXPORT bool EnumerateCoatings(const char* coatingTable, size_t * pHandle, char * nameBuffer, const int bufSize);


/** \brief release an non 0 enumeration handle returned by EnumerateCoatings()
 * \param handle a non null handle returned by EnumerateCoatings()
 */
DLL_EXPORT void ReleaseCoatingEnumHandle(size_t handle);

/** \brief Create a new coating in a coating table
 *
 * \param coatingTable Name of the coating table to new coating will be added to
 * \param coatingName the name to be given to the new coating
 * \param indexTable The index table where the substrate material will be found
 * \param substrateMaterial the name of the substrate material
 * \return true if the coating was created; false if it wasn't. The OptiXLastError is set appropiately
 */
DLL_EXPORT bool CreateCoating(const char* coatingTable, const char * coatingName, const char* indexTable, const char* substrateMaterial);


/** \brief Removes a coating from a coating table
 *
 * \param coatingTable name of the table from where the coating must be removed
 * \param coatingName name of the coating to remove
 * \return true if the layer was removed ; false if it couldn't, in particular because the coating is still in use. The OptiXLastError is set appropiately
 */
DLL_EXPORT bool RemoveCoating(const char* coatingTable, const char * coatingName);


/** \brief Adds a new layer on top of the layer stack of a particular coating
 *
 * \param coatingTable name of the table where the coating to modify is located
 * \param coatingName name of the coating to modify
 * \param indexTable Table where the optical parameters of the layer material can be found
 * \param layerMaterial name of the layer material
 * \param thickness the thickness of the layer (in m). Must be positive
 * \param compactness ratio of the layer density to that of the material tabulated value. Should be positive (usually close to 1.)
 * \return true if the layer was added; false if it wasn't. The OptiXLastError is set appropiately
 */
DLL_EXPORT bool AddCoatingLayer(const char* coatingTable, const char * coatingName, const char* indexTable,
                                const char* layerMaterial, double thickness, double compactness);

/** \brief inserts  a new layer inside or on top of the layer stack of a particular coating
 *
 * \param coatingTable name of the table where the coating to modify is defined
 * \param coatingName name of the coating to modify
 * \param layerIndex position of the new layer in the stack after insertion. If layerIndex <0 then the layer is added on top of the stack.
 * \n if layerIndex > LayerNumber an error will occur
 * \param indexTable Table where the optical parameters of the layer material can be found
 * \param layerMaterial name of the layer material
 * \param thickness the thickness of the layer (in m). Must be positive
 * \param compactness ratio of the layer density to that of the material tabulated value. Should be positive (usually close to 1.)
 * \return true if the layer was inserted or added; false if it wasn't. The OptiXLastError is set appropiately
 */
DLL_EXPORT bool InsertCoatingLayer(const char* coatingTable, const char * coatingName, int64_t layerIndex,
                                   const char* indexTable, const char* layerMaterial, double thickness, double compactness);

/** \brief retrieves the number of layers fdefine in a specified Coating
 *
 * \param[in] coatingTable  name of the table where the coating is defined
 * \param[in] coatingName coatingName name of the coating to modify
 * \param[out] layerNumber variable to return the number of defined layers
 * \return true if the the number of layers was returned in layerNumber; false if it wasn't. The OptiXLastError is set appropiately
 */
DLL_EXPORT bool GetLayerNumber(const char* coatingTable, const char * coatingName, size_t *layerNumber);


/** \brief Change the material or parameters of one layer in a coating
 *
 * \param coatingTable name of the table where the coating to modify is defined
 * \param coatingName name of the coating to modify
 * \param layerIndex position of the layer to modify. if  layerIndex > LayerNumber an error will occur
 * \param indexTable Table where the optical parameters of the layer material can be found
 * \param layerMaterial name of the layer material
 * \param thickness the thickness of the layer (in m). Must be positive
 * \param compactness ratio of the layer density to that of the material tabulated value. Should be positive (usually close to 1.)
 * \return true if the layer was inserted or added; false if it wasn't. The OptiXLastError is set appropiately

 */
DLL_EXPORT bool SetCoatingLayer(const char* coatingTable, const char * coatingName, size_t layerIndex,
                                const char* indexTable, const char* layerMaterial, double thickness, double compactness);

DLL_EXPORT bool GetCoatingLayer(const char* coatingTable, const char * coatingName, size_t layerIndex,
                                char* matBuffer, const int bufSize, double *p_thickness, double *p_compactness);


DLL_EXPORT bool SetCoatingRoughness(const char* coatingTable, const char * coatingName, double roughness);

DLL_EXPORT bool GetCoatingRoughness(const char* coatingTable, const char * coatingName, double *p_roughness);



DLL_EXPORT bool SetCoatingTableAngles(const char* coatingTable, double angleMin, double angleMax, size_t numAngles);

DLL_EXPORT bool GetCoatingTableAngles(const char* coatingTable, double *p_angleMin, double *p_angleMax, size_t *p_numAngles);


DLL_EXPORT bool SetCoatingTableEnergies(const char* coatingTable, double energyMin, double energyMax,
                                        int64_t numEnergies, bool logSpacing);

DLL_EXPORT bool GetCoatingTableEnergies(const char* coatingTable, double *p_energyMin, double *p_energyMax,
                                        int64_t *p_numEnergies, bool *p_logSpacing);




/** \} */     // ingroup reflectivityAPI


#ifdef __cplusplus
}
#endif


#endif // HAS_REFLEX


#endif // REFLECTIVITYAPI_H_INCLUDED
