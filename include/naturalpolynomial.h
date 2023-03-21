#ifndef HEADER_33323F44985F7B41
#define HEADER_33323F44985F7B41

/**
*************************************************************************
*   \file       naturalpolynomial.h

*
*   \brief     definition file
*
*
*
*
*   \author             François Polack  <francois.polack@synchroton-soleil.fr>
*   \date               Creation : 2023-03-10

*   \date               Last update: 2023-03-10

*
*
 ***************************************************************************/

#ifndef NATURALPOLYNOMIAL_H
#define NATURALPOLYNOMIAL_H

#include "polynomial.h"


class NaturalPolynomial : public Polynomial
{
    public:
        using Polynomial::VectorXType, Polynomial::MatrixXType, Polynomial::ArrayXType, Polynomial::ArrayXXType;
        NaturalPolynomial(){}
        virtual ~NaturalPolynomial(){}
        virtual inline string getOptixClass(){return "NaturalPolynomial";}

        virtual VectorXType getBaseValues(Index Norder, FloatType Xpos, VectorXType & derivative, VectorXType &second);
        virtual ArrayXXType getBaseValues(Index Norder, const Ref<ArrayXType>& Xpos, ArrayXXType& derivative, Ref<ArrayXXType> *second=NULL );
    protected:

    private:
};

#endif // NATURALPOLYNOMIAL_H
#endif // header guard

