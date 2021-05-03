#ifndef HEADER_1FE2B1E63800AF25
#define HEADER_1FE2B1E63800AF25

////////////////////////////////////////////////////////////////////////////////
/**
*      \file           gratingbase.h
*
*      \brief         GratingBase and Pattern class declarations
*
*      GratingBase defines alignment and propagation function common to optical elements
*   \n Pattern is a virtual class defining the GratingVector() function and is derived in HOlo and Poly1D classes according to the ruling type
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
 *
 * This class  provides the gratingVector() function which need to be overridden by actual implementation classes
 *
 */
class Pattern
{
    public:
        /** Default constructor */
        Pattern(){}
        /** Default destructor */
        virtual ~Pattern(){}

        /** Line density vector in  grating coordinates
        *       \todo should be made pure virtual when derived classes are setup */
        EIGEN_DEVICE_FUNC virtual Surface::VectorType gratingVector(Surface::VectorType position,
                                Surface::VectorType normal=Surface::VectorType::UnitZ());
};

/** \brief Base class for all gratings
 *
 *     Parameters defined  by the GratingBase class
 *     -----------------------------------------
 *
 *   Name of parameter | UnitType | Description
 *   ----------------- | -------- | --------------
 *   \b order_align | Dimensionless | The alignment order
 *   \b order_use   | Dimensionless | The utilization order (normally same as order_align)
 *
 * these parameters belong to the GratingGroup and cannot be optimized (flags set to 1)
 */
class GratingBase :  virtual public Surface, virtual public Pattern
{
    public:
        /** \brief constructor overrides Surface constructor and sets the diffraction order parameters*/
        GratingBase(bool transparent=false, string name="" ,Surface * previous=NULL);

        /** \brief Default destructor */
        virtual ~GratingBase();
        virtual  inline string getOptixClass(){return "GratingBase";}/**< return the derived class name ie. GratingBase */


        /** \brief Orients the grating on the given order and wavelength and defines the related geometric space transforms
        * \param alWavelength  the \b alignment \b wavelength (m)
        *  \n If reflective:
        *   sets the grating angle Omega and the rotation angle Psi
        *   so that the exit direction of the main ray is satisfied
        *  \n If transmissive: the alignment axis is not changed */
        int setFrameTransforms(double alWavelength);

        virtual RayType& transmit(RayType& ray);       /**< \brief Implementation of transmission grating  */

        virtual RayType& reflect(RayType& ray);    /**<  \brief Implementation of reflexion grating  */



    protected:
        int m_alignmentOrder;/**< \brief order to be used for aligning*/
        int m_useOrder;/**< \brief order to be used for propagation */
        IsometryType psiTransform;/**< \brief transform from surface space to propagation space
            *   this transform combines the psi rotation around the surface Z axis and the axis permutation between surface and propagation space representations */
    private:
};

#endif // GRATINGBASE_H
#endif // header guard

