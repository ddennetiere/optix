////////////////////////////////////////////////////////////////////////////////
/**
*      \file           gratingbase.cpp
*
*      \brief         Grating base class implementation
*
*      \author         François Polack <francois.polack@synchroton-soleil.fr>
*      \date        2020-10-22  Creation
*      \date        Last update
*
*/
///////////////////////////////////////////////////////////////////////////////////
//
//             REVISIONS
//
////////////////////////////////////////////////////////////////////////////////////
#include "gratingbase.h"


GratingBase::GratingBase():Surface(false, "AbstractGrating") // this should never be called since grating are never used abstract but agregated to a surface subtype
{
   m_order=0; //ctor
   m_density=1.e-6;
}

GratingBase::~GratingBase()
{
    //dtor
}

EIGEN_DEVICE_FUNC  GratingBase::VectorType GratingBase::gratingVector(VectorType position, VectorType normal)
{
    return VectorType::UnitX()*m_density;   // calcul dans le plan de description de la surface (XY), pas dans l'espace d propagation
}


int GratingBase::align(double wavelength)         /**< \todo to be validated */
{
    // retrouve ou définit l'orientation absolue du trièdre d'entrée
    RotationType inputFrameRot; // rotation part
    VectorType inputFrameTranslation;  // translation part
    if (m_previous==NULL)
    {
        inputFrameRot= RotationType::Identity() ;
        inputFrameTranslation.setZero();
    }
    else
    {
         inputFrameRot= m_previous->exitFrame().linear(); // extract linear= base rotation
         inputFrameTranslation= m_previous->exitFrame().translation();
    }

    // NB pour calculer la position de l'optique dans le repère absolu local on utilise les desaxements Rx(phi+Dphi)*Ry(-theta-Dtheta), Rz(psi+Dpsi)
    // mais pas pour calculer le le reference frame sortant Rx(phi)*Ry(-2*theta)

    // positionne la surface par rapport à la précédente
    Parameter param;
    RayBaseType inRay=(m_previous==NULL)?RayBaseType::OZ() : RayBaseType(VectorType::Zero(), inputFrameRot.col(2) ) ;  // alignment exit Ray is normalized and its position is at previous optics
    getParameter("distance", param);
    (inRay+=param.value).rebase();  // inray a maintenant son origine à la position absolue de la surface
    m_translationFromPrevious=inRay.position();



    getParameter("phi",param);
    FloatType phi(param.value);
    m_exitFrame=inputFrameRot*AngleAxis<FloatType>(phi, VectorType::UnitZ());
    getParameter("Dphi",param);
    phi+=param.value;

 // la rotation Phi doit encore être corrigée de chi avant d'être appliquée dépendant du réseau

    // central diffraction vector G

    //  gratingVector retourne le vecteur lineDensity dans le repère de définition et la rotation psi s'applique au vectur G
	getParameter("psi",param);
    FloatType psi=param.value;  // angle de rotation classique autour de la normale

    getParameter("theta",param);
    FloatType theta=param.value;
    //  rotations d'alignement du réseau pour que rayon sortant soit défini par phi et theta
    FloatType chi=0;    // Chi est l'angle de rotation autour de l'axe Z dans le repère  après rotation psi (rotation conique)
                    // l'angle chi s'ajoute à l'angle phi
    FloatType omega=0 ;  // omega est la rotation du réseau autour du nouvel axe X tourné de Phi autour du  rayon incident (rotation tangentielle)
                    // l'angle omega s'ajoute à l'angle theta
    // si la surface est transmissive l'axe d'alignement de sortie reste celui d'entrée mais la rotation phi change
    // le trièdre.   La normale pointe vers l'aval et theta donne le désalignement de la surface / l'axe d'entrée autour de OX

    if(m_transmissive)
        psiTransform=AngleAxis<FloatType>(psi,VectorType::UnitZ());
    else        // si la surface est réflective clacul de chi et omega
    {
        psiTransform=Matrix<FloatType,4,4>(m_FlipSurfCoefs);
        psiTransform*=AngleAxis<FloatType>(psi,VectorType::UnitZ());

        VectorType G=psiTransform*gratingVector(VectorType::Zero())*m_order*wavelength; // G dans leplan tangent ==> Gy==0

        m_exitFrame*=AngleAxis<FloatType>(-2.*theta,VectorType::UnitX()) ; // axe X nouveau

        G/=(2.*sin(theta)); // angle=theta  (la formule est identique en x et en Z
        if(abs(G(0)) >1. || abs(G(2))>1.)
            return -1;  // cannot align
        chi=asin(G(0));
        omega=asin(G(2));

    }

    m_surfaceDirect= IsometryType(inputFrameRot)*AngleAxis<FloatType>(phi+chi, VectorType::UnitZ()); // rot/nouveau Z

    getParameter("Dtheta",param);
    theta+=omega+param.value;
    m_surfaceDirect*=AngleAxis<FloatType>(-theta, VectorType::UnitX()) ;  // convention déviation vers le haut si phi=0, vers l'extérieur anneau si phi=Pi/2 (M_PI_2)


    m_exitFrame.translation()=inputFrameTranslation+m_translationFromPrevious;
    m_frameDirect=m_exitFrame.linear();
    m_frameInverse=m_frameDirect.inverse();


 //  ici on ne peut pas utiliser psiTransform parceque'il faut tenir compte de Dpsi

    getParameter("Dpsi",param);
    psi+=param.value;
    if(!m_transmissive)// si reflection
    {
        m_surfaceDirect*= Matrix<FloatType,4,4>(m_FlipSurfCoefs); // la surface est basculée noormale vers Y
    }

    m_surfaceDirect*=AngleAxis<FloatType>(psi, VectorType::UnitZ()) ;

    VectorType surfShift;
    getParameter("DX",param);
    surfShift(0)=param.value;
    getParameter("DY",param);
    surfShift(1)=param.value;
    getParameter("DZ",param);
    surfShift(2)=param.value;
    m_surfaceDirect.pretranslate(surfShift);
    m_surfaceInverse=m_surfaceDirect.inverse();

    return 0;
}

RayType& GratingBase::transmit(RayType& ray)
{

    VectorType normal;

    intercept(ray, &normal);    // intercept effectue le changement de repère entrée/sortie
    if(ray.m_alive)
    {
        if(m_recording==RecordInput)
            m_impacts.push_back(ray);

        VectorType G=m_surfaceDirect*gratingVector(m_surfaceInverse*ray.position())*m_order*ray.m_wavelength; // le vecteur réseau exprimé dans le repère de calcul (absolu local)
        // G par  construction est dans le plan tangent G. Normal=0

            FloatType KinPerp=normal.dot(ray.direction());
            VectorType KoutParal=ray.direction()-KinPerp*normal +G ; // = KinParal +G
            FloatType KoutPerp2= 1.L - KoutParal.squaredNorm();

            if(KoutPerp2 >0)
                ray.direction()=KoutParal+copysignl(sqrtl(KoutPerp2), KinPerp)* normal ;
            else
                ray.m_alive=false; // evanescent


        if(m_recording==RecordOutput)
            m_impacts.push_back(ray);
    }
    else if(m_recording!=RecordNone)
            m_impacts.push_back(ray);

    return ray;
}

RayType& GratingBase::reflect(RayType& ray)
{
    VectorType normal;

    intercept(ray, &normal);    // intercept effectue le changement de repère entrée/sortie
    if(ray.m_alive)
    {
        if(m_recording==RecordInput)
            m_impacts.push_back(ray);

        VectorType G=m_surfaceDirect*gratingVector(m_surfaceInverse*ray.position())*m_order*ray.m_wavelength; // le vecteur réseau exprimé dans le repère de calcul (absolu local)
        // G par  construction est dans le plan tangent G.Normal=0

            FloatType KinPerp=normal.dot(ray.direction());
            VectorType KoutParal=ray.direction()-KinPerp*normal +G ; // = KinParal +G
            FloatType KoutPerp2= 1.L - KoutParal.squaredNorm();

            if(KoutPerp2 >0)
                ray.direction()=KoutParal-copysignl(sqrtl(KoutPerp2), KinPerp)* normal ;
            else
                ray.m_alive=false; // evanescent


        if(m_recording==RecordOutput)
            m_impacts.push_back(ray);
    }
    else if(m_recording!=RecordNone)
            m_impacts.push_back(ray);

    return ray;
}


