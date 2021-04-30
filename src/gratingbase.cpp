////////////////////////////////////////////////////////////////////////////////
/**
*      \file           gratingbase.cpp
*
*      \brief         Grating base class implementation
*
*      \author         Fran�ois Polack <francois.polack@synchroton-soleil.fr>
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
EIGEN_DEVICE_FUNC  Surface::VectorType Pattern::gratingVector( Surface::VectorType position,  Surface::VectorType normal)
{
    throw ElementException("Pattern base class virtual functions should never be called", __FILE__, __func__, __LINE__);
    return  Surface::VectorType::Zero();   // calcul dans le plan de description de la surface (XY), pas dans l'espace d propagation
}


GratingBase::GratingBase(bool transparent, string name ,Surface * previous):Surface(transparent,name, previous)
{
    Parameter param;
    param.group=GratingGroup;
    param.flags=NotOptimizable;
    param.type=Dimensionless;
    param.value=m_alignmentOrder=m_useOrder=1;
    defineParameter("order_align", param);
    defineParameter("order_use", param);
    setHelpstring("order_align", "The grating order used for alignment ");  // complete la liste de infobulles de la classe
    setHelpstring("order_use", "The grating order used for work ");
}

GratingBase::~GratingBase()
{
    //dtor
}


// wavelength is the alignment wavelength
int GratingBase::setFrameTransforms(double alWavelength)         /**< \todo to be validated */
{

    //mise � jour des ordres d'alignement et d'usage
    Parameter param;
    getParameter("order_align",param);
    m_alignmentOrder=param.value;
    getParameter("order_use",param);
    m_useOrder=param.value;

    // retrouve ou d�finit l'orientation absolue du tri�dre d'entr�e
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

    // NB pour calculer la position de l'optique dans le rep�re absolu local on utilise les desaxements Rx(phi+Dphi)*Ry(-theta-Dtheta), Rz(psi+Dpsi)
    // mais pas pour calculer le le reference frame sortant Rx(phi)*Ry(-2*theta)

    // positionne la surface par rapport � la pr�c�dente
    RayBaseType inRay=(m_previous==NULL)?RayBaseType::OZ() : RayBaseType(VectorType::Zero(), inputFrameRot.col(2) ) ;  // alignment exit Ray is normalized and its position is at previous optics
    getParameter("distance", param);
    (inRay+=param.value).rebase();  // inray a maintenant son origine � la position absolue de la surface
    m_translationFromPrevious=inRay.position();



    getParameter("phi",param);
    FloatType phi(param.value);
    m_exitFrame=inputFrameRot*AngleAxis<FloatType>(phi, VectorType::UnitZ());
    getParameter("Dphi",param);
    phi+=param.value;

 // la rotation Phi doit encore �tre corrig�e de chi avant d'�tre appliqu�e d�pendant du r�seau

    // central diffraction vector G

    //  gratingVector retourne le vecteur lineDensity dans le rep�re de d�finition et la rotation psi s'applique au vectur G
	getParameter("psi",param);
    FloatType psi=param.value;  // angle de rotation classique autour de la normale

    getParameter("theta",param);
    FloatType theta=param.value;
    //  rotations d'alignement du r�seau pour que rayon sortant soit d�fini par phi et theta
    FloatType chi=0;    // Chi est l'angle de rotation autour de l'axe Z dans le rep�re  apr�s rotation psi (rotation conique)
                    // l'angle chi s'ajoute � l'angle phi
    FloatType omega=0 ;  // omega est la rotation du r�seau autour du nouvel axe X tourn� de Phi autour du  rayon incident (rotation tangentielle)
                    // l'angle omega s'ajoute � l'angle theta
    // si la surface est transmissive l'axe d'alignement de sortie reste celui d'entr�e mais la rotation phi change
    // le tri�dre.   La normale pointe vers l'aval et theta donne le d�salignement de la surface / l'axe d'entr�e autour de OX

    if(m_transmissive)
        psiTransform=AngleAxis<FloatType>(psi,VectorType::UnitZ());
    else        // si la surface est r�flective, calcul de chi et omega
    {
        psiTransform=Matrix<FloatType,4,4>(m_FlipSurfCoefs);
        psiTransform*=AngleAxis<FloatType>(psi,VectorType::UnitZ());

        // aligne sur le point 0 et la normale selon Z. S'il y a des desalignement l'intercept r�el sera diff�rent de l'alignement th�orique
        VectorType G=psiTransform*gratingVector(VectorType::Zero(), VectorType::UnitZ())*m_alignmentOrder*alWavelength; // G dans le plan tangent ==> Gy==0

        m_exitFrame*=AngleAxis<FloatType>(-2.*theta,VectorType::UnitX()) ; // axe X nouveau

        G/=(2.*sin(theta)); // angle=theta  (la formule est identique en x et en Z
        if(abs(G(0)) >1. || abs(G(2))>1.)
        {
            SetOptiXLastError(getName()+" grating cannot diffract the given wavelength in the given direction", __FILE__, __func__);
            return -1;  // cannot align
        }
        chi=asin(G(0));
        omega=asin(G(2));

    }

    m_surfaceDirect= IsometryType(inputFrameRot)*AngleAxis<FloatType>(phi+chi, VectorType::UnitZ()); // rot/nouveau Z

    getParameter("Dtheta",param);
    theta+=omega+param.value;
    m_surfaceDirect*=AngleAxis<FloatType>(-theta, VectorType::UnitX()) ;  // convention d�viation vers le haut si phi=0, vers l'ext�rieur anneau si phi=Pi/2 (M_PI_2)


    m_exitFrame.translation()=inputFrameTranslation+m_translationFromPrevious;
    m_frameDirect=m_exitFrame.linear();
    m_frameInverse=m_frameDirect.inverse();


 //  ici on ne peut pas utiliser psiTransform parce que'il faut tenir compte de Dpsi

    getParameter("Dpsi",param);
    psi+=param.value;
    if(!m_transmissive)// si reflection
    {
        m_surfaceDirect*= Matrix<FloatType,4,4>(m_FlipSurfCoefs); // la surface est bascul�e noormale vers Y
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

    intercept(ray, &normal);    // intercept effectue le changement de rep�re entr�e/sortie
    if(ray.m_alive)
    {
        if(m_recording==RecordInput)
            m_impacts.push_back(ray);

        VectorType G=m_surfaceDirect*gratingVector(m_surfaceInverse*ray.position(), m_surfaceInverse*normal)*m_useOrder*ray.m_wavelength; // le vecteur r�seau exprim� dans le rep�re de calcul (absolu local)
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

    intercept(ray, &normal);    // intercept effectue le changement de rep�re entr�e/sortie
    if(ray.m_alive)
    {
        if(m_recording==RecordInput)
            m_impacts.push_back(ray);

        VectorType G=m_surfaceDirect*gratingVector(m_surfaceInverse*ray.position(), m_surfaceInverse*normal)*m_useOrder*ray.m_wavelength; // le vecteur r�seau exprim� dans le rep�re de calcul (absolu local)
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

