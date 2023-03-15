#ifndef HEADER_19E4D58BFDA94B81
#define HEADER_19E4D58BFDA94B81

/**
*************************************************************************
*   \file       legendrepolynomial.h

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

#ifndef LEGENDREPOLYNOMIAL_H
#define LEGENDREPOLYNOMIAL_H

#include "polynomial.h"


class LegendrePolynomial : public Polynomial
{
    public:
        LegendrePolynomial(){}
        LegendrePolynomial(double Xmin, double Xmax, double Ymin, double Ymax)
        {
            m_Xbound << Xmin, Xmax;
            m_Ybound << Ymin, Ymax;
        }
        virtual ~LegendrePolynomial(){}
        virtual inline ArrayXType Xnormalize(const ArrayXd &xpos)
        {
            double Kx=2./(m_Xbound(1)-m_Xbound(0));
            double X0=(m_Xbound(1)+m_Xbound(0))/2.;
            ArrayXd xnormed=Kx*(xpos-X0);
            if ((xnormed.maxCoeff() > 1.) || (xnormed.minCoeff() < -1. ))
                throw ParameterException("Values in xpos vector should be in the [-1, +1] range", __FILE__, __func__, __LINE__);
            return xnormed.cast<FloatType>();
        }
        virtual inline ArrayXType Ynormalize(const ArrayXd &ypos)
        {
            double Ky=2./(m_Ybound(1)-m_Ybound(0));
            double Y0=(m_Ybound(1)+m_Ybound(0))/2.;
            ArrayXd ynormed=Ky*(ypos-Y0);
            if ((ynormed.maxCoeff() > 1.) || (ynormed.minCoeff() < -1. ))
                throw ParameterException("Values in ypos vector should be in the [-1, +1] range", __FILE__, __func__, __LINE__);
            return ynormed.cast<FloatType>();
        }

        virtual VectorXType getBaseValues(int Norder, FloatType Xpos, VectorXType & derivative, VectorXType &second);
        virtual ArrayXXType getBaseValues(int Norder, const Ref<ArrayXType>& Xpos, ArrayXXType& derivative, Ref<ArrayXXType> *second=NULL );
    protected:
        Array2d  m_Xbound, m_Ybound;
};

#endif // LEGENDREPOLYNOMIAL_H
#endif // header guard

