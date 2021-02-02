#ifndef HEADER_1FE2B1E63800AF25
#define HEADER_1FE2B1E63800AF25

////////////////////////////////////////////////////////////////////////////////
/**
*      \file           gratingbase.h
*
*      \brief         GratingBase class declarations
*      \todo         Derived classes RuledGrating and HoloGrating vith specific implementation of (should be) virtual function gratingVector()
*
*      \author         François Polack <francois.polack@synchroton-soleil.fr>
*      \date         2020-10-22  Creation
*      \date         Last update 2020-10-30
*

*/
///////////////////////////////////////////////////////////////////////////////////
//
//             REVISIONS
//
////////////////////////////////////////////////////////////////////////////////////
#ifndef GRATINGBASE_H
#define GRATINGBASE_H

#include "surface.h"


/** \brief Base class for all gratings
 */
class GratingBase : virtual public Surface
{
    public:
        /** Default constructor */
        GratingBase();
        /** Default destructor */
        virtual ~GratingBase();
        virtual  inline string getRuntimeClass(){return "GratingBase";}/**< return the derived class name ie. GratingBase */

        /** Line density vector in  grating coorinates
        *       \todo should be made pure virtual*/
        EIGEN_DEVICE_FUNC virtual VectorType gratingVector(VectorType position, VectorType normal=VectorType::UnitZ());

        /** \brief Orients the grating on the given order and wavelength and defines the related geometric space transforms
        * \param wavelength  the alignment wavelength (m)
        * If reflective:
        *   sets the grating angle Omega and the rotation angle Psi
        *   so that the exit direction ofthe main ray is satisfied
        *   if transmissive the alignment axis is not changed */
        virtual int align(double wavelength);

        virtual RayType& transmit(RayType& ray);       /**< \brief Implementation of transmission grating  */

        virtual RayType& reflect(RayType& ray);    /**<  \brief Implementation of reflexion grating  */

    protected:
        int m_order;
        double m_density; /**< \todo à évaluer : implementation de base d'un réseau uniforme   */
        IsometryType psiTransform;
    private:
};

#endif // GRATINGBASE_H
#endif // header guard

