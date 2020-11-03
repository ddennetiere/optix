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
