#ifndef HEADER_4A628506A6948E78
#define HEADER_4A628506A6948E78

////////////////////////////////////////////////////////////////////////////////
/**
*      \file           sphere.h
*
*      \brief         Spherical surface class definition
*
*      \author         François Polack <francois.polack@synchroton-soleil.fr>
*      \date        2020-10-28  Creation
*      \date         Last update
*

*/
///////////////////////////////////////////////////////////////////////////////////
//
//             REVISIONS
//
////////////////////////////////////////////////////////////////////////////////////
#ifndef SPHERE_H
#define SPHERE_H

#include "quadric.h"


/** \brief Describes a spherical surface
 *
 *    The class has one specific parameter belonging to the ShapeGroup
 *     -----------------------------------------
 *
 *   Name of parameter | UnitType | Description
 *   ----------------- | -------- | --------------
 *   \b curvature | InverseDistance | Curvature (1/Rc) of the sphere
 */
class Sphere :  virtual public Surface, public Quadric
{
    public:
        /** \brief  Default constructor. Initialize curvature to 0.*/
        Sphere();
        /** Default destructor */
        virtual ~Sphere(){}
        /**< \brief return the derived class name ie. Sphere */
        virtual inline string getOptixClass(){return "Sphere";}

        virtual inline string getSurfaceClass(){return "Sphere";}/**< \brief return the most derived shape class name of this object */


        /** \brief Change parameters and recaculate the surface if needed
         * \param name parameter name
         * \param param parameter
         * \return true if parameter name is valid for this object and was set; false otherwise
         */
        inline bool setParameter(string name, Parameter& param)
        {
            if(! Surface::setParameter(name, param))
                    return false;
            if(name=="curvature")
                createSurface();
            return true;
        }
    protected:
        void createSurface(); /**< \brief Initialize the local equation. Called when curvature is changed  */
    private:
};

#endif // SPHERE_H
#endif // header guard

