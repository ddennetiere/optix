#ifndef COLLECTIONS_H_INCLUDED
#define COLLECTIONS_H_INCLUDED

/**
*************************************************************************
*   \file       collections.h

*
*   \brief     collections definition file
*
*
*
*
*   \author             François Polack  <francois.polack@synchroton-soleil.fr>
*   \date               Creation : 2021-03-15

*   \date               Last update: 2021-03-15

*
*
 ***************************************************************************/



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


/** \brief class managing a vector of C_strings (unused)
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




#endif // COLLECTIONS_H_INCLUDED
