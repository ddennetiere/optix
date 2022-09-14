/**
 *************************************************************************
*   \file           ReflectivityAPI.cpp
*
*   \brief             implementation file
*
*
*
*   \author             François Polack  <francois.polack@synchroton-soleil.fr>
*   \date               Creation: 2022-03-22
*   \date               Last update: 2022-03-22
 ***************************************************************************/


#ifdef HAS_REFLEX
#include "ReflectivityAPI.h"
#include "elementBase.h"
#include <fstream>
#include <iostream>

/****************************   DataBases    ******************************************/

DLL_EXPORT const char* OpenDatabase(const char* filepath)
{
    ClearOptiXError();
    string key, filestr(filepath);
    fstream file(filestr, ios_base::in);
    if(!file.is_open())
    {
         SetOptiXLastError(string("Can't open file  ")+ filestr , __FILE__, __func__);
         return "" ;
    }
    size_t posext=filestr.find_last_of("."), posnam=filestr.find_last_of("/\\")+1;
    key=filestr.substr(posnam, posext-posnam);
    cout << "opening " << filestr  <<endl;
    file.close();

    try {
        auto retpair=dataBases.emplace(key,filestr);
        if(!retpair.second)
        {
            SetOptiXLastError(string("database ")+ key + " already exists" , __FILE__, __func__);
            return "";
        }
    }
    catch (runtime_error& rte)
    {
        SetOptiXLastError(string("can't open the database ")+ key + " reason: " + rte.what() , __FILE__, __func__);
        return "";
    }

    return key.c_str();
}

DLL_EXPORT bool CloseDatabase(const char* database)
{
    ClearOptiXError();

    map<string,XDatabase>::iterator dbit = dataBases.find(database);
    if(dbit==dataBases.end())
    {
        SetOptiXLastError(string("Database ")+ database + " is not currently open" , __FILE__, __func__);
        return false;
    }
    if(dbit->second.refCount() >0)
    {
        char str[256];
        sprintf(str,"Database %s is currently used by %lld objects", database, dbit->second.refCount());
        SetOptiXLastError(str , __FILE__, __func__);
        return false;
    }
    dataBases.erase(dbit);
    return true;
}

DLL_EXPORT bool EnumerateDatabases(size_t * pHandle, char * nameBuffer, const int bufSize)
{
    ClearOptiXError();
    map<string,XDatabase>::iterator* pit;
    if(*pHandle==0)
        pit=new map<string,XDatabase>::iterator(dataBases.begin());
    else
        pit=(map<string,XDatabase>::iterator* )*pHandle;

    strncpy(nameBuffer,(*pit)->first.c_str(), bufSize);
    if(bufSize < (int) (*pit)->first.size()+1)
    {
        SetOptiXLastError("nameBuffer is too small",__FILE__,__func__);
        delete pit;
        *pHandle=0;
        return false;
    }

    if(++(*pit)== dataBases.end())
    {
         delete pit;
         *pHandle=0;
    }
    else
        *pHandle=(size_t) pit;

    return true;
}

DLL_EXPORT void ReleaseDatabaseEnumHandle(size_t handle)
{
    if(handle)
        delete (map<string, XDatabase>::iterator*) handle;
}


DLL_EXPORT bool EnumerateDbaseEntries(const char* dbaseName, size_t * pHandle, char * nameBuffer, const int bufSize)
{
    ClearOptiXError();
    map<string,XDatabase>::iterator dbit = dataBases.find(dbaseName);
    if(dbit==dataBases.end())
    {
        SetOptiXLastError(string("Database ")+ dbaseName + " is not currently open" , __FILE__, __func__);
        return false;
    }
    string name;
    dbit->second.enumerateEntries(pHandle,name);
    if((size_t)bufSize <= name.size())
    {
        SetOptiXLastError(string("Database ")+ dbaseName + ": provided buffer is too small" , __FILE__, __func__);
        XDatabase::ReleaseHandle(*pHandle);
        *pHandle=0;
        return false;
    }
    strncpy(nameBuffer, name.c_str(),bufSize);
    return true;
}

DLL_EXPORT void ReleaseDBentryEnumHandle(size_t Handle)
{
    if(Handle)
        XDatabase::ReleaseHandle(Handle);  // methode statique de la classe
}


/****************************   Index Tables    ******************************************/

DLL_EXPORT bool CreateIndexTable(const char* name)
{
    ClearOptiXError();
    auto retpair=indexTables.emplace(name, MaterialTable(name));
    if( ! retpair.second)
    {
        SetOptiXLastError(string("Material table ")+ name + " already exists" , __FILE__, __func__);
        return false;
    }

    return true;
}

DLL_EXPORT bool DeleteIndexTable(const char* name)
{
    ClearOptiXError();

    map<string,MaterialTable>::iterator pit = indexTables.find(name);
    if(pit==indexTables.end())
    {
        SetOptiXLastError(string("There is currently no IndexTable named ")+ name , __FILE__, __func__);
        return false;
    }
    if(pit->second.refCount() >0)
    {
        char str[256];
        sprintf(str,"Index Table %s is currently used by %lld objects", name, pit->second.refCount());
        SetOptiXLastError(str , __FILE__, __func__);
        return false;
    }
    indexTables.erase(pit);
    return true;
}

DLL_EXPORT bool EnumerateIndexTables(size_t * pHandle, char * nameBuffer, const int bufSize)
{
    ClearOptiXError();
    map<string,MaterialTable>::iterator* pit;
    if(*pHandle==0)
        pit=new map<string,MaterialTable>::iterator(indexTables.begin());
    else
        pit=(map<string,MaterialTable>::iterator* )*pHandle;

    strncpy(nameBuffer,(*pit)->first.c_str(), bufSize);

    if(bufSize < (int) (*pit)->first.size()+1)
    {
        SetOptiXLastError("nameBuffer is too small",__FILE__,__func__);
        delete pit;
        *pHandle=0;
        return false;
    }

    if(++(*pit)== indexTables.end())
    {
         delete pit;
         *pHandle=0;
    }
    else
        *pHandle=(size_t) pit;

    return true;
}

DLL_EXPORT void ReleaseIndexTableEnumHandle(size_t handle)
{
    if(handle)
        delete (map<string, MaterialTable>::iterator*) handle;
}



DLL_EXPORT bool AddMaterial(const char* table, const char* database, const char* material)
{
    ClearOptiXError();
    map<string,MaterialTable>::iterator itit = indexTables.find(table);
    if(itit==indexTables.end())
    {
        SetOptiXLastError(string("IndexTable  ")+ table + " is not currently defined" , __FILE__, __func__);
        return false;
    }
    map<string,XDatabase>::iterator dbit = dataBases.find(database);
    if(dbit==dataBases.end())
    {
        SetOptiXLastError(string("Database ")+ database + " is not currently open" , __FILE__, __func__);
        return false;
    }

    try{
        itit->second.addMaterial(dbit->second, material);
    }
    catch (runtime_error &rte)
    {
        SetOptiXLastError(string("Failed to add material ")+ material+" to "+ table+" table from "+ database + " Reason:"+rte.what() , __FILE__, __func__);
        return false;
    }
    return true;
}

DLL_EXPORT bool AddCompound(const char* table, const  char* database, const char* formula,double density)
{
    ClearOptiXError();
    map<string,MaterialTable>::iterator itit = indexTables.find(table);
    if(itit==indexTables.end())
    {
        SetOptiXLastError(string("IndexTable  ")+ table + " is not currently defined" , __FILE__, __func__);
        return false;
    }
    map<string,XDatabase>::iterator dbit = dataBases.find(database);
    if(dbit==dataBases.end())
    {
        SetOptiXLastError(string("Database ")+ database + " is not currently open" , __FILE__, __func__);
        return false;
    }
    try{
        itit->second.addCompoundMaterial(dbit->second, formula,density);
    }
    catch (runtime_error &rte)
    {
        SetOptiXLastError(string("Failed to add compound ")+ formula+" to "+ table+" table from "+ database + " Reason:"+rte.what() , __FILE__, __func__);
        return false;
    }
    return true;
}

DLL_EXPORT bool RemoveMaterial(const char* indexTable, const char* materialName)
{
    ClearOptiXError();
    map<string,MaterialTable>::iterator itit = indexTables.find(indexTable);
    if(itit==indexTables.end())
    {
        SetOptiXLastError(string("IndexTable  ")+ indexTable + " is not currently defined" , __FILE__, __func__);
        return false;
    }
    MaterialTable & matTable=itit->second;
    bool removed;
    try
    {
        removed=matTable.removeMaterialEntry(materialName);
    }
    catch(runtime_error &rte)
    {
        SetOptiXLastError(string("Material entry ")+ materialName + " is not currently defined in IndexTable " +indexTable, __FILE__, __func__);
        return false;
    }
    if(!removed)
    {
        SetOptiXLastError(string("Material entry ")+ materialName + " of IndexTable " +indexTable + " is still in use " , __FILE__, __func__);
        return false;
    }
    return true;
}



DLL_EXPORT bool EnumerateMaterials(const char* indexTable, size_t * pHandle, char * nameBuffer, const int bufSize)
{
    ClearOptiXError();
    map<string,MaterialTable>::iterator itit = indexTables.find(indexTable);
    if(itit==indexTables.end())
    {
        SetOptiXLastError(string("IndexTable  ")+ indexTable + " is not currently defined" , __FILE__, __func__);
        return false;
    }
    string name;
    itit->second.enumerateMaterials(pHandle,name);
    if((size_t)bufSize <= name.size())
    {
        SetOptiXLastError(string("IndexTable ")+ indexTable + ": provided buffer is too small" , __FILE__, __func__);
        MaterialTable::ReleaseHandle(*pHandle);
        *pHandle=0;
        return false;
    }
    strncpy(nameBuffer, name.c_str(),bufSize);
    return true;
}

DLL_EXPORT void ReleaseMaterialEnumHandle(size_t Handle)
{
    if(Handle)
        MaterialTable::ReleaseHandle(Handle);  // methode statique de la classe
}



/****************************   Coating Tables    ******************************************/

DLL_EXPORT bool CreateCoatingTable(const char* name)
{
    ClearOptiXError();
    auto retpair=coatingTables.emplace(name, CoatingTable(name));
    if( ! retpair.second)
    {
        SetOptiXLastError(string("Coating table ")+ name + " already exists" , __FILE__, __func__);
        return false;
    }

    return true;
}


DLL_EXPORT bool EnumerateCoatingTables(size_t * pHandle, char * nameBuffer, const int bufSize)
{
    ClearOptiXError();
    map<string,CoatingTable>::iterator* tabit;
    if(*pHandle==0)
        tabit=new map<string,CoatingTable>::iterator(coatingTables.begin());
    else
        tabit=(map<string,CoatingTable>::iterator* )*pHandle;

    strncpy(nameBuffer,(*tabit)->first.c_str(), bufSize);

    if(bufSize < (int) (*tabit)->first.size()+1)
    {
        SetOptiXLastError("nameBuffer is too small",__FILE__,__func__);
        delete tabit;
        *pHandle=0;
        return false;
    }

    if(++(*tabit)== coatingTables.end())
    {
         delete tabit;
         *pHandle=0;
    }
    else
        *pHandle=(size_t) tabit;

    return true;
}

DLL_EXPORT void ReleaseCoatingTableEnumHandle(size_t handle)
{
    if(handle)
        delete (map<string, CoatingTable>::iterator*) handle;
}

DLL_EXPORT bool EnumerateCoatings(const char* coatingTable, size_t * pHandle, char * nameBuffer, const int bufSize)
{
    ClearOptiXError();
    map<string,CoatingTable>::iterator tabit = coatingTables.find(coatingTable);
    if(tabit==coatingTables.end())
    {
        SetOptiXLastError(string("CoatingTable  ")+ coatingTable + " is not currently defined" , __FILE__, __func__);
        return false;
    }
    string name;
    tabit->second.enumerateCoatings(pHandle,name);
    if((size_t)bufSize <= name.size())
    {
        SetOptiXLastError(string("CoatingTable ")+ coatingTable + ": provided buffer is too small" , __FILE__, __func__);
        CoatingTable::ReleaseHandle(*pHandle);
        *pHandle=0;
        return false;
    }
    strncpy(nameBuffer, name.c_str(),bufSize);
    return true;
}


DLL_EXPORT void ReleaseCoatingEnumHandle(size_t Handle)
{
    if(Handle)
        CoatingTable::ReleaseHandle(Handle);  // methode statique de la classe
}


DLL_EXPORT bool CreateCoating(const char* coatingTable, const char * coatingName, const char* indexTable, const char* substrateMaterial)
{
    ClearOptiXError();
    map<string,CoatingTable>::iterator ctabit = coatingTables.find(coatingTable);
    if(ctabit==coatingTables.end())
    {
        SetOptiXLastError(string("CoatingTable  ")+ coatingTable + " is not currently defined" , __FILE__, __func__);
        return false;
    }
    map<string,MaterialTable>::iterator matit = indexTables.find(indexTable);
    if(matit==indexTables.end())
    {
        SetOptiXLastError(string("Index Table  ")+ indexTable + " is not currently defined" , __FILE__, __func__);
        return false;
    }
    try
    {
        MaterialTable::MatEntry &substrateEntry= matit->second.getMaterial(substrateMaterial);
        ctabit->second.addCoating(coatingName, substrateEntry);
    }
    catch(runtime_error & rte)
    {
        SetOptiXLastError(string("CoatingTable ")+ coatingTable + " coating " + coatingName +" creation failed ; reason: " + rte.what() , __FILE__, __func__);
        return false;
    }
    return true;

}


DLL_EXPORT bool RemoveCoating(const char* coatingTable, const char * coatingName)
{
    map<string,CoatingTable>::iterator ctabit = coatingTables.find(coatingTable);
    if(ctabit==coatingTables.end())
    {
        SetOptiXLastError(string("CoatingTable  ")+ coatingTable + " is not currently defined" , __FILE__, __func__);
        return false;
    }
    try
    {
        if(!ctabit->second.deleteCoating(coatingName))
        {
            SetOptiXLastError(string("Cannot remove coating ")+coatingName+" from "+ coatingTable+ " because it is still used", __FILE__, __func__);
            return false;
        }
    }
    catch(runtime_error &rte)
    {
        SetOptiXLastError(string("Cannot find the coating ")+coatingName+" in table "+ coatingTable, __FILE__, __func__);
        return false;
    }
    return true;
}


DLL_EXPORT bool AddCoatingLayer(const char* coatingTable, const char * coatingName,
                                const char* indexTable, const char* layerMaterial, double thickness, double compactness)
{
    ClearOptiXError();
    if(thickness <0)
    {
        SetOptiXLastError(string("cannot add a layer with negative thickness" ), __FILE__, __func__);
        return false;
    }
    if(compactness <0)
    {
        SetOptiXLastError(string("cannot add a layer with negative compactness") , __FILE__, __func__);
        return false;
    }
    map<string,CoatingTable>::iterator ctabit = coatingTables.find(coatingTable);
    if(ctabit==coatingTables.end())
    {
        SetOptiXLastError(string("CoatingTable  ")+ coatingTable + " is not currently defined" , __FILE__, __func__);
        return false;
    }
    map<string,MaterialTable>::iterator matit = indexTables.find(indexTable);
    if(matit==indexTables.end())
    {
        SetOptiXLastError(string("Index Table  ")+ indexTable + " is not currently defined" , __FILE__, __func__);
        return false;
    }
    MaterialTable::MatEntry *p_matEntry;
    try{
        p_matEntry= &(matit->second.getMaterial(layerMaterial));
    }
    catch (runtime_error & rte)
    {
        SetOptiXLastError(string("Material ")+ layerMaterial + " was not found in table  " + indexTable , __FILE__, __func__);
        return false;
    }
    try
    {

        Coating &coat=ctabit->second.getCoating(coatingName);
        coat.addLayer(*p_matEntry, thickness);
        int64_t nlayers=coat.getLayers();
        coat.setLayerCompactness(nlayers-1,compactness);
    }
    catch(runtime_error &rte)
    {
        SetOptiXLastError(string("CoatingTable ")+ coatingTable + " cannot add layer " +layerMaterial+ " ; reason: "+ rte.what(),
                           __FILE__, __func__);
        return false;
    }
    return true;
}

DLL_EXPORT bool InsertCoatingLayer(const char* coatingTable, const char * coatingName, int64_t layerIndex,
                                const char* indexTable, const char* layerMaterial, double thickness, double compactness)
{
    if(layerIndex <0)
        return AddCoatingLayer(coatingTable, coatingName, indexTable, layerMaterial, thickness,compactness);

    ClearOptiXError();
    if(thickness <0)
    {
        SetOptiXLastError(string("cannot insert a layer with negative thickness" ), __FILE__, __func__);
        return false;
    }
    if(compactness <0)
    {
        SetOptiXLastError(string("cannot insert a layer with negative compactness") , __FILE__, __func__);
        return false;
    }
    map<string,CoatingTable>::iterator ctabit = coatingTables.find(coatingTable);
    if(ctabit==coatingTables.end())
    {
        SetOptiXLastError(string("CoatingTable  ")+ coatingTable + " is not currently defined" , __FILE__, __func__);
        return false;
    }
    map<string,MaterialTable>::iterator matit = indexTables.find(indexTable);
    if(matit==indexTables.end())
    {
        SetOptiXLastError(string("Index Table  ")+ indexTable + " is not currently defined" , __FILE__, __func__);
        return false;
    }
    MaterialTable::MatEntry *p_matEntry;
    try{
        p_matEntry= &(matit->second.getMaterial(layerMaterial));
    }
    catch (runtime_error & rte)
    {
        SetOptiXLastError(string("Material ")+ layerMaterial + " was not found in table  " + indexTable , __FILE__, __func__);
        return false;
    }
    try
    {

        Coating &coat=ctabit->second.getCoating(coatingName);
        int64_t nlayers=coat.getLayers();
        if(layerIndex >= nlayers)
        {
            SetOptiXLastError(string("Can't insert the layer : insertion index exceeds the number of defined layers" ), __FILE__, __func__);
            return false;
        }
        coat.insertLayer(layerIndex,*p_matEntry, thickness);
        coat.setLayerCompactness(layerIndex,compactness);
    }
    catch(runtime_error &rte)
    {
        SetOptiXLastError(string("CoatingTable ")+ coatingTable + " cannot insert the layer " +layerMaterial+ " ; reason: "+ rte.what(),
                           __FILE__, __func__);
        return false;
    }
    return true;
}

DLL_EXPORT bool GetLayerNumber(const char* coatingTable, const char * coatingName, size_t *layerNumber)
{
    ClearOptiXError();
    map<string,CoatingTable>::iterator ctabit = coatingTables.find(coatingTable);
    if(ctabit==coatingTables.end())
    {
        SetOptiXLastError(string("CoatingTable  ")+ coatingTable + " is not currently defined" , __FILE__, __func__);
        return false;
    }
    try
    {
        Coating &coat=ctabit->second.getCoating(coatingName);
        *layerNumber=coat.getLayers();
    }
    catch(runtime_error &rte)
    {
        SetOptiXLastError(string("Cannot find the coating ")+coatingName+" in table "+ coatingTable, __FILE__, __func__);
        return false;
    }
    return true;
}


DLL_EXPORT bool SetCoatingLayer(const char* coatingTable, const char * coatingName, size_t layerIndex,
                                const char* indexTable, const char* layerMaterial, double thickness, double compactness)
{
    ClearOptiXError();
    if(thickness <0)
    {
        SetOptiXLastError(string("cannot assign a negative thickness to a layer" ), __FILE__, __func__);
        return false;
    }
    if(compactness <0)
    {
        SetOptiXLastError(string("cannot assign negative compactness to a layer") , __FILE__, __func__);
        return false;
    }
    map<string,CoatingTable>::iterator ctabit = coatingTables.find(coatingTable);
    if(ctabit==coatingTables.end())
    {
        SetOptiXLastError(string("CoatingTable  ")+ coatingTable + " is not currently defined" , __FILE__, __func__);
        return false;
    }
    map<string,MaterialTable>::iterator matit = indexTables.find(indexTable);
    if(matit==indexTables.end())
    {
        SetOptiXLastError(string("Index Table  ")+ indexTable + " is not currently defined" , __FILE__, __func__);
        return false;
    }
    MaterialTable::MatEntry *p_matEntry;
    try{
        p_matEntry= &(matit->second.getMaterial(layerMaterial));
    }
    catch (runtime_error & rte)
    {
        SetOptiXLastError(string("Material ")+ layerMaterial + " was not found in table  " + indexTable , __FILE__, __func__);
        return false;
    }
    try
    {
        Coating &coat=ctabit->second.getCoating(coatingName);
        size_t nlayers=coat.getLayers();
        if(layerIndex > nlayers)
        {
            SetOptiXLastError(string("Layer index ")+ to_string(layerIndex) + "is out of " + coatingName + " coating range"  , __FILE__, __func__);
            return false;
        }
        if(p_matEntry !=&coat.getLayerMaterial(layerIndex))
            coat.setLayerMaterial(layerIndex,*p_matEntry);
        if(thickness != coat.getlayerThickness(layerIndex))
            coat.setLayerThickness(layerIndex,thickness);
        if(compactness != coat.getLayerCompactness(layerIndex))
            coat.setLayerCompactness(layerIndex,compactness);
    }
    catch(runtime_error &rte)
    {
        SetOptiXLastError(string("CoatingTable ")+ coatingTable + " cannot change layer material to " +layerMaterial+ " ; reason: "+ rte.what(),
                           __FILE__, __func__);
        return false;
    }
    return true;
}

DLL_EXPORT bool GetCoatingLayer(const char* coatingTable, const char * coatingName, size_t layerIndex,
                                char* matBuffer, const int bufSize, double *p_thickness, double *p_compactness)
{
    ClearOptiXError();
    map<string,CoatingTable>::iterator ctabit = coatingTables.find(coatingTable);
    if(ctabit==coatingTables.end())
    {
        SetOptiXLastError(string("CoatingTable  ")+ coatingTable + " is not currently defined" , __FILE__, __func__);
        return false;
    }
    Coating *p_coat;
    try
    {
        p_coat=&(ctabit->second.getCoating(coatingName));
    }
    catch (runtime_error &rte)
    {
        SetOptiXLastError(string("Coating  ")+ coatingName + " is not currently defined in table "+ coatingTable , __FILE__, __func__);
        return false;
    }
    size_t nlayers=p_coat->getLayers();
    if(layerIndex > nlayers)
    {
        SetOptiXLastError(string("Layer index ")+ to_string(layerIndex) + "is out of " + coatingName + " coating range"  , __FILE__, __func__);
        return false;
    }
    MaterialTable::MatEntry &matEntry=p_coat->getLayerMaterial(layerIndex);
    if(sprintf_s(matBuffer,bufSize,"%s:%s",matEntry.parentTable->getName(),matEntry.material.getName()) <=0)
    {
        SetOptiXLastError(string("Buffer is  too small for the qualified coating name"), __FILE__, __func__);
        return false;
    }
    *p_thickness= p_coat->getlayerThickness(layerIndex);
    *p_compactness= p_coat->getLayerCompactness(layerIndex);

    return true;
}


DLL_EXPORT bool SetCoatingRoughness(const char* coatingTable, const char * coatingName, double roughness)
{
    ClearOptiXError();
    map<string,CoatingTable>::iterator ctabit = coatingTables.find(coatingTable);
    if(ctabit==coatingTables.end())
    {
        SetOptiXLastError(string("CoatingTable  ")+ coatingTable + " is not currently defined" , __FILE__, __func__);
        return false;
    }
    Coating *p_coat;
    try
    {
        p_coat=&(ctabit->second.getCoating(coatingName));
    }
    catch (runtime_error &rte)
    {
        SetOptiXLastError(string("Coating  ")+ coatingName + " is not currently defined in table "+ coatingTable , __FILE__, __func__);
        return false;
    }
    p_coat->setRoughness(roughness);
    return true;
}


DLL_EXPORT bool GetCoatingRoughness(const char* coatingTable, const char * coatingName, double *p_roughness)
{
    ClearOptiXError();
    map<string,CoatingTable>::iterator ctabit = coatingTables.find(coatingTable);
    if(ctabit==coatingTables.end())
    {
        SetOptiXLastError(string("CoatingTable  ")+ coatingTable + " is not currently defined" , __FILE__, __func__);
        return false;
    }
    Coating *p_coat;
    try
    {
        p_coat=&(ctabit->second.getCoating(coatingName));
    }
    catch (runtime_error &rte)
    {
        SetOptiXLastError(string("Coating  ")+ coatingName + " is not currently defined in table "+ coatingTable , __FILE__, __func__);
        return false;
    }
    *p_roughness=p_coat->getRoughness();
    return true;
}


DLL_EXPORT bool SetCoatingTableAngles(const char* coatingTable, double angleMin, double angleMax, size_t numAngles)
{
    ClearOptiXError();
    map<string,CoatingTable>::iterator ctabit = coatingTables.find(coatingTable);
    if(ctabit==coatingTables.end())
    {
        SetOptiXLastError(string("CoatingTable  ")+ coatingTable + " is not currently defined" , __FILE__, __func__);
        return false;
    }
    try {
        ctabit->second.setGrazingAngleRange(angleMin, angleMax, numAngles);
    }
    catch(invalid_argument & invarg)
    {
        SetOptiXLastError(string("CoatingTable  ")+ coatingTable + " invalid  angle range " , __FILE__, __func__);
        return false;
    }

    return true;
}



DLL_EXPORT bool GetCoatingTableAngles(const char* coatingTable, double *p_angleMin, double *p_angleMax, size_t *p_numAngles)
{
    ClearOptiXError();
    map<string,CoatingTable>::iterator ctabit = coatingTables.find(coatingTable);
    if(ctabit==coatingTables.end())
    {
        SetOptiXLastError(string("CoatingTable  ")+ coatingTable + " is not currently defined" , __FILE__, __func__);
        return false;
    }
    if(ctabit->second.getGrazingAngleRange(p_angleMin, p_angleMax, p_numAngles))
        return true;

    SetOptiXLastError(string("CoatingTable  ")+ coatingTable + " the angle tabulation grid is not yet defined", __FILE__, __func__);
    return false;

}

DLL_EXPORT bool SetCoatingTableEnergies(const char* coatingTable, double energyMin, double energyMax,
                                        int64_t numEnergies, bool logSpacing)
{
    ClearOptiXError();
    map<string,CoatingTable>::iterator ctabit = coatingTables.find(coatingTable);
    if(ctabit==coatingTables.end())
    {
        SetOptiXLastError(string("CoatingTable  ")+ coatingTable + " is not currently defined" , __FILE__, __func__);
        return false;
    }
    try {
        ctabit->second.setEnergyRange(energyMin, energyMax, numEnergies, logSpacing);
    }
    catch(invalid_argument & invarg)
    {
        SetOptiXLastError(string("CoatingTable  ")+ coatingTable + " invalid  energy range, "+invarg.what() , __FILE__, __func__);
        return false;
    }
 //   cout << "Table status = " << hex << ctabit->second.getStatus() << dec <<endl;

    return true;
}

DLL_EXPORT bool GetCoatingTableEnergies(const char* coatingTable, double *p_energyMin, double *p_energyMax,
                                        int64_t *p_numEnergies, bool *p_logSpacing)
{
    ClearOptiXError();
    map<string,CoatingTable>::iterator ctabit = coatingTables.find(coatingTable);
    if(ctabit==coatingTables.end())
    {
        SetOptiXLastError(string("CoatingTable  ")+ coatingTable + " is not currently defined" , __FILE__, __func__);
        return false;
    }
    if(ctabit->second.getEnergyRange(p_energyMin, p_energyMax, p_numEnergies, p_logSpacing))
        return true;

    SetOptiXLastError(string("CoatingTable  ")+ coatingTable + " the energy tabulation grid is not yet defined", __FILE__, __func__);
    return false;

}

DLL_EXPORT bool CoatingTableCompute(const char* coatingTable)
{
    ClearOptiXError();
    map<string,CoatingTable>::iterator ctabit = coatingTables.find(coatingTable);
    if(ctabit==coatingTables.end())
    {
        SetOptiXLastError(string("CoatingTable  ")+ coatingTable + " is not currently defined" , __FILE__, __func__);
        return false;
    }
  //  cout << " Coating table status ="<< hex << ctabit->second.getStatus() << dec <<endl;
    try{
        ctabit->second.computeReflectivity();
    }
    catch(runtime_error &rte)
    {
        SetOptiXLastError(string("CoatingTable  ")+ coatingTable + " cannot compute the reflectivity; reason: "+rte.what(), __FILE__, __func__);
        return false;
    }
    return true;
}


DLL_EXPORT bool GetCoatingTableStatus(const char* coatingTable, short * pstatus)
{
    ClearOptiXError();
    map<string,CoatingTable>::iterator ctabit = coatingTables.find(coatingTable);
    if(ctabit==coatingTables.end())
    {
        SetOptiXLastError(string("CoatingTable  ")+ coatingTable + " is not currently defined" , __FILE__, __func__);
        return false;
    }
    *pstatus= ctabit->second.getStatus() ;
    return true;
}

 #endif // HAS_REFLEX

