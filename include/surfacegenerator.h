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

*   \date               Last update: 2024-05-23

*
*
 ***************************************************************************/


#include "fractalsurface.h"
//#include "EigenSafeInclude.h" Included from fractalsurface.h
//#include <iostream>

#include "wavefront.h"
#include <libxml/tree.h>

using namespace Eigen;
class Surface; // limited forward declaration avoiding reentrant declarations in headers

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
        SurfaceErrorGenerator(Surface* parent=NULL)
        {
            m_limits.setZero();
            m_sampling.setZero();
            m_detrendMask.resize(2,2);
            m_detrendMask << 1.,1.,1.,0.;
            parentSurface=parent;
        }


        virtual ~SurfaceErrorGenerator(){}/**< \brief default destructor */

//         inline void setSurfaceSampling(const Ref<MatrixXd> &limits , double ymax, double ystep)
//        {
//            m_limits << xmin, ymin, xmax, ymax;
//            m_sampling << xstep,ystep;
//        }
//

        /** \brief  defines the limits of the surface and the sampling pitchs of the
         * \param xmin Low X limit of the generated surface
         * \param xmax High X limit of the generated surface
         * \param xstep X interval between 2 points [m unit]
         * \param ymin Low Y limit of the generated surface
         * \param ymax High Y limit of the generated surface
         * \param ystep Y interval between 2 points [m unit]
         */
        inline void setSurfaceSampling(double xmin, double xmax, double xstep, double ymin, double ymax, double ystep)
        {
            m_limits << xmin, ymin, xmax, ymax;
            m_sampling << xstep,ystep;
        }

        /** \brief returns limits of the surface error map and the requested sampling steps
         *
         * \param xstep location to return the approximate x pitch (the actual one is adjusted to make an interger number of steps in the x interval)
         * \param ystep location to return the approximate y pitch (the actual one is adjusted to make an interger number of steps in the y interval)
         * \return the limits the aperture limits into which the surface is defined. \(mins in the first row and maxs in the second; X in first column and Y in the second)
         */
        inline const Array22d& getSurfaceSampling(double* xstep=NULL, double* ystep=NULL)
        {
            if(xstep)
                *xstep=m_sampling(0);
            if(ystep)
                *ystep=m_sampling(1);
            return m_limits;
        }

        /** \brief Set the fractal parameter of the PSD in the X or Y direction
         *
         * \param axe string "X" or "Y" specifying the axe to set
         * \param N the number of frequency segments in the log/log PSD curve
         * \param exponents The array of N fractal exponents
         * \param frequencies the array of N-1 transition frequencies \f$ [in m^{-1}] \f$
         * \throw an instance of ParameterException if axe name is invalid or an instance of Parameter warning if one of the exponents is >0
         *
         */
        inline void setFractalParameters(const char* axe, const int N, const double *exponents, const double *frequencies)
        {
            m_fractalSurf.setXYfractalParams(axe, N, exponents, frequencies);
        }

        /** \brief Retrieves the fractal parameters of the surface error generator
         *
         * \return the fractal parameters in a FractaParameter struct
         */
        inline const FractalParameters& getFractalParameters(){return m_fractalSurf.fracParms;}

        /** \brief Defines the Legendre polynomials which will be  forced to zero
         *
         * \param detrend a mask array. If the value of coefficient (n,m) is not zero, the corresponding Legendre polynomials (n,m) forced to zero
         *  if a non initialized matrix is passed, detrending will be inhibited.
         */
        inline void setDetrending(const ArrayXXd& detrend)
        {
            m_detrendMask=detrend.unaryExpr([](double x){return x==0? 0. : 1.;});
        }

        /** \brief Retrieves the mask defining the Legendre polynomials which are  forced to zero
         *
         * \return A the detrending mask as an Eigen::Array
         *
         */
        inline const ArrayXXd& getDetrending() {return m_detrendMask;}

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

        /** \brief retrieve the matrix of maximum sigma values of the random Legendre polynomials defining the low frequency part
         *
         * \param[in,out] nonZ  location to return the sigma of the high frequency part not defined by legendre polynomials
         * \return ca reference to the array of Legendre maximum sigma values
         *
         */
        inline const ArrayXXd& getSigmas(double * nonZ)
        {
            if(nonZ)
                *nonZ=m_nonZsigma;
            return Legendre_ubound;
        }

        /** \brief Generate a surface model with the statically defined parameters
         *
         * \return  an array containing the height of the surface model [m unit]
         * \throw an instance of ParameterException if the generator parameters are not or improperly set
         */
        ArrayXXd generate( );

        void operator>>(xmlNodePtr surfnode);
        void operator<<(xmlNodePtr generatornode);

    protected:

        Surface *parentSurface=NULL;
        Array22d m_limits; /**< \brief limits the aperture limits into which the surface is defined. \(mins in the first row and maxs in the second; X in first column and Y in the second)
         *
         *  - \f$   limits =  \left[ {\begin{array}{cc}     x_{min} & y_{min} \\     x_{max} & y_{max} \\   \end{array} } \right]  \f$ */
        Array2d m_sampling; /**< \brief target X and Y sampling steps. They will be adjusted to match an integer number of grid points */
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


