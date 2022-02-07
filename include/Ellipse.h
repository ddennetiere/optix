#ifndef ELLIPSE_H
#define ELLIPSE_H


/**
*************************************************************************
*   \file       Ellipse.h

*
*   \brief     definition file of the Ellipse class. An elliptical Region used for aperture stops definition
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

#include "region.h"

/** \brief  This class defines a elliptic 2D region to be used as an aperture stop defining element.
 */
class Ellipse: public Region
{
    public:
        /** \brief Default creator: actually creates a circle of radius 1 (m)
         */
        Ellipse();
        virtual ~Ellipse(){}

        virtual inline std::string getOptixClass(){return "Ellipse";}

        /** \brief Defines an elliptical region from axis sizes, center position and orientation angle
         *
         * \param a Half length of the large axis
         * \param b Half length of the small axis
         * \param xcenter abscissa of the ellipse center (default 0)
         * \param ycenter ordinate of the ellipse center (default 0)
         * \param angle  orientation angle of the large axis with respect to the X axis in rd (default 0)
         */
        Ellipse(double a, double b, double xcenter=0, double ycenter=0, double angle=0);
        // on pourrait également définir une ellipse inscrite dans un parallélogramme (2 tangentes), mais il faut en plus definir 1 point sur  (ou une tagente à)l'ellipse
        /** \brief Create a circular region
         *
         * \param radius radius of the circle
         * \param xcenter X coordinate of the circle center
         * \param ycenter Y coordinate of the circle center
         */
        void setCircle(double radius,  double xcenter=0, double ycenter=0);
        Location locate(const Ref<Vector2d> &point);
        void move(double angle, const Ref<Vector2d> &translation);
        void setSymmetric(const Ref<Vector2d> &point, const Ref<Vector2d> &dir);
        void setSymmetric(const Ref<Vector2d> &point);

    protected:

    private:
        Matrix3d m_Mat;
};

#endif // ELLIPSE_H

