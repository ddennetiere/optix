#ifndef WAVEFRONT_H_INCLUDED
#define WAVEFRONT_H_INCLUDED

////////////////////////////////////////////////////////////////////////////////
/**
*      \file           wavefront.h
*
*      \brief         TODO  fill in file purpose
*
*      \author         François Polack <francois.polack@synchroton-soleil.fr>
*      \date        2020-11-29  Creation
*      \date         Last update
*

*/
///////////////////////////////////////////////////////////////////////////////////
//
//             REVISIONS
//
////////////////////////////////////////////////////////////////////////////////////
#include <Eigen/Eigen>
#include <iostream>

using namespace Eigen;

EIGEN_DEVICE_FUNC ArrayXXd Legendre(int Norder, const Ref<ArrayXd>& Xpos, ArrayXXd& derivative );

/** \brief Computes a wavefront interpolation by 2D legendre polynomials on the given aperture from  a set of aperure points and transverse aberrations
 *
 * \param Nx number of polynomials of the X basis (degree <Nx)
 * \param Ny number of polynomials of the Y basis (degree <Ny)
 * \param WFdata Transversa aberration array  stored in aperture points order. Columns are respectively
 *      X component of transverse aberration, Y component of transverse aberration, X aperture angle, Y aperture angle,
 * \param Xaperture Bounds (Min, Max) of X aperture angle for Legendre definition along X
 * \param Yaperture Bounds of Y aperture angle for Legendre definition along Y
 * \return The Nx x Ny array of coefficients of Legendre polynomials describing the wavefront error to the given degrees and best fitting  the transverse aberration data
 */
EIGEN_DEVICE_FUNC ArrayXXd LegendreIntegrateSlopes(int Nx, int Ny, const Ref<ArrayX4d>& WFdata,
                                                   const Ref<Array2d>& Xaperture, const Ref<Array2d>& Yaperture);

EIGEN_DEVICE_FUNC ArrayXXd LegendreSurface(const Ref<ArrayXd>& Xpos, const Ref<ArrayXd>& Ypos, const Ref<Array22d>& bounds, const Ref<MatrixXd>& legendreCoefs );

#endif // WAVEFRONT_H_INCLUDED
