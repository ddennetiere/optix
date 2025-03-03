/**
 *************************************************************************
*   \file           naturalpolynomial.cpp
*
*   \brief             implementation file
*
*
*
*   \author             François Polack  <francois.polack@synchroton-soleil.fr>
*   \date               Creation: 2023-03-10
*   \date               Last update: 2023-03-10
 ***************************************************************************/#include "naturalpolynomial.h"

#include "naturalpolynomial.h"


Polynomial::VectorXType NaturalPolynomial::getBaseValues(Index Norder, FloatType Xpos, VectorXType & derivative, VectorXType &second)
{

    VectorXType value=VectorXType::Ones(Norder);
    derivative=second= VectorXType::Zero(Norder);
    for(Index i=1, im1=0; i <Norder ; ++i, ++im1)
    {
        value(i)=Xpos*value(im1);
        derivative(i)=i*value(im1);
        second(i)=i*derivative(im1);
    }
    return value;
}

Polynomial::ArrayXXType NaturalPolynomial::getBaseValues(Index Norder, const Ref<ArrayXType>& Xpos, ArrayXXType& derivative, Ref<ArrayXXType> *second)
{
    Index Xsize=Xpos.size();
    ArrayXXType fvalue(Xsize, Norder);
    derivative.resize(Xsize, Norder);
    fvalue.col(0).setOnes();
    derivative.col(0).setZero();
    if(second)
    {
        second->resize(Xsize,Norder);
        second->col(0).setZero();
    }
    for(Index i=1, im1=0; i <Norder ; ++i, ++im1)
    {
        fvalue.col(i)=Xpos*fvalue.col(im1);
        derivative.col(i)= i*fvalue.col(im1);
        if(second)
            second->col(i)= i*derivative.col(im1);
    }

    return fvalue;
}
