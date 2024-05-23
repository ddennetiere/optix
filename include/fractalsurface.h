#ifndef FRACTALSURFACE_H
#define FRACTALSURFACE_H

/**
*************************************************************************
*   \file       fractalsurface.h

*
*   \brief     auxiliary class and functions to generate random surfaces with prescribed fractal statistics
*
*
*
*
*   \author             François Polack  <francois.polack@synchroton-soleil.fr>
*   \date               Creation : 2024-04-29

*   \date               Last update: 2024-04-29

*
*
 ***************************************************************************/
#include <vector>
#include <cstdio>
#include <fstream>

#include "ctypes.h"  // this includes Eigen and iostream

using Eigen::ArrayXXd, Eigen::Ref, Eigen::VectorXd;

/** \brief class for generating fractal surfaces
 *  \warning
 *  The gaussian generator uses  \b std::random_device \b as a source of random number.
 *  It might not be available on non Windows system and could be replaced by \b default_random_engine \b
 */
class FractalSurface
{
    public:
        /** \brief Construct a new fractalSurface object with default parameters.
         *
         * Default fractal PSDs have only one segment and a fractal exponent of -1 both in X and Y direction
         */
        FractalSurface();

        virtual ~FractalSurface();

        inline void setFractalParameters(const FractalParameters &fracparams)
        {
            setXYfractalParams("X",fracparams.nx, fracparams.exponent_x, fracparams.frequency_x);
            setXYfractalParams("Y",fracparams.ny, fracparams.exponent_y, fracparams.frequency_y);
        }
        FractalParameters getFractalParameters()
        {
            return fracParms;
        }

        /** \brief Set the fractal parameter of the PSD in the X or Y direction
         *
         * \param axe string "X" or "Y" specifying the axe to set
         * \param N the number of frequency segments in the log/log PSD curve
         * \param exponents The array of N fractal exponents
         * \param frequencies the array of N-1 transition frequencies
         * \throw an instance of ParameterException if axe name is invalid or an instance of Parameter warning if one of the exponents is >0
         *
         */
        void setXYfractalParams(const char* axe, const int N, const double *exponents, const double *frequencies);

        /** \brief Generate a tabulated random surface with definite parameters
         *
         * \param xSize number of surface points to generate in the X direction
         * \param xStep tabulation step in the X direction
         * \param ySize number of surface points to generate in the Y direction
         * \param yStep tabulation step in the Y direction
         */
        ArrayXXd generate(int32_t xSize, double xStep, int32_t ySize, double yStep);

        /** \brief Detrend the given surface array according to a mask
         *
         * \param[in,out] surface the data array. it will be detrended in place.
         * \param[in] mask A mask of 0 and non-zero values. Legendre polynomial fits corresponding to non-zero values will be removed from surface
         * \return an array of same size as mask  containing the Legendre natural  coefficients of the fit after detrending (ie must be 0 where mask is non-0)
         */
        ArrayXXd detrend(ArrayXXd& surface, const Ref<ArrayXXd>& mask);



      //  Eigen::ArrayXXd m_surface;

    protected:

        inline int span(double x)
        {
            if (x <= prim23[0]) return prim23[0];
            if( x > *prim23.rbegin()) return pow(2,(int)ceil(log(x)/log(2.)));
            auto pos = std::lower_bound(prim23.begin(), prim23.end(), x);
            //return  std::distance(prim23.begin(), pos) ;
            return *pos;
        }

        /** \brief Define the Fourier filter needed to convert random gaussian variable to the requested fractal law
         *
         * \param N number of points in real as in Fourier space
         * \param dstep tabulation step in real space
         * \param nseg number of segments of the fractal law
         * \param exponent the array of exponent of the fractal law (nseg  values)
         * \param ftrans the array of transition frequencies (nseg-1 values, unuses if nseg=1)
         * \return the filter vector to be applied in Fourier space
         */
        VectorXd frequencyFilter(int N, double dstep, int nseg, double * exponent, double *ftrans);
        FractalParameters fracParms;   /**< vector of transition frequencies if size  */

    private:
        const static std::vector<int> prim23;

};

#endif // FRACTALSURFACE_H


