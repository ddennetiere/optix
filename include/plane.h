#ifndef HEADER_5989607AD2A40F7E
#define HEADER_5989607AD2A40F7E

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
        /**Constructor */
        Plane(string name="" ,Surface * previous=NULL); //:Surface("Plane"){Surface::m_transmissive=true;}
        /** Default destructor */
        virtual ~Plane(); //{}

        /** \brief Orients the surface plane (see important note)
         *
         *  This function is called by the OpticalElement classes after a call to setFrameTransforms
         * \param wavelength  Only used by gratings
         * \return 0 if OK       */
        virtual inline int align(double wavelength=0){
//            Surface::align(0);
            m_hyperplane=RayType::PlaneType(VectorType::UnitZ(),0); //plan perpendiculaire à OZ passant par l'origine
            m_hyperplane.transform(m_surfaceDirect);
            return 0;
        }
//        virtual int align(double wavelength=0)=0;/**< \brief Pure virtual function <b> must be implemented </b> in derived class*/

        virtual inline string getOptixClass(){return "Plane";}/**< return the derived class name ie. Plane */

        virtual inline string getSurfaceClass(){return "Plane";}/**< \brief return the most derived shape class name of this object */


        /** \brief computes the intercept of ray with this plane surface  in the surface local absolute frame and sets the new origin at the intercept
        *
        *   Ray must be expressed in **this** surface frame, in input as in output
        *   \param[in,out]  ray  on input : the last ray position expressed in this surface frame. in output the ray positionned on the surface in this surface frame
        *   \param[out] normal address of a vector which, if not null,  will receive the surface normal (normalized) at the intercept point
        *   \return a reference to the modified ray
        */
        virtual VectorType intercept(RayBaseType& ray, VectorType * normal=NULL)
        {
            // ray-=m_translationFromPrevious; //since 19/06/2024 DO NOT change ref frame from previous to this surface
            if(ray.m_alive)
            {
                if(normal)
                    *normal=m_hyperplane.normal();  // this vector is normalized by construction
                ray.moveToPlane(m_hyperplane).rebase();
            }
            return ray.position();  // == origin() puisque rebase
        }

        /** \brief Dumps internal data to standard output */
        virtual void dumpData()
        {
            cout  <<  "m_hyperplane" << endl << m_hyperplane.coeffs().transpose() << endl;
            cout  <<  "normal" << endl << m_hyperplane.normal().transpose() << endl<<endl;
            ElementBase::dumpData();
        }

    protected:
        RayType::PlaneType m_hyperplane;
    private:
};

#endif // PLANE_H
#endif // header guard

