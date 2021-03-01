/**
 *************************************************************************
*   \file           Poly1D.cpp
*
*   \brief             implementation file
*
*
*
*   \author             François Polack  <francois.polack@synchroton-soleil.fr>
*   \date               Creation: 2021-02-21
*   \date               Last update: 2021-02-23
 ***************************************************************************/#include "Poly1D.h"

#include "Poly1D.h"

Poly1D::Poly1D(int degree):m_degree(degree)
{
    if(degree <1)
        degree=0;
    m_coeffs.setZero(degree);
    Parameter param;
    param.value=degree;
    param.type=Dimensionless;
    param.group=GratingGroup;
    defineParameter("degree", param);  // par défaut 0
    setHelpstring("degree", "Degree of the line density polynomial");
    param.value=0;
    param.type=DistanceMoins1;
    defineParameter("lineDensity", param);  // par défaut 0
    setHelpstring("lineDensity", "Central line density ");  // complete la liste de infobulles de la classe
    if(degree >0)
    {
        param.value=0;
        param.type=DistanceMoinsN;
        char namebuf[32], parminfo[48];
        for (int i=1; i < degree; ++i)
        {
            sprintf(namebuf, "lineDensityCoeff_%d", i);
            sprintf(parminfo, "Line density coefficient at order %d", i);
            defineParameter(namebuf, param);  // par défaut 0
            setHelpstring(namebuf, parminfo );  // complete la liste de infobulles de la classe
        }

    }

}

bool Poly1D::setParameter(string name, Parameter& param)
{
    bool success=true;
    if(name.compare(0,6,"degree"))
    {
        if(param.value <0)
            success= false;
        if(param.value > m_degree)
        {
            ArrayXd ccopy=m_coeffs; // Save present coefficients
            m_coeffs.resize(param.value); // resize reallocates the array
            m_coeffs.head(m_degree)=ccopy;
            m_coeffs.tail(param.value-m_degree).setZero();
            m_degree=param.value;
        }
        else if(param.value < m_degree)
        {
            ArrayXd ccopy=m_coeffs; // Save present coefficients
            m_coeffs.resize(param.value); // resize reallocates the array
            m_coeffs=ccopy.head(param.value);
            m_degree=param.value;
        }
        success= true; // rien à faire si param.value==degree
    }
    else if(name.compare(0,11,"lineDensity")==0)
    {
        if(name.length()==11)
            m_coeffs(0)=param.value;
        else
        {
            int index;
            if(scanf(name.c_str(),"lineDensityCoeff_%d",&index)==1)
                m_coeffs(index)=param.value;
            else
                success=false;
        }
    }
    else
        success=false;

    if (success)
        return true;
    SetOptiXLastError("invalid grating parameter",__FILE__,__func__);
    return false;

}


EIGEN_DEVICE_FUNC  Surface::VectorType Poly1D::gratingVector(Surface::VectorType position, Surface::VectorType normal)
{

  Surface::VectorType G=normal.cross(Surface::VectorType::UnitY()).normalized();

  double density=0;
  for(int n= m_degree-1; n>0; --n)
  {
      density+=m_coeffs(n);
      density*=position(0);
  }
  density+=m_coeffs(0);
  G*=density;

  return G;
}

