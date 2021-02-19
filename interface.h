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

extern "C"
{
    DLL_EXPORT size_t CreateElement(const char* type, const char* name);
    DLL_EXPORT size_t GetElementID(const char* elementName);
    DLL_EXPORT void GetElementName(size_t elementID, char* strBuffer, int bufSize);
    DLL_EXPORT size_t RemoveElement_byName(const char* name);
    DLL_EXPORT size_t RemoveElement_byID(size_t elementID);
    DLL_EXPORT void ChainElement_byName(const char* previous, const char* next);
    DLL_EXPORT void ChainElement_byID(size_t prevID, size_t nextID);
    DLL_EXPORT size_t GetPreviousElemente(size_t elementID);
    DLL_EXPORT size_t GetNextElement(size_t elementID);
    DLL_EXPORT bool SetParameter(size_t elementID,const char* paramTag, Parameter paramData);
    DLL_EXPORT bool GetParameter(size_t elementID,const char* paramTag, Parameter* paramData);
    DLL_EXPORT bool GetNextParameter(size_t elementID, size_t * nextPtr, char* tagBuffer, int bufSize , Parameter* paramData);
//    DLL_EXPORT void FreeNextPointer(size_t nextPtr);  cette fonction n'a pas d'objet clair
    DLL_EXPORT void SaveSystem(const char* filename);
    DLL_EXPORT bool OpenSystem(const char* filename);

}


#endif // INTERFACE_H_INCLUDED
