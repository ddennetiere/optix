#ifndef RAY_H_INCLUDED
#define RAY_H_INCLUDED

////////////////////////////////////////////////////////////////////////////////
/**
*      \file           ray.h
*
*      \brief         Ray class declaration
*
*      \author         François Polack <francois.polack@synchroton-soleil.fr>
*      \date        2020-10-12  Creation
*      \date         Last update
*

*/
///////////////////////////////////////////////////////////////////////////////////
//
//             REVISIONS
//
////////////////////////////////////////////////////////////////////////////////////

#include "raybase.h"
#include <complex>


/** \brief  implement Raybase class and provides metadata such as photometric information
 *
 *
 */
template <typename scalar>
class Ray:public RayBase<scalar>{
public:
    typedef std::complex<double> ComplexType;

    inline Ray():m_wavelength(0), m_amplitude_S(1.), m_amplitude_P(0), m_alive(true){}    /**< \brief default constructor */

    inline Ray(RayBase<scalar>&& base, double wavelength=0, ComplexType amplitudeS=1., ComplexType amplitudeP=0) :
        RayBase<scalar>(base), m_wavelength(wavelength), m_amplitude_S(amplitudeS), m_amplitude_P(amplitudeP),m_alive(true){} /**< \brief
    *       constructor from a RayvBase object of same type and metaparameters */

    template<typename otherScalar>
    inline Ray(RayBase<otherScalar>&& base, double wavelength=0, ComplexType amplitudeS=1., ComplexType amplitudeP=0) :
        RayBase<scalar>(base), m_wavelength(wavelength), m_amplitude_S(amplitudeS), m_amplitude_P(amplitudeP),m_alive(true){} /**<  \brief
    *   type conversion constructor with meta parameters    */

    template<typename otherScalar>
    inline Ray(Ray<otherScalar> & ray) :
        RayBase<scalar>(ray), m_wavelength(ray.m_wavelength), m_amplitude_S(ray.m_amplitude_S),
                m_amplitude_P(ray.m_amplitude_P), m_alive(ray.m_alive){} /**<  \brief
    *   copyc constructor with type conversion */


    virtual ~Ray(){}    /**< \brief virtual destructor */

     /** \brief save the ray into a binary output stream
     */
     friend std::fstream & operator << (std::fstream & s, const Ray& ray)
     {
         s << (RayBase<scalar>)ray;
         s.write( (char*)&ray.m_wavelength, datasize);
         return s;
     }

     /** \brief retrieves a ray from a binary input stream
     */
     friend std::fstream & operator >> (std::fstream & s, const Ray& ray)
     {
         s >> (RayBase<scalar>)ray;
         s.read((char*) &ray.m_wavelength, datasize);
         return s;
     }


// data :  // ne pas modifier  sans reviser les opérateurs >> et  <<
    static const  int datasize=5*sizeof(double)+sizeof(bool);
    double m_wavelength;
    ComplexType m_amplitude_S, m_amplitude_P; // en attente d'une implémentation possible
    bool m_alive;
};

#endif // RAY_H_INCLUDED
