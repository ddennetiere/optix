////////////////////////////////////////////////////////////////////////////////
/**
*      \file           grating.h
*
*      \brief         Grating class declarations
*
*      \author         François Polack <francois.polack@synchroton-soleil.fr>
*      \date        2020-10-22  Creation
*      \date         Last update
*

*/
///////////////////////////////////////////////////////////////////////////////////
//
//             REVISIONS
//
////////////////////////////////////////////////////////////////////////////////////
#ifndef GRATING_H
#define GRATING_H

#include "surface.h"


/** \brief Base class for all gratings
 */
class Grating : virtual public Surface
{
    public:
        /** Default constructor */
        Grating();
        /** Default destructor */
        virtual ~Grating();
        virtual inline string getRuntimeClass(){return "Grating";}/**< return the derived class name ie. Grating */
        EIGEN_DEVICE_FUNC virtual VectorType gratingVector(VectorType position);/**< Line density vector in  grating coordinates */
        virtual int align(double wavelength);/**< \brief Orients the grating on the given order and wavelength and defines the related geometric space transforms
                * \param wavelength  the alignment wavelength (m)
                * If reflective:
                *   sets the grating angle Omega and the rotation angle Psi
                *   so that the exit direction ofthe main ray is satisfied
                *   if transmissive the alignment axis is not changed */
    virtual RayType& transmit(RayType& ray);       /**< \brief Implementation of transmission grating  */

    virtual RayType& reflect(RayType& ray);    /**<  \brief Implementation of reflexion grating  */

    protected:
        int m_order;
        double m_density; /**< \todo à évaluer : implementation de base d'un réseau uniforme   */
    private:
};

#endif // GRATING_H
