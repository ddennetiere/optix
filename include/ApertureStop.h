#ifndef APERTURESTOP_H
#define APERTURESTOP_H

/**
*************************************************************************
*   \file       ApertureStop.h

*
*   \brief     definition file of ApertureStop class
*
*
*
*
*   \author             François Polack  <francois.polack@synchroton-soleil.fr>
*   \date               Creation : 2022-02-04

*   \date               Last update: 2022-02-04

*
*
 ***************************************************************************/
#include <map>
#include "region.h"
#include <libxml/tree.h>

//using namespace std; no longer valid
using std::map;
using std::vector;

/** \brief a class implementing a composite aperture, generated as a superimposition of clear and opaque areas
 *
 *  The object is composed of an unlimited number of polygonal or elliptic regions.
 *  The inner part of each region can be either transparent either opaque.
 *  Each region impose its transparency (or opacity) in its inner part to all underlying regions.
 *  The outside of all regions is given a transparency inverse to that of the first region in the stack
 */
class ApertureStop
{
    public:

        ApertureStop(){}    /**< \brief creates an empty ApertureStop container with no obstructing region*/

        /** \brief destructor. Clear the region list and destroy the object
         *
         *  the destructor will destroy all regions of the list with the delete operator. It mean that the regions must be created on the stack before assignation
         */
        virtual ~ApertureStop();

        /** \brief Get the transmission factor of the composite aperture at a given point
         *
         * \param point a constant reference to the point where the transmission is requested
         * \return the transmission value at this point. Can be 0 or 1.
         *
         */
        double getTransmissionAt(const Ref<Vector2d> &point);

        /** \brief Adds a region to the top of the region list and return its index
         *
         * \param pRegion a pointer to the Region object to be added to the list
         * \return the index of the added element in the region list, equal to new size -1
         */
        size_t addRegion(Region* pRegion);

        /** \brief  insert a region at a given position in of the region list
         *
         * \param index the index that the new region should have after insertion
         * \param pRegion a pointer to the Region object to be inserted into the list
         * \return true if successful; false if index is out of range
         */
        bool insertRegion(size_t index, Region *pRegion);

      //  size_t addRegion(string regionType, bool transparent=true);

        /** \brief Define the transparency of the inner part of the specified region
         *
         * \param index The index of the regien whose transparency will be modified
         * \param transparent transparency value: true if the inner part of the region is transparent, false on the reverse
         * \return true if successful; false if index is out of range
         */
        bool setRegionTransparency(size_t index, bool transparent);


        /** \brief Replace the region at a given position in of the region list by a new one
         *
         * \param index the index of the region to be replaced
         * \param pRegion a pointer to the new Region object to be inserted in the list
         * \return true if successful; false if index is out of range
         */
        bool replaceRegion(size_t index, Region* pRegion);

        /** \brief Remove the region from the region list
         *
         * \param index index in the region list of the element to remove
         * \return true if element was removed ; false if the index is out of range
         */
        bool removeRegion(size_t index);

        /** \brief  Gets a pointer to the element at index position
         *
         * \param index the index in the region list of the region to be retreived
         * \return a pointer to the retreived Region object
         */
        Region* getRegion(size_t index);

        size_t getRegionCount(){return m_regions.size();}   /**< \brief return the number of region assigned to this apertureStop */

        friend xmlNodePtr operator<<(xmlNodePtr doc, const ApertureStop & generator);
        friend xmlNodePtr operator>>(xmlNodePtr doc, const ApertureStop & generator);

    protected:
        vector<Region*> m_regions;/**< \brief list of references of the Regions defining this aperture */

    private:
};

#endif // APERTURESTOP_H


