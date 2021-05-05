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


/** \brief Computes the Legendre polynomials and its first derivative values for a given set of points inside the [-1,+1] range
 * \ingroup GlobalCpp
 * \param[in] Norder number of Legendre \f$ P_n \f$ returned [0 < n < Norder-1]
 * \param[in] Xpos an Array[size] of abscissas, [ -1 < Xpos < 1], where \f$ P_n \f$ and \f$ P'_n \f$ will be computed.
 * \param[out] derivative an Array to receive the derivatives \f$ P'_n \f$. It will be resized to [size, Norder]  on return
 * \return an Array of size [size, Norder], containing the \f$ P_n \f$ values
 */
 EIGEN_DEVICE_FUNC ArrayXXd Legendre(int Norder, const Ref<ArrayXd>& Xpos, ArrayXXd& derivative );

/** \brief Computes a wavefront interpolation by 2D legendre polynomials on the given aperture from  a set of aperure points and transverse aberrations
 * \ingroup GlobalCpp
 * \param Nx number of polynomials of the X basis (degree <Nx)
 * \param Ny number of polynomials of the Y basis (degree <Ny)
 * \param WFdata Transverse aberration array  stored in aperture points order. Columns are respectively
 *      X component of transverse aberration, Y component of transverse aberration, X aperture angle, Y aperture angle,
 * \param Xaperture Bounds (Min, Max) of X aperture angle for Legendre definition along X
 * \param Yaperture Bounds of Y aperture angle for Legendre definition along Y
 * \return The Nx x Ny array of coefficients of Legendre polynomials describing the wavefront error to the given degrees and best fitting  the transverse aberration data
 */
EIGEN_DEVICE_FUNC ArrayXXd LegendreIntegrateSlopes(int Nx, int Ny, const Ref<ArrayX4d>& WFdata,
                                                   const Ref<Array2d>& Xaperture, const Ref<Array2d>& Yaperture);

/** \brief Computes the value of a function interpolated by bidimensionnal Legendre polynomial on a rectangular grid.
 * \ingroup GlobalCpp
 * \param Xpos Array of the computation abscissas
 * \param Ypos Array of the computation ordinates
 * \param bounds const Array of the X and Y bounds on which the Legendre are computed. All (Xpos, Ypos) points must fall inside this rectangle
 * \param legendreCoefs A Matrix[Nx,Ny] containing the coefficients of \f$ F_{n_x},_{n_y}(x,y) = P_{n_x}(x) P_{n_y}(y) \f$
 * \return A Array with the surface interpolated values.
 *
 */ EIGEN_DEVICE_FUNC ArrayXXd LegendreSurface(const Ref<ArrayXd>& Xpos, const Ref<ArrayXd>& Ypos, const Ref<Array22d>& bounds, const Ref<MatrixXd>& legendreCoefs );

#endif // WAVEFRONT_H_INCLUDED
