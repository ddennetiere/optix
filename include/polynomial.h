#ifndef POLYNOMIAL_H_INCLUDED
#define POLYNOMIAL_H_INCLUDED

/**
*************************************************************************
*   \file       polynomial.h

*
*   \brief     definition file
*
*
*
*
*   \author             François Polack  <francois.polack@synchroton-soleil.fr>
*   \date               Creation : 2023-03-08

*   \date               Last update: 2023-03-08

*
*
 ***************************************************************************/

#include "EigenSafeInclude.h"
#include <iostream>
#include <unsupported/Eigen/CXX11/Tensor>
#include "types.h"

using namespace Eigen;

 /** \brief This virtual class defines the generic functionalities of a polynomial surface
  *
  * It defines virtual function for evaluating height and slopes, interpolation and fits
  * which are implemented in derived classes
  */
 class Polynomial
 {
 public:
    typedef VectorX<FloatType> VectorXType;
    typedef MatrixX<FloatType> MatrixXType;
    typedef ArrayX<FloatType> ArrayXType;
    typedef ArrayXX<FloatType> ArrayXXType;

    Polynomial(){}      /**< \brief  default constructor */

    virtual ~Polynomial(){}     /**< \brief virtual destructor */

    Polynomial(double Xmin, double Xmax, double Ymin, double Ymax)
    {
        m_Xlimit << Xmin, Xmax;
        m_Ylimit << Ymin, Ymax;
        m_Kx=2./(m_Xlimit(1)-m_Xlimit(0));
        m_X0=(m_Xlimit(1)+m_Xlimit(0))/2.;
        m_Ky=2./(m_Ylimit(1)-m_Ylimit(0));
        m_Y0=(m_Ylimit(1)+m_Ylimit(0))/2.;
    }

    inline void setLimits(double Xmin, double Xmax, double Ymin, double Ymax)
    {
        m_Xlimit << Xmin, Xmax;
        m_Ylimit << Ymin, Ymax;
        m_Kx=2./(m_Xlimit(1)-m_Xlimit(0));
        m_X0=(m_Xlimit(1)+m_Xlimit(0))/2.;
        m_Ky=2./(m_Ylimit(1)-m_Ylimit(0));
        m_Y0=(m_Ylimit(1)+m_Ylimit(0))/2.;
    }


    /** \brief construct a polynomial surface from the matrix of the coefficients on the polynomial base
     *
     * \param coeffs a matrix of coefficient (x, y) which will be converted to doubles
     */
    template<typename derived>
    Polynomial(const Eigen::DenseBase<derived> &  coeffs):m_coeffs(coeffs){}

    virtual string getOptixClass()=0;

    MatrixXType && coeffs(){return std::move(m_coeffs);}/**< \brief returns the polynomial coefficients in a Matrix of double as a Rvalue reference */

    /** \brief defines a Polynomial by setting all coefficients with type conversion
     * \param newcoeffs  a matrix or array of any type containing the polynomial new coefficients
     */
    template<typename derived, typename FloatType>
    void setCoeffs(  const  Ref<Matrix<derived,Dynamic,Dynamic> > & newcoeffs)
        { m_coeffs  =  newcoeffs.template cast<FloatType>();}


    inline ArrayXType Xnormalize(const ArrayXd &xpos, int warnLimits=0)
    {
        ArrayXd xnormed=m_Kx*(xpos-m_X0);
        if (warnLimits &((xnormed.maxCoeff() > 1.) || (xnormed.minCoeff() < -1. )))
            throw ParameterException("Values in xpos vector should be in the [-1, +1] range", __FILE__, __func__, __LINE__);
        return xnormed.cast<FloatType>();
    }

    inline ArrayXType Ynormalize(const ArrayXd &ypos, int warnLimits=0)
    {
        ArrayXd ynormed=m_Ky*(ypos-m_Y0);
        if (warnLimits &((ynormed.maxCoeff() > 1.) || (ynormed.minCoeff() < -1. )))
            throw ParameterException("Values in ypos vector should be in the [-1, +1] range", __FILE__, __func__, __LINE__);
        return ynormed.cast<FloatType>();
    }

    /** \brief This functions returns the values and first derivatives of the base 1D polynomial for an array of X values
     *
     * \param[in] Norder the order of the polynomial base = max degree -1)
     * \param[in] Xpos constant array of normalized X values
     * \param[in,out] derivative ArrayXXd&   reference  to an Eigen array to receive the dérivative. It will be resized
     *          to (Xpos.size(), Norder) and overwritten
     * \param[out] second pointer to an Eigen Array or Matrix to received the second derivative. If NULL it is not computed
     * \return an Eigen Array of size (Xpos.size(), Norder)  containing the values of each base polynomial at the given X values
     */
    virtual ArrayXXType getBaseValues(int Norder, const Ref<ArrayXType>& Xpos, ArrayXXType& derivative, Ref<ArrayXXType> *second=NULL )=0;
//  virtual ArrayXXd getBaseValues(int Norder, const Ref<ArrayXd>& Xpos, ArrayXXd& derivative, Ref<ArrayXXd> *second=NULL )=0;

    /** \brief compute the value of the polynomial base function and the 2 first derivatives at one specified point
     *
     * \param Norder order of the polynomial base
     * \param Xpos normalized position where values must be computed
     * \param derivative the reference of an Eigen Array to return the first derivatives
     * \param second the reference of an Eigen Array to return the first derivatives
     * \return an array containing the base polynomial values a the given position
     */
    virtual VectorXType getBaseValues(int Norder, FloatType Xpos, VectorXType & derivative, VectorXType &second)=0;

    ArrayXXd surfaceHeight(const Ref<ArrayXd>& Xpos, const Ref<ArrayXd>& Ypos );

    Tensor<double,3> surfaceSlopes(const Ref<ArrayXd>& Xpos, const Ref<ArrayXd>& Ypos );

    /** \brief fits the given height data by a polynomial of specified order
     *
     *  This is a pure virtual function which is defined in derived classes
     * \param Nx the polynomial order in the X parameter
     * \param Ny the polynomial order in the Y parameter
     * \param heightdata the data to be fitted in a 3 columns array. Columns are X, Y, Z in this order
     * \return the rms of fit residuals
     */
    double fitHeights(int Nx, int Ny, const Ref<ArrayX3d>& heightdata);

    /** \brief fits the surface whose slopes are given in input by a polynomial of specified order
     *
     *  This function is common to all polynomial types
     * \param Nx the polynomial order in the X parameter
     * \param Ny the polynomial order in the Y parameter
     * \param slopedata the data to be fitted in a 4 columns array. Columns are X, Y, dZ/dX, dZ/dY in this order
     * \return std::pair<double,double>
     *
     */
    std::pair<double,double> fitSlopes(int Nx, int Ny, const Ref<ArrayX4d>& slopedata);

    /** \brief Compute the ray intercept in the surface reference frame
     *
     * \param ray the ray the intercept is looked for (in the surface reference frame)
     * \param normal=NULL optional vector to return the normalized normal vector
     * \return the computed intercept. If algorithm fails, the function  will throw an OptiXException.
     *  If no solution is found the ray will be marked as lost
     */
    RayBaseType::VectorType intercept(RayBaseType& ray, RayBaseType::VectorType * normal=NULL );

 protected  :
    MatrixXType m_coeffs;
    Array2d  m_Xlimit, m_Ylimit;
    double m_Kx,m_Ky, m_X0, m_Y0;
 };


#endif // POLYNOMIAL_H_INCLUDED
