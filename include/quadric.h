#ifndef QUADRIC_H_INCLUDED
#define QUADRIC_H_INCLUDED

////////////////////////////////////////////////////////////////////////////////
/**
*      \file           quadric.h
*
*      \brief         Quadric surface class declaration
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
#include "surface.h"



/** \brief Implements an optical surface wich can be described by a quadratic form,  sphere, cylinders, ellipsoids and hyperboloids
 *
 *      Uses the Surface base class implementation of reflect() and transmit()  */
class Quadric : virtual public Surface
{
    public:
        /** Default constructor */
        Quadric();          //:Surface("Quadric"){Surface::m_transmissive=false;}

        /** Default destructor */
        virtual ~Quadric();     //{}

        /** \brief Orients the quadric surface (see important note)
         *
         * <b> Must be called after a call to the base class function </b> Surface::align() or Grating::align() otherwise  the space transforms have old or default values
         * \param wavelength  Only used by gratings
         * \return 0 if OK       */
        virtual inline int align(double wavelength=0)
        {
            m_alignedQuadric = m_surfaceInverse.matrix().transpose() * m_localQuadric * m_surfaceInverse.matrix();
            return 0;
        }

//        virtual int align(double wavelength=0)=0;/**< \brief  align <b> must be implemented </b> in derived class*/

        virtual inline string getOptixClass(){return "Quadric";}/**< return the derived class name ie. Quadric */

        /** \brief computes the intercept of ray with this quadric surface in the surface local absolute frame and sets the new origin at the intercept
        *
        *   \param  ray  on input : the ray in the previous surface frame. in output the ray positionned on the surface in this surface frame
        *   \param normal adress of a vector which, if not null,  will receive the surface normal (normalized) at the intercept point
        *   \return a reference to the modified ray
        */
        virtual VectorType intercept(RayBaseType& ray, VectorType * normal=NULL);

    /**  \brief Dumps internal data to standard output */
        virtual void dumpData()
        {
            cout  <<  "m_alignedQuadric" << endl << m_alignedQuadric << endl;
            cout  <<  "m_localQuadric" << endl << m_localQuadric << endl<<endl;
            ElementBase::dumpData();
        }

    protected:
        RayType::QuadricType m_alignedQuadric;
        RayType::QuadricType m_localQuadric;
    private:
};

#endif // QUADRIC_H_INCLUDED
