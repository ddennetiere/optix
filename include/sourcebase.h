#ifndef SOURCEBASE_H
#define SOURCEBASE_H

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


#include "Surface.h"


/** \brief Implement the basic mechanisms for a source object
 *
 *  The class namely defines a generate() function which initiates the congruence of rays to be propagated
 */
class SourceBase : public virtual Surface
{
    public:
        /** \brief Default constructor. In derived class constructors, the constructor of Surface must be called with transparent=true  */
        SourceBase();
        /** Default destructor */
        virtual ~SourceBase();

        /** \brief  generates rays in the impact list
         *
         * \param wavelength the radiation wavelength of the generated congruence of rays
         * \param polar The polarization of the generated rays - S (along X), P (along Y), R (right circular), L (left circular) are allowed
         * \return  the number of rays generated by this call (can differ from the number of rays accumulated in impacts)
         *
         *  Generate() can be called several times with different wavelengths since each ray carries its wavelength.
         *  Radiate() will propagate all ray impacts accumulated in the source since the last clearImpacts() call. */
        virtual int generate(const double wavelength,const char polar='S')=0;    // pure virtual {cout <<"errorBase class\n";return -1;};  /**<  \todo should be made pure virtual later*/

        /** \brief implements the required pure virtual intercept() function of the base class,
         *  so that the element is completely transparent
         *
         * \param ray  the ray in previous space if there is one
         * \param normal  The Unit Z vector will be always return
         * \return Unchanged ray position but expressed in this exit space
         */
        virtual VectorType intercept(RayBaseType& ray, VectorType *normal=NULL)
        {
            ray-=m_translationFromPrevious; // change ref fram from previous to this surface  ray is not rebased
            if(ray.m_alive)
            {
                if(normal)
                    *normal=VectorType::UnitZ();  // le vect unitaire sur OZ
            }
            return ray.position();
        }

        /** \brief required implementation of align function
         *
         * \param wavelength double
         * \return always 0 (success) for non gratings
         * \todo define sources which do not radiate along Z axis
         */
        inline int align(double wavelength)
        {
            return setFrameTransforms(wavelength);  // this call only defines the space transforms
        }

        /** \brief  change the wavelength of all the generated rays stored in inpacts
         *
         * \param wavelength the new wavelength for all stored rays
         */
        inline void setWavelength(double wavelength)
        {
            vector<RayType>::iterator it;
            for(it=m_impacts.begin(); it!= m_impacts.end(); ++it)
                it->m_wavelength=wavelength;
        }

        /** \brief propagate all generated rays stored in impacts
         * \todo This function need to be parallelized. it would need to create system clones fro thread safety
         */
        inline int radiate()
        {
            int losses=0;
            if(m_next==0)
                return 0;
            vector<RayType>::iterator it;
            RayType propRay;
            for(it=m_impacts.begin(); it != m_impacts.end(); ++it)
            {
                m_next->propagate(propRay=*it);   // on propage une cope de *it. La propagation modifie directement le rayon propagé
                if(! propRay.m_alive)
                    ++losses;
            }
            return losses;
        }

        /** \brief  Start a specific ray tracing for Wavefront extraction and PSF computation
         *
         * \param wavelength The wavelength to radiate <b> in m </b>
         * \param Xaperture 1/2 NA aperture in the X direction (range [0,1])
         * \param Yaperture 1/2 NA aperture in the Y direction (range [0,1])
         * \param Xsize total number of grid points in the X dimension (odd number suggested)
         * \param Ysize otal number of grid points in the Y dimension (odd number suggested)
         * \param polar The type of polarization. Can be 'S' (default) polarized along X or 'P' polarized along Y or circular R' or 'L'
         */
        virtual void waveRadiate(double wavelength, double Xaperture, double Yaperture, size_t Xsize, size_t Ysize, char polar='S'); // will be specialized for computed source like undulators

    protected:

    private:
};


#endif // SOURCEBASE_H


