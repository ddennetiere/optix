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

/** \brief class for generating fractal surfaces
 *  \warning
 *  The gaussian generator uses  \b std::random_device \b as a source of random number.
 *  It might not be available on non Windows system and could be replaced by \b default_random_engine \b
 */
class FractalSurface
{
    public:
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

        /** \brief
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
        void generate(int32_t xSize, double xStep, int32_t ySize, double yStep);

        Eigen::ArrayXXd detrend(const Eigen::Ref<Eigen::ArrayXXd>& mask);

        /** \brief Dump the 2D array m_surface to a binary file
         *
         *  The file contains first the dimensions of the array (Nx, Ny) as two int32_t  integers,
         *  immediately followed by the array values in doubles, the X dimension varying the fastest
         * \param filename Name or path of the file. If the file exists, it will be overwrited
         *  \throw an instance of runtime error if the file is inaccessible in write mode
         *
         */
        void toFile(string filename)
        {
            std::cout << m_surface.rows() << " x " << m_surface.cols() << std::endl;
            std::cout << "sigma=" << m_surface.matrix().norm()/sqrt(m_surface.rows()*m_surface.cols()) << std::endl;
            int32_t n[]={(int32_t) m_surface.rows(), (int32_t) m_surface.cols()};

            std::fstream file(filename, std::ios::binary |std::ios::out);
            if(!file.is_open())
                throw runtime_error("file is locked by another application");

            file.write((char*) n, 2*sizeof(int32_t));
            file.write((char*)m_surface.data(), n[0]*n[1]*sizeof(double));
            file.close();
        }

        Eigen::ArrayXXd m_surface;

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
        Eigen::VectorXd frequencyFilter(int N, double dstep, int nseg, double * exponent, double *ftrans);
        FractalParameters fracParms;   /**< vector of transition frequencies if size  */

    private:
        const static std::vector<int> prim23;

};

#endif // FRACTALSURFACE_H


