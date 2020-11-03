/**
 *************************************************************************
*   \file            surface.cpp
*
*   \brief Surface base class implementation file
*
*
*
*   \author             François Polack  <francois.polack@synchroton-soleil.fr>
*   \date               Creation: 2020-10-05
*   \date               Last update: 2020-29-05
 ***************************************************************************/


#include "surface.h"

map<string, string> Surface::m_helpstrings;
int Surface::m_nameIndex=0;



Surface::Surface(bool transparent, string name, Surface* previous):m_name(name),m_transmissive(transparent),m_recording(RecordNone),
        m_previous(previous), m_next(NULL), m_isaligned(false)
{
    Parameter param;
    param.type=Angle;
    defineParameter("theta", param);
    defineParameter("phi", param);
    defineParameter("psi", param);
    defineParameter("Dtheta", param);
    defineParameter("Dphi", param);
    defineParameter("Dpsi", param);
    param.type=Distance;
    defineParameter("distance", param);
    defineParameter("DX", param);
    defineParameter("DY", param);
    defineParameter("DZ", param);

    if (previous)
        m_previous->m_next= this;
    setHelpstrings();
    if(name.empty())
    {
        char strName[16];
        sprintf(strName,"Surface%d",m_nameIndex++);
        m_name=strName;
    }
    m_exitFrame.setIdentity();
    m_surfaceDirect.setIdentity();
    m_surfaceInverse.setIdentity();
    m_frameDirect.setIdentity();
    m_frameInverse.setIdentity();
    m_translationFromPrevious.setZero();
}

int Surface::align(double wavelength)  /**< alignement par défaut des surfaces non diffractives  \n le paramètre wavelength n'est utilisé que pour les réseaux  */
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
         inputFrameRot= m_previous->m_exitFrame.linear(); // extract linear= base rotation
         inputFrameTranslation=m_previous->m_exitFrame.translation();
    }

    // NB pour calculer la position de l'optique dans le repère absolu local on utilise les desaxements Rx(phi+Dphi)*Ry(-theta-Dtheta), Rz(psi+Dpsi)
    // mais pas pour calculer le le reference frame sortant Rx(phi)*Ry(-2*theta)

    // positinne la surface par rapport à la précédente
    Parameter param;
    RayBaseType inRay=(m_previous==NULL)?RayBaseType::OX() : RayBaseType(m_previous->m_exitFrame.translation(), inputFrameRot.col(0) ) ;  // alignment exit Ray is normalized and its position is at previous optics
    getParameter("distance", param);
    (inRay+=param.value).rebase();  // inray a maintenant son origine à la position absolue de la surface
    m_translationFromPrevious=inRay.position();

    IsometryType rayTransform;

    getParameter("phi",param);
    FloatType angle(param.value);
    rayTransform=IsometryType(AngleAxis<FloatType>(angle, inputFrameRot.col(0)));
    getParameter("Dphi",param);
    angle+=param.value;
    m_surfaceDirect=IsometryType(AngleAxis<FloatType>(angle, inputFrameRot.col(0)));

    getParameter("theta",param);
    angle=param.value;
    if(m_transmissive) // si la surface est transmissive l'axe d'alignement de sortie reste celui d'entrée mais la rotation phi change le trièdre
        angle -=M_PI_2; // La normale pointe vers l'aval et theta donne le désalignement de la surface / l'axe d'entrée autour de Y
    else
        rayTransform*=AngleAxis<FloatType>(-2.*angle, inputFrameRot.col(1)) ;

    getParameter("Dtheta",param);
    angle+=param.value;
    m_surfaceDirect*=AngleAxis<FloatType>(-angle, inputFrameRot.col(1)) ;  // convention déviation vers le haut si phi=0, vers l'extérieur anneau si phi=Pi/2 (M_PI_2)

    m_frameDirect=rayTransform*inputFrameRot;
    m_exitFrame.translation()=inputFrameTranslation+m_translationFromPrevious;
    m_exitFrame.linear()=m_frameDirect;
    m_frameInverse=m_frameDirect.inverse();


    getParameter("psi",param);
    angle=param.value;
    getParameter("Dpsi",param);
    angle+=param.value;
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


void Surface::setHelpstrings()
{
  setHelpstring("distance", "distance to previous"); // sources normally have distance=0
  setHelpstring("theta", "Chief ray half-deviation"); // transmissive surfaces shoud set the transmissive flag
  setHelpstring("phi", "Surface reference frame rotation around the incident chief ray");
  setHelpstring("psi", "Surface reference frame rotation around its normal"); // rotation order is phi, theta, psi
  setHelpstring("Dtheta", "incidence theta correction");
  setHelpstring("Dphi", "frame rotation phi correction");
  setHelpstring("Dpsi", "in-plane rotation psi correction");
  setHelpstring("DX", "X offset in surface reference frame");
  setHelpstring("DY", "Y offset in surface reference frame");
  setHelpstring("DZ", "Z offset in surface reference frame");
}




RayType& Surface::transmit(RayType& ray)
{
    if(!ray.m_alive)
        return ray;
    intercept(ray); // intercept effectue le changement de repère entrée/sortie
    return ray;
}

RayType& Surface::reflect(RayType& ray)    /**<  this implementation simply reflect the ray on the tangent plane */
{
    if(!ray.m_alive)
        return ray;

    VectorType normal;
    intercept(ray, &normal);
    ray.direction()-=2.*ray.direction().dot(normal)*normal;
    return ray;
}


void Surface::propagate(RayType& ray)
{
    if(m_recording==RecordInput)
        m_impacts.push_back(ray);
    if(m_transmissive)
        transmit(ray);
    else
        reflect(ray);
    if(m_recording==RecordOutput)
        m_impacts.push_back(ray);

    if(m_next!=NULL)
        m_next->propagate(ray);
}


void Surface::clearImpacts()
{
    m_impacts.clear();
    if(m_next!=NULL)
        m_next->clearImpacts();
}

void Surface::reserveImpacts(int n)
{
    m_impacts.reserve(n);
    if(m_next!=NULL)
        m_next->reserveImpacts(n);
}

int Surface::alignFromHere(double wavelength)
{
    int ret=align(wavelength);
    if(ret)
    {
        m_isaligned=false;
        return ret;
    }
    else
        m_isaligned=true;
    if(m_next!=NULL)
        m_next->alignFromHere(wavelength);
    return 0;
}

bool Surface::isAligned()/**< Eventuellement retourner le pointeur du 1er élément non aligné et NULL si OK */
{
    if(! m_isaligned )
        return false;
    if(m_next!=NULL)
        return m_next->isAligned();
    else
        return true;
}
