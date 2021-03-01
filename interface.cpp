////////////////////////////////////////////////////////////////////////////////
/**
*      \file           interface.cpp
*
*      \brief         TODO  fill in file purpose
*
*      \author         François Polack <francois.polack@synchroton-soleil.fr>
*      \date        2020-11-12  Creation
*      \date        Last update
*
*/
///////////////////////////////////////////////////////////////////////////////////
//
//             REVISIONS
//
////////////////////////////////////////////////////////////////////////////////////

#include <map>
#include <vector>
#include <iostream>
#include "interface.h"
#include "gridSource.h"
#include "opticalelements.h"
#include "files.h"


/** \brief a class implementing a dictionary of optical elements from the library  which is a specialization of the map<string, ElementBase*>  class
 */
class ElementCollection:public map<string, ElementBase*>
{
    typedef map<string, ElementBase*> BaseMap;
public:
    ElementCollection(){}/**< \brief Creates an empty dictionary */
    ~ElementCollection()
    {
        map<string, ElementBase*>::iterator it;
        for(it=begin(); it!=end();  ++it)
            delete it->second;
    }

    /** \brief removes an entry in the dictionary by name
    *    \param key the entry name */
    int erase(string key)
    {
        iterator it=find(key);
        if(it==end())
            return 0; // element doesn't exist
        delete it->second;
        return BaseMap::erase(key);
    }
    /** \brief removes an entry of the dictionary by iterator pointer
    *   \param pos the iterator pointing on the element to remove*/
    iterator erase (iterator pos)
    {
        if(pos==end())
            return pos;
        delete pos->second;
        return BaseMap::erase(pos);
    }

    /** \brief removes a range of entries
    *   \param first iterator pointing to the first element to remove
    *   \param last iterator pointing to the last element to remove*/
    iterator erase(const_iterator first, const_iterator last)
    {
        for(const_iterator it=first; it!=last; ++it)
            delete it->second;
        if(last!=end())
            delete last->second;
        return BaseMap::erase(first,last);
    }

    /**  \brief clears the whole dictionary */
    inline void clear(){ erase(begin(),end());}
} ;


/** \brief class managing a vector of C_strings
 */
class StringVector:public vector<char*>
{
    typedef vector<char*> VectorBase;
public:
    iterator erase(const_iterator it)
    {
        delete []*it;
        return VectorBase::erase(it);
    }
    iterator erase(const_iterator first, const_iterator last)
    {
        for(const_iterator it=first; it!=last; ++it)
                delete []*it;
        if(last!=end())
            delete [] *last;
        return VectorBase::erase(first, last);
    }

    ~StringVector()
    {
        if (size()!=0)
            VectorBase::erase(begin(), end());
    };
};

ElementCollection System;/**< \brief dictionary of all elements created through this interface  */
StringVector stringData; /**< \todo seem unused*/

extern "C"
{

    DLL_EXPORT size_t CreateElement(const char* type, const char* name)
    {
        ElementBase * surf;
        try {
                surf=(ElementBase*)CreateElementObject(type,name);
            }
            catch(ElementException& ex)
            {
                 cout << " Object not created: "<< ex.what();
                 return 0;
            }

        System.insert(pair<string, ElementBase*>(name, surf) );
        return (size_t)surf;
    }

    DLL_EXPORT bool EnumerateElements(size_t * pHandle, size_t* elemID, char * nameBuffer, const int bufSize)
    {
        map<string,ElementBase*>::iterator* pit;
        if(*pHandle==0)
            pit=new map<string,ElementBase*>::iterator(System.begin());
        else
            pit=(map<string,ElementBase*>::iterator* )*pHandle;

        if(bufSize < (int) (*pit)->first.size()+1)
            { SetOptiXLastError("nameBuffer is too small",__FILE__,__func__); delete pit;  return false; }
        strncpy(nameBuffer,(*pit)->first.c_str(), bufSize);
        *elemID=(size_t) (*pit)->second;
        if(++(*pit)== System.end())
        {
             delete pit;
             *pHandle=0;
             return false;
        }
        *pHandle=(size_t) pit;
        return true;
    }

    DLL_EXPORT void ReleaseElementEnumHandle(size_t handle)
    {
        if(handle)
            delete (map<string, ElementBase*>::iterator*) handle;
    }

    DLL_EXPORT size_t GetElementID(const char* ElementName)
    {
        return (size_t) System.find(ElementName)->second;
    }

    DLL_EXPORT void GetElementName(size_t ElementID, char* strBuffer, int bufSize)
    {
        strncpy(strBuffer,((ElementBase*)ElementID)->getName().c_str(), bufSize);
    }

    DLL_EXPORT size_t RemoveElement_byName(const char* name)
    {
        map<string,ElementBase*>:: iterator it= System.find(name);
        if(it!=System.end())
            it=System.erase(it);
        if(it==System.end())
            return 0;
        return (size_t) it->second;
    }

    DLL_EXPORT size_t RemoveElement_byID(size_t elementID)
    {
        string name= ((ElementBase*)elementID)->getName();
        return RemoveElement_byName(name.c_str());
    }

    DLL_EXPORT void ChainElement_byName(const char* previous, const char* next)
    {
        ElementBase* sprev=(previous[0]==0) ? NULL : System.find(previous)->second;
        ElementBase* snext=(next[0]==0) ? NULL : System.find(next)->second;
        if(!sprev)
        {
            if(snext)
                snext->setPrevious(NULL);
        }
        else
        {
            if(!snext)
                sprev->setNext(NULL);
            else
                sprev->setNext(snext);
        }
    }

    DLL_EXPORT void ChainElement_byID(size_t prevID, size_t nextID)
    {
        if(prevID==0)
        {
            if(nextID!=0)
               ((ElementBase*)nextID)->setPrevious(NULL);
        }
        else
        {
            if(nextID==0)
                ((ElementBase*)prevID)->setNext(NULL);
            else
                ((ElementBase*)prevID)->setNext((ElementBase*)nextID);
        }
    }

    DLL_EXPORT size_t GetPreviousElement(size_t elementID)
    {
        return (size_t) ((ElementBase*)elementID)->getPrevious();
    }

    DLL_EXPORT size_t GetNextElement(size_t elementID)
    {
        return (size_t) ((ElementBase*)elementID)->getNext();
    }

    DLL_EXPORT bool SetParameter(size_t elementID, const char* paramTag, Parameter paramData)
    {
        return ((ElementBase*)elementID)->setParameter(paramTag, paramData);
    }

    DLL_EXPORT bool GetParameter(size_t elementID, const char* paramTag, Parameter* paramData)
    {
        return ((ElementBase*)elementID)->getParameter(paramTag, *paramData);
    }

    DLL_EXPORT bool EnumerateParameters(size_t elementID, size_t * pHandle, char* tagBuffer, const int bufSize , Parameter* paramData)
    {
        map<string, Parameter>::iterator* pRef;
        if (*pHandle==0)
            pRef= new map<string, Parameter>::iterator( ((ElementBase*)elementID)->parameterBegin() );
        else
            pRef= (map<string, Parameter>::iterator*) *pHandle;

        strncpy(tagBuffer, (char*)((*pRef)->first).c_str(), bufSize);
        *paramData=(*pRef)->second;

        if(++(*pRef) ==((ElementBase*)elementID)->parameterEnd() )
        {
            delete pRef;
            *pHandle=0;
            return false;
        }
        * pHandle=(size_t) pRef;
        return true;
    }

    DLL_EXPORT void ReleaseParameterEnumHandle(size_t handle)
    {
        if(handle)
            delete (map<string, Parameter>::iterator*) handle;
    }


//    DLL_EXPORT void FreeNextPointer(size_t nextPtr)  // il n'y a aucune raison de libérer la mémoire localisée par ce pointeur
//    {
//        map<string, Parameter>::iterator* pRef;
//        if(nextPtr==0)
//            delete (map<string, Parameter>::iterator*) nextPtr;
//    }

    DLL_EXPORT bool SaveSystem(const char* filename)
    {
        TextFile file(filename, ios::out);
        if(!file.is_open())
        {
            SetOptiXLastError("Can't open the file for writing",__FILE__,__func__);
            return false;
        }
        map<string,ElementBase*>::iterator it;
        for (it=System.begin(); it!=System.end(); ++it)
        {
            file << *(it->second);
            if(file.fail())
            {
                SetOptiXLastError("Text file write error",__FILE__,__func__);
                return false;
            }
        }
        file.close();
        return true;
    }

    DLL_EXPORT bool LoadSystem(const char* filename)
    {
        TextFile file(filename, ios::in);
        if(!file.is_open())
        {
            SetOptiXLastError("Can't open the file for reading",__FILE__,__func__);
            return false;
        }
        string sClass, sName, sPrev,sNext, paramName;
        Parameter param;
        size_t ElementBaseID;

        System.clear();
        while(!file.eof()) // loop of ElementBase creation
        {
            file >> sClass;
            if(sClass.empty())
                break;
            file >> sName >> sNext >> sPrev >> paramName;
            if(file.fail())
                { SetOptiXLastError("File reading error",__FILE__,__func__);  return false; }
            ElementBaseID=CreateElement(sClass.c_str(), sName.c_str() );
            if(ElementBaseID==0)
            {
                char errstr[128];
                sprintf(errstr, "Cannot create element %s of type %s", sName.c_str(), sClass.c_str());
                SetOptiXLastError("File reading error",__FILE__,__func__);
                return false;
            }
            while(!paramName.empty())
            {
               file >> param;
               if(file.fail())
                    { SetOptiXLastError("File reading error",__FILE__,__func__);  return false; }
               if(!SetParameter(ElementBaseID, paramName.c_str(), param))
               {
                    char errstr[128];
                    sprintf(errstr, "Cannot create element %s of type %s", sName.c_str(), sClass.c_str());
                    SetOptiXLastError("File reading error",__FILE__,__func__);
                    return false;
               }
               file >> paramName;
               if(file.fail())
                    { SetOptiXLastError("File reading error",__FILE__,__func__);  return false; }
            }

            file.ignore('\n');
        }               // end ElementBase  creation

        file.seekg(0);
                while(!file.eof()) // loop of ElementBase creation
        {
            file >> sClass;
            if(file.fail())
                { SetOptiXLastError("File reading error",__FILE__,__func__);  return false; }
            if(sClass.empty())
                break;
            file >> sName >> sNext ; //>> sPrev ;   // On peut se contenter d'appeler seulement set next qui se chargera de mettre à jour les 2 elements connectés
            if(file.fail())
                { SetOptiXLastError("File reading error",__FILE__,__func__);  return false; }
            if(!sNext.empty() )   // inutile d'agir si sNext empty; les nouvelles surfaces ont tous leurs liens nuls
                System.find(sName)->second->setNext(System.find(sNext)->second); // ces deux ElementBases existent; elles viennent d'être créées.

            file.ignore('\n'); // skip prameters
        }
        return true;
    }

}
