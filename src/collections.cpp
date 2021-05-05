/**
 *************************************************************************
*   \file           collections.cpp
*
*   \brief         ElementCollection implementation file
*
*
*
*   \author             François Polack  <francois.polack@synchroton-soleil.fr>
*   \date               Creation: 2021-05-05
*   \date               Last update: 2021-05-05
 ***************************************************************************/
#include "collections.h"
#include "opticalelements.h"

ElementCollection::~ElementCollection()
{
    map<string, ElementBase*>::iterator it;
    for(it=begin(); it!=end();  ++it)
        delete it->second;
}
ElementBase* ElementCollection::createElement(const string type, const string name)
{
    ClearOptiXError();
    if(find(name)!=end())
    {
        SetOptiXLastError("Name already exists in the current system", __FILE__, __func__);
        return NULL;
    }
    ElementBase * elem=0;
    try {
            elem=(ElementBase*)CreateElementObject(type,name);
        }
        catch(ElementException& ex)
        {
             SetOptiXLastError( "Invalid element type", __FILE__,__func__);
             return NULL;
        }

    insert(pair<string, ElementBase*>(name, elem) );
    ValidIDs.insert((size_t) elem);
    return elem;
}

int ElementCollection::erase(string key)
{
    iterator it=find(key);
    if(it==end())
        return 0; // element doesn't exist
    delete it->second;
    ValidIDs.erase( (size_t) it->second );
    return BaseMap::erase(key);
}

ElementCollection::iterator  ElementCollection::erase (iterator pos)
{
    if(pos==end())
        return pos;
    delete pos->second;
    ValidIDs.erase( (size_t) pos->second );
    return BaseMap::erase(pos);
}

ElementCollection::iterator ElementCollection::erase(const_iterator first, const_iterator last)
{
    for(const_iterator it=first; it!=last; ++it)
    {
        delete it->second;
        ValidIDs.erase( (size_t) it->second );
    }
    if(last!=end())
    {
        delete last->second;
        ValidIDs.erase( (size_t) last->second );
    }
    return BaseMap::erase(first,last);
}
