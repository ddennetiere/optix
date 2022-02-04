#ifndef COLLECTIONS_H_INCLUDED
#define COLLECTIONS_H_INCLUDED

/**
*************************************************************************
*   \file       collections.h

*
*   \brief     collection classes definition file
*
*
*
*   \author             François Polack  <francois.polack@synchroton-soleil.fr>
*   \date               Creation : 2021-03-15

*   \date               Last update: 2021-05-15

*
*
 ***************************************************************************/

#include <set>
#include "elementbase.h"


/** \brief a class implementing a dictionary of optical elements from the library  which is a specialization of the map<string, ElementBase*>  class
 *  And \e will manage the création and deletion of new elements
 */
class ElementCollection:public map<string, ElementBase*>
{
    typedef map<string, ElementBase*> BaseMap;
public:
    ElementCollection(){}/**< \brief Creates an empty dictionary */
    ~ElementCollection();/**< \brief Destroys all elements and releases memory  */

    /** \brief
     *
     * \param type the type (class) of the new element ( \see opticalelements.h)
     * \param name the name of the new element
     * \return a pointer to the new element
     */
    ElementBase* createElement(const string type, const string name);

    /** \brief removes an entry in the dictionary by name
    *    \param key the entry name */
    int erase(string key);

    /** \brief removes an entry of the dictionary by iterator pointer
    *   \param pos the iterator pointing on the element to remove
    *   \return an iterator to the element following the removed one in the list
    */
    iterator erase (iterator pos);

    /** \brief removes a range of entries
    *   \param first iterator pointing to the first element to remove
    *   \param last iterator pointing to the last element to remove
    *   \return an iterator to the element following the last removed one in the list
    */
    iterator erase(const_iterator first, const_iterator last);

    /**  \brief clears the whole dictionary */
    inline void clear(){ erase(begin(),end());}

    /** \brief  Checks if the ID is recorded in the element collection
     * \param ID the Id to check
     * \return true if present */
    inline bool isValidID(size_t ID)
    {
        set<size_t>::iterator it=ValidIDs.find(ID);
        return it!=ValidIDs.end();
    }

public : // attributes
    set<size_t> ValidIDs;/**< Liste des identificateurs (elemIDs) présents dans la collection */

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
