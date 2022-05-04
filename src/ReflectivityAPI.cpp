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
    auto retpair=indexTables.emplace(name, MaterialTable());
    if( ! retpair.second)
    {
        SetOptiXLastError(string("Material table ")+ name + " already exists" , __FILE__, __func__);
        return false;
    }

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



DLL_EXPORT bool AddMaterial(const char* table, char* database, const char* material)
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

DLL_EXPORT bool AddCompound(const char* table, char* database, const char* formula,double density)
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
        XDatabase::ReleaseHandle(*pHandle);
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


 #endif // HAS_REFLEX

