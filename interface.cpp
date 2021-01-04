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


class SurfaceCollection:public map<string, Surface*>
{
    typedef map<string, Surface*> BaseMap;
public:
    SurfaceCollection(){}
    ~SurfaceCollection()
    {
        map<string, Surface*>::iterator it;
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

SurfaceCollection System;
StringVector stringData;

extern "C"
{

    DLL_EXPORT size_t CreateSurface(const char* type, const char* name)
    {
        Surface * surf=NULL;
        string s_type(type);
        if (s_type=="XYGridSource")
            surf=new XYGridSource(name);

        else if (s_type=="RadialGridSource")
            surf= new RadialGridSource(name);

        else if (s_type=="Mirror<Plane>" || s_type=="PlaneMirror")
            surf= new Mirror<Plane>(name);
        else if (s_type=="Mirror<Sphere>" || s_type=="SphericalMirror")
            surf= new Mirror<Sphere>(name);
        else if (s_type=="Mirror<Cylinder>" || s_type=="CylindricalMirror")
            surf= new Mirror<Cylinder>(name);

        else if (s_type=="Film<Plane>" || s_type=="PlaneFilm")
            surf= new Film<Plane>(name);
        else if (s_type=="Film<Sphere>" || s_type=="SphericalFilm")
            surf= new Film<Sphere>(name);
        else if (s_type=="Film<Cylinder>" || s_type=="CylindricalFilm")
            surf= new Film<Cylinder>(name);
        else
            cout << " Object not created:  invalid Surface class\n";

        System.insert(pair<string, Surface*>(name, surf) );
        return (size_t)surf;
    }

    DLL_EXPORT size_t GetSurfaceID(const char* surfaceName)
    {
        return (size_t) System.find(surfaceName)->second;
    }

    DLL_EXPORT void GetSurfaceName(size_t surfaceID, char* strBuffer, int bufSize)
    {
        strncpy(strBuffer,((Surface*)surfaceID)->getName().c_str(), bufSize);
    }

    DLL_EXPORT size_t RemoveSurface_byName(const char* name)
    {
        map<string,Surface*>:: iterator it= System.find(name);
        if(it!=System.end())
            it=System.erase(it);
        if(it==System.end())
            return 0;
        return (size_t) it->second;
    }

    DLL_EXPORT size_t RemoveSurface_byID(size_t surfaceID)
    {
        string name= ((Surface*)surfaceID)->getName();
        return RemoveSurface_byName(name.c_str());
    }

    DLL_EXPORT void ChainSurface_byName(const char* previous, const char* next)
    {
        Surface* sprev=(previous[0]==0) ? NULL : System.find(previous)->second;
        Surface* snext=(next[0]==0) ? NULL : System.find(next)->second;
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

    DLL_EXPORT void ChainSurface_byID(size_t prevID, size_t nextID)
    {
        if(prevID==0)
        {
            if(nextID!=0)
               ((Surface*)nextID)->setPrevious(NULL);
        }
        else
        {
            if(nextID==0)
                ((Surface*)prevID)->setNext(NULL);
            else
                ((Surface*)prevID)->setNext((Surface*)nextID);
        }
    }

    DLL_EXPORT bool SetParameter(size_t surfaceID, const char* paramTag, Parameter paramData)
    {
        return ((Surface*)surfaceID)->setParameter(paramTag, paramData);
    }

    DLL_EXPORT bool GetParameter(size_t surfaceID, const char* paramTag, Parameter* paramData)
    {
        return ((Surface*)surfaceID)->getParameter(paramTag, *paramData);
    }

    DLL_EXPORT bool GetNextParameter(size_t surfaceID, size_t * nextPtr, char* tagBuffer, int bufSize , Parameter* paramData)
    {
        map<string, Parameter>::iterator* pRef;
        if (*nextPtr==0)
        {
            pRef= new map<string, Parameter>::iterator( ((Surface*)surfaceID)->parameterBegin() );
        }
        else
            pRef= (map<string, Parameter>::iterator*) *nextPtr;

        strncpy(tagBuffer, (char*)((*pRef)->first).c_str(), bufSize);
        *paramData=(*pRef)->second;

        if(++(*pRef) ==((Surface*)surfaceID)->parameterEnd() )
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
        map<string,Surface*>::iterator it;
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
        size_t surfaceID;

        System.clear();
        while(!file.eof()) // loop of surface creation
        {
            file >> sClass;
            if(sClass.empty())
                break;
            file >> sName >> sNext >> sPrev >> paramName;
            surfaceID=CreateSurface(sClass.c_str(), sName.c_str() );

            while(!paramName.empty())
            {
               file >> param;
               SetParameter(surfaceID, paramName.c_str(), param);

               file >> paramName;
            }

            file.ignore('\n');
        }               // end surface  creation

        file.seekg(0);
                while(!file.eof()) // loop of surface creation
        {
            file >> sClass;
            if(sClass.empty())
                break;
            file >> sName >> sNext ; //>> sPrev ;   // On peut se contenter d'appeler seulement set next qui se chargera de mettre à jour les 2 elements connectés
            if(!sNext.empty() )   // inutile d'agir si sNext empty; les nouvelles sy=urfaces ou tous leurs liens nuls
                System.find(sName)->second->setNext(System.find(sNext)->second); // ces deux surfaces existent; elles viennent d'âtre créées.

            file.ignore('\n'); // skip prameters
        }
        return true;
    }

}
