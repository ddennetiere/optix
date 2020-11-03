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

EIGEN_DEVICE_FUNC  GratingBase::VectorType GratingBase::gratingVector(VectorType position)
{
    return VectorType::UnitX()*m_density;
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

    // positinne la surface par rapport à la précédente
    Parameter param;
    RayBaseType inRay=(m_previous==NULL)?RayBaseType::OX() : RayBaseType(m_previous->exitFrame().translation(), inputFrameRot.col(0) ) ;  // alignment exit Ray is normalized and its position is at previous optics
    getParameter("distance", param);
    (inRay+=param.value).rebase();  // inray a maintenant son origine à la position absolue de la surface
    m_translationFromPrevious=inRay.position();

    // central diffraction vector
    VectorType G=gratingVector(VectorType::Zero())*m_order*wavelength;

    IsometryType rayTransform;

    getParameter("phi",param);
    FloatType angle(param.value);
    rayTransform=IsometryType(AngleAxis<FloatType>(angle, inputFrameRot.col(0)));
    getParameter("Dphi",param);
    angle+=param.value;
    m_surfaceDirect=IsometryType(AngleAxis<FloatType>(angle, inputFrameRot.col(0)));

    getParameter("theta",param);
    angle=param.value;
    double omega=0 ;  // omega est la rotation du réseau autour du nouvel axe Y tourné de Phi autour du  rayon incident
    double psi=0; // angle de rotation classique autour de la normale

    if(m_transmissive) // si la surface est transmissive l'axe d'alignement de sortie reste celui d'entrée mais la rotation phi change me trièdre
        angle -=M_PI_2; // La normale pointe vers l'aval et theta donne le désalignement de la surface / l'axe d'entrée autour de Y
                /**<  \todo add an "align on exit" switch  to change the exit axis  and frame */
    else
    {
        rayTransform*=AngleAxis<FloatType>(-2.*angle, inputFrameRot.col(1)) ;
        double s=G.norm()/(2.*sin(angle)); // theta
        if(abs(s) >1.)
            return -1;  // cannot align
        omega=asin(s);
    }

    getParameter("Dtheta",param);
    angle+=omega+param.value;
    m_surfaceDirect*=AngleAxis<FloatType>(-angle, inputFrameRot.col(1)) ;  // convention déviation vers le haut si phi=0, vers l'extérieur anneau si phi=Pi/2 (M_PI_2)

    m_frameDirect=rayTransform*inputFrameRot;
    m_exitFrame.translation()=inputFrameTranslation+m_translationFromPrevious;
    m_exitFrame.linear()=m_frameDirect;
    m_frameInverse=m_frameDirect.inverse();

    param.value=psi;
    setParameter("psi",param);

    getParameter("Dpsi",param);
    angle=psi+param.value;
    m_surfaceDirect*=AngleAxis<FloatType>(angle, inputFrameRot.col(2)) ;

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
    if(!ray.m_alive)
        return ray;

    VectorType normal;
    intercept(ray, &normal);    // intercept effectue le changement de repère entrée/sortie

    VectorType G=gratingVector(ray.position())*m_order*ray.m_wavelength;
    FloatType normG=G.norm();

    if(normG < 1e-10)
        return ray;   // diffraction négligeable

    VectorType dirG=G/normG;
    VectorType T=normal.cross(dirG);
    FloatType gProj=dirG.dot(ray.direction())+normG;
    FloatType tProj=T.dot(ray.direction());
    FloatType nProj2=1.L - gProj*gProj -tProj*tProj;
    if(nProj2 >0)
        ray.direction()=gProj*dirG+ tProj*T +sqrtl(nProj2)* normal ;
    else
        ray.m_alive=false;
    return ray;
}

RayType& GratingBase::reflect(RayType& ray)
{
    if(!ray.m_alive)
        return ray;

    VectorType normal;
    intercept(ray, &normal);    // intercept effectue le changement de repère entrée/sortie

    VectorType G=gratingVector(ray.position())*m_order*ray.m_wavelength;
    FloatType normG=G.norm();

    if(normG < 1e-10)
    {
        ray.direction()-=2.*ray.direction().dot(normal)*normal;  // diffraction négligeable reflexion miroir
        return ray;
    }

    VectorType dirG=G/normG;
    VectorType T=normal.cross(dirG);
    FloatType gProj=dirG.dot(ray.direction())+normG;
    FloatType tProj=T.dot(ray.direction());
    FloatType nProj2=1.L - gProj*gProj -tProj*tProj;
    if(nProj2 >0)
        ray.direction()=gProj*dirG+ tProj*T + sqrtl(nProj2)* normal ;
    else
        ray.m_alive=false;

    return ray;
}

