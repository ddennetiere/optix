/**
 *************************************************************************
*   \file           legendrepolynomial.cpp
*
*   \brief             implementation file
*
*
*
*   \author             François Polack  <francois.polack@synchroton-soleil.fr>
*   \date               Creation: 2023-03-15
*   \date               Last update: 2023-03-15
 ***************************************************************************/#include "legendrepolynomial.h"

#include "legendrepolynomial.h"


Polynomial::VectorXType LegendrePolynomial::getBaseValues(Index Norder, FloatType Xpos, VectorXType & derivative, VectorXType &second)
{
    // pos values must be  normalized € [-1,+1]
    VectorXType value=VectorXType::Ones(Norder);
    double c1,c2;
    derivative.resize(Norder);
    second.resize(Norder);
    value(1)=Xpos;
    derivative(1)=1.;
    derivative(0)=second(1)=second(0)=0;

    for(Index i=2; i <Norder; ++i)
    {
        c1=(2.*i-1.)/i;
        c2=double(i-1)/i;
        value(i)=c1* Xpos*value(i-1)-c2*value(i-2);
        derivative(i)=c1*(Xpos*derivative(i-1)+value(i-1))-c2*derivative(i-2);
        second(i)=c1*(Xpos*second(i-1)+ 2.*derivative(i-1))-c2*second(i-2);
//        derivative(i)=i * value(i-1) + Xpos * derivative(i-1);
//        second(i)= (i+1)* derivative(i-1)  + Xpos * second (i-1);
    }
    return value;
    // todo la recurrence P'n=n Pn-1 +x P'n-1  generalisable
}

Polynomial::ArrayXXType LegendrePolynomial::getBaseValues(Index Norder, const Ref<ArrayXType>& Xpos, ArrayXXType& derivative, Ref<ArrayXXType> *second)
{
    // pos vector is called normalized € [-1,+1]
    Index Xsize=Xpos.size();
    ArrayXXType fvalue(Xsize, Norder);

    derivative.resize(Xsize, Norder);

    double c1,c2;

    fvalue.col(0).setOnes();
    fvalue.col(1)= Xpos;
    derivative.col(0).setZero();
    derivative.col(1).setOnes();
    if(second)
    {
        second->resize(Xsize, Norder);
        second->leftCols(2).setZero();
    }

    for(Index icol=2; icol <Norder; icol++)
    {
        c1=(2.*icol-1.)/icol;
        c2=double(icol-1)/icol;
        fvalue.col(icol)=c1* Xpos*fvalue.col(icol-1)-c2*fvalue.col(icol-2);
        derivative.col(icol)=c1*(Xpos*derivative.col(icol-1)+fvalue.col(icol-1))-c2*derivative.col(icol-2);
        if(second)      // si dérivée seconde utile (col 0 et 1 == zero)
            second->col(icol)=c1*(Xpos*second->col(icol-1)+ 2.*derivative.col(icol-1))-c2*second->col(icol-2);
    }

    return fvalue;
}
