#ifndef HEADER_BEDF05445AAE40CB
#define HEADER_BEDF05445AAE40CB

/**
*************************************************************************
*   \file       Poly1D.h

*
*   \brief     definition file
*
*
*
*
*   \author             François Polack  <francois.polack@synchroton-soleil.fr>
*   \date               Creation : 2021-02-21

*   \date               Last update: 2021-02-23

*
*
 ***************************************************************************/

#ifndef POLY1D_H
#define POLY1D_H

#include "gratingbase.h"
#include "elementbase.h"


/** \class Poly1D
 *  \brief  this class define a  variable spaced line pattern parallel to the Y axis with a polynomial spacing law. It implements the  gratingVector() functionof the abstract base class Pattern

 *    The class has (degree+1) specific parameters belonging to the SourceGroup
 *     -----------------------------------------
 *
 *   Name of parameter | UnitType | Description
 *   ----------------- | -------- | --------------
 *   \b degree | Dimensionless | Degree of the line density polynomial; default=3
 *   \b lineDensity | InverseDistance  | Central line density
 *   \b lineDensityCoeff_\e n | Distance ^-\e n | Line density coefficient at order \e n
 * <em>   with 1 < n < degree </em>
 */
class Poly1D : virtual public Surface, virtual public Pattern
{
    public:
        Poly1D(int degree=3);/**< \brief constructor a pattern of grating lines whose projections on the tangent plane are parallel to Y and spaced along X in the given polynomial law
         *  \param degree the degree of the line density polynomial default is 3*/
        virtual ~Poly1D(){}

        /** \brief record pattern definition parameters. This function \b must be overridden in derived class and call after the Surface::SetParameter() function was called.
         * \param name parameter name
         * \param param Parameter object
         * \return true if the parameter was recognized and properly set, false otherwise
         */
        virtual  bool setParameter(string name, Parameter& param);
        virtual  inline string getRuntimeClass(){return "Poly1D";}/**< return the derived class name ie. Poly1D */



        /** \brief compute the local line density vector
         *
         * \param[in]  position position where the line density vector must be computed
         * \param[in] normal  vector normal to the surface at position (usually position and normal are provided by the Surface::intercept() function)
         * \return the line density vector at position. it is perpendicular to normal and Y (in surface reference frame)
         */
        EIGEN_DEVICE_FUNC virtual Surface::VectorType gratingVector(Surface::VectorType position,
                                Surface::VectorType normal);

    protected:
        int m_degree;       /**< degree of line density polynomial */
        ArrayXd m_coeffs;   /**< The array of line density polynomial coefficients */
    private:
};

#endif // POLY1D_H
#endif // header guard

