////////////////////////////////////////////////////////////////////////////////
/**
*      \file           sourcebase.cpp
*
*      \brief         Base class of sources implementataion
*
*      \author         François Polack <francois.polack@synchroton-soleil.fr>
*      \date        2020-11-01  Creation
*      \date        Last update
*
*/
///////////////////////////////////////////////////////////////////////////////////
//
//             REVISIONS
//
////////////////////////////////////////////////////////////////////////////////////
#include "sourcebase.h"

SourceBase::SourceBase():Surface(true,"Source") {} //


SourceBase::~SourceBase(){}

void SourceBase::waveRadiate(double wavelength, double Xaperture, double Yaperture, size_t Xsize, size_t Ysize, char polar)
{
    VectorXd gridX=VectorXd::LinSpaced(-Xaperture, Xaperture, Xsize);
    VectorXd gridY=VectorXd::LinSpaced(-Yaperture, Yaperture, Ysize);
    clearImpacts();
    VectorType org=VectorType::Zero(),dir=VectorType::Zero();
    RayType::ComplexType Samp, Pamp;
    if(polar=='S')
        Samp=1., Pamp=0;
    else if(polar=='P')
        Samp=0, Pamp=1.;
    else if(polar=='R')
        Samp=sqrt(2.), Pamp=complex<double>(0,sqrt(2.));
    else if(polar=='L')
        Samp=sqrt(2.), Pamp=complex<double>(0,-sqrt(2.));
    else
        throw ParameterException("invalid polarization (S, P, R or L only are  allowed)", __FILE__, __func__, __LINE__);
    for(size_t j=0; j< Ysize; ++j)
    {
        for (size_t i=0; i < Xsize; ++i)
        {
            dir << gridX(i), gridY(j), 1.;
            m_impacts.emplace_back(RayBaseType(org,dir),wavelength,Samp,Pamp );
        }
    }
    radiate();
}
