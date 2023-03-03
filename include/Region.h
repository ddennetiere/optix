#ifndef REGION_H
#define REGION_H

/**
*************************************************************************
*   \file       Region.h

*
*   \brief    Definition file of the Region class and environment structures needed for implementing aperture and obstruction stops.
*
*
*
*
*   \author             François Polack  <francois.polack@synchroton-soleil.fr>
*   \date               Creation : 2022-02-02

*   \date               Last update: 2022-02-02

*
*
 ***************************************************************************/

#include "EigenSafeInclude.h"
#include <iostream>
#include <float.h> // pour DBL_EPSILON

/** \ingroup enums
 *  \brief Enumerated values use to return the position of a point with respect to a region
 */
enum Location {
    undetermined=-9999, /**< The position could not be determined (algorithm failure) */
    outside=-1,         /**< The point is outside the closed region */
    border=0,           /**< The point is on the region border */
    inside=1            /**< The point is inside the region */
};

using namespace Eigen;

/** \brief A pure virtual base class for defining binary transmitting or stopping areas
 *
 *    All derived class must implement the locate and move functions
 */
class Region
{
    public:
        /** \brief  default constructor. Region transparency is true by default
         *
         * \param transparent true if the region is transparent inside opaque outside,  false in the opposite case
         */
        Region(bool transparent=true):m_transparent(transparent) {}
        virtual ~Region() {}    /**< default constructor. Does nothing  */

        virtual inline std::string getOptixClass()=0;   /**< \brief return the derived class name of this object */

        void setTransparency(bool transparent){m_transparent=transparent; } /**< \brief make the region inside transparent and the outside opaque */

     //   void setOpaque() {transparent=false;}  /**< \brief make the region inside opaque and the outside transparent */

        bool isTransparent(){return m_transparent;}   /**< \brief return whether or not the region inside is transparent */

        /** \brief function returning the location of a point relative to the Region.
         *
         *  This is a pure virtual function which MUST be defined in any derived classe.
         * \param point the reference to a Vector2d containing the point to be located
         * \return type must be \ref Location
         */
        virtual Location locate(const Ref<Vector2d> &point)=0;

        /** \brief moves a region by applying a positive isometry
         *
         *  This is a pure virtual function which MUST be defined in any derived classe.
         * \param angle the angle of the rotation part of the isometric transformation applied to the region
         * \param translation the reference to a Vector2d containing the translation part of the transform
         */
        virtual void move(double angle, const Ref<Vector2d> &translation)=0;

        /** \brief Change the region for its symmetric with respect to a given axis
         *
         * \param point the reference to a Vector2d containing one point of the symmetry axis
         * \param dir the reference to a Vector2d parallel to the direction of the symmetry axis (need not be unity)
         */
        virtual void setSymmetric(const Ref<Vector2d> &point, const Ref<Vector2d> &dir)=0;

        /** \brief Change the region for its symmetric with respect to a given point
         *
         * \param point the reference to a Vector2d containing the symmetry point
         */
        virtual void setSymmetric(const Ref<Vector2d> &point)=0;


    protected:
        bool m_transparent; /**< \brief An indicator of whether the region is transparent inside and opaque outside (true) or vice versa (false)  */

    private:
};

#endif // REGION_H
