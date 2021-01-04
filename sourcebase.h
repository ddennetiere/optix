////////////////////////////////////////////////////////////////////////////////
/**
*      \file           sourcebase.h
*
*      \brief         Base class of sources
*
*      \author         François Polack <francois.polack@synchroton-soleil.fr>
*      \date        2020-11-01  Creation
*      \date         Last update
*

*/
///////////////////////////////////////////////////////////////////////////////////
//
//             REVISIONS
//
////////////////////////////////////////////////////////////////////////////////////
#ifndef SOURCEBASE_H
#define SOURCEBASE_H

#include "Surface.h"


class SourceBase : public virtual Surface
{
    public:
        /** \brief Default constructor le constructeur de Surface doit être appelé dans les classes dérivée avec transparent=true */
        SourceBase();
        /** Default destructor */
        virtual ~SourceBase();
        virtual int generate(double wavelength){return 0;}  /**< \brief generates ray in the impact list
                    *   \return the number of generated rays;  ray#0 is always the chief ray \todo should be made pure virtual later*/

        /** \brief implements the required pure virtual intercept() function of the base class,
         *  so that the element is completely transparent
         *
         * \param ray  the ray in previous space if there is one
         * \param normal  The Unit Z vector will be always return
         * \return Unchanged ray position but expressed in this exit space
         */
        virtual EIGEN_DEVICE_FUNC VectorType intercept(RayType& ray, VectorType *normal=NULL)
        {
            ray-=m_translationFromPrevious; // change ref fram from previous to this surface
            if(ray.m_alive)
            {
                if(normal)
                    *normal=VectorType::UnitZ();  // le vect unitaire sur OZ
            }
            return ray.position();
        }

        /** \brief  change the wavelength ao all the generated rays stored in inpacts
         *
         * \param wavelength the new wavelength for all stored rays
         */
        inline void setWavelength(double wavelength)
        {
            vector<RayType>::iterator it;
            for(it=m_impacts.begin(); it!= m_impacts.end(); ++it)
                it->m_wavelength=wavelength;
        }

        inline void radiate()
        {
            vector<RayType>::iterator it;
            for(it=m_impacts.begin(); it != m_impacts.end(); ++it)
                m_next->propagate(*it);
        }
    protected:

    private:
};


#endif // SOURCEBASE_H
