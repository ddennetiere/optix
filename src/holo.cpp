/**
 *************************************************************************
*   \file           holo.cpp
*
*   \brief             implementation file
*
*
*
*   \author             François Polack  <francois.polack@synchroton-soleil.fr>
*   \date               Creation: 2021-02-23
*   \date               Last update: 2021-02-23
 ***************************************************************************/#include "holo.h"

 #include "holo.h"
 #include <cmath>  // pour copysign

// #define HOLO_EULERIAN  // juste pour mieux lire

#ifdef HOLO_EULERIAN
Holo::Holo()
{
    Parameter param;
    param.value=m_holoWavelength=3.511e-7; // 351.1 nm Argon line JY default  (pourquoi pas 257.3 ou 244 ?)
    param.type=Distance;
    param.group=GratingGroup;
    param.flags=NotOptimizable;
    defineParameter("recordingWavelength", param);  // par défaut 0
    setHelpstring("recordingWavelength", "Recording wavelength of the holographic pattern");  // complete la liste de infobulles de la classe
    param.flags=0; // other parameters can be optimized

    param.value=m_inverseDistance1=m_inverseDistance2=0;
    param.type=InverseDistance;
    defineParameter("inverseDist1", param);  // par défaut 0
    setHelpstring("inverseDist1", "Reciprocal distance of the 1st construction point (with sign)");
    defineParameter("inverseDist2", param);  // par défaut 0
    setHelpstring("inverseDist2", "Reciprocal distance of the 2nd construction point (with sign)");
    param.type=Angle;
    param.value=0;
    defineParameter("azimuthAngle1", param);  // par défaut 0
    setHelpstring("azimuthAngle1", "Azimuth angle (psi) of the 1st construction point");
    defineParameter("azimuthAngle2", param);  // par défaut 0
    setHelpstring("azimuthAngle2", "Azimuth angle (psi) of the 2nd construction point");
    param.value=M_PI/4.5;    //   40° rasance min de JY 20°
    m_direction1 << cos(param.value), sin(param.value),1.;
    defineParameter("elevationAngle1", param);  // par défaut 0
    setHelpstring("elevationAngle1", "Elevation angle (theta) of the 1st construction point");
    param.value=1.1429146   ; // 53.94 ==> 1000 l/mm
    m_direction2 << cos(param.value), sin(param.value),1.;
    defineParameter("elevationAngle2", param);  // par défaut 0
    setHelpstring("elevationAngle2", "Elevation angle (theta) of the 2nd construction point");
}

// pour éviter les doublons on ne traite ici que les paramètres propres aux réseaux holo
bool Holo::setParameter(string name, Parameter& param)
{
  //  On suppose que  la fonction Surface::setParameter() a été appelée au préalable par la classe dérivée
    bool success=true;
    double theta=0, psi=0, cost;
    if(name.compare(0,19,"recordingWavelength")==0)
        m_holoWavelength=param.value;
    else if(name.compare(0,12,"inverseDist1")==0)
    {
        m_inverseDistance1=param.value;
        m_direction1(2)=copysign(m_direction1(2), m_inverseDistance1);
    }
    else if(name.compare(0,12,"inverseDist2")==0)
    {
        m_inverseDistance2=param.value;
        m_direction2(2)=copysign(m_direction2(2), m_inverseDistance2);
    }
    else if(name.compare(0,13,"azimuthAngle1")==0)
    {
        psi=param.value;
        getParameter("elevationAngle1", param);
        cost=cos(param.value);
        m_direction1 << cost*cos(psi),cost*sin(psi), copysign(sin(param.value), m_inverseDistance1);
    }
    else if(name.compare(0,13,"azimuthAngle2")==0)
    {
        psi=param.value;
        getParameter("elevationAngle2", param);
        cost=cos(param.value);
        m_direction2 << cost*cos(psi), cost*sin(psi), copysign(sin(param.value), m_inverseDistance2);
    }
    else if(name.compare(0,15,"elevationAngle1")==0)
    {
        theta=param.value;
        getParameter("azimuthAngle1", param);
        cost=cos(theta);
        m_direction1 << cost*cos(param.value),cost*sin(param.value), copysign(sin(theta), m_inverseDistance1);
    }
    else if(name.compare(0,15,"elevationAngle2")==0)
    {
        theta=param.value;
        getParameter("azimuthAngle2", param);
        cost=cos(theta);
        m_direction2 << cost*cos(param.value),cost*sin(param.value), copysign(sin(theta), m_inverseDistance2);
    }
    else
        success=false;
    if(!success)
        SetOptiXLastError("invalid grating parameter",__FILE__,__func__);
    return success;
 }

EIGEN_DEVICE_FUNC  Surface::VectorType Holo::gratingVector(Surface::VectorType position, Surface::VectorType normal)
{
 // le calcul est effectué dans le référentiel propre à la surface
    Surface::VectorType G, du;
    du=(m_direction1-m_inverseDistance1*position).normalized();
    du-=(m_direction2-m_inverseDistance2*position).normalized();
    G=(du-du.dot(normal)*normal);  //  projection de "du" sur le plan tangent local-
 // densité de traits = delta cos(theta)
    return G;
}


 #else
  #ifdef HOLO_CARTESIAN

 Holo::Holo()
{
    char coords[]={'X','Y','Z'};

    C1 << -1.,0.2,0;
    C2 << -0.5, 0.2,0;
    m_holoWavelength=3.511e-7; // 256 nm par défaut
    Parameter param;
    param.value=m_holoWavelength;
    param.type=Distance;
    param.group=GratingGroup;
    param.flags=NotOptimizable;
    defineParameter("recordingWavelength", param);  // par défaut 0
    setHelpstring("recordingWavelength", "Recording wavelength of the holographic pattern");  // complete la liste de infobulles de la classe
    param.flags=0; // other parameters can be optimized
    for(int i=0; i <3; ++i)
    {
        char name[32];
        sprintf(name,"constructionP1_%c",coords[i]);
        param.value=C1(i);
        defineParameter(name, param);  // par défaut 0
        setHelpstring(name, string("First construction point ")+coords[i] + " coordinate");
        sprintf(name,"constructionP2_%c",coords[i]);
        param.value=C2(i);
        defineParameter(name, param);  // par défaut 0
        setHelpstring(name, string("Second construction point ")+coords[i] + " coordinate");
    }
}


// pour éviter les doublons on ne traite ici que les paramètres propres aux réseaux holo
bool Holo::setParameter(string name, Parameter& param)
{
  //  On suppose que  la fonction Surface::setParameter() a été appelée au préalable par la classe dérivée
    bool success=true;
    if(name.compare(0,19,"recordingWavelength")==0)
        m_holoWavelength=param.value;
    else if(name.compare(0,13,"constructionP")==0)
    {
        int index;
        char cc;
        if(scanf(name.c_str(),"constructionP%d_%c",&index, &cc)==2)
        {
            if(index==1)
                C1((int)cc-(int)'X')=param.value;
            else if(index==2)
                C2((int)cc-(int)'X')=param.value;
            else
                success=false;
        }
        else
            success=false;
    }
    else
        success=false;
    if(success)
        return true;
    SetOptiXLastError("invalid grating parameter",__FILE__,__func__);
    return false;
 }


EIGEN_DEVICE_FUNC  Surface::VectorType Holo::gratingVector(Surface::VectorType position, Surface::VectorType normal)
{
 // le calcul est effectué dans le référentiel propre à la surface
    Surface::VectorType G, du;
    du=(C1-position).normalized();
    du-=(C2-position).normalized();
    G=(du-du.dot(normal)*normal);  //  projection de "du" sur le plan tangent local-
 // densité de traits = delta cos(theta)
    return G;
}
  #else
        #error HOLOGRAPHIC MODEL NOT DEFINED
  #endif // HOLOE_EULERIAN
#endif // HOLO_CARTESIAN

