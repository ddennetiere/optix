#ifndef HEADER_727AFAF7DBF181AB
#define HEADER_727AFAF7DBF181AB

/**
*************************************************************************
*   \file       RevolutionQuadric.h

*
*   \brief     definition file
*
*
*
*
*   \author             François Polack  <francois.polack@synchroton-soleil.fr>
*   \date               Creation : 2021-06-23

*   \date               Last update: 2021-06-23

*
*
 ***************************************************************************/

#ifndef REVOLUTIONQUADRIC_H
#define REVOLUTIONQUADRIC_H

#include "quadric.h"


/** \brief Describes a a revolution quadric from its focal points
 *
 *  The quadric is tangent to the X Y plane at the origin. Its revolution axis is oriented along OX.\n
 *  It is fully defined by the focal points and the angle between focal rays and X axis
 *
 *    The class has 3 specific parameters belonging to the ShapeGroup
 *     -----------------------------------------
 *
 *   Name of parameter | UnitType | Description
 *   ----------------- | -------- | --------------
 *   \b invp  | InverseDistance | reciprocal distance \f$ p^{-1} \f$ of "source" focus (P) from origin
 *   \b invq  | InverseDistance | reciprocal distance \f$ q^{-1} \f$ of "source" focus (Q) from origin*
 *   \b theta0 |     Angl    | angle \f$ \theta_0 \f$   at surface center between focal rays and tangent plane
 *
 *  Point \f$ P= ( \frac { cos \theta_0}{p^{-1}}, -\frac { sin \theta_0}{p^{-1}} )\f$ ;
 *  Point \f$ Q= ( \frac { cos \theta_0}{q^{-1}},  \frac { sin \theta_0}{q^{-1}} )\f$
 *
 *  Due to this convention, the quadic
 *  -   an ellipsoid if the product \f$ p^{-1}   q^{-1} \f$ is negative.
 *  -   an hyperboloid if the product \f$ p^{-1}   q^{-1} \f$ is positive
 *  -   a paraboloid if either \f$ p^{-1} =0 \f$ , either \f$ q^{-1} =0 \f$
 *  * Warning : \f$ p^{-1} =  q^{-1} \f$ is forbidden and will result as an error at any time.
 *
 * \sa More details to be found in  <a href="../../Coniques_pqTheta.docx">Coniques_pqTheta.docx</a>
 */
class RevolutionQuadric : public virtual Surface, public Quadric
{
    public:

        /** \brief  Constructor with default initialization
        *
        *   Initialization values
        *   -   \f$ p^{-1}  =-1 \  m^{-1} \f$
        *   -   \f$ q^{-1}  =\  1 \  m^{-1} \f$
        *   -   \f$ \theta_0 = 0\f$
        */
        RevolutionQuadric();

        /** \brief Default destructor   */
        virtual ~RevolutionQuadric(){}

        /** \brief return the derived class name ie. RevolutionQuadric */
        virtual inline string getOptixClass(){return "RevolutionQuadric";}
        virtual inline string getSurfaceClass(){return "RevolutionQuadric";}/**< \brief return the most derived shape class name of this object */


        /** \brief Change parameters and recaculate the surface if needed
         * \param name parameter name
         * \param param parameter data
         * \return true if parameter name is valid for this object and was set; false otherwise,  and OptixLastError is set.
         */
        inline bool setParameter(string name, Parameter& param)
        {
            if(! Surface::setParameter(name, param))
                    return false;
            try
            {

                if(name=="invp" || name=="invq" || name=="theta0")
                    createSurface();
                return true;
            }
            catch(ParameterException& ex)
            {
                SetOptiXLastError(ex.what()+ "called from ",__FILE__, __func__);
                return false;
            }
        }

    protected:
        /** \brief Initialize the local quadric equation from the parameter set (called each time a shape parameter is changed)  */
        void createSurface();
    private:
};

#endif // REVOLUTIONQUADRIC_H
#endif // header guard

