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

/** \brief This class defines a polygonal 2D region to be used as an aperture stop defining element.
 */
class Polygon: public Region
{
    public:

        Polygon();  /**< Default constructor. Contruct a transparent null polygon with 0 side */
        virtual ~Polygon(){}    /**<  Default destructor*/

        virtual inline std::string getOptixClass(){return "Polygon";}

        /** \brief constructs a closed polygonal region from an array of vertices. The region is transparent by default
         *
         * \param vertices a reference to an array containing the vertex points
         */
        Polygon(const Ref<Array2Xd> &vertices);

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

        /** \brief Adds a vertex point afte the specified one
         *
         * \param nprevious the vertex index after which the new vertex must be added
         * \param x the abscissa of the new point
         * \param y the ordinate of the new point
         */
        void insertVertex(size_t nprevious, double x, double y);

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
         * \param angle the angle of the rotation part of the isometric transformation applied to the region
         * \param translation the reference to a Vector2d containing the translation part of the transform
         */
         void move(double angle, const Ref<Vector2d> &translation);

         void setSymmetric(const Ref<Vector2d> &point, const Ref<Vector2d> &dir);
         void setSymmetric(const Ref<Vector2d> &point);

         /** \brief dump the internal data arrays (vertices, side vectors and side equations) to std_out
          */
         void dump(){std::cout << m_vertices <<  std::endl << m_vects <<std::endl<< m_sides <<std::endl;}
    protected:
        bool checkConvex();
        size_t m_size;
        Matrix2Xd m_vertices, m_vects;
    private:
        Vector2d m_refPoint;
        bool m_convex;
        Matrix3Xd m_sides;
};

#endif // POLYGON_H

