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


class ElementCollection:public map<string, ElementBase*>
{
    typedef map<string, ElementBase*> BaseMap;
public:
    ElementCollection(){}
    ~ElementCollection()
    {
        map<string, ElementBase*>::iterator it;
        for(it=begin(); it!=end();  ++it)
            delete it->second;
    }

    int erase(string key)
    {
        iterator it=find(key);
        if(it==end())
            return 0; // element doesn't exist
        delete it->second;
        return BaseMap::erase(key);
    }
    iterator erase (iterator pos)
    {
        if(pos==end())
            return pos;
        delete pos->second;
        return BaseMap::erase(pos);
    }
    iterator erase(const_iterator first, const_iterator last)
    {
        for(const_iterator it=first; it!=last; ++it)
            delete it->second;
        if(last!=end())
            delete last->second;
        return BaseMap::erase(first,last);
    }
    inline void clear(){ erase(begin(),end());}
} ;


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

ElementCollection System;
StringVector stringData;

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

    DLL_EXPORT bool SetParameter(size_t elementID, const char* paramTag, Parameter paramData)
    {
        return ((ElementBase*)elementID)->setParameter(paramTag, paramData);
    }

    DLL_EXPORT bool GetParameter(size_t elementID, const char* paramTag, Parameter* paramData)
    {
        return ((ElementBase*)elementID)->getParameter(paramTag, *paramData);
    }

    DLL_EXPORT bool GetNextParameter(size_t elementID, size_t * nextPtr, char* tagBuffer, int bufSize , Parameter* paramData)
    {
        map<string, Parameter>::iterator* pRef;
        if (*nextPtr==0)
        {
            pRef= new map<string, Parameter>::iterator( ((ElementBase*)elementID)->parameterBegin() );
        }
        else
            pRef= (map<string, Parameter>::iterator*) *nextPtr;

        strncpy(tagBuffer, (char*)((*pRef)->first).c_str(), bufSize);
        *paramData=(*pRef)->second;

        if(++(*pRef) ==((ElementBase*)elementID)->parameterEnd() )
        {
            delete pRef;
            *nextPtr=0;
            return false;
        }
        * nextPtr=(size_t) pRef;
        return true;
    }

//    DLL_EXPORT void FreeNextPointer(size_t nextPtr)  // il n'y a aucune raison de libérer la mémoire localisée par ce pointeur
//    {
//        map<string, Parameter>::iterator* pRef;
//        if(nextPtr==0)
//            delete (map<string, Parameter>::iterator*) nextPtr;
//    }

    DLL_EXPORT void SaveSystem(const char* filename)
    {
        TextFile file(filename, ios::out);
        map<string,ElementBase*>::iterator it;
        for (it=System.begin(); it!=System.end(); ++it)
            file << *(it->second);
        file.close();
    }

    DLL_EXPORT bool OpenSystem(const char* filename)
    {
        TextFile file(filename, ios::in);
        if(!file.is_open())
            return false;

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
            ElementBaseID=CreateElement(sClass.c_str(), sName.c_str() );

            while(!paramName.empty())
            {
               file >> param;
               SetParameter(ElementBaseID, paramName.c_str(), param);

               file >> paramName;
            }

            file.ignore('\n');
        }               // end ElementBase  creation

        file.seekg(0);
                while(!file.eof()) // loop of ElementBase creation
        {
            file >> sClass;
            if(sClass.empty())
                break;
            file >> sName >> sNext ; //>> sPrev ;   // On peut se contenter d'appeler seulement set next qui se chargera de mettre à jour les 2 elements connectés
            if(!sNext.empty() )   // inutile d'agir si sNext empty; les nouvelles sy=urfaces ou tous leurs liens nuls
                System.find(sName)->second->setNext(System.find(sNext)->second); // ces deux ElementBases existent; elles viennent d'âtre créées.

            file.ignore('\n'); // skip prameters
        }
        return true;
    }

}
