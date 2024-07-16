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
*   \date               Last update: 2021-02-03
 ***************************************************************************/


#include "surface.h"
//#include "sourcebase.h"
#include "wavefront.h" // needed from Legendre fits of optical surfaces and wave-fronts
#include "fractalsurface.h" // needed to generate surface errors
#include <exception>

#define NFFT_PRECISION_DOUBLE
#include <nfft3mp.h>
extern void Init_Threads();



/** \brief helper functor to extract the coefficient-wise minimum value of two arrays
 */
template<typename Scalar_>  struct minOf2Op
{
  const Scalar_ operator()(const Scalar_& x, const Scalar_& y) const { return x < y ? x : y; }
};

void Surface::applyPerturbation(Vector2d& spos, RayType& ray, VectorType& normal)
{
    if(!m_errorMap)
        return;  // if mistakenly called while no interpolator is defined , will apply no perturbation
    VectorType deltaN=VectorType::Zero();  // - deltaN will be the perturbation of the normal  in surface pane
    Vector2d grad;

    double z= m_errorMap->valueGradient(spos(0),spos(1),grad);
    switch (m_errorMethod) {
    case SimpleShift:
        ray.moveTo(z/ray.direction().dot(normal)).rebase(); //new intercept then actualize spos and grad
        spos=(m_surfaceInverse*ray.position()).head(2).cast<double>();
        m_errorMap->valueGradient(spos(0),spos(1),grad);
    case LocalSlope:   //proceed to normal correction
        break;
    case SurfOffset:
        {   //instead of changing the surface equation we move the ray by z along the normal
            VectorType SurfShift=z*normal;
            ray-=SurfShift; // no need to rebase, m_distance remains 0
            try{
                intercept(ray, &normal); // compute shifted intercept
            }catch(...) {
                throw_with_nested(InterceptException(string("Intercept exception in " )+  m_name + " catch from "  ,
                            __FILE__, __func__, __LINE__));
            }
            ray+=SurfShift;  //switch back to unshifted space and compute the new spos
            spos=(m_surfaceInverse*ray.position()).head(2).cast<double>();
            if(m_errorMap->isValid(spos)) // if inside limits compute the normal correction
                m_errorMap->valueGradient(spos(0),spos(1),grad);
            else
                return;
        }
        break;//proceed to normal correction
    default:
        throw RayException(string("Invalid error method identifier, within ") +  m_name + " from " ,
                        __FILE__, __func__, __LINE__);
    }
    // normal correction is the same in all cases
    deltaN.head(2)=grad.cast<FloatType>(); // convert to long-double
    normal-=m_surfaceDirect*deltaN;   // in principle we should normalize  is it required ?
}


RayType& Surface::transmit(RayType& ray)
{
    ray-=m_translationFromPrevious;
    intercept(ray); // intercept n'effectue  pas le changement de repère previous to this. The position is updated only if the ray is alive
    if(ray.m_alive)
    {
        if(m_recording==RecordInput)
            m_impacts.push_back(ray);

        if(enableApertureLimit && m_apertureActive)
        {
            Vector2d pos=(m_surfaceInverse*ray.position()).head(2).cast<double>();
            double T=m_aperture.getTransmissionAt(pos);
            ray.m_amplitude_P*=T;
            ray.m_amplitude_S*=T;
        }

        if(m_recording==RecordOutput)
            m_impacts.push_back(ray);
    }
    else if(m_recording)
        m_impacts.push_back(ray);

    if(m_OPDvalid && m_recording)
        m_OPDvalid=false;

    return ray;
}

RayType& Surface::reflect(RayType& ray)    /*  this implementation simply reflect the ray on the tangent plane at intercept position*/
{
        ray-=m_translationFromPrevious;
        VectorType normal;
    try{
        intercept(ray, &normal);
    }catch(...) {
            throw_with_nested(InterceptException(string("Intercept exception in " )+  m_name + " catch from "  ,
                        __FILE__, __func__, __LINE__));
    }
//    catch(EigenException & eigexcpt) {
//        throw InterceptException(eigexcpt.what()+"\nEigenException within  " +  m_name + " from "  ,
//                                        __FILE__, __func__, __LINE__);
//    }catch(RayException & excpt)
//    {
//        throw InterceptException(excpt.what()+"\nRayException within " +  m_name + " from "  ,
//                                        __FILE__, __func__, __LINE__);
//    }catch(...)
//    {
//        throw InterceptException(string("Unknown Exception within ") +  m_name + " from" ,
//                                        __FILE__, __func__, __LINE__);
//    }
    try{
        if(ray.m_alive)
        {
            if(m_recording==RecordInput)
                m_impacts.push_back(ray);
            // find pos in surface frame 2024/05/27 moved out of aperture case as needed also by surf.errors
            Vector2d spos=(m_surfaceInverse*ray.position()).head(2).cast<double>();

            // if surface aerrors are active we must take care of the local normal (and Z) perturbation
            if(m_errorMap && enableSurfaceErrors && m_errorMethod )
            {   //we use pos in surface frame check if ray is inside the definition area
                if( m_errorMap->isValid(spos))
                    applyPerturbation(spos, ray, normal);
                if( m_errorMap->isValid(spos)) // spos might be changed by applyPerturbation
                {
                    ray.m_amplitude_P=0; //amplitude are nulled but ray is still propagated without perturbation
                    ray.m_amplitude_S=0;
                }

            }
            // aperture takes into account a possible shift due to surface errors, so actual spos
            if(enableApertureLimit && m_apertureActive)
            {
                double T=m_aperture.getTransmissionAt(spos);
                ray.m_amplitude_P*=T;
                ray.m_amplitude_S*=T;
            }

            VectorType indir(ray.direction());  // we need to make a full copy here
            double singrazing=-ray.direction().dot(normal);
            ray.direction()+=2.*singrazing*normal;

// On convertit la polarisation dans l'espace d'entrée. les polarisation frames sont toujours alignées ou tournées autour du rayon entrant
// si les surfaces sont alignées la transformation pol.transpose()*pol0 est l'identité
            Matrix<double,3,2> pol0, pol;  // 2D frames of polarizations
            pol0.col(0)=ray.m_vector_S.cast<double>(); // S direction in exit space of previous
            pol0.col(1)=indir.cross(ray.m_vector_S).cast<double>(); // P  direction in exit space of previous

            ray.m_vector_S=normal.cross(indir).normalized(); // new S direction for this element (in input and output spaces)
            pol.col(0)=ray.m_vector_S.cast<double>();
            pol.col(1)=indir.cross(ray.m_vector_S).cast<double>(); //new P direction of this element in input space
            Vector2cd A0, A;
            A0 << ray.m_amplitude_S, ray.m_amplitude_P;
            A=pol.transpose()*pol0 *A0; // les matrices pol sont unitaires
            ray.m_amplitude_S=A(0);
            ray.m_amplitude_P=A(1);

            if(m_recording==RecordOutput)
                m_impacts.push_back(ray);
        }
        else if(m_recording)
                m_impacts.push_back(ray);

        if(m_OPDvalid && m_recording)
            m_OPDvalid=false;

        return ray;
    } catch(EigenException & eigexcpt) {
        throw RayException(eigexcpt.what()+"\nEigenException within " +  m_name + " from "  ,
                                        __FILE__, __func__, __LINE__);
    }catch(RayException & excpt)
    {
        throw RayException(excpt.what()+"\nRayException within " +  m_name + " rfrom "  ,
                                        __FILE__, __func__, __LINE__);
    }catch(...)
    {
        throw RayException(string("Unknown Exception catch in ") +  m_name + " r" ,
                                        __FILE__, __func__, __LINE__);
    }

}

#define XMLSTR (xmlChar*)

void Surface::operator>>(xmlNodePtr elemnode)
{
//    cout << "Entering Surface::operator>>()\n";
    if(m_recording)
        xmlNewProp (elemnode, XMLSTR "rec", XMLSTR std::to_string(m_recording).c_str());
    if(m_errorMethod)
        xmlNewProp(elemnode, XMLSTR "error_method", XMLSTR std::to_string(m_errorMethod).c_str());
    if(hasParameter("error_limits"))
    {
//        cout << "Surface Error generator active\n";
        xmlNewProp(elemnode, XMLSTR "error_generator", XMLSTR "on");
    }
//    else
//        cout << "Surface Error generator in-active\n";
    m_aperture >> elemnode;  // does nothing if region.size() == 0
//    if(m_errorGenerator)     // seulement si le pointeur est valide
//        *m_errorGenerator >> elemnode;
}

void Surface::operator<<(xmlNodePtr surfnode)
{
    xmlChar* sprop= xmlGetProp(surfnode, XMLSTR "rec");
    if(sprop)
    {
        setRecording((RecordMode)atoi((char*)sprop));
        xmlFree(sprop);
    }
    sprop= xmlGetProp(surfnode, XMLSTR "error_method");
    if(sprop)
    {
        setErrorMethod((ErrorMethod)atoi((char*)sprop));
        xmlFree(sprop);
    }
    sprop= xmlGetProp(surfnode, XMLSTR "error_generator");
    if(sprop)
    {
        setErrorGenerator();
        xmlFree(sprop);
    }

    // set aperture if aperture child exists in children
    xmlNodePtr curnode=xmlFirstElementChild(surfnode);
    while(curnode)
    {
        if(xmlStrcmp(curnode->name, XMLSTR "aperture")==0)
        {
//            cout << "loading aperture\n";
            m_aperture << curnode;
//            cout <<"aperture loaded\n";
        }

//        if(xmlStrcmp(curnode->name, XMLSTR "error_generator")==0)
//        {
//            if(!m_errorGenerator)    // if normally used to load a system, m_errorGenerator should be always NULL
//                m_errorGenerator=new SurfaceErrorGenerator;
//            *m_errorGenerator << curnode;
//        }

        curnode=xmlNextElementSibling(curnode);
    }

}


void Surface::clearImpacts()
{
    m_impacts.clear();
    m_OPDvalid=false;
    m_lostCount=0;
    if(m_next!=NULL)
    {
        Surface* psurf=dynamic_cast<Surface*>(m_next);
        if(psurf)
            psurf->clearImpacts();
        else    //this is a group
            throw ElementException("Group object not implemented", __FILE__,__func__);  // should call here clearImpacts of the group class
    }
}

void Surface::reserveImpacts(int n)
{
    if(m_recording!=RecordNone) // added 18/05/23 (no reason to reserve space for non recording surfaces)
        m_impacts.reserve(n);
    if(m_next!=NULL)
        dynamic_cast<Surface*>(m_next)->reserveImpacts(n);
}

int Surface::getImpacts(vector<RayType> &impacts, FrameID frame)
{
    int lostCount=0;

    impacts.reserve(m_impacts.size());
    vector<RayType>::iterator it;
    for(it=m_impacts.begin(); it != m_impacts.end(); ++it)
    {
//        RayType ray(*it);
        RayType ray=*it;
        if(!ray.m_alive)
        {
            ++lostCount;
            continue; // invalid rays are not returned but counted as "lost" . The alive ray count is given by impacts.size()
                    // NB in principle the total number of rays dead and alive can be obtained by impacts.capacity() unless shrink_to_fit is called
        }
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
    return lostCount;
}

int Surface::getSpotDiagram(Diagram & spotDiagram, double distance)
{
//    if(spotDiagram.m_spots)
//        delete[] spotDiagram.m_spots;
//    cout << "getting diagram of  "  << m_name <<  " n " << m_impacts.size() << "  mem " << &m_impacts[0] << endl;

    if(spotDiagram.m_dim < 4)
        throw std::invalid_argument("SpotDiagram argument should have a vector dimension of at least 4");


    vector<RayType> impacts;
    spotDiagram.m_lost=getImpacts(impacts,AlignedLocalFrame);

    spotDiagram.m_count=impacts.size();
    if(spotDiagram.m_count==0)
        return 0;
    if(spotDiagram.m_spots)  // buffer is allocated
        if(spotDiagram.m_reserved < spotDiagram.m_count) // too small ?
        {
            delete [] spotDiagram.m_spots;
            spotDiagram.m_spots=0;
        }
    if(! spotDiagram.m_spots)
    {
        spotDiagram.m_spots=new double[spotDiagram.m_dim *spotDiagram.m_count];
        spotDiagram.m_reserved = spotDiagram.m_count;
    }

    Map<Matrix<double,Dynamic, Dynamic> > spotMat(spotDiagram.m_spots, spotDiagram.m_dim,  spotDiagram.m_count);
    Map<VectorXd> vMean(spotDiagram.m_mean, spotDiagram.m_dim),vSigma(spotDiagram.m_sigma,spotDiagram.m_dim);
    Map<VectorXd> vMin(spotDiagram.m_min,spotDiagram.m_dim), vMax(spotDiagram.m_max,spotDiagram.m_dim);

    vector<RayType>::iterator pRay;

    RayType::PlaneType obsPlane(VectorType::UnitZ(), -distance);  // equation UnitZ*X - distance =0
    Index ip;
    for(ip=0, pRay=impacts.begin(); pRay!=impacts.end(); ++pRay)
    {
        if(pRay->m_alive)  // cette précaution est inutile getImpact ne retourne que des rayons valides (et seulement le compt des rayons perdus)
        {
            pRay->moveToPlane(obsPlane);
            spotMat.block<2,1>(0,ip)=pRay->position().segment(0,2).cast<double>();
            spotMat.block<2,1>(2,ip)=pRay->direction().segment(0,2).cast<double>();

            double Is,Ip;
            complex<double> prod;
            if(spotDiagram.m_dim>4)
                spotMat(4,ip)=pRay->m_wavelength;
            if(spotDiagram.m_dim>5)
            {
                Is=norm(pRay->m_amplitude_S);
                Ip=norm(pRay->m_amplitude_P);
                spotMat(5,ip)=Is+Ip;
            }
            if(spotDiagram.m_dim>8)
            {
                prod=conj(pRay->m_amplitude_S)*pRay->m_amplitude_P;
                spotMat(6,ip)=Is-Ip;
                spotMat(7,ip)=prod.real();
                spotMat(8,ip)=prod.imag();
            }

            ++ip;
        }
    }
    vMin=spotMat.rowwise().minCoeff();
    vMax=spotMat.rowwise().maxCoeff();
    vMean=spotMat.rowwise().mean();
//        Array4d temp=spotMat.rowwise().squaredNorm()/spotDiagram.m_count;
//        vSigma=(temp-vMean.array().square()).sqrt();
    vSigma=(spotMat.rowwise().squaredNorm().array()/spotDiagram.m_count-vMean.array().square()).sqrt();
    return ip;
}

Tensor<int32_t,3> Surface::getFocalDiagram(const int dims[3], const double zbound[2], double* xbound, double * ybound)
{
    Tensor<int32_t,3> diagram;
    vector<RayType> impacts;
    /*int lost=*/ getImpacts(impacts,AlignedLocalFrame);

    int spotcount=impacts.size();
    if(spotcount ==0)
        return diagram;

    diagram.resize(dims[0], dims[1], dims[2]);
    diagram.setZero();
//    RayType::PlaneType obsPlane(VectorType::UnitZ(), 0);

    Matrix<FloatType,2,Dynamic> pos(2,spotcount), dir(2,spotcount);
    Index ip;
    vector<RayType>::iterator pRay;
    for(ip=0, pRay=impacts.begin(); pRay!=impacts.end(); ++pRay)
    {
        if(pRay->m_alive) // useless precaution
        {
//            pRay->moveToPlane(obsPlane);
            pos.col(ip)=pRay->position().segment(0,2);
            dir.col(ip)=pRay->direction().segment(0,2)/pRay->direction()(2);
            ++ip;
        }
    }

    Vector<FloatType,Dynamic> zval=Vector<FloatType, Dynamic>::LinSpaced(dims[2],zbound[0],zbound[1]);
    Array<FloatType,Dynamic,Dynamic> matx, maty;
//    cout << "zval\n" << zval.transpose() << endl;

    matx=(zval*dir.row(0)).array().rowwise()+pos.row(0).array(); //  xstep).round().cast<int>();
    maty=(zval*dir.row(1)).array().rowwise()+pos.row(1).array(); // /ystep).round().cast<int>();
//    cout << "size="<< matx(seqN(0, 2, nz-1),all).size() <<endl;
    xbound[0]=matx(seqN(0, 2, dims[2]-1),all).minCoeff() ;
    xbound[1]=matx(seqN(0, 2, dims[2]-1),all).maxCoeff() ;
    ybound[0]=maty(seqN(0, 2, dims[2]-1),all).minCoeff() ;
    ybound[1]=maty(seqN(0, 2, dims[2]-1),all).maxCoeff() ;

    double xstep=(xbound[1]-xbound[0])/(dims[0]-1);
    double ystep=(ybound[1]-ybound[0])/(dims[1]-1);
    cout <<"X " << xbound[0] <<", " << xbound[1] << "  step " <<  xstep << endl;
    cout <<"y " << ybound[0] <<", " << ybound[1] << "  step " <<  ystep << endl;
    matx=(matx-xbound[0])/xstep;
    maty=(maty-ybound[0])/ystep;

    Index i,iz=0;
    for( i=0; i< spotcount; ++i)
    {
        for(iz=0; iz <dims[2]; ++iz)
        {
            int ix=(int)round(matx(iz,i));
            int iy=(int)round(maty(iz,i));
//            if(iy <0 || ix >=dims[0] || iy <0 || iy >=dims[1])
//                cout << "index overflow (" << iz << ", " <<ix << ", " <<iy <<")\n";
//            else
               ++ diagram(ix,iy,iz);
        }
    }
//    cout << endl << i << "   " << iz << endl;
    return diagram;
}


int Surface::getImpactData(Diagram &impactData, FrameID frame)
{
 //   cout << "getting diagram of  "  << m_name <<  " n " << m_impacts.size() << "  mem " << &m_impacts[0] << endl;

//    vector<RayType> impacts;
//    impactData.m_lost=getImpacts(impacts,LocalAbsoluteFrame);
    if(impactData.m_dim < 6)
        throw std::invalid_argument("Impact data argument should have a vector dimension of at least 6");

    impactData.m_count=m_impacts.size();
    if(impactData.m_count==0)
        return 0;
    if(impactData.m_spots)  // buffer is allocated
        if(impactData.m_reserved < impactData.m_count) // too small ?
        {
            delete [] impactData.m_spots;
            impactData.m_spots=0;
        }
    if(! impactData.m_spots)
    {
        impactData.m_spots=new double[impactData.m_dim *impactData.m_count];
        impactData.m_reserved = impactData.m_count;
    }

    Map<Matrix<double,Dynamic, Dynamic> > spotMat(impactData.m_spots, impactData.m_dim,  impactData.m_count);
    Map<VectorXd> vMean(impactData.m_mean, impactData.m_dim),vSigma(impactData.m_sigma,impactData.m_dim);
    Map<VectorXd> vMin(impactData.m_min,impactData.m_dim), vMax(impactData.m_max,impactData.m_dim);

    vector<RayType>::iterator pRay;
    Index ip;
    impactData.m_lost=0;
    for(ip=0, pRay=m_impacts.begin(); pRay!=m_impacts.end(); ++pRay)
    {
        if(pRay->m_alive)
        {
            switch(frame)
            {
            case AlignedLocalFrame:  // if impacts are correctly computed position is identical to origin (rebased on surface)
                spotMat.block<3,1>(0,ip)=(m_frameInverse*pRay->origin()).cast<double>();
                spotMat.block<3,1>(3,ip)=(m_frameInverse*pRay->direction()).cast<double>();
                break;
            case SurfaceFrame:
                spotMat.block<3,1>(0,ip)=(m_surfaceInverse*pRay->origin()).cast<double>();
                spotMat.block<3,1>(3,ip)=(m_surfaceInverse*pRay->direction()).cast<double>();
                break;
            case GeneralFrame:
                spotMat.block<3,1>(0,ip)=(pRay->origin()+m_exitFrame.translation()).cast<double>();
                spotMat.block<3,1>(3,ip)=pRay->direction().cast<double>();
                break;
            case LocalAbsoluteFrame:
                spotMat.block<3,1>(0,ip)=pRay->origin().cast<double>();
                spotMat.block<3,1>(3,ip)=pRay->direction().cast<double>();
                break;
            }
            double Is,Ip;
            complex<double> prod;
            if(impactData.m_dim>6)
                spotMat(6,ip)=pRay->m_wavelength;
            if(impactData.m_dim>7)
            {
                Is=norm(pRay->m_amplitude_S);   // atn norm(complex<double) retourne la magnitude square, alors que norm(Eigen::Matrix) retourne la norme euclidienne= sqrt(mag square)
                Ip=norm(pRay->m_amplitude_P);
                spotMat(7,ip)=Is+Ip;
            }
            if(impactData.m_dim>10)
            {
                prod=conj(pRay->m_amplitude_S)*pRay->m_amplitude_P;
                spotMat(8,ip)=Is-Ip;
                spotMat(9,ip)=prod.real();
                spotMat(10,ip)=prod.imag();
            }

            ++ip;
        }
        else
            ++impactData.m_lost;
    }

    vMin=spotMat.rowwise().minCoeff();
    vMax=spotMat.rowwise().maxCoeff();
    vMean=spotMat.rowwise().mean();

    vSigma=(spotMat.rowwise().squaredNorm().array()/impactData.m_count-vMean.array().square()).sqrt();
    return ip;
}

int Surface::getCaustic(Diagram& causticData)
{
//    if(causticData.m_spots)
//        delete[] causticData.m_spots;
    if(causticData.m_dim < 4)
        throw std::invalid_argument("Caustic data argument should have a vector dimension of at least 4");
    vector<RayType> impacts;
    causticData.m_lost=getImpacts(impacts,AlignedLocalFrame);
//    if(impacts.size()==0)
//    {
//        causticData.m_spots=NULL;
//        return 0;
//    }

    ArrayXXd causticMat(causticData.m_dim,impacts.size() );

    //  minimum de distance à l'axe oz   param t= (DP.U0 + DP.U U0.U) (1-U0.U^2)  avec U0=UnitZ DP =(P-P0)= P
    //              donc t=(Pz+ P.U Uz  )(1-Uz^2)

    vector<RayType>::iterator pRay;
//    causticData.m_dropped=0;
    Index ip;
    for(ip=0, pRay=impacts.begin(); pRay!=impacts.end(); ++pRay)
    {
        long double Ut2 = pRay->direction()[0]*pRay->direction()[0] + pRay->direction()[1]*pRay->direction()[1];
        if(Ut2 < 1e-12)
        {
//            ++causticData.m_dropped;
            continue;   //skip too small angles
        }
        // calcule l'éloignement du point où le rayon est le plus proche de l'axe d'alignement (OZ dans le repère local)
        long double t=(pRay->position()[2] +  pRay->position().dot(pRay->direction())* pRay->direction()[2])/ Ut2;
        causticMat.block<3,1>(0,ip)=pRay->position(t).cast<double>();
        causticMat(3,ip)=pRay->m_wavelength;
        ++ip;
    }

    causticMat.conservativeResize(NoChange, ip);

    causticData.m_count=ip;

    if(ip==0)
        return 0;
    if(causticData.m_spots)  // buffer is allocated
        if(causticData.m_reserved < ip) // too small ?
        {
            delete [] causticData.m_spots;
            causticData.m_spots=0;
        }
    if(! causticData.m_spots)
    {
        causticData.m_spots=new double[causticData.m_dim*ip];
        causticData.m_reserved = ip;
    }

    Map<Array<double,Dynamic, Dynamic> > spotMat(causticData.m_spots, causticData.m_dim,  ip);
    Map<VectorXd> vMean(causticData.m_mean, causticData.m_dim),vSigma(causticData.m_sigma, causticData.m_dim);
    Map<VectorXd> vMin(causticData.m_min,causticData.m_dim), vMax(causticData.m_max, causticData.m_dim);

    vMin=causticMat.rowwise().minCoeff();
    vMax=causticMat.rowwise().maxCoeff();
    vMean=causticMat.rowwise().mean();
    vSigma=(causticMat.matrix().rowwise().squaredNorm().array()/ip - vMean.array().square()).sqrt();
    cout << "Min " << vMin.transpose() << "\nMax "<< vMax.transpose() << "\nMean " << vMean.transpose() << "\nSigma " << vSigma.transpose() << endl;

    causticMat.swap(spotMat);
    return ip;
}


int Surface::getWavefrontData(Diagram & WFdata, double distance)
{
    if(WFdata.m_dim < 5)
        throw std::invalid_argument("WavefrontData argument should have a vector dimension of at least 5");

    if(WFdata.m_spots)
    delete[] WFdata.m_spots;

    vector<RayType> impacts;
    WFdata.m_lost=getImpacts(impacts,AlignedLocalFrame);

    if(impacts.size()==0)
    {
        WFdata.m_spots=NULL;
        return 0;
    }

    VectorType referencePoint= VectorType::UnitZ()*distance;
    Array4Xd WFmat(WFdata.m_dim, impacts.size() );

    vector<RayType>::iterator pRay;
    Index ip;
    for(ip=0, pRay=impacts.begin(); pRay!=impacts.end(); ++pRay, ++ip)
    {
        //On calcule la projectiondu point de référence sur chaque rayon. C'est l'écart aberrant.
        // celui-ci est ensuite projeté sur les deux directions de référence du plan normal à chaque rayon,
        // pour être ensuite égalées aux dérivées du front d'onde par rapport aux angles d'ouverture
        VectorType delta=pRay->projection(referencePoint)-referencePoint;
        WFmat(0,ip)= (delta(0)*pRay->direction()(2)- delta(2)*pRay->direction()(0) ) /sqrtl(1.L-pRay->direction()(1)*pRay->direction()(1));
        WFmat(1,ip)= (delta(1)*pRay->direction()(2)- delta(2)*pRay->direction()(1) ) /sqrtl(1.L-pRay->direction()(0)*pRay->direction()(0));
        WFmat.block<2,1>(2,ip)= pRay->direction().segment(0,2).cast<double>();
        WFmat(4,ip)=pRay->m_wavelength;
    }
    Map<Array<double,Dynamic, Dynamic> > WFMap(WFdata.m_spots, WFdata.m_dim,  ip);
    WFMap.swap(WFmat);
    return ip;
}

MatrixXd Surface::getWavefontExpansion(double distance, Index Nx, Index Ny, Array22d& XYbounds)
{
    MatrixXd LegendreCoefs;
    vector<RayType> impacts;
    getImpacts(impacts,AlignedLocalFrame);

    if(impacts.size()==0)
        return LegendreCoefs; //Returns a matrix whose size() is zero

    VectorType referencePoint= VectorType::UnitZ()*distance;
    ArrayX4d slopeMat(impacts.size(),4 );

    vector<RayType>::iterator pRay;
    Index ip;
    // on charge dans le tableau slope mat (cf GetWavefrontData mais attention c'est la transposée de la fonction précédente)
    // dans les colonnes 0 et 1, l'aberration transverse selon x et y (dans le plan perpendiculaire au rayon)
    // dans les colonnes 2 et 4 les coefficient directeur de la direction du rayon
    for(ip=0, pRay=impacts.begin(); pRay!=impacts.end(); ++pRay, ++ip)
    {

        VectorType delta=pRay->projection(referencePoint)-referencePoint;
        slopeMat(ip,0)= (delta(0)*pRay->direction()(2)- delta(2)*pRay->direction()(0) ) /sqrtl(1.L-pRay->direction()(1)*pRay->direction()(1));
        slopeMat(ip,1)= (delta(1)*pRay->direction()(2)- delta(2)*pRay->direction()(1) ) /sqrtl(1.L-pRay->direction()(0)*pRay->direction()(0));
        slopeMat.block<1,2>(ip,2)= pRay->direction().segment(0,2).cast<double>();  // la direction U
    }
    XYbounds.row(0)=slopeMat.block(0,2,ip,2).colwise().minCoeff(); // ici il est permis de faire min max sur les valeurs passées dans XY bounds
    XYbounds.row(1)=slopeMat.block(0,2,ip,2).colwise().maxCoeff();

    cout << "bounds\n" <<XYbounds.col(0).transpose() << endl<< XYbounds.col(1).transpose() << endl;

    LegendreCoefs=LegendreIntegrateSlopes(Nx,Ny, slopeMat, XYbounds.col(0), XYbounds.col(1));

    return LegendreCoefs;
}


void Surface::computeOPD(double distance, Index Nx, Index Ny)
{
    /**< \todo  Tester dans cette fonction si les rayons actuellement stockés sont utilisables ou non */

    /**< \todo Dans le cas où les surfaces portent des perturbations. Le fit sur legendre ne rend pas les perturbations "haute et moyenne"  fréquence.

        Peut-on recupérer les fluctuation de direction et en inférer une "rugosité" ?*/

    MatrixXd LegendreCoefs;
    vector<RayType> impacts;
    getImpacts(impacts,AlignedLocalFrame);

    if(impacts.size()< (size_t) 2*Nx*Ny)
        throw std::runtime_error("The impact number is too small to compute a valid OPD");

    VectorType referencePoint= VectorType::UnitZ()*distance;
    // the slopeMat arrays contains the data required for LegendreIntegrateteSlopes namely transverse X & Y aberration, then  X & Y aperture angles
    ArrayX4d slopeMat(impacts.size(),4 );  // impacts ne contient que les rayons 'alive'

    vector<RayType>::iterator pRay;
    Index ip;

    m_OPDdata.resize(impacts.size(),3);
    m_amplitudes.resize(impacts.size(),2);

    for(ip=0, pRay=impacts.begin(); pRay!=impacts.end(); ++pRay, ++ip)
    {

        VectorType delta=pRay->projection(referencePoint)-referencePoint;
        slopeMat(ip,0)= (delta(0)*pRay->direction()(2)- delta(2)*pRay->direction()(0) ) /sqrtl(1.L-pRay->direction()(1)*pRay->direction()(1)); // l'aberration transversale
        slopeMat(ip,1)= (delta(1)*pRay->direction()(2)- delta(2)*pRay->direction()(1) ) /sqrtl(1.L-pRay->direction()(0)*pRay->direction()(0));
        slopeMat.block<1,2>(ip,2)= pRay->direction().segment(0,2).cast<double>(); // la direction U
        m_amplitudes(ip,0)=pRay->m_amplitude_S;
        m_amplitudes(ip,1)=pRay->m_amplitude_P;
    }
    m_XYbounds.row(0)=slopeMat.block(0,2,ip,2).colwise().minCoeff(); // ici il est permis de faire min max sur les valeurs passées dans XY bounds
    m_XYbounds.row(1)=slopeMat.block(0,2,ip,2).colwise().maxCoeff();

    cout << "bounds\n" <<m_XYbounds.col(0).transpose() << endl<< m_XYbounds.col(1).transpose() << endl;

    LegendreCoefs=LegendreIntegrateSlopes(Nx,Ny, slopeMat, m_XYbounds.col(0), m_XYbounds.col(1));

    // jusqu'ici pas de différence avec la fonction  si ce n'est la sauvegarde des amplitudes complexe dans m_amplitudes

    m_OPDdata.leftCols(2)=slopeMat.rightCols(2);
    // This is the "low frequency" component o the OPD perturbation. What about the residuals ? how can they be injected in the the computation ?
    m_OPDdata.col(2)=Legendre2DInterpolate(slopeMat.col(2), slopeMat.col(3), m_XYbounds, LegendreCoefs);
    m_NxOPD=Nx;
    m_NyOPD=Ny;
    m_OPDvalid=true;
}

void Surface::computePSF(ndArray<std::complex<double>,3> &PSF, Array2d& pixelSizes, double lambda, double oversampling, double distOffset)
{
    Init_Threads(); // will initialize The MT mode if not yet done

    std::array<size_t,3> dims=PSF.dimensions();
    if(dims[2]<2)
        throw std::invalid_argument("Last dimension of argument PSF should be at least 2");
    if(oversampling <1. )
        throw std::invalid_argument("Oversampling factor should be greater than 1.");
    size_t arraysize=dims[0]*dims[1];
    Map<ArrayXXcd> Spsf(PSF.data(),dims[0], dims[1]);
    Map<ArrayXXcd> Ppsf(PSF.data()+arraysize*2*sizeof(double),dims[0], dims[1]);

    nfft_plan plan;

    nfft_init_2d(&plan,dims[1],dims[0],m_OPDdata.rows());

    Array2d unorm, ucenter;
    unorm = (m_XYbounds.row(1)-m_XYbounds.row(0))*oversampling;
    ucenter=(m_XYbounds.row(1)+m_XYbounds.row(0))/ 2. ;// unorm.transpose();

    pixelSizes=pixelSizes.binaryExpr(lambda/unorm, minOf2Op<double>()); // on garde le plus petit pixel entre oversampling et pixel demandé
    unorm=lambda/pixelSizes;  // On calcule la normalisation d'ouverture en conséquence


    //pixel=lambda/unorm;
    // if OPD is positive then the remaining path to the ref point is less.  Hence the minus sign
    complex<double> i2pi_lambda(0,-2.*M_PI/lambda);/**< \todo  Extensive checking of the phase shifts signs. \n The sign of phase shifts must be consistent throughout, with the convention that wave propagates
                *   in the form \f$ e^{i k z} \f$  */

    Map<Array2Xd> Ux(plan.x,2,m_OPDdata.rows()); // il faut normaliser et transposer les 2 premières colonnes de m_OPDdata
    Ux=(m_OPDdata.leftCols(2).transpose().colwise()-ucenter).colwise()/unorm;

    ArrayXd correctOPD=m_OPDdata.col(2);
    if(distOffset !=0)
        correctOPD+=m_OPDdata.leftCols(2).matrix().rowwise().squaredNorm().array()*distOffset/2.;  //  vrai au 2° ordre en ouverture 4° ordre possible : +norm()^2*distoffset/8

    if(plan.flags & PRE_ONE_PSI)
        nfft_precompute_one_psi(&plan);

    Map<ArrayXcd> F((complex<double>*)plan.f,m_OPDdata.rows());
    Map<ArrayXXcd> Tf((complex<double>*)plan.f_hat,dims[1],dims[0] );

    F= exp( correctOPD*i2pi_lambda)*m_amplitudes.col(0)  ;
    nfft_adjoint(&plan);
    Spsf=Tf.transpose()/m_OPDdata.rows();

    F= exp( correctOPD*i2pi_lambda)*m_amplitudes.col(1)  ;
    nfft_adjoint(&plan);
    Ppsf=Tf.transpose()/m_OPDdata.rows();

    nfft_finalize(&plan);  // ceci desalloue toute la structure plan

}

void Surface::computePSF(ndArray<std::complex<double>,4> &PSF, Array2d &pixelSizes, ArrayXd &distOffset, double lambda, double oversampling)
{
    Init_Threads(); // will initialize The MT mode if not yet done
    std::array<size_t,4> dims=PSF.dimensions();
    if(dims[3]<2)
        throw std::invalid_argument("Fourth dimension of argument PSF should be  2");
    if(oversampling <1. )
        throw std::invalid_argument("Oversampling factor should be greater than 1.");
    size_t arraysize=dims[0]*dims[1];
    size_t stacksize=arraysize*dims[2];

    nfft_plan plan;

    nfft_init_2d(&plan,dims[1],dims[0],m_OPDdata.rows()); // m_OPDdata.rows() is the number of contributing rays

    Array2d unorm, ucenter;
    unorm = (m_XYbounds.row(1)-m_XYbounds.row(0))*oversampling;
    ucenter=(m_XYbounds.row(1)+m_XYbounds.row(0))/ 2. ;// unorm.transpose();

    pixelSizes=pixelSizes.binaryExpr(lambda/unorm, minOf2Op<double>()); // on garde le plus petit pixel entre oversampling et pixel demandé
    unorm=lambda/pixelSizes;  // On calcule la normalisation d'ouverture en conséquence


    //pixel=lambda/unorm;
    // if OPD is positive then the remaining path to the ref point is less.  Hence the minus sign
    complex<double> i2pi_lambda(0,-2.*M_PI/lambda);/**< \todo  Extensive checking of the phase shifts signs. \n The sign of phase shifts must be consistent throughout, with the convention that wave propagates
                *   in the form \f$ e^{i k z} \f$  */

    //Assign the angular directions of contributing rays to  plan.x
    Map<Array2Xd> Ux(plan.x,2,m_OPDdata.rows()); // il faut normaliser et transposer les 2 premières colonnes de m_OPDdata
    Ux=(m_OPDdata.leftCols(2).transpose().colwise()-ucenter).colwise()/unorm;


    if(plan.flags & PRE_ONE_PSI)
        nfft_precompute_one_psi(&plan);

    complex<double> *pSdat=PSF.data(), *pPdat=PSF.data()+stacksize;
    for(Index ioffset=0; ioffset < distOffset.size(); ++ioffset, pSdat+=arraysize, pPdat+=arraysize )
    {
        Map<ArrayXcd> F((complex<double>*)plan.f,m_OPDdata.rows());
        Map<ArrayXXcd> Tf((complex<double>*)plan.f_hat,dims[1],dims[0] );
        Map<ArrayXXcd> Spsf(pSdat,dims[0], dims[1]);
        Map<ArrayXXcd> Ppsf(pPdat,dims[0], dims[1]);
        // ADD the quadratic phase correction corresponding to the distance to the reference point
        ArrayXd correctOPD=m_OPDdata.col(2) +
                m_OPDdata.leftCols(2).matrix().rowwise().squaredNorm().array()*distOffset(ioffset)/2.;  // approximation faible ouerture

        F= exp( correctOPD*i2pi_lambda)*m_amplitudes.col(0)  ;
        nfft_adjoint(&plan);
        Spsf=Tf.transpose()/m_OPDdata.rows();

        F= exp( correctOPD*i2pi_lambda)*m_amplitudes.col(1)  ;
        nfft_adjoint(&plan);
        Ppsf=Tf.transpose()/m_OPDdata.rows();

    }
   nfft_finalize(&plan);  // ceci desalloue toute la structure plan
}


bool Surface::setParameter(string name, Parameter& param)
{
//    cout << m_name  << " set " << name <<endl;
    // we bypass
    return ElementBase::setParameter(name, param);
    //some validation based on a sigle prameter settin
    if(name=="error_limits" )
    {
        if(!(param.flags & ArrayData))    // else  case is handled by the base class
            if(param.paramArray->dims[0] !=2 || param.paramArray->dims[1] !=2)
            {
                SetOptiXLastError(string("parameter name ")+ name + " must be a 2x2 array", __FILE__, __func__, __LINE__);
                return false;
            }
    }
    if(name=="sampling")
    {
        if(!(param.flags & ArrayData))    // else  case is handled by the base class
            if(param.paramArray->dims[0]* param.paramArray->dims[1] !=2)
            {
                SetOptiXLastError(string("parameter name ")+ name + " must be a 2x1 or 1x2 array", __FILE__, __func__, __LINE__);
                return false;
            }
    }
    if(name=="residual_sigma" && param.value <0)
    {
                SetOptiXLastError(string("parameter name ")+ name + " cannot have a negative value", __FILE__, __func__, __LINE__);
                return false;
    }
//     cout << "try to set\n";
    // record parameter new value
    if(! ElementBase::setParameter(name, param))
    {
        cout << "Error: " <<LastError << endl;
         return false;
    }

   // if one of these error generator parameter was change, invalidate the generator
    if(name=="fractal_exponent_x" || name=="fractal_frequency_x" || name=="fractal_exponent_y" || name=="fractal_frequency_y"
       || name=="error_limits" || name=="sampling" || name=="detrending" || name=="low_Zernike" )
       m_ErrorGeneratorValid=false;

   return true;
}


void Surface::setErrorGenerator()
{
    if(hasParameter("error_limits"))    // useles (and even dangerous to create
        return;

    Parameter param;
    param.type=InverseDistance;
    param.group=SurfErrorGroup;
    param.flags=NotOptimizable | ArrayData;
    param.paramArray=new ArrayParameter; //default constructor set dims to 0 and data to NULL the new arrayParameter will be deleted with param
    // the following parameter are created uninitilized that is dims=(0;0)

    defineParameter("fractal_frequency_x", param);
    setHelpstring("fractal_frequency_x", "frequency limits of the X PSD segments"); // default 1 segment not limited

    defineParameter("fractal_frequency_y", param);
    setHelpstring("fractal_frequency_y", "frequency limits of the Y PSD segments"); // default 1 segment not limited

    param.type=Distance;
    defineParameter("error_limits", param);
    setHelpstring("error_limits", "Bounds of the area where surface errors are defined (xmin, xmax, ymin, ymax)");

    defineParameter("sampling", param);
    setHelpstring("sampling", "Approximate sampling steps of the generated surface errors");

    defineParameter("low_Zernike", param);
    setHelpstring("low_Zernike", "Matrix of the max sigma values of low Legendre expansion");


    // Fractal exponents are initialized to -1.
    param.type=Dimensionless;
    delete param.paramArray ;
    param.paramArray = new ArrayParameter(1,1);
    param.paramArray->data[0]=-1.;

    defineParameter("fractal_exponent_x", param);
    setHelpstring("fractal_exponent_x", "fractal exponents of the X PSD");
    defineParameter("fractal_exponent_y", param);
    setHelpstring("fractal_exponent_y", "fractal exponents of the Y PSD");

    // detrend tip and tilts
    Array22d detrend;
    detrend << 1., 1., 1., 0 ;
    *param.paramArray=detrend;
    defineParameter("detrending", param);
    setHelpstring("detrending", "Low Legendre detrending mask");

    param.type=Distance;
    param.flags=NotOptimizable;
    param.value=0;
    defineParameter("residual_sigma", param);
    setHelpstring("residual_sigma", "RMS height error after subtraction of constrained Legendre");

    m_ErrorGeneratorValid=false;

    if(m_errorMap)
        delete m_errorMap;
    m_errorMap=NULL;
}

void Surface::unsetErrorGenerator()
{
    removeParameter("fractal_exponent_x");
    removeParameter("fractal_frequency_x");
    removeParameter("fractal_exponent_y");
    removeParameter("fractal_frequency_y");
    removeParameter("error_limits");
    removeParameter("sampling");
    removeParameter("detrending");
    removeParameter("low_Zernike");
    removeParameter("residual_sigma");
}

bool Surface::validateErrorGenerator()
{
    if(m_ErrorGeneratorValid) // don't test again if no parameter was changed
        return true;

    m_ErrorGeneratorValid=true;
    Parameter param;
    string errorstring="Incorrect initialization of surface ";
    errorstring+=m_name+ " :\n";

    char format[256]="the number of %s fractal transition frequencies (%d) doesn't match the number of exponents (%d)\n";
    char buf[256];
    getParameter("fractal_exponent_x", param) ;
    int nexp=param.paramArray->dims[0]*param.paramArray->dims[1];
    getParameter("fractal_frequency_x", param) ;
    int nfreq=param.paramArray->dims[0]*param.paramArray->dims[1];
    if(!nexp)
    {
        errorstring+="No X fractal exponent defined \n";
        m_ErrorGeneratorValid=false;
    }
    if(nfreq <nexp-1)
    {
        sprintf(buf,format, "X", nfreq ,nexp);
        errorstring+=buf;
        m_ErrorGeneratorValid=false;
    }

    getParameter("fractal_exponent_y", param) ;
    nexp=param.paramArray->dims[0]*param.paramArray->dims[1];
    getParameter("fractal_frequency_y", param) ;
    nfreq=param.paramArray->dims[0]*param.paramArray->dims[1];
    if(!nexp)
    {
        errorstring+="No Y fractal exponent defined \n";
        m_ErrorGeneratorValid=false;
    }
    if(nfreq <nexp-1)
    {
        sprintf(buf,format, "Y", nfreq ,nexp);
        errorstring+=buf;
        m_ErrorGeneratorValid=false;
    }

    Vector4d Limits;
    Vector2d steps;
    strcpy(format,"%s parameter should contain %d elements, %d given\n");
    string name="error_limits";
    int dim=4;
    for(int i=0; i<2; ++i)
    {
        getParameter(name, param) ;
        int n=param.paramArray->dims[0]*param.paramArray->dims[1];
        if(n!=dim)
        {
            sprintf(buf,format, name,  dim ,n);
            errorstring+=buf;
            m_ErrorGeneratorValid=false;
        }
        else if(i==0) //sampling has 4 values
            Limits=Map<Vector4d>(param.paramArray->data);
        else // sampling has 2 values
            steps=Map<Vector2d>(param.paramArray->data);

        name="sampling"; dim=2;
    }

    int xpoints(round((Limits(1)-Limits(0))/steps(0)));  // this is the interva number
    int ypoints(round((Limits(3)-Limits(2))/steps(1)));
    double xstep=(Limits(1)-Limits(0))/xpoints++;
    double ystep=(Limits(3)-Limits(2))/ypoints++;

    cout << "The generated error map is defined on " << xpoints << " x " << ypoints << " soit " <<  xpoints*ypoints << " points\n";
    if( xpoints < 10 || ypoints < 8 )
    {
        errorstring+="The number of points is abnormally small, check 'surface_limits' and 'sampling' parameters\n";
        m_ErrorGeneratorValid=false;
    }
    else//param still contains sampling we can put the real value in the sampling parameter
    {
        param.paramArray->data[0]=xstep;
        param.paramArray->data[1]=ystep;
        ElementBase::setParameter("sampling", param);   // here  we call the elementbase method to avoid invalidation
    }

    getParameter("detrending",param);
    int Nx=param.paramArray->dims[0];
    int Ny=param.paramArray->dims[1];
    getParameter("low_Zernike",param);
    Nx = Nx > param.paramArray->dims[0] ? Nx : param.paramArray->dims[0];
    Ny = Ny > param.paramArray->dims[1] ? Ny : param.paramArray->dims[1];
    if (Nx >= xpoints || Ny >= ypoints )
    {
        errorstring+="The number of interpolation grid points is too small for the requested degree of Zernike polynomials \n";
        m_ErrorGeneratorValid=false;
    }

    if(!m_ErrorGeneratorValid)
        SetOptiXLastError(errorstring+ "in ",__FILE__, __func__);

    cout << "Error map generation parameters validated\n";
    return m_ErrorGeneratorValid;
}

bool Surface::generateSurfaceErrors(double* total_sigma, MatrixXd& Legendre_sigmas )
{
    if(!hasParameter("error_limits"))
    {
        SetOptiXLastError(string("No surface error generator defined for surface ")+ m_name, __FILE__, __func__, __LINE__);
        return false;
    }
//    cout <<"generating surface errors of 'Surface':" << m_name << endl;
    if(!validateErrorGenerator())
        return false;

    FractalSurface fractalSurf;
    Parameter param1, param2;

    getParameter("fractal_exponent_x",param1);
    int nexp=param1.paramArray->dims[0]*param1.paramArray->dims[1];
    if(nexp==1)
        fractalSurf.setXYfractalParams("X",1, param1.paramArray->data, NULL);
    else
    {
        getParameter("fractal_frequency_x",param2);
       //int nfreq=param2.paramArray->dims[0]*param2.paramArray->dims[1];
        fractalSurf.setXYfractalParams("X",nexp, param1.paramArray->data, param2.paramArray->data);
    }

    getParameter("fractal_exponent_y",param1);
    nexp=param1.paramArray->dims[0]*param1.paramArray->dims[1];
    if(nexp==1)
        fractalSurf.setXYfractalParams("Y",1, param1.paramArray->data, NULL);
    else
    {
        getParameter("fractal_frequency_y",param2);
       //int nfreq=param2.paramArray->dims[0]*param2.paramArray->dims[1];
        fractalSurf.setXYfractalParams("Y",nexp, param1.paramArray->data, param2.paramArray->data);
    }
//        cout << "fractal parameters set\n";

    getParameter("error_limits",param1); // the number of parameter was already validated
    Array22d limits(Map<Array22d>(param1.paramArray->data)); // permanent copy of limits
    getParameter("sampling",param2);
    Map<Vector2d> steps(param2.paramArray->data); //temporary mapping
    int xpoints(round((limits(1)-limits(0))/steps(0)));  // this is the interval number
    int ypoints(round((limits(3)-limits(2))/steps(1)));

//        cout << "ready to generate a fractal map of " << xpoints << " x "  << ypoints << " intervals\n";
    // increment to the number of points and generate the fractal surface
    ArrayXXd surfError=fractalSurf.generate(++xpoints, steps(0), ++ypoints, steps(1));

       cout << "map generated "<< xpoints << " x "  << ypoints << " points\n";

    getParameter("residual_sigma", param1);
    double sigmaRes=param1.value;

    getParameter("detrending",param1);
    getParameter("low_Zernike",param2);
    Map<ArrayXXd> detrend(param1.paramArray->data, param1.paramArray->dims[0], param1.paramArray->dims[1]);
    Map<ArrayXXd> legendre(param2.paramArray->data, param2.paramArray->dims[0], param2.paramArray->dims[1]);
    // detrend and legendre matrix are non-permanent
//        cout <<  "detrend:\n" << detrend << "\nzernike:\n" << legendre << endl;

    int Nx=legendre.rows() > detrend.rows() ? legendre.rows() : detrend.rows();
    int Ny=legendre.cols() > detrend.cols() ? legendre.cols() : detrend.cols();

    MatrixXd detrendCoeffs=MatrixXd::Zero(Nx,Ny);
    ArrayXXd mask=ArrayXXd::Zero(Nx,Ny); // this will be the applied zeroing mask
    if(legendre.size())
        mask.topLeftCorner(legendre.rows(), legendre.cols())=legendre;
    if(detrend.size())
        mask.topLeftCorner(detrend.rows(),detrend.cols())+=detrend;
//    cout << "detrend mask;\n"  << mask << endl;
    if(mask.size())
        detrendCoeffs=fractalSurf.detrend(surfError,mask);
//    else
//        cout << "Mask size== 0\n";
     //cout<<" no detrend applied\n";
    //scale the detrended surface errors to match the given sigma value
    double sigmagen=sqrt(surfError.matrix().squaredNorm()/surfError.size());
    surfError *= sigmaRes/sigmagen;
    detrendCoeffs*=sigmaRes/sigmagen; // detrending Legendre residuals in detrendCoeffs; not directly usable

//        cout << "fractal ok, will generate Zernike\n";
    // generate the low frequency part from rand Legendre coefficient of specified extent
    // beware that the given extent are sigma normalized but the Grid generating function wants natural Legendre in input
    if(legendre.size())
    {
        MatrixXd  Lcoeffs= LegendreFromNormal(legendre) * ArrayXXd::Random(legendre.rows(), legendre.cols());
//            cout << "random legendres:\n" << Lcoeffs << endl;
        surfError+= LegendreSurfaceGrid(surfError.rows(), surfError.cols(), Lcoeffs);
//        if(Lcoeffs.rows()==detrendCoeffs.rows() && Lcoeffs.cols()==detrendCoeffs.cols())
//            Lcoeffs+=detrendCoeffs;

        Legendre_sigmas=LegendreNormalize(Lcoeffs);
        *total_sigma=sqrt(sigmaRes*sigmaRes+Legendre_sigmas.squaredNorm());
        cout <<"constrained Legendre normalized:\n" << Legendre_sigmas ;
        cout << "\n total constrained sigma=" << *total_sigma <<endl;;
    }
//            cout << "surface errors computed\n";
    if(!m_errorMap)
        m_errorMap= new  BidimSpline(3);
    m_errorMap->setFromGridData(limits, surfError);

//    cout << "spline interpolator set\n";


    return true;
    //finally we don't want this function to be recursive
//    if(m_next!=NULL )
//        return m_next->generateSurfaceErrors(); // this call will only propagate the function  until the next element derived from the Surface class

}

ArrayXXd Surface::getSurfaceErrors()
{
    ArrayXXd heights;
    if(m_errorMap)
    {
        int nx, ny; // Number of intervals
        Array22d limits=m_errorMap->getSampling(&nx, &ny);

        ArrayXd xval=ArrayXd::LinSpaced(++nx, limits(0), limits(1));
        ArrayXd yval=ArrayXd::LinSpaced(++ny, limits(2), limits(3));

        MatrixXd deriv; // unused
        MatrixXd interx=m_errorMap->interpolator(X, xval, deriv);
        MatrixXd intery=m_errorMap->interpolator(Y, yval, deriv);

        heights= interx.transpose()* m_errorMap->getControlValues()*intery;
    }

    return heights;
}

#ifdef HAS_REFLEX
void Surface::removeCoating()
{
    if(m_pCoating)
    {
        --(*m_pCoating);
        m_pCoating=NULL;
    }
}

bool Surface::setCoating(string tableName, string coatingName)
{
    ClearOptiXError();
    if (isSource())
    {
        SetOptiXLastError("This element is a source and can't be given a coating" , __FILE__, __func__);
        return false;
    }
    map<string,CoatingTable>::iterator ctabit = coatingTables.find(tableName);
    if(ctabit==coatingTables.end())
    {
        SetOptiXLastError(string("CoatingTable  ")+ tableName + " is not currently defined" , __FILE__, __func__);
        return false;
    }
    Coating *p_coat;
    try
    {
        p_coat=&(ctabit->second.getCoating(coatingName));
    }
    catch (runtime_error &rte)
    {
        SetOptiXLastError(string("Coating  ")+ coatingName + " is not currently defined in table "+ tableName , __FILE__, __func__);
        return false;
    }
    if(p_coat!=m_pCoating) // ne fait rien si le coating est remplacé par lui-même mais ne génère pas d'erreur
    {
        if(m_pCoating)
            --(*m_pCoating);
        m_pCoating=p_coat;
        ++(*m_pCoating);
    }
    return true;
}
#endif //HAS_REFLEX
