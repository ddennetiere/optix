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
*   \date               Last update: 2021-08-09
 ***************************************************************************/#include "holo.h"

 #include "holo.h"
 #include <cmath>  // pour copysign

 #define HOLO_EULERIAN  // juste pour mieux lire

#ifdef HOLO_EULERIAN
Holo::Holo()
{
    Parameter param;
    m_lineDensity=1.e6;
    param.value=m_holoWavelength=3.511e-7; // 351.1 nm Argon line JY default  (pourquoi pas 257.3 ou 244 ?)
    param.type=Distance;
    param.group=GratingGroup;
    param.flags=NotOptimizable;
    defineParameter("recordingWavelength", param);  // 3.511e-7
    setHelpstring("recordingWavelength", "Recording wavelength of the holographic pattern");  // complete la liste de infobulles de la classe
    param.value=m_lineDensity;  // 1000 mm^-1
    param.type=InverseDistance;
    defineParameter("lineDensity", param);
    setHelpstring("lineDensity", "Line density at grating center ");  // complete la liste de infobulles de la classe
    param.flags=0; // other parameters can be optimized

    param.value=m_inverseDistance1=m_inverseDistance2=0;

    defineParameter("inverseDist1", param);  // par défaut 0
    setHelpstring("inverseDist1", "Reciprocal distance of the 1st construction point (with sign)");
    defineParameter("inverseDist2", param);  // par défaut 0
    setHelpstring("inverseDist2", "Reciprocal distance of the 2nd construction point (with sign)");
    param.type=Angle;
    param.value=0;
    defineParameter("azimuthAngle1", param);  // par défaut 0 (onde plane)
    setHelpstring("azimuthAngle1", "Azimuth angle (psi) of the 1st construction point");
    defineParameter("azimuthAngle2", param);  // par défaut 0
    setHelpstring("azimuthAngle2", "Azimuth angle (psi) of the 2nd construction point");
    param.value=-M_PI/4.5;    //   40° rasance min de JY 20° points de constructions côté entrée
    m_direction1 << cos(param.value),0,  sin(param.value);
    defineParameter("elevationAngle1", param);  // par défaut 40°
    setHelpstring("elevationAngle1", "Elevation angle (theta) of the 1st construction point");
    m_direction2(0)=m_direction1[0]-m_holoWavelength*m_lineDensity;
    m_direction2(1)=0;
    m_direction2(2)=-sqrt(1.-m_direction2(0)*m_direction2(0));
}
/** \brief solves the following equation for \f$ \theta_2 \f$ and sets the  m_direction2 vector appropriately
 *
 *  \f$ (N \lambda)^2_ = cos \theta_1 ^2 +cos \theta_2 ^2 - 2 cos \theta_1 cos \theta_2 cos( \psi_1-\psi_2) \f$
 * \return  true if the equation as a solution and the vector Direction2 was appropriately set; false otherwise
 */
bool Holo::defineDirection2()
{
    Parameter param;
    double cost1, cost2, psi2, dpsi, nl, cs;


    if(!getParameter("elevationAngle1", param))
        cout << "elev angle 1 not defined\n";
    cost1=cos(param.value);
    double signElev=copysign(1.,param.value);  // changer ce signe signifie propagation en sens inverse donc prohibé pour le calcul par contre Theta pourrait etre supérieur à Pi/2 en val absolue
    if(!getParameter("azimuthAngle1",param))
        cout << "azimuth angle 1 not defined\n";
    dpsi=param.value;
    if(!getParameter("azimuthAngle2",param))
        cout << "azimuth angle 2 not defined\n";
    psi2=param.value;
    dpsi-=psi2;
    cs=cost1*sin(dpsi);
    nl=m_lineDensity*m_holoWavelength;
    if(nl < abs(cs))
        return  false;
    cost2=cost1*cos(dpsi)-sqrt((nl+cs)*(nl-cs));   // cost1 > assumed  vrai si on suit les valeurs recommandées

//    cout << "N lambda" <<nl << " cost1 " << cost1 << " cost2 " <<cost2 << " dpsi " << dpsi  <<  " cost1-nl "<<cost1-nl<<endl;


    m_direction2 << cost2*cos(psi2) , cost2*sin(psi2),  signElev*sqrt(1.-cost2*cost2);

//    cout << "\nconstruction points\n" << 1./m_inverseDistance1 << "  " <<  m_direction1.transpose() <<endl <<
//                1./m_inverseDistance2 << "  " <<  m_direction2.transpose() << endl << endl;
    return true;
}


// pour éviter les doublons on ne traite ici que les paramètres propres aux réseaux holo
bool Holo::setParameter(string name, Parameter& param)
{
  //  On suppose que  la fonction Surface::setParameter() a été appelée au préalable par la classe dérivée
//    cout << "setting  " << name << endl;
    bool success=true;
    double psi=0, cost, sint;
    Parameter auxParam;
    if(name.compare(0,19,"recordingWavelength")==0)
        m_holoWavelength=param.value;
    else if(name.compare(0,11,"lineDensity")==0)
    {
        m_lineDensity=param.value;
        success=defineDirection2();
    }
    else if(name.compare(0,12,"inverseDist1")==0)
    {
        m_inverseDistance1=param.value;
    }
    else if(name.compare(0,12,"inverseDist2")==0)
    {
        m_inverseDistance2=param.value;
    }
    else if(name.compare(0,13,"azimuthAngle1")==0)
    {
        psi=param.value;
        getParameter("elevationAngle1", auxParam);
        cost=cos(auxParam.value);
        m_direction1 << cost*cos(psi), cost*sin(psi),sin(auxParam.value);
        success=defineDirection2();  // pour garder la même densité de traits au centre
    }
    else if(name.compare(0,15,"elevationAngle1")==0)
    {
        cost=cos(param.value);
        sint=sin(param.value);
        getParameter("azimuthAngle1", auxParam);
        m_direction1 << cost*cos(auxParam.value), cost*sin(auxParam.value), sint;
        success=defineDirection2();
    }
    else if(name.compare(0,13,"azimuthAngle2")==0)
    {
        success=defineDirection2();
    }

    else
        success=false;
//    if(!success)// modif FP  09-08-21
//        SetOptiXLastError("invalid grating parameter",__FILE__,__func__);
//        {
//            Parameter tparam;
//            getParameter("azimuthAngle1", tparam);
//            cout << "Azimuth 1  "<< tparam.value << endl;
//        }
//
//        cout<< "Direction1  " << m_direction1.transpose()<<endl;
//        cout<< "Direction2  " << m_direction2.transpose()<<endl;
    return success;
 }

Surface::VectorType Holo::gratingVector(const Surface::VectorType &position, const Surface::VectorType &normal)
{
 // le calcul est effectué dans le référentiel propre à la surface
    Surface::VectorType G, du;
    du =(m_direction1-m_inverseDistance1*position).normalized();
    du-=(m_direction2-m_inverseDistance2*position).normalized();
    G=(du-du.dot(normal)*normal)/m_holoWavelength;  //  projection de "du" sur le plan tangent local-
 // densité de traits = delta cos(theta)
    return G;
}

void Holo::getPatternInfo(double halfLength, double halfWidth, GratingPatternInfo *patInfo)
{
    // l'intercept avec la surface (orientée) doit être calculé dans le référentiel absolu local et ramené dans le référentiel surface
    Surface::VectorType org=Surface::VectorType::Zero(); // origine du rayon utilisé pour caluler l'intercept (dans le référentiel Surface)
    Surface::VectorType dir=m_surfaceDirect*Surface::VectorType::UnitZ(); // direction invariante du rayon //ON directement transformée dans repère absolu local
    Surface::VectorType position=org, normal=Surface::VectorType::UnitZ(); // dans referentiel surface pour calcul de la densité centrale
    Surface::VectorType G;
    RayBaseType ray;

    G=gratingVector(position,normal);  //grating vector at center
    double d0=G.norm();
    std::cout << "central line density:" << d0 << std::endl;
    patInfo->lineTilt=asin(G.normalized().dot(Surface::VectorType::UnitY()));

    VectorXd X=VectorXd::LinSpaced(5,-halfLength, halfLength);
    VectorXd N=VectorXd::Zero(5);
    for(Index i=0; i< X.size(); ++i)
    {
        org(0)=X(i);
        ray=RayBaseType(m_surfaceDirect*org,dir); // useless to set wavelength and other parameters
        position=m_surfaceInverse*intercept(ray,&normal);
        N(i)=gratingVector(position,normal).norm();
        cout << i << "  " << N(i) << endl;

    }

    MatrixXd A=MatrixXd::Ones(5,4);
    for(Index i=1; i <4; ++i )
        A.col(i).array()=A.col(i-1).array()*X.array();

    MatrixXd M=(A.transpose()*A).inverse();
    double* pcoeffs=(double*)&patInfo->AxialLineDensity;
    Map<VectorXd>C(pcoeffs, 4);
    C=M*A.transpose()*N;

    org(0)=0;
    org(1)=halfWidth;
    ray=RayBaseType(m_surfaceDirect*org,dir);
    position=m_surfaceInverse*intercept(ray,&normal);

    G=gratingVector(position,normal).normalized();
    org(1)=-halfWidth;
    ray=RayBaseType(m_surfaceDirect*org,dir);
    position=m_surfaceInverse*intercept(ray,&normal);
    double theta=acos(G.dot(gratingVector(position,normal).normalized()));
    patInfo->lineCurvature=2*halfWidth/cos(patInfo->lineTilt)/theta;
}

double Holo::lineNumber(const Surface::VectorType &position)
{
    FloatType N1,N2;
    N1=sqrtl(1.L-(m_direction1-m_inverseDistance1*position).norm())-1.L;
    N2=sqrtl(1.L-(m_direction2-m_inverseDistance2*position).norm())-1.L;

    return N2/(m_holoWavelength*m_inverseDistance2)-N1/(m_holoWavelength*m_inverseDistance1);
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


Surface::VectorType Holo::gratingVector(Surface::VectorType position, Surface::VectorType normal)
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

