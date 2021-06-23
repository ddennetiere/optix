#ifndef HEADER_2EAB188D8C944ED0
#define HEADER_2EAB188D8C944ED0

/**
*************************************************************************
*   \file       ConicBaseCylinder.h

*
*   \brief     definition file
*
*
*
*
*   \author             François Polack  <francois.polack@synchroton-soleil.fr>
*   \date               Creation : 2021-06-20

*   \date               Last update: 2021-06-20

*
*
 ***************************************************************************/

#ifndef CONICBASECYLINDER_H
#define CONICBASECYLINDER_H

#include "quadric.h"

/** \brief Describes a cylindrical surface the base of which is a conic
 *
 *  The cylinder  is tangent to the X Y plane at the origin and the generatrix is directed along the Y axis.\n
 *  It is fully defined by the focal points and the angle between focal rays and X axis
 *
 *    The class has 3 specific parameters belonging to the ShapeGroup
 *     -----------------------------------------
 *
 *   Name of parameter | UnitType | Description
 *   ----------------- | -------- | --------------
 *   \b invp  | InverseDistance | reciprocal distance \f$ p^{-1} \f$ of "source" focus (P) from origin
 *   \b invq  | InverseDistance | reciprocal distance \f$ q^{-1} \f$ of "image" focus (Q) from origin*
 *   \b theta0 |     Angle      | angle \f$ \theta_0 \f$ between focal rays and X axis
 *
 *  "source" focus \f$ P= ( \frac { cos \theta_0}{p^{-1}}, -\frac { sin \theta_0}{p^{-1}} )\f$ ;
 *  "image"  focus  \f$ Q= ( \frac { cos \theta_0}{q^{-1}},  \frac { sin \theta_0}{q^{-1}} )\f$
 *
 *  Due to this convention, the cylinder base (directrix) is
 *  -   an ellipse if \f$ p^{-1}   q^{-1} < 0\f$
 *  -   an hyperbola if \f$ p^{-1}   q^{-1} < 0\f$
 *  -   a parabola if either \f$ p^{-1} =0 \f$ , either \f$ q^{-1} =0 \f$
 *  * Warning : \f$ p^{-1} =  q^{-1} \f$ is forbidden and will result as an error at any time.
 *
 * \sa More details to be found in  <a href="../../Coniques_pqTheta.docx">Coniques_pqTheta.docx</a>
 */
class ConicBaseCylinder : virtual public Surface, public Quadric
{
    public:
        /** \brief  Constructor with default initialization
        *
        *   Initialization values
        *   -   \f$ p^{-1}  =-1 \  m^{-1} \f$
        *   -   \f$ q^{-1}  =\  1 \  m^{-1} \f$
        *   -   \f$ \theta_0 =0 \f$
        */
        ConicBaseCylinder();

        /** \brief Default destructor   */
        virtual ~ConicBaseCylinder(){}

        /** \brief return the derived class name ie. ConicBaseCylinder */
        virtual inline string getOptixClass(){return "ConicBaseCylinder";}

        /** \brief Change parameters and recaculate the surface if needed
         * \param name parameter name
         * \param param parameter data
         * \return true if parameter name and value are valid for this object and was set; false otherwise and OptixLastError is set.
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

#endif // CONICBASECYLINDER_H


#endif // header guard

