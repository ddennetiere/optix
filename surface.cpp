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
#include "sourcebase.h"

map<string, string> Surface::m_helpstrings;
int Surface::m_nameIndex=0;

FloatType Surface::m_FlipSurfCoefs[]={0, 0, 1, 0,  1, 0, 0, 0,  0, 1, 0, 0,   0, 0, 0, 1 };

TextFile& operator<<(TextFile& file, const Parameter& param)
{
//        file << param.value << '\0' << param.bounds[0] << '\0' << param.bounds[1] << '\0' << param.multiplier << '\0' <<
//            param.type << '\0' << param.group << '\0' << param.flags << '\0' ;
    file << param.value << param.bounds[0] << param.bounds[1] << param.multiplier << (uint32_t)param.type <<
                        (uint32_t)param.group << (uint32_t)param.flags;
    if(file.fail()) throw TextFileException("Error while reading Parameter from File", __FILE__, __func__, __LINE__);

    return file;
}

TextFile& operator>>( TextFile& file,  Parameter& param)
{   uint32_t t1,t2;
    file >> param.value  >> param.bounds[0] >> param.bounds[1] >> param.multiplier  >> t1 >> t2 >> param.flags;
    param.type =UnitType(t1);
    param.group =ParameterGroup(t2);

    return file;
}

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
    /**< \todo Should we keep the same sign convention on theta angle for transmissive and reflective elements ? */
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
        m_surfaceDirect*= Matrix<FloatType,4,4>(m_FlipSurfCoefs); // la surface est basculée noormale vers Y
    }

    m_surfaceDirect*=AngleAxis<FloatType>(angle, VectorType::UnitZ()) ;

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
    if(ray.m_alive)
        intercept(ray); // intercept effectue le changement de repère previous to this
    if(m_recording!=RecordNone)
            m_impacts.push_back(ray);
    return ray;
}

RayType& Surface::reflect(RayType& ray)    /**<  this implementation simply reflect the ray on the tangent plane */
{
    if(ray.m_alive)
    {
        VectorType normal;
        intercept(ray, &normal);
        if(m_recording==RecordInput)
            m_impacts.push_back(ray);
        ray.direction()-=2.*ray.direction().dot(normal)*normal;
        if(m_recording==RecordOutput)
            m_impacts.push_back(ray);
    }
    else if(m_recording!=RecordNone)
            m_impacts.push_back(ray);

    return ray;
}


 //void Surface::propagate(RayType& ray)  // has been inlined



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

vector<RayType> Surface::getImpacts(FrameID frame)
{
    vector<RayType> impacts;

    impacts.reserve(m_impacts.size());
    vector<RayType>::iterator it;
    for(it=m_impacts.begin(); it != m_impacts.end(); ++it)
    {
        RayType ray(*it);
        switch(frame)
        {
        case AlignedLocalFrame:
            ray.origin()=m_frameInverse*it->origin();
            ray.direction()=m_frameInverse*it->direction();
            break;
        case SurfaceFrame:
            ray.origin()=m_surfaceInverse*it->origin();
            ray.direction()=m_surfaceInverse*it->direction();
            break;
        case GeneralFrame:
            ray+=m_exitFrame.translation();
            break;
        case LocalAbsoluteFrame:
            break;
        }
        impacts.push_back(ray);

    }
    return impacts;
}

int Surface::getSpotDiagram(SpotDiagram& spotDiagram, double distance)
{
    if(spotDiagram.m_spots)
        delete[] spotDiagram.m_spots;

    vector<RayType> impacts=move(getImpacts(AlignedLocalFrame));
    spotDiagram.m_count=impacts.size();
    if(! spotDiagram.m_count)
    {
        spotDiagram.m_spots=NULL;
        return 0;
    }
    else
        spotDiagram.m_spots=new double[4*spotDiagram.m_count];

    Map<Matrix<double,4, Dynamic> > spotMat(spotDiagram.m_spots, 4,  spotDiagram.m_count);
    Map<Vector4d> vMean(spotDiagram.m_mean),vSigma(spotDiagram.m_sigma);
    Map<Vector4d> vMin(spotDiagram.m_min), vMax(spotDiagram.m_max);

    vector<RayType>::iterator pRay;

    RayType::PlaneType obsPlane(VectorType::UnitZ(), -distance);  // equation UnitZ*X - distance =0
    Index ip;
    for(ip=0, pRay=impacts.begin(); pRay!=impacts.end(); ++pRay, ++ip)
    {
        pRay->moveToPlane(obsPlane);
        spotMat.block<2,1>(0,ip)=pRay->position().segment(0,2).cast<double>();
        spotMat.block<2,1>(2,ip)=pRay->direction().segment(0,2).cast<double>();
    }
    vMin=spotMat.rowwise().minCoeff();
    vMax=spotMat.rowwise().maxCoeff();
    vMean=spotMat.rowwise().mean();
//        Array4d temp=spotMat.rowwise().squaredNorm()/spotDiagram.m_count;
//        vSigma=(temp-vMean.array().square()).sqrt();
    vSigma=(spotMat.rowwise().squaredNorm().array()/spotDiagram.m_count-vMean.array().square()).sqrt();
    return ip;
}

int Surface::getCaustic(CausticDiagram& causticData)
{
    if(causticData.m_spots)
    delete[] causticData.m_spots;

    vector<RayType> impacts=move(getImpacts(AlignedLocalFrame));

    if(impacts.size()==0)
    {
        causticData.m_spots;
        return 0;
    }

    Array3Xd causticMat(3,impacts.size() );

    //  minimum de distance à l'axe oz   param t= (DP.U0 + DP.U U0.U) (1-U0.U^2)  avec U0=UnitZ DP =(P-P0)= P
    //              donc t=(Pz+ P.U Uz  )(1-Uz^2)

    vector<RayType>::iterator pRay;
    Index ip;
    for(ip=0, pRay=impacts.begin(); pRay!=impacts.end(); ++pRay)
    {
        long double Ut2 = pRay->direction()[0]*pRay->direction()[0] + pRay->direction()[1]*pRay->direction()[1];
        if(Ut2 < 1e-12) continue;   //skip too small angles
        long double t=(pRay->position()[2] +  pRay->position().dot(pRay->direction())* pRay->direction()[2])/ Ut2;
        causticMat.col(ip)=pRay->position(t).cast<double>();
        ++ip;
    }
    causticData.m_count=ip;
    causticMat.conservativeResize(NoChange, ip);
    causticData.m_spots=new double[3*ip];
    Map<Array<double,3, Dynamic> > spotMat(causticData.m_spots, 3,  ip);
    Map<Vector3d> vMean(causticData.m_mean),vSigma(causticData.m_sigma);
    Map<Vector3d> vMin(causticData.m_min), vMax(causticData.m_max);

    vMin=causticMat.rowwise().minCoeff();
    vMax=causticMat.rowwise().maxCoeff();
    vMean=causticMat.rowwise().mean();
    vSigma=(causticMat.matrix().rowwise().squaredNorm().array()/ip - vMean.array().square()).sqrt();
    cout << "Min " << vMin.transpose() << "\nMax "<< vMax.transpose() << "\nMean " << vMean.transpose() << "\nSigma " << vSigma.transpose() << endl;

    causticMat.swap(spotMat);
    return ip;
}


int Surface::getWavefrontData(SpotDiagram& WFdata, double distance)
{
    if(WFdata.m_spots)
    delete[] WFdata.m_spots;

    vector<RayType> impacts=move(getImpacts(AlignedLocalFrame));

    if(impacts.size()==0)
    {
        WFdata.m_spots;
        return 0;
    }

    VectorType referencePoint= VectorType::UnitZ()*distance;

    Array4Xd WFmat(4,impacts.size() );

    //

    vector<RayType>::iterator pRay;
    Index ip;
    for(ip=0, pRay=impacts.begin(); pRay!=impacts.end(); ++pRay, ++ip)
    {

        VectorType delta=pRay->projection(referencePoint)-referencePoint;
        WFmat(0,ip)= (delta(0)*pRay->direction()(2)- delta(2)*pRay->direction()(0) ) /sqrtl(1.L-pRay->direction()(1)*pRay->direction()(1));
        WFmat(1,ip)= (delta(1)*pRay->direction()(2)- delta(2)*pRay->direction()(1) ) /sqrtl(1.L-pRay->direction()(0)*pRay->direction()(0));
        WFmat.block<2,1>(2,ip)= pRay->direction().segment(0,2).cast<double>();

    }
    return ip;
}


TextFile& operator<<(TextFile& file,  Surface& surface)
{
    file << surface.getRuntimeClass(); // <<'\0';
    file << surface.m_name;
    if(surface.m_previous)
        file << surface.m_previous->getName() ;
    else
        file << string();


    if(surface.m_next)
        file << surface.m_next->getName();
    else
        file << string();
//    file << '\0';


    map<string,Parameter>::iterator it;
    for(it=surface.m_parameters.begin(); it != surface.m_parameters.end(); ++it)
    {
        file << it->first  ;
        file << it->second;
    }
    file << '\0' << '\n';
    return file;
}

 bool Surface::isSource()
{
    if(dynamic_cast<SourceBase*>(this))
        return true;
    else
        return false;
}

Surface* Surface::getSource()
{
    SourceBase *pSource, *ps;
    Surface* pSurf=m_previous;
    while(pSurf)
    {
        ps=dynamic_cast<SourceBase*>(pSurf);
        if(ps)
            pSource=ps;
        pSurf=pSurf->m_previous;
    }
}
