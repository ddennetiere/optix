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


 Holo::Holo()
{
    char coords[]={'X','Y','Z'};

    C1 << -1.,0.2,0;
    C2 << -0.5, 0.2,0;
    m_holoWavelength=2.56e-7; // 256 nm par défaut
    Parameter param;
    param.value=m_holoWavelength;
    param.type=Distance;
    param.group=GratingGroup;
    defineParameter("recordingWavelength", param);  // par défaut 0
    setHelpstring("recordingWavelength", "Recording wavelength of the holographic pattern");  // complete la liste de infobulles de la classe

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

    Surface::VectorType G, du;
    du=(position-C1).normalized();
    du-=(position-C2).normalized();
    G=(du-du.dot(normal)*normal).normalized();  // composante de delta u dans le plan tangent
    G*=du.dot(G);   // densité de traits = delta cos(theta)
    return G;
}

