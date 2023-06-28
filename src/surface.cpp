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
#include "wavefront.h"
#define NFFT_PRECISION_DOUBLE
#include <nfft3mp.h>
extern void Init_Threads();



/** \brief helper functor to extract the coefficient-wise minimum value of two arrays
 */
template<typename Scalar_>  struct minOf2Op
{
  const Scalar_ operator()(const Scalar_& x, const Scalar_& y) const { return x < y ? x : y; }
};


RayType& Surface::transmit(RayType& ray)
{

    intercept(ray); // intercept effectue le changement de repère previous to this. The position is updated only if the ray is alive
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

        VectorType normal;
    try{
        intercept(ray, &normal);
    } catch(EigenException & eigexcpt) {
        throw InterceptException(eigexcpt.what()+"\nEigenException within  " +  m_name + " from "  ,
                                        __FILE__, __func__, __LINE__);
    }catch(RayException & excpt)
    {
        throw InterceptException(excpt.what()+"\nRayException within " +  m_name + " from "  ,
                                        __FILE__, __func__, __LINE__);
    }catch(...)
    {
        throw InterceptException(string("Unknown Exception within ") +  m_name + " from" ,
                                        __FILE__, __func__, __LINE__);
    }
    try{
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
        throw RayException(excpt.what()+"\nRayException withinin " +  m_name + " rfrom "  ,
                                        __FILE__, __func__, __LINE__);
    }catch(...)
    {
        throw RayException(string("Unknown Exception catch in ") +  m_name + " r" ,
                                        __FILE__, __func__, __LINE__);
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

    if(spotDiagram.m_dim < 5)
        throw std::invalid_argument("SpotDiagram argument should have a vector dimension of at least 5");


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
        if(pRay->m_alive)
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

/*EIGEN_DEVICE_FUNC*/
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
    /**< \todo Tester dans cette fonction si les calculs actuellement stockés sont utilisables ou non */

    MatrixXd LegendreCoefs;
    vector<RayType> impacts;
    getImpacts(impacts,AlignedLocalFrame);

    if(impacts.size()< (size_t) 2*Nx*Ny)
        throw std::runtime_error("The impact number is to small to compute a valid OPD");

    VectorType referencePoint= VectorType::UnitZ()*distance;
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


    if(plan.flags & PRE_ONE_PSI)
        nfft_precompute_one_psi(&plan);

    complex<double> *pSdat=PSF.data(), *pPdat=PSF.data()+stacksize;
    for(Index ioffset=0; ioffset < distOffset.size(); ++ioffset, pSdat+=arraysize, pPdat+=arraysize )
    {
        Map<ArrayXcd> F((complex<double>*)plan.f,m_OPDdata.rows());
        Map<ArrayXXcd> Tf((complex<double>*)plan.f_hat,dims[1],dims[0] );
        Map<ArrayXXcd> Spsf(pSdat,dims[0], dims[1]);
        Map<ArrayXXcd> Ppsf(pPdat,dims[0], dims[1]);

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
