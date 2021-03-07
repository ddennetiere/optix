/**
 *************************************************************************
*   \file            elementbase.cpp
*
*   \brief Element base class implementation file
*
*
*
*   \author             François Polack  <francois.polack@synchroton-soleil.fr>
*   \date               Creation: 2021-02-02
*   \date               Last update: 2021-02-02
 ***************************************************************************/


#include "elementbase.h"
#include "sourcebase.h"


map<string, string> ElementBase::m_helpstrings;
int ElementBase::m_nameIndex=0;

FloatType ElementBase::m_FlipSurfCoefs[]={0, 0, 1, 0,  1, 0, 0, 0,  0, 1, 0, 0,   0, 0, 0, 1 };

char LastError[256];
bool OptiXError=false;

//char* LastError=LastErrorBuffer;



ElementBase::ElementBase(bool transparent, string name, ElementBase* previous):m_name(name), m_previous(previous), m_next(NULL),
                    m_parent(NULL), m_transmissive(transparent), m_isaligned(false)
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

int ElementBase::setFrameTransforms(double wavelength)
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

    // positionne la surface par rapport à la précédente
    Parameter param;
    RayBaseType inRay=(m_previous==NULL)?RayBaseType::OZ() : RayBaseType(VectorType::Zero(), inputFrameRot.col(2) ) ;  // alignment exit Ray is normalized and its position is at previous optics
    getParameter("distance", param);
    (inRay+=param.value).rebase();  // inray a maintenant son origine à la position absolue de la surface
    m_translationFromPrevious=inRay.position();



    getParameter("phi",param);
    FloatType angle(param.value);
    m_exitFrame=inputFrameRot*AngleAxis<FloatType>(angle, VectorType::UnitZ());
    getParameter("Dphi",param);
    angle+=param.value;

    m_surfaceDirect= IsometryType(inputFrameRot)*AngleAxis<FloatType>(angle, VectorType::UnitZ()); // rot/nouveau Z

    getParameter("theta",param);
    angle=param.value;
    if(!m_transmissive) // si la surface est transmissive l'axe d'alignement de sortie reste celui d'entrée mais la rotation phi change
                // le trièdre.   La normale pointe vers l'aval et theta donne le désalignement de la surface / l'axe d'entrée autour de OX
        m_exitFrame*=AngleAxis<FloatType>(-2.*angle,VectorType::UnitX()) ; // axe X nouveau

    getParameter("Dtheta",param);
    angle+=param.value;
    /** \todo Should we keep the same sign convention on theta angle for transmissive and reflective elements ? */
    m_surfaceDirect*=AngleAxis<FloatType>(-angle, VectorType::UnitX()) ;  // convention déviation vers le haut si phi=0, vers l'extérieur anneau si phi=Pi/2 (M_PI_2)

   // m_frameDirect=rayTransform;
    m_exitFrame.translation()=inputFrameTranslation+m_translationFromPrevious;
    m_frameDirect=m_exitFrame.linear();
    m_frameInverse=m_frameDirect.inverse();


    getParameter("psi",param);
    angle=param.value;
    getParameter("Dpsi",param);
    angle+=param.value;
    if(!m_transmissive)// si reflection
    {
        m_surfaceDirect*= Matrix<FloatType,4,4>(m_FlipSurfCoefs); // la surface est basculée normale vers Y
    }

    m_surfaceDirect*=AngleAxis<FloatType>(angle, VectorType::UnitZ()) ; // rotation psi

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


void ElementBase::setHelpstrings()
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

int ElementBase::alignFromHere(double wavelength)
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

bool ElementBase::isAligned()/**< Eventuellement retourner le pointeur du 1er élément non aligné et NULL si OK */
{
    if(! m_isaligned )
        return false;
    if(m_next!=NULL)
        return m_next->isAligned();
    else
        return true;
}

 bool ElementBase::isSource()
{
    if(dynamic_cast<SourceBase*>(this))
        return true;
    else
        return false;
}

ElementBase* ElementBase::getSource()
{
    SourceBase * pSource=NULL, *ps;
    ElementBase* pSurf=m_previous;
    while(pSurf)
    {
        ps=dynamic_cast<SourceBase*>(pSurf);
        if(ps)
            pSource=ps;
        pSurf=pSurf->getPrevious();
    }
    return pSource;
}



TextFile& operator<<(TextFile& file,  ElementBase& elem)
{
    string namestr;
    file << elem.getRuntimeClass(); // <<'\0';
    file << elem.m_name;

    if(elem.m_previous)
        file << elem.m_previous->getName();
    else
        file << string();

    if(elem.m_next)
        file << elem.m_next->getName();
    else
        file << string();
//    file << '\0';

    map<string,Parameter>::iterator it;
    for(it=elem.m_parameters.begin(); it != elem.m_parameters.end(); ++it)
    {
        file << it->first  ;
        file << it->second;
    }
    file << '\0' << '\n';
    return file;
}

TextFile& operator>>(TextFile& file,  ElementBase* elem)
{
    string str;
    Parameter param;
    if (elem) delete elem;
    elem=NULL;
    file >>str; // gets runtime class
    return file;
}
