#ifndef POLYNOMIALSURFACE_H_INCLUDED
#define POLYNOMIALSURFACE_H_INCLUDED

/**
*************************************************************************
*   \file       polynomialsurface.h

*
*   \brief     definition file
*
*
*
*
*   \author             François Polack  <francois.polack@synchroton-soleil.fr>
*   \date               Creation : 2023-03-15

*   \date               Last update: 2023-03-15

*
*
 ***************************************************************************/
#include "surface.h"
#include "polynomial.h"

/** \brief Template class describing a surface defined by an expansion on polynomial basis functions \f$ P_n(X) P_m(Y) \f$
 *
 * The class depends on the template parameter PolyType, which defines the polynomial functions of the base.
 * Two bases are presently available : Natural polynomials  and  Legendre Polynomial
  *  \see Polynomial, NaturalPolynomial, LegendrePolynomial
 *
 *  The class defines two parameters of  Array Type
 *     -----------------------------------------
 *
 *   Name of parameter | UnitType | Description
 *   ----------------- | -------- | --------------
 *   \b surfaceLimits  | distance | The limits inside which the polynomials are defined and \f$ abs(P_n(X)) < 1. \f$ \n a 2 x 2 \ref ArrayParameter defining \f$ X_{min} , X_{max} , Y_{min} , Y_{max} \f$ , in this order
 *   \b coefficients   | distance | The coefficients of the polynomial expansion\n (X dimension varying fastest)
 *
 *  these parameters belong to the \ref ShapeGroup They have an array type and cannot be optimized ( Parameter::flags set to 1 | #ArrayData )
 */
template<class PolyType>
class PolynomialSurface:virtual public Surface, public PolyType
{
    public:
        using PolyType::m_coeffs, PolyType::m_Xlimit, PolyType::m_Ylimit;
        using Polynomial::MatrixXType;


        PolynomialSurface(string name="", Surface *previous=NULL);
        virtual  ~PolynomialSurface(){}

        virtual inline string getOptixClass(){return PolyType::getOptixClass()+"Surface";}

        virtual inline string getSurfaceClass(){return PolyType::getOptixClass()+"Surface";}/**< \brief return the most derived shape class name of this object */


        /** \brief This function is called by the OpticalElement classes after a call to setFrameTransforms but does nothing
         *
         * The surface is aligned with respect to the definition plane, irrespectively to the normal orientation at origin point
         * \param wavelength unused
         * \return always 0
         */
        virtual inline int align(double wavelength=0){return 0;} // La surface est alignée sur le plan de définition pas sur la normale à l'origine

        virtual VectorType intercept(RayBaseType& ray, VectorType * normal=NULL)
        {
            ray-=m_translationFromPrevious; // change ref fram from previous to this surface  ray is not rebased
            if(ray.m_alive)
            {
                ray.transform(m_surfaceInverse);
                VectorType ntemp;
                PolyType::intercept(ray,&ntemp); // move the ray to intercept and rebase it, in surface frame
                ray.transform(m_surfaceDirect);
                if(!ray.m_alive)
                    cout << m_name << " ray was lost\n";
                if(normal)
                    *normal=m_surfaceDirect*ntemp;
            }
            return ray.position();
        };

        bool setParameter(string name,  Parameter& param);

        /** \brief fits the polynomial surface to the given height data
         *
         * \param Nx number of coefficients in X (degree -1)
         * \param Ny number of coefficients in y (degree -1)
         * \param paramarray The data points in a ArrayParameter struct. Data point must given in a 3 column array, respectively x, y, z,
         *      aligned in column major order, point index dimension (paramarray.dim[0]) varying fastest.
         * \return the rms height value of fit residuals
         */
        double fitHeightData(Index Nx, Index Ny, const ArrayParameter & paramarray);


        /** \brief fits the polynomial surface to the given height data
         *
         * \param Nx number of coefficients in X (degree -1)
         * \param Ny number of coefficients in y (degree -1)
         * \param paramarray The data points in a ArrayParameter struct. Data point must given in a 4 column array, respectively x, y, dz/dx, dz/dy,
         *      aligned in column major order, point index dimension (paramarray.dim[0]) varying fastest.
         * \return  the rms x and y slope values of fit residuals
         */
        std::pair<double,double>  fitSlopeData(Index Nx, Index Ny, const ArrayParameter & paramarray);


    protected:

};


template<class PolyType>
PolynomialSurface<PolyType>::PolynomialSurface(string name, Surface *previous):Surface(false,name, previous)  //{Surface::m_transmissive=false;}
{
    Parameter param;
    param.type=Distance;
    param.group=ShapeGroup;
    param.flags=NotOptimizable | ArrayData;
    param.paramArray=new ArrayParameter(1,1);
    param.paramArray->data[0]=0.;
 //   cout << "paramArray defined\n";
    defineParameter("coefficients", param);  // Definition makes a deep copy of param, so it can be reused
 //   cout << "coefficients are set\n";
    setHelpstring("coefficients", "Array of coefficients of the polynomial function");  // complete la liste de infobulles de la classe Surface
    Array22d limits;
    limits << -1., -1., 1., 1. ;
 //   cout << "defining limits\n";
    *param.paramArray=limits;
 //  cout << "limits are assigned\n";
    defineParameter("surfaceLimits", param);
 //   cout << "surfaceLimits are defined\n";
    setHelpstring("surfaceLimits", "Rectangular area on which the polynomial base is defined,\n in an array of 4 numbers (Xmin, Xmax, Ymin, Ymax).  ");  // complete la liste de infobulles dea classe Surface
 //   cout << "deleting temp param struct\n";
}

template<class PolyType>
bool PolynomialSurface<PolyType>::setParameter(string name, Parameter& param)
{
    char errmsg[80];
   // cout << "in PolynomialSurface setParameter\n";
    if(! Surface::setParameter(name, param)) // this call  update the parameter in memory  but do not carry any parameter related action
    if(name=="surfaceLimits") // do specific creation actions
    {
        if(!(param.flags & ArrayData))
        {
            SetOptiXLastError("surfaceLimits must be an array type parameter", __FILE__, __func__);
            return false;
        }
        if(param.paramArray->dims[0]*param.paramArray->dims[1]<4)
        {
            SetOptiXLastError("Array size of surfaceLimits parameter must be at least 4", __FILE__, __func__);
            return false;
        }
        double * lim= param.paramArray->data;
        if(lim[0] == lim[1])
        {
            sprintf(errmsg,"X-range of surfaceLimits [%g,%g], has a null extent",lim[0],lim[1]);
            SetOptiXLastError(errmsg, __FILE__, __func__);
            return false;
        }
        if(lim[2] == lim[3])
        {
            sprintf(errmsg,"Y-range of surfaceLimits [%g,%g], has a null extent",lim[2],lim[3]);
            SetOptiXLastError(errmsg, __FILE__, __func__);
            return false;
        }
        Polynomial::setLimits(lim[0], lim[1], lim[2], lim[3]);
    }
    if(name=="coefficients") // do specific creation actions
    {
        //cout << "setting PolynomialSurface coeffs\n";
        if(!(param.flags & ArrayData))
        {

            SetOptiXLastError("coefficients must be an array type parameter", __FILE__, __func__);
            return false;
        }
        if(param.paramArray->dims[0]*param.paramArray->dims[1]<1)
        {
            SetOptiXLastError("Array size of coefficients parameter must be at least 1", __FILE__, __func__);
            return false;
        }
     //   MatrixXd && dcoeff=param.paramArray->matrix();
        m_coeffs=param.paramArray->matrix().template cast<long double>();
    }
    return true;
}


template<class PolyType>
double PolynomialSurface<PolyType>::fitHeightData(Index Nx, Index Ny, const ArrayParameter & paramarray)
{
    if(paramarray.dims[1] <3 )
        throw ParameterException("paramarray parameter should have at least 3 columns (dim[1])", __FILE__, __func__, __LINE__);
    if(paramarray.dims[0] < Nx*Ny)
        throw ParameterException("Not enough data points for the required fit", __FILE__, __func__, __LINE__);
    Map<ArrayX3d> height(paramarray.data, paramarray.dims[0], 3);
    if (height.col(0).minCoeff()< m_Xlimit[0] || height.col(0).maxCoeff()> m_Xlimit[1] ||
        height.col(1).minCoeff()< m_Ylimit[0] || height.col(1).minCoeff()< m_Ylimit[1] )
            throw ParameterException("some data points are out of the definition rectangle", __FILE__, __func__, __LINE__);
    double sigma=PolyType::fitHeights(Nx,Ny, height);
    Parameter param;
    Surface::getParameter("coefficients", param);
    ArrayXXd && dcoeffs=m_coeffs.template cast<double>();
    *param.paramArray=dcoeffs;
    Surface::setParameter("coefficients", param);
    return sigma;
}

template<class PolyType>
std::pair<double,double>  PolynomialSurface<PolyType>::fitSlopeData(Index Nx, Index Ny, const ArrayParameter & paramarray)
{
    if(paramarray.dims[1] <4 )
        throw ParameterException("paramarray parameter should have at least 4 columns (dim[1])", __FILE__, __func__, __LINE__);
    if(paramarray.dims[0] < Nx*Ny)
        throw ParameterException("Not enough data points for the required fit", __FILE__, __func__, __LINE__);
    Map<ArrayX4d> slope(paramarray.data, paramarray.dims[0], 4);
    if (slope.col(0).minCoeff()< m_Xlimit[0] || slope.col(0).maxCoeff()> m_Xlimit[1] ||
        slope.col(1).minCoeff()< m_Ylimit[0] || slope.col(1).minCoeff()< m_Ylimit[1] )
            throw ParameterException("some data points are out of the definition rectangle", __FILE__, __func__, __LINE__);
    std::pair<double,double>  sigmavals =PolyType::fitSlopes(Nx,Ny, slope);
    Parameter param;
    Surface::getParameter("coefficients", param);
    ArrayXXd && dcoeffs=m_coeffs.template cast<double>();
    *param.paramArray=dcoeffs;
    Surface::setParameter("coefficients", param);
    return sigmavals;
}
#endif // POLYNOMIALSURFACE_H_INCLUDED
