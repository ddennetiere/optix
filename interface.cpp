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
#include <set>
#include <vector>
#include <iostream>
#include "interface.h"
#include "sources.h"
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
    *   \param pos the iterator pointing on the element to remove
    *   \return an iterator to the element following the removed one in the list
    */
    iterator erase (iterator pos)
    {
        if(pos==end())
            return pos;
        delete pos->second;
        return BaseMap::erase(pos);
    }

    /** \brief removes a range of entries
    *   \param first iterator pointing to the first element to remove
    *   \param last iterator pointing to the last element to remove
    *   \return an iterator to the element following the last removed one in the list
    */
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
set<size_t> ValidIDs;
StringVector stringData; /**< \todo seem unused*/

inline bool IsValidID(size_t ID)
{
    set<size_t>::iterator it=ValidIDs.find(ID);
    return it!=ValidIDs.end();
}

extern "C"
{
    DLL_EXPORT bool IsElementValid(size_t  ID){return IsValidID(ID);}

    DLL_EXPORT bool GetOptiXLastError(char* buffer, int bufferSize)
    {
        if(OptiXError )
        {
            if(buffer)
                strncpy(buffer,LastError, bufferSize);
            OptiXError=false;
            return true;
        }

        if(buffer)
                strncpy(buffer,"No Error", bufferSize);
        return false;

    }


    DLL_EXPORT size_t CreateElement(const char* type, const char* name)
    {
        ClearOptiXError();
        if(System.find(name)!=System.end())
        {
            SetOptiXLastError("Name already exists in the current system", __FILE__, __func__);
            return 0;
        }
        ElementBase * elem=0;
        try {
                elem=(ElementBase*)CreateElementObject(type,name);
            }
            catch(ElementException& ex)
            {
                 SetOptiXLastError( "Invalid element type", __FILE__,__func__);
                 return 0;
            }

        System.insert(pair<string, ElementBase*>(name, elem) );
        ValidIDs.insert((size_t) elem);
        return (size_t)elem;
    }

    DLL_EXPORT bool EnumerateElements(size_t * pHandle, size_t* elemID, char * nameBuffer, const int bufSize)
    {
        map<string,ElementBase*>::iterator* pit;
        if(*pHandle==0)
            pit=new map<string,ElementBase*>::iterator(System.begin());
        else
            pit=(map<string,ElementBase*>::iterator* )*pHandle;

        strncpy(nameBuffer,(*pit)->first.c_str(), bufSize);
        *elemID=(size_t) (*pit)->second;
        if(bufSize < (int) (*pit)->first.size()+1)
        {
            SetOptiXLastError("nameBuffer is too small",__FILE__,__func__);
            delete pit;
            *pHandle=0;
            return false;
        }

        if(++(*pit)== System.end())
        {
             delete pit;
             *pHandle=0;
        }
        else
            *pHandle=(size_t) pit;

        return true;
    }

    DLL_EXPORT void ReleaseElementEnumHandle(size_t handle)
    {
        if(handle)
            delete (map<string, ElementBase*>::iterator*) handle;
    }

    DLL_EXPORT size_t GetElementID(const char* elementName)
    {

        map<string,ElementBase*>:: iterator it= System.find(elementName);
        if(it==System.end())
            return 0;
        return (size_t) it->second;
    }

    DLL_EXPORT bool GetElementName(size_t elementID, char* strBuffer, int bufSize)
    {
        if(!IsValidID(elementID))
        {
            SetOptiXLastError("invalid element ID", __FILE__, __func__);
            return false;
        }
        strncpy(strBuffer,((ElementBase*)elementID)->getName().c_str(), bufSize);
        if(bufSize <(int)((ElementBase*)elementID)->getName().size()+1)
        {
            SetOptiXLastError("Buffer too small, name was truncated", __FILE__, __func__);
            return false;
        }
        return true;
    }

    DLL_EXPORT bool GetElementType(size_t elementID, char* strBuffer, int bufSize)
    {
        if(!IsValidID(elementID))
        {
            SetOptiXLastError("invalid element ID", __FILE__, __func__);
            return false;
        }
        strncpy(strBuffer,((ElementBase*)elementID)->getRuntimeClass().c_str(), bufSize);
        if(bufSize <(int)((ElementBase*)elementID)->getRuntimeClass().size()+1)
        {
            SetOptiXLastError("Buffer too small, type was truncated", __FILE__, __func__);
            return false;
        }
       return true;
    }

    DLL_EXPORT bool RemoveElement_byName(const char* name)
    {
        map<string,ElementBase*>:: iterator it= System.find(name);
        if(it==System.end())
            return false ;

        ValidIDs.erase( (size_t) it->second );
        System.erase(it);
        return true;
    }

    DLL_EXPORT bool RemoveElement_byID(size_t elementID)
    {
        set<size_t>::iterator it=ValidIDs.find(elementID);
        if(it==ValidIDs.end())
            return false;
        string name= ((ElementBase*)elementID)->getName();
        return RemoveElement_byName(name.c_str());
    }

    DLL_EXPORT bool ChainElement_byName(const char* previous, const char* next)
    {
        map<string, ElementBase*>::iterator it;
        ElementBase* elprev=NULL;
        ElementBase* elnext=NULL;

        if(previous[0]!=0)
        {
            it==System.find(previous);
            if(it==System.end())
                return false;
            elprev=it->second;
        }
        if (next[0]!=0)
        {
            it=System.find(next);
            if(it==System.end())
                return false;
            elnext=it->second;
        }
        if(!elprev)
        {
            if(elnext)
                elnext->setPrevious(NULL);
            else
                return false;
        }
        else
            elprev->setNext(elnext);
        return true;
    }

    DLL_EXPORT bool ChainElement_byID(size_t prevID, size_t nextID)
    {
        if(prevID==0)
        {
            if(nextID!=0 && IsValidID(nextID))
            {
                ((ElementBase*)nextID)->setPrevious(NULL);
                return true;
            }
            else
                return false;
        }
        else if(!IsValidID(prevID))
            return false;

        if(nextID==0)
            ((ElementBase*)prevID)->setNext(NULL);
        else
            ((ElementBase*)prevID)->setNext((ElementBase*)nextID);

        return true;

    }

    DLL_EXPORT size_t GetPreviousElement(size_t elementID)
    {
        if(IsValidID(elementID))
            return (size_t) ((ElementBase*)elementID)->getPrevious();
        else
            return 0;
    }

    DLL_EXPORT size_t GetNextElement(size_t elementID)
    {
        if(IsValidID(elementID))
            return (size_t) ((ElementBase*)elementID)->getNext();
        else
            return 0;
    }

    DLL_EXPORT bool SetParameter(size_t elementID, const char* paramTag, Parameter paramData)
    {
        if(IsValidID(elementID))
            return ((ElementBase*)elementID)->setParameter(paramTag, paramData);
        else
            return false;
    }

    DLL_EXPORT bool GetParameter(size_t elementID, const char* paramTag, Parameter* paramData)
    {
        if(IsValidID(elementID))
            return ((ElementBase*)elementID)->getParameter(paramTag, *paramData);
            else
                return false;
    }

    DLL_EXPORT bool EnumerateParameters(size_t elementID, size_t * pHandle, char* tagBuffer, const int bufSize , Parameter* paramData)
    {
        if(!IsValidID(elementID))
        {
            SetOptiXLastError("Invalid element ID", __FILE__, __func__);
            return false;
        }

        map<string, Parameter>::iterator* pRef;
        if (*pHandle==0)
            pRef= new map<string, Parameter>::iterator( ((ElementBase*)elementID)->parameterBegin() );
        else
            pRef= (map<string, Parameter>::iterator*) *pHandle;

        strncpy(tagBuffer, (char*)((*pRef)->first).c_str(), bufSize);
        *paramData=(*pRef)->second;
        if(bufSize <(int) (*pRef)->first.size()+1)
        {
            SetOptiXLastError("Buffer too small", __FILE__, __func__);
            delete pRef;
            *pHandle=0;
            return false;
        }

        if(++(*pRef) ==((ElementBase*)elementID)->parameterEnd() )
        {
            delete pRef;
            *pHandle=0;
            return true;
        }
        * pHandle=(size_t) pRef;
        return true;
    }

    DLL_EXPORT void ReleaseParameterEnumHandle(size_t handle)
    {
        if(handle)
            delete (map<string, Parameter>::iterator*) handle;
    }


    DLL_EXPORT int Align(size_t elementID, double wavelength)
    {
        ClearOptiXError();
        if(wavelength <0)
        {
            SetOptiXLastError("Invalid wavelength", __FILE__, __func__);
            return -2;
        }
        if(IsValidID(elementID))
            return ((ElementBase*)elementID)->alignFromHere(wavelength);
        else
        {
            SetOptiXLastError("Invalid element ID", __FILE__, __func__);
            return 2;
        }

    }


    DLL_EXPORT bool Generate(size_t elementID, double wavelength)
    {
        ClearOptiXError();
        if(!IsValidID(elementID))
        {
            SetOptiXLastError("Invalid element ID", __FILE__, __func__);
            return false;
        }
        if( !((ElementBase*)elementID)->isSource())
        {
            SetOptiXLastError("Element is not a source", __FILE__, __func__);
            return false;
        }
        if(wavelength <0)
        {
            SetOptiXLastError("Invalid wavelength", __FILE__, __func__);
            return false;
        }

            return ((SourceBase*)elementID)->generate(wavelength);
    }

    DLL_EXPORT bool Radiate(size_t elementID)
    {
        ClearOptiXError();
        if(!IsValidID(elementID))
        {
            SetOptiXLastError("Invalid element ID", __FILE__, __func__);
            return false;
        }
        if( !((ElementBase*)elementID)->isSource())
        {
            SetOptiXLastError("Element is not a source", __FILE__, __func__);
            return false;
        }
        ((SourceBase*)elementID)->radiate();
        return true;
    }

    DLL_EXPORT bool RadiateAt(size_t elementID, double wavelength)
    {
        ClearOptiXError();
        if(!IsValidID(elementID))
        {
            SetOptiXLastError("Invalid element ID", __FILE__, __func__);
            return false;
        }
        if( !((ElementBase*)elementID)->isSource())
        {
            SetOptiXLastError("Element is not a source", __FILE__, __func__);
            return false;
        }
        if(wavelength <0)
        {
            SetOptiXLastError("Invalid wavelength", __FILE__, __func__);
            return false;
        }
        ((SourceBase*)elementID)->setWavelength(wavelength);
        ((SourceBase*)elementID)->radiate();
        return true;
    }

    DLL_EXPORT bool ClearImpacts(size_t elementID)
    {
        ClearOptiXError();
        if(!IsValidID(elementID))
        {
            SetOptiXLastError("Invalid element ID", __FILE__, __func__);
            return false;
        }
        Surface* psurf=dynamic_cast<Surface*>((ElementBase*)elementID);
        if(psurf)
        {
            psurf->clearImpacts();
            return true;
        }
           // else this is a group
        SetOptiXLastError("Group object not implemented", __FILE__,__func__);
        return false;
    }


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

        System.clear(); // destroys all elements
        ValidIDs.clear();
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
