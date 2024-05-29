#ifndef BIDIMSPLINE_H
#define BIDIMSPLINE_H

/**
*************************************************************************
*   \file       bidimspline.h

*
*   \brief     definition file
*
*
*
*
*   \author             François Polack  <francois.polack@synchroton-soleil.fr>
*   \date               Creation : 2023-06-30

*   \date               Last update: 2023-06-30

*
*
 ***************************************************************************/

#define USE_SPARSE_ALGEBRA

#include "EigenSafeInclude.h"
#include <iostream>
#include <stdexcept>

#ifdef USE_SPARSE_ALGEBRA
#include <Eigen/SparseCore>
#include <Eigen/SparseLU>
#endif // USE_SPARSE_ALGEBRA

#include "ctypes.h"

//  uncomment to print debug lines
//#define DEBUG_SPLINE_

using namespace Eigen;

enum Axis {
    X=0,
    Y
};


/** \brief This class interpolates a surface on a rectangular area with the tensorial product of 1D cubic-Bsplines
 */
class BidimSpline
{
    public:
        /** \brief Constructs a BicubSpline interpolator of given degree
         *
         * \param _degree The degree of the spline interpolating basis function (usually 3)
         */
        BidimSpline(int _degree=3):degree(_degree){}
        /** \brief Copy constructor         */
        BidimSpline(const BidimSpline  &srcSpline):degree(srcSpline.degree),
                    muX(srcSpline.muX), muY(srcSpline.muY), m_controlValues(srcSpline.m_controlValues) {}
        virtual ~BidimSpline(){}

        inline void clearData()
        {
            muX=muY=ArrayXd();
            m_controlValues=MatrixXd();
        }

        /** \brief check the validity of the spline interpolant at the given position
         *
         * \param position the position (x, y) to check
         * \return true if the spline interpolant is valid and position is inside the definirion rectangle
         *  \todo could be inlined
         */
        bool isValid(const Vector2d & position);

        /** \brief Defines the 2D spline interpolator of a surface known on evenly spaced control points
         *
         * \param limits the aperture limits into which the surface is defined. \(mins in the first row and maxs in the second; X in first column and Y in the second)
         *
         *  - \f$   limits =  \left[ {\begin{array}{cc}     x_{min} & y_{min} \\     x_{max} & y_{max} \\   \end{array} } \right]  \f$
         * \param gridData Array of the surface points in an evenly spaced grid inside the rectangle defined by limits
         *
         */
        void setFromGridData(const Array22d& limits, const Ref<ArrayXXd> & gridData );

        Array22d getLimits();

        /** \brief Find the index of the  X knot interval where the given control value u lies
         *
         * \param[in] axe the axe X or Y along which the input value must be located
         * \param[in] u double The control value which is search for in the X knot vector
         * \return int The interval index where u was found
         *
         */
        inline int span(Axis axe, double u)
        {
            ArrayXd &mu=(axe==X) ? muX :muY;
            if (u <= mu(0)) return degree;
            double* pos = std::upper_bound(mu.data()+degree-1, mu.data()+mu.size()-degree, u);
            return  std::distance(mu.data(), pos) - 1 ;
        }

        /** \brief  Fast computation of the non zero basis functions along one axis at the control value u and of the first derivative if requested.
         *
         *   This function use de BOOR algorithm and is about 2 times faster than the standard Eigen implementation
         *
         * \param[in] axe the axe X or Y along which the splineFunction values are computed
         * \param[in] u  The control value
         * \param[out] C  Matrix reference for returning the computed values
         *    \n   on return \b C  is  set either to a one column Matrix  containing the non zero basis function values if deriv=false
         *    \n  either to a Matrix of dimension \p [degree+1,2] containing fonction values in col(0) andthe 1st derivative in col(1)
         * \warning this parameter must be defined as a Matrix<Dynamic,Dynamic> in all cases to satisfy Eigen dimension checks
         * \param[in] deriv  if true, the 1st derivative is computed, if false only the fonction values are returned
         * \return  the index of the interval of the knot vector where the control value \p u  is falling
         *
         */
        int basisFunctions(Axis axe, double u, MatrixXd & C, bool deriv);

        /** \brief Alternate method for getting the values of the basis functions and and their 1st derivatives
         *
         * This function directly returns the vector or matrix of results  and the intervalin which u was found is return in interval
         * \param axe the axe X or Y along which the splineFunction values are computed
         * \param u The control value
         * \param interval [out] Interval into which u was found
         * \param deriv flag to request or cancel the return of 1s derivatives
         * \return An araay of either 1 or two columns
         * \warning This function seems not as practical as the other over overload and could be removed in future
         */
        ArrayXXd basisFunctions(Axis axe, double u, Index* interval, bool deriv);

        /** \brief Computes the values of the non null the basis functions and of all derivatives at the specified location \p u
         *      \n Uses the FP faster algorithm
         * \param[in] axe the axe X or Y along which the splineFunction values and derivatives are computed
         * \param u double
         * \param C BasicSplineType&
         * \return int
         *
         */
        int   basisFunctionDerivatives(Axis axe, double u, Ref<ArrayXXd> C);


        MatrixXd interpolator(Axis axe, const Ref< ArrayXd> & uval, MatrixXd & deriv);

        /** \brief gets the interpolated value for the given parameters
         *
         * \param x the X parameter at which an interpolated value is requested
         * \param y the Y parameter at which an interpolated value is requested
         * \return the interpolated value
         *
         */
        inline double operator ()(double x, double y)
        {
            if(x <muX(0)|| x >muX(muX.size()-1) || y <muY(0)|| y >muY(muY.size()-1))
                throw std::runtime_error("argument out of the valid range");

            MatrixXd splineX, splineY;
            Index p=basisFunctions(X,x,splineX,false)-degree;
            Index q=basisFunctions(X,x,splineY,false)-degree;

            return (splineX.transpose() * m_controlValues.block(p,q, degree+1,degree+1) *  splineY)(0,0);
        }

        /** \brief gets the interpolated value for the given parameters and the local gradient
         *
         * \param x the X parameter at which an interpolated value is requested
         * \param y the Y parameter at which an interpolated value is requested
         * \param gradient a vector of size 2 where the gradient a given position is returned
         * \return the interpolated value
         *
         */
        inline double valueGradient(double x, double y, Ref<Vector2d> gradient)
        {
            if(x <muX(0)|| x >muX(muX.size()-1) || y <muY(0)|| y >muY(muY.size()-1))
                throw std::runtime_error("argument out of the valid range");
            Index n=degree+1 ;

            MatrixXd splineX, splineY;
            Index p=basisFunctions(X,x,splineX, true)-degree;
            Index q=basisFunctions(Y,y, splineY, true)-degree;

            MatrixXd mat= splineX.transpose() * m_controlValues.block(p,q, n,n) *  splineY;
            gradient(0)=mat(1,0);
            gradient(1)=mat(0,1);
            return mat(0,0);
        }

        /** \brief Defines the nodes of interpolation by distributing them evenly in N equal intervals over the segment[kstart,kend]
        *
        *       K supplementary nodes  equal to \c kstart are added at the vector head  and K-1 nodes equal to \c kend at the tail.
        *       La dimension totale du vecteur des noeuds est N+2 K et est retournée par la fonction. K= degré des splines
         *
         * \param axe the reference of the axe on which the nodes are defined. (Either X or Y)
         * \param N Number of intervals to define on the interpolation segment
         * \param mustart first point of the interpolation segment
         * \param muend last point of the interpolation segment
         * \return the overall size of the created node vector
         *
         */
        inline int setUniformKnotBase(Axis axe, int N, double mustart, double muend)
        {
            ArrayXd &mu=(axe==X) ? muX :muY;
            int nsz=N+ 2*degree;
            if(mu.size() !=nsz)
            {
                mu.resize(nsz);             // if mu is resized m_controlValues data are released
                m_controlValues=MatrixXd();
            }

            mu.segment(0, degree)=VectorXd::Constant(degree,mustart);
            mu.segment(degree, N +1)=VectorXd::LinSpaced(N+1, mustart,muend);
            mu.segment(N + degree +1, degree - 1)=VectorXd::Constant(degree-1,muend);
            return mu.size();
        }


        /** \brief defines the control points in order to fit a grid sampled surface
         *
         * \param[in] Xvalues  1D array containing the x values of the sampling grid
         * \param[in] Yvalues  1D array containing the y values of the sampling grid
         * \param[in] Zvalues  2D array containing the sampled data points
         */
        void buildControlPoints(const Ref<ArrayXd> &Xvalues, const Ref<ArrayXd> &Yvalues,
                                const Ref<ArrayXXd> &Zvalues);


       /** \brief Special solver for evenly spaced data sampled at control points and degree=3
        *
        *  The system to solve has a tridiagonal matrix with all rows identical but the two first and last ones. It is such expressed that the subdiagonal is all 1.
        *
        *  It is solved in two steps. First the lines are iteratively combined 2 by 2 to eliminate the subdiagonal and have 1 everywhere on the diagonal. Then the new equations are iteratively solved from the last one.
        * \param input The sampled data. It can be a Vector or or an array, in which case the equation is solved for each column of the input
        * \return ArrayXXd an array 2 rows larger and having the same number of columns as the input
        *
        */
       ArrayXXd uniformSpaced1DSolve(const Ref<ArrayXXd>& input);

       /** \brief Solve and transpose in one operation.\n This is strictly equivalent to uniformSpaced1DSolve(input.transpose()).
        *
        * \param input  The sampled data. It can be a RowVector or or an array, in which case the equation is solved for each row of the input
        * \return ArrayXXd an array 2 rows larger than input has columns. Its number of columns is the number of rows of input
        */
       ArrayXXd uniformSpaced1DSolveT(const Ref<ArrayXXd> &input);

       /** \brief This function returns a constant reference to the control values
        */
       inline const MatrixXd& controlValues(){return m_controlValues; }


       ArrayXXd getLegendreFit(int Nx, int Ny, SurfaceStats* pStats=NULL);

    protected:
        int  degree; /**< \brief the degree of the interpolating spline */
        ArrayXd muX; /**< \brief X array of grid control points */
        ArrayXd muY; /**< \brief Y array of grid control points  */
        MatrixXd m_controlValues; /**< \brief The array of control values */
    private:
};

#endif // BIDIMSPLINE_Hr


