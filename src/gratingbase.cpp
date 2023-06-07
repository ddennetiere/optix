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
//
//EIGEN_DEVICE_FUNC  Surface::VectorType Pattern::gratingVector(const Surface::VectorType &position, const  Surface::VectorType &normal)
//{
//    throw ElementException("Pattern base class virtual functions should never be called", __FILE__, __func__, __LINE__);
//    return  Surface::VectorType::Zero();   // calcul dans le plan de description de la surface (XY), pas dans l'espace d propagation
//}


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

    //mise à jour des ordres d'alignement et d'usage
    Parameter param;
    getParameter("order_align",param);
    m_alignmentOrder=param.value;
    getParameter("order_use",param);
    m_useOrder=param.value;

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
    else        // si la surface est réflective, calcul de chi et omega
    {
        VectorType G;

        psiTransform=Matrix<FloatType,4,4>(m_FlipSurfCoefs);
        psiTransform*=AngleAxis<FloatType>(psi,VectorType::UnitZ());

        // aligne sur le point 0 et la normale selon Z. S'il y a des desalignement l'intercept réel sera différent de l'alignement théorique
        G=psiTransform*gratingVector(VectorType::Zero(), VectorType::UnitZ())*m_alignmentOrder*alWavelength; // G dans le plan tangent ==> Gy==0
        // après la transformation Psi le vecteur G est tourné dans le plan XZ (Y=0)
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
    m_surfaceDirect*=AngleAxis<FloatType>(-theta, VectorType::UnitX()) ;  // convention déviation vers le haut si phi=0, vers l'extérieur anneau si phi=Pi/2 (M_PI_2)


    m_exitFrame.translation()=inputFrameTranslation+m_translationFromPrevious;
    m_frameDirect=m_exitFrame.linear();
    m_frameInverse=m_frameDirect.inverse();


 //  ici on ne peut pas utiliser psiTransform parce que'il faut tenir compte de Dpsi

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

#ifdef ALIGNMENT_DUMP
    cout << m_name <<" surface_direct:\n" << m_surfaceDirect.matrix() << endl;
    cout << "           exit frame:\n" << m_exitFrame.matrix() <<endl <<endl;
#endif // ALIGNMENT_DUMP

    return 0;
}

RayType& GratingBase::transmit(RayType& ray)
{

    VectorType normal;

    intercept(ray, &normal);    // intercept effectue le changement de repère entrée/sortie (update seulement si alive)
    if(ray.m_alive)
    {
        if(m_recording==RecordInput)
            m_impacts.push_back(ray);

        if(!inhibitApertureLimit && m_apertureActive)
        {
            Vector2d pos=(m_surfaceInverse*ray.position()).head(2).cast<double>();
            double T=m_aperture.getTransmissionAt(pos);
            ray.m_amplitude_P*=T;
            ray.m_amplitude_S*=T;
        }

        VectorType G=m_surfaceDirect*gratingVector(m_surfaceInverse*ray.position(), m_surfaceInverse*normal)*m_useOrder*ray.m_wavelength; // le vecteur réseau exprimé dans le repère de calcul (absolu local)
        // G par  construction est dans le plan tangent G. Normal=0

            FloatType KinPerp=normal.dot(ray.direction());
            VectorType KoutParal=ray.direction()-KinPerp*normal +G ; // = KinParal +G
            FloatType KoutPerp2= 1.L - KoutParal.squaredNorm();

            if(KoutPerp2 >0)
            {
                VectorType indir(ray.direction()); //memorize input direction
                ray.direction()=KoutParal+copysignl(sqrtl(KoutPerp2), KinPerp)* normal ;
                // see Surface::reflect()
                Matrix<double,3,2> pol0, pol;  // 2D frames of polarizations
                pol0.col(0)=ray.m_vector_S.cast<double>(); // S direction in exit space of previous
                pol0.col(1)=indir.cross(ray.m_vector_S).cast<double>(); // P  direction in exit space of previous

                pol.col(0)=indir.cross(normal).normalized().cast<double>();// new S direction for this element (in inpout and output spaces)
                pol.col(1)=indir.cross(ray.m_vector_S).cast<double>(); //new P direction of this element in input space
                Vector2cd A0, A;
                A0 << ray.m_amplitude_S, ray.m_amplitude_P;
                A=pol.transpose()*pol0 *A0; // les matrices pol sont unitaires
                ray.m_amplitude_S=A(0);
                ray.m_amplitude_P=A(1);
                ray.m_vector_S=ray.direction().cross(normal).normalized(); // new S direction for this element in  output space)
            }
            else
                ray.m_alive=false; // evanescent


        if(m_recording==RecordOutput)
            m_impacts.push_back(ray);
    }
    else if(m_recording)
            m_impacts.push_back(ray);

    if(m_OPDvalid && m_recording)
            m_OPDvalid=false;

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

        if(!inhibitApertureLimit && m_apertureActive)
        {
            Vector2d pos=(m_surfaceInverse*ray.position()).head(2).cast<double>();
            double T=m_aperture.getTransmissionAt(pos);
            ray.m_amplitude_P*=T;
            ray.m_amplitude_S*=T;
        }

        VectorType G=m_surfaceDirect*gratingVector(m_surfaceInverse*ray.position(), m_surfaceInverse*normal)*m_useOrder*ray.m_wavelength; // le vecteur réseau exprimé dans le repère de calcul (absolu local)
        // G par  construction est dans le plan tangent G.Normal=0

            FloatType KinPerp=normal.dot(ray.direction());
            VectorType KoutParal=ray.direction()-KinPerp*normal +G ; // = KinParal +G
            FloatType KoutPerp2= 1.L - KoutParal.squaredNorm();

            if(KoutPerp2 >0)
            {
                VectorType indir(ray.direction()); //memorize input direction
                ray.direction()=KoutParal-copysignl(sqrtl(KoutPerp2), KinPerp)* normal ;

                Matrix<double,3,2> pol0, pol;  // 2D frames of polarizations
                pol0.col(0)=ray.m_vector_S.cast<double>(); // S direction in exit space of previous
                pol0.col(1)=indir.cross(ray.m_vector_S).cast<double>(); // P  direction in exit space of previous

                pol.col(0)=normal.cross(indir).normalized().cast<double>();// new S direction for this element (in inpout and output spaces)
                pol.col(1)=indir.cross(ray.m_vector_S).cast<double>(); //new P direction of this element in input space
                Vector2cd A0, A;
                A0 << ray.m_amplitude_S, ray.m_amplitude_P;
                A=pol.transpose()*pol0 *A0; // les matrices pol sont unitaires
                ray.m_amplitude_S=A(0);
                ray.m_amplitude_P=A(1);
                ray.m_vector_S=ray.direction().cross(normal).normalized(); // new S direction for this element in  output space)
            }
            else
                ray.m_alive=false; // evanescent


        if(m_recording==RecordOutput)
            m_impacts.push_back(ray);
    }
    else if(m_recording)
            m_impacts.push_back(ray);

    if(m_OPDvalid && m_recording)
            m_OPDvalid=false;

    return ray;
}


