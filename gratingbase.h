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
#include "OptixException.h"

/** \brief Describes the grating line spacing
 * This class only provides the gratingVector() function which need to be overriden by actual implementation classes
 */
class Pattern
{
        /** Default constructor */
        Pattern(){}
        /** Default destructor */
        virtual ~Pattern(){}
    public:
        /** Line density vector in  grating coordinates
        *       \todo should be made pure virtual when derived classes are setup */
        EIGEN_DEVICE_FUNC virtual Surface::VectorType gratingVector(Surface::VectorType position,
                                Surface::VectorType normal=Surface::VectorType::UnitZ());
};

/** \brief Base class for all gratings
 */
class GratingBase : virtual public Surface, virtual public Pattern
{
    public:
        /** Default constructor */
        GratingBase();
        /** Default destructor */
        virtual ~GratingBase();
        virtual  inline string getRuntimeClass(){return "GratingBase";}/**< return the derived class name ie. GratingBase */


        /** \brief Orients the grating on the given order and wavelength and defines the related geometric space transforms
        * \param wavelength  the \b alignment \b wavelength (m)
        *  \n If reflective:
        *   sets the grating angle Omega and the rotation angle Psi
        *   so that the exit direction of the main ray is satisfied
        *  \n If transmissive: the alignment axis is not changed */
        int setFrameTransforms(double wavelength);

        virtual RayType& transmit(RayType& ray);       /**< \brief Implementation of transmission grating  */

        virtual RayType& reflect(RayType& ray);    /**<  \brief Implementation of reflexion grating  */

    protected:
        int m_alignmentOrder;
        int m_useOrder;
        IsometryType psiTransform;/**< transformation incluant la rotation psi autour de la normale et le passage de la représentation surface à la représentation espace */
    private:
};

#endif // GRATINGBASE_H
#endif // header guard

