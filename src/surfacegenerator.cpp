/**
 *************************************************************************
*   \file           errorgenerator.cpp
*
*   \brief             implementation file
*
*
*
*   \author             François Polack  <francois.polack@synchroton-soleil.fr>
*   \date               Creation: 2024-05-21
*   \date               Last update: 2024-05-21
 ***************************************************************************/


#include "surfacegenerator.h"

ArrayXXd SurfaceErrorGenerator::generate(int32_t xpoints, double xstep, int32_t ypoints, double ystep)
{
    if (m_nonZsigma <=0)
        throw ParameterException(string("The surface error generator is not properly initialized, at "),
                                            __FILE__, __func__, __LINE__);

    int Nx=Legendre_ubound.rows() > m_detrendMask.rows() ? Legendre_ubound.rows() : m_detrendMask.rows();
    int Ny=Legendre_ubound.cols() > m_detrendMask.cols() ? Legendre_ubound.cols() : m_detrendMask.cols();
        if (Nx >= xpoints || Ny >= ypoints )
            throw ParameterException(string("Bad dimensions: the internal surface size doesn't allow the requested Legendre fit, in "),
                                            __FILE__, __func__, __LINE__);

    ArrayXXd surface=m_fractalSurf.generate(xpoints, xstep, ypoints, ystep);

    if(Nx*Ny==0)
        return surface;

    ArrayXXd mask=ArrayXXd::Zero(Nx,Ny);
    if(Legendre_ubound.size())
        mask.topLeftCorner(Legendre_ubound.rows(), Legendre_ubound.cols())=Legendre_ubound;
    if(m_detrendMask.size())
        mask.topLeftCorner(m_detrendMask.rows(),m_detrendMask.cols())+=m_detrendMask;

    m_fractalSurf.detrend(surface,m_detrendMask);
    //scale the detrended surface to match the given sigma value
    surface *= m_nonZsigma/surface.matrix().norm();

    if(Legendre_ubound.size())
    {
        MatrixXd  Lcoeffs= Legendre_ubound * ArrayXXd::Random(Legendre_ubound.rows(), Legendre_ubound.cols());
        surface+= LegendreSurfaceGrid(surface.rows(), surface.cols(), Lcoeffs);
    }
    return surface;
}

