#ifndef HEADER_A41E56F526BF17F7
#define HEADER_A41E56F526BF17F7

////////////////////////////////////////////////////////////////////////////////
/**
*      \file           cylinder.h
*
*      \brief         Cylinder surface class definition
*
*      \author         François Polack <francois.polack@synchroton-soleil.fr>
*      \date        2020-10-29  Creation
*      \date         Last update
*

*/
///////////////////////////////////////////////////////////////////////////////////
//
//             REVISIONS
//
////////////////////////////////////////////////////////////////////////////////////
#ifndef CYLINDER_H
#define CYLINDER_H

#include "quadric.h"


/** \brief Describes a cylindrical surface
 *    The class has two specific parameters belonging to the ShapeGroup
 *     -----------------------------------------
 *
 *   Name of parameter | UnitType | Description
 *   ----------------- | -------- | --------------
 *   \b curvature | InverseDistance | Curvature (1/Rc) of the cylinder
 *   \b axis_angle | Angle | Orientation of the axis;  default=0: tangential cylinder
 */
class Cylinder : public Quadric
{
    public:
        /** Default constructor */
        Cylinder();
        /** Default destructor */
        virtual ~Cylinder(){}
        virtual inline string getRuntimeClass(){return "Cylinder";}

        /** \brief Change parameters and recaculate the surface if needed
         * \param name parameter name
         * \param param parameter
         * \return true if parameter exists and was set ; false if parameeter doesnt exist
         */
        inline bool setParameter(string name, Parameter& param)
        {
            if(! Surface::setParameter(name, param))
                    return false;
            if(name=="curvature" || name=="axis_angle")
                createSurface();
            return true;
        }
    protected:
        void createSurface(); /**< \brief Inilialize the local equation. Called when a parameter is changed  */
    private:
};

#endif // CYLINDER_H
#endif // header guard

