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
*   \date               Last update: 2021-08-09
 ***************************************************************************/#include "Poly1D.h"

#include "Poly1D.h"

Poly1D::Poly1D(int degree):m_degree(degree)
{
//    cout << "initializing PolyGrating with degree= " << degree << "\n";
    if(degree <1)
        degree=0;
    m_degree=degree;
    m_coeffs.setZero(m_degree+1);
    Parameter param;
    param.value=m_degree;
    param.type=Dimensionless;
    param.group=GratingGroup;
    defineParameter("degree", param);  // par défaut 0
    setHelpstring("degree", "Degree of the line density polynomial");
    param.value=1e6;
    param.type=DistanceMoins1;
    defineParameter("lineDensity", param);  // par défaut 1000 tpmm
    setHelpstring("lineDensity", "Central line density ");  // complete la liste de infobulles de la classe
    m_coeffs(0)=param.value;
    if(degree >0)
    {
        param.value=0;
        param.type=DistanceMoinsN;
        char namebuf[32], parminfo[48];
        for (int i= degree; i >0; --i)
        {
            sprintf(namebuf, "lineDensityCoeff_%d", i);
            sprintf(parminfo, "Line density coefficient at order %d", i);
            defineParameter(namebuf, param);  // par défaut 0
            setHelpstring(namebuf, parminfo );  // complete la liste de infobulles de la classe
        }
    }
//    cout << "initial coeffs :\n" << m_coeffs.transpose() << endl;
}

bool Poly1D::setParameter(string name, Parameter& param)
{
//    cout << "setting Poly1D parameter " << name << endl;
    bool success=true;
    if(name.compare(0,6,"degree")==0)
    {
        if(param.value <0)
            success= false;
        char namebuf[32];
        if(param.value > m_degree)
        {
            ArrayXd ccopy=m_coeffs; // Save present coefficients
            m_coeffs.resize(param.value+1); // resize reallocates the array
            m_coeffs.head(m_degree)=ccopy;
            m_coeffs.tail(param.value-m_degree).setZero();
            // ca ne suffit pas il faut ajouter des aramètres
            Parameter newParam;
            newParam.value=0;
            newParam.type=DistanceMoinsN;
            char parminfo[48];
            for(int i=param.value; i >m_degree; --i)
            {
                sprintf(namebuf, "lineDensityCoeff_%d", i);
                sprintf(parminfo, "Line density coefficient at order %d", i);
                defineParameter(namebuf, newParam);  // par défaut 0
                setHelpstring(namebuf, parminfo );  // complete la liste de infobulles de la classe
            }
            m_degree=param.value;
        }
        else if(param.value < m_degree)
        {
            ArrayXd ccopy=m_coeffs; // Save present coefficients
            m_coeffs.resize(param.value+1); // resize reallocates the array
            m_coeffs=ccopy.head(param.value+1);
            // ca ne suffit pas il faut supprimer des aramètres
            for(int i=m_degree; i>param.value; --i)
            {
                sprintf(namebuf, "lineDensityCoeff_%d", i);
                removeParameter(namebuf);
            }
            m_degree=param.value;
        }
        success= true; // rien à faire si param.value==degree
    }
    else if(name.compare(0,11,"lineDensity")==0)
    {
//        cout << "setting the line density coeffs\n";
        if(name.length()==11)
        {
//            cout << "setting central line density to " << param.value << " m^-1\n\n";
            m_coeffs(0)=param.value;
        }
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
// Modif FP 2021-08-09
//    if (!success)
//        SetOptiXLastError("invalid grating parameter",__FILE__,__func__);
    return success;

}

EIGEN_DEVICE_FUNC  Surface::VectorType Poly1D::gratingVector(const Surface::VectorType &position, const Surface::VectorType &normal)
{

  Surface::VectorType G=normal.cross(Surface::VectorType::UnitY()).normalized();

  double density=0;
  for(int n= m_degree; n>0; --n)
  {
      density+=m_coeffs(n);
      density*=position(0);
  }
  density+=m_coeffs(0);
  G*=density;

  return G;
}

