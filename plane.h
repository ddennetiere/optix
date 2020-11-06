////////////////////////////////////////////////////////////////////////////////
/**
*      \file           plane.h
*
*      \brief         Plane surface declaration
*
*      \author         François Polack <francois.polack@synchroton-soleil.fr>
*      \date        2020-10-13  Creation
*      \date         Last update
*

*/
///////////////////////////////////////////////////////////////////////////////////
//
//             REVISIONS
//
////////////////////////////////////////////////////////////////////////////////////
#ifndef PLANE_H
#define PLANE_H

#include "surface.h"


/** \brief Inplements a plane optical surface
 *
 *      Uses the Surface base class implementation of reflect() and transmit()*/
class Plane : virtual public Surface
{
    public:
        /** Default constructor */
        Plane(string name="" ,Surface * previous=NULL); //:Surface("Plane"){Surface::m_transmissive=false;}
        /** Default destructor */
        virtual ~Plane(); //{}

        /** \brief Orients the surface plane (see important note)
         *
         * <b> Must be called after a call to the base class function  </b> Surface::align() or Grating::align() otherwise  the space transforms have old or default values
         * \param wavelength  Only used by gratings
         * \return 0 if OK       */
        virtual inline int align(double wavelength=0){
//            Surface::align(0);
            m_hyperplane=RayType::PlaneType(VectorType::UnitZ(),0); //plan perpendiculaire à OZ passant par l'origine
            m_hyperplane.transform(m_surfaceDirect);
            return 0;
        }
//        virtual int align(double wavelength=0)=0;/**< \brief Pure virtual function <b> must be implemented </b> in derived class*/

        virtual inline string getRuntimeClass(){return "Plane";}/**< return the derived class name ie. Plane */

        /** \brief computes the intercept of ray with this plane surface  in the surface local absolute frame and sets the new origin at the intercept
        *
        *   \param  ray  on input : the ray in the previous surface frame. in output the ray positionned on the surface in this surface frame
        *   \param normal adress of a vector which, if not null,  will receive the surface normal (normalized) at the intercept point
        *   \return a reference to the modified ray
        */
        EIGEN_DEVICE_FUNC virtual VectorType intercept(RayType& ray, VectorType * normal=NULL)
        {
            ray-=m_translationFromPrevious; // change ref fram from previous to this surface
            if(normal)
                *normal=m_hyperplane.normal();  // this vector is normalized by construction
            ray.moveToPlane(m_hyperplane).rebase();
            return ray.position();  // == origin() puisque rebase
        }

    protected:
        RayType::PlaneType m_hyperplane;
    private:
};

#endif // PLANE_H
