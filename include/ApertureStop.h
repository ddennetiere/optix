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

using namespace std;

/** \brief a class implementing a composite aperture, generated as a superimposition of clear and opaque areas
 *
 *  The object is composed of an unlimited number of polygonal or elliptic regions.
 *  The inner part of each region can be either transparent either opaque, the outer part being the opposite.
 *  \n If the region is transparent inside, its transparency combines with the transparency of the underlying layers by a logical or (||) operation
 *  \n If the region is opaque inside, its transparency combines with the transparency of the underlying layers by a logical and (&&) operation
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

        /** \brief Adds a region to the region list and return its index
         *
         * \param pRegion a pointer to the Region object to be added to the list
         * \return the index of the added element in the region list, equal to new size -1
         */
        size_t addRegion(Region* pRegion, bool transparent=true);


      //  size_t addRegion(string regionType, bool transparent=true);

        bool replaceRegion(size_t index, Region*pRegion);

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

    protected:
        vector<Region*> m_regions;/**< \brief list of references of the Regions defining this aperture */

    private:
};

#endif // APERTURESTOP_H


