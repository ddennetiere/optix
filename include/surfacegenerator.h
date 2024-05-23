#ifndef ERRORGENERATOR_H
#define ERRORGENERATOR_H

/**
*************************************************************************
*   \file       surfacegenerator.h

*
*   \brief     definition file
*
*
*
*
*   \author             François Polack  <francois.polack@synchroton-soleil.fr>
*   \date               Creation : 2024-05-21

*   \date               Last update: 2024-05-21

*
*
 ***************************************************************************/

#include "fractalsurface.h"
//#include "EigenSafeInclude.h" Included from fractalsurface.h
//#include <iostream>

#include "wavefront.h"

using namespace Eigen;

/** \brief This class generate random surface errors matching given statistical properties
 */
class SurfaceErrorGenerator
{
    public:
        /** \brief Defines a surface height error generator with default initialization
         * It allows parametrized detrending and  specification of  particular Rectangular Zernike content
         *
         * Presently only a fractal surfaces model is  available for the high frequency part.
         * default constructor set the X & Y fractal model as uniform in frequency with a PSD fractal exponent of -1.
         * it also define 1 and 0 order detrending
         */
        SurfaceErrorGenerator()
        {
            m_detrendMask.resize(2,2);
            m_detrendMask << 1.,1.,1.,0.;
        }


        virtual ~SurfaceErrorGenerator(){}/**< \brief default destructor */


        /** \brief Set the fractal parameter of the PSD in the X or Y direction
         *
         * \param axe string "X" or "Y" specifying the axe to set
         * \param N the number of frequency segments in the log/log PSD curve
         * \param exponents The array of N fractal exponents
         * \param frequencies the array of N-1 transition frequencies
         * \throw an instance of ParameterException if axe name is invalid or an instance of Parameter warning if one of the exponents is >0
         *
         */
        inline void setFractalParameters(const char* axe, const int N, const double *exponents, const double *frequencies)
        {
            m_fractalSurf.setXYfractalParams(axe, N, exponents, frequencies);
        }

        /** \brief Defines the Legendre polynomials which will be  forced to zero
         *
         * \param detrend a mask array. If the value of coefficient (n,m) is not zero, the corresponding Legendre polynomials (n,m) forced to zero
         *  if a non initialized matrix is passed, detrending will be inhibited.
         */
        inline void setDetrending(const ArrayXXd& detrend)
        {
            m_detrendMask=detrend.unaryExpr([](double x){return x==0? 0. : 1.;});
        }

        /** \brief defines,first which Legendre Polynomials will be randomly set and the maximum sigma value they will be given ;
         * second, the  height error sigma of the remaining surface components.
         *
         * \param nonZsigma [unit is m] The contribution of non constrained Legendre polynomials to the RMS height errors. Non constrained polynomials are those which are not defined by setDetrendin and Zmax.
         * \param Zmax [unit is m] Reference of an array or matrix defining the Legendre polynomials which will be randomly defined and the maximum contribution of each one to the surface height error sigma.
         */
        inline void setSigmas(double nonZsigma, const Ref<ArrayXXd>& Zmax)
        {
            m_nonZsigma=nonZsigma;
            Legendre_ubound=LegendreFromNormal(Zmax);
        }

        /** \brief Generate a surface model with the statically defined parameters
         *
         * \param xpoints number of X points defining the surface model
         * \param xstep X interval between 2 points [m unit]
         * \param ypoints  number of Y points defining the surface model
         * \param ystep X interval between 2 points [m unit]
         * \return  and array containing the height of the surface model [m unit]
         */
        ArrayXXd generate(int32_t xpoints, double xstep, int32_t ypoints, double ystep);


    protected:

        double m_nonZsigma=0; /**< \brief rms height target value of the generated surface constrained Zernike terms excluded */
        FractalSurface m_fractalSurf; /**< \brief The fractal surface generator. (presently, the only implemented generator) */
        ArrayXXd  m_detrendMask; /**< \a mask for detrending. Legendre polynomials corresponding to non zero values will be fit and subtracted.  */

        /** \brief the maximum rms contribution to the height errors of the Legendre polynomials allowed to be randomly defined.
         *
         *  Where non-zero: the value is used as the upper limit of rms contribution of the corresponding polynomial
         *  Where zero, the rms contribution of the corresponding polynomial is not limited.  \warning coeff=0 DOES NOT MEAN upper limit=0
         */
        ArrayXXd  Legendre_ubound;

    private:
};

#endif // ERRORGENERATOR_H


