#ifndef POLYGON_H
#define POLYGON_H

/**
*************************************************************************
*   \file       Polygon.h
*
*
*   \brief     definition file of the Polygon class. A polygonal Region used for aperture stops definition.
*
*
*   \author             François Polack  <francois.polack@synchroton-soleil.fr>
*   \date               Creation : 2022-01-24
*
*   \date               Last update: 2022-01-24
*
*
*
 ***************************************************************************/
#include "region.h"
#include <libxml/tree.h>

/** \brief This class defines a polygonal 2D region to be used as an aperture stop defining element.
 */
class Polygon: public Region
{
    public:

        /** \brief  Default constructor. Construct a transparent null polygon with 0 side. transparency is true by default
         *
         * \param transparent true if the region is transparent inside opaque outside,  false in the opposite case
         */
        Polygon(bool transparent=true);

        virtual inline std::string getOptixClass(){return "Polygon";}

        /** \brief constructs a closed polygonal region from an array of vertices. The region is transparent by default
         *
         * \param vertices a reference to an array containing the vertex points
         */
        Polygon(const Ref<Array2Xd> &vertices);

        size_t getNumSides(){return m_size;}/**< \brief returns the number of sides of the polygon */

        Matrix2Xd getVertices(){return m_vertices.leftCols(m_size);} /**< \brief returns the vertex of the polygon in an m_size width array (without the repeated end point)*/

        /** \brief defines a new polygon from an array of vertices
         *
         * \param vertices a reference to an array containing the vertex points
         */
        void setVertices(const Ref<Array2Xd> &vertices);

        /** \brief Change one vertex of the polygon
         *
         * \param n  the index of the vertex to be changed
         * \param x the abscissa of the new point
         * \param y the ordinate of the new point
         */
        void changeVertex(size_t n, double x, double y);

        /** \brief suppress one of the vertex points
         *
         * \param n the index of the vertex to remove
         */
        void deleteVertex(size_t n);

        /** \brief Adds a vertex point at the specified one
         *
         * \param pos the position before which the new vertex will be added
         * \param x the abscissa of the new point
         * \param y the ordinate of the new point
         */
        void insertVertex(size_t pos, double x, double y);

        /** \brief checks whether the polygon is convex or not
         *
         * \return tue if convex ; false otherws
         *
         */
        bool isConvex(){return m_convex;}

        /** \brief Defines a rectangular polygon from the length of the sides and the center position
         *
         * \param xsize the length of the sides parallels to the X axis
         * \param ysize the length of the sides parallels to the Y axis
         * \param xcenter the abscissa of the center
         * \param ycenter the ordinate of the center
         */
        void setRectangle(double xsize, double ysize, double xcenter, double ycenter);

        /** \brief returns the location of a point relative to the Polygon.
         *
         *  Note that no check is done for detecting self intersecting sides, in which case the result would have no sense
         * \param point the reference to a Vector2d containing the point to be located
         * \return a \ref Location type value
         */
        Location locate(const Ref<Vector2d> &point);

        /** \brief moves a Polygon by applying a positive isometry
         *
         * \param angle the angle of the rotation part of the isometric transformation applied to the region in radians
         * \param translation the reference to a Vector2d containing the translation part of the transform
         */
         void move(double angle, const Ref<Vector2d> &translation);

         void setSymmetric(const Ref<Vector2d> &point, const Ref<Vector2d> &dir);
         void setSymmetric(const Ref<Vector2d> &point);

         /** \brief \brief \e Debugging \e function: dumps the internal data arrays (vertices, side vectors and side equations) to std::out
          */
         void dump(){std::cout << m_vertices <<  std::endl << m_vects <<std::endl<< m_sides <<std::endl;}

//        friend xmlNodePtr operator<<(xmlNodePtr apernode, const Polygon & polygon);
//        friend xmlNodePtr operator>>(xmlNodePtr apernode,  Polygon & polygon);
        void operator>>(xmlNodePtr apernode);
        void operator<<(xmlNodePtr regnode);

    protected:
        bool checkConvex();/**< \brief auxiliary function called during object construction in order to check whether the simplified location algorithm is applicable  */
        size_t m_size; /**< \brief the number of vertices (and sides of this polygon */
        Matrix2Xd m_vertices;/**< \brief a 2 x size+1  matrix containing the vertex points. Vertex[0] is repeated as Vertex[size] for algorithm performance */
        Matrix2Xd m_vects;  /**< \brief a 2 x size+1  matrix containing the side vectors. Vector[0] is repeated as Vector[size] for algorithm performance */

    private:
        /**< \brief a ref point for defining the secant direction in the non-convex location algorithm
        *
        *   This point must be inside the smallest convex polygonal region enclosing this polygon. It is presently defined as the barycenter of the vertices  */
        Vector2d m_refPoint;
        bool m_convex; /**< \brief boolean value to keep track of the polygon convexity property */
        Matrix3Xd m_sides;  /**<  \brief a 2 x size matrix containing  the parameter of the line equation of the sides. (used by non-convex location algorithm*/
};

#endif // POLYGON_H

