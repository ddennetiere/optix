#ifndef HEADER_C3B192B048ADFF68
#define HEADER_C3B192B048ADFF68


#include "quadric.h"

/**
*************************************************************************
*   \file       cone.h

*
*   \brief     definition file
*
*
*
*
*   \author             François Polack  <francois.polack@synchroton-soleil.fr>
*   \date               Creation : 2023-05-10

*   \date               Last update: 2023-05-10

*
*
 ***************************************************************************/

#ifndef CONE_H
#define CONE_H

/** \brief Describes a conical surface defined by its apex and a directrix circle
 *
 *   The surface is positioned such as the X axis is one generatrix and the apex is the point of coordinates (-x0, 0,0)
 *   the directrix circle passing by the origin has a curvature 1/r and its plane is inclined by \f$ theta \f$ around the direction Y
 *   It is a revolution cone only if the apex is on the circle normal passing by its center.
 *
 *    The class has three specific parameters belonging to the ShapeGroup
 *     -----------------------------------------
 *
 *   Name of parameter | UnitType | Description
 *   ----------------- | -------- | --------------
 *   \b curvature | InverseDistance | Curvature (1/Rc) of the directric circle passing through the origin point
 *   \b directrix_angle | Angle | Inclination angle of the directrix circle plane with respect to the YOZ plane
 *   \b apex_distance  | Distance | distance from apex to the origin point
 *
 *  The 3 parameter are continuous over the value 0. However when x0=0 the cone degenerates in 2 planes : XOY and
 *  the directrix plane, and the intercept function may fail
 *
 *  \image html  cone.png "Construction geometry of the cone shape"
 */
class Cone: public Quadric
{
    public:
        /** default constructor    */
        Cone();
        /** default destructor    */
        virtual ~Cone(){}
        virtual inline string getOptixClass(){return "Cone";}
        virtual inline string getSurfaceClass(){return "Cone";}/**< \brief return the most derived shape class name of this object */

        /** \brief Change parameters and recaculate the surface if needed
         * \param name parameter name
         * \param param parameter
         * \return true if parameter exists and was set ; false if parameeter doesnt exist
         */
        inline bool setParameter(string name, Parameter& param)
        {
            if(! Surface::setParameter(name, param))
                    return false;
            if( name=="curvature" || name=="directrix_angle" || name=="apex_distance" )
                createSurface();
            return true;
        }
    protected:
        void createSurface(); /**< \brief Inilialize the local equation. Called when a parameter is changed  */
    private:
};

#endif // CONE_H
#endif // header guard

