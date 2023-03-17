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

        virtual ~LegendrePolynomial(){}

        virtual inline string getOptixClass(){return "LegendrePolynomial";}

        virtual VectorXType getBaseValues(int Norder, FloatType Xpos, VectorXType & derivative, VectorXType &second);
        virtual ArrayXXType getBaseValues(int Norder, const Ref<ArrayXType>& Xpos, ArrayXXType& derivative, Ref<ArrayXXType> *second=NULL );
    protected:

};

#endif // LEGENDREPOLYNOMIAL_H
#endif // header guard

