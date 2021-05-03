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


RayType& Surface::transmit(RayType& ray)
{

    intercept(ray); // intercept effectue le changement de repère previous to this
    if(m_recording!=RecordNone)
            m_impacts.push_back(ray);
    return ray;
}

RayType& Surface::reflect(RayType& ray)    /**<  this implementation simply reflect the ray on the tangent plane */
{

    VectorType normal;

    intercept(ray, &normal);
    if(ray.m_alive)
    {
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


void Surface::clearImpacts()
{
    m_impacts.clear();
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
        RayType ray(*it);
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

int Surface::getSpotDiagram(SpotDiagramExt& spotDiagram, double distance)
{
//    if(spotDiagram.m_spots)
//        delete[] spotDiagram.m_spots;
    cout << "getting diagram of  "  << m_name <<  " n " << m_impacts.size() << "  mem " << &m_impacts[0] << endl;

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
            spotMat(4,ip)=pRay->m_wavelength;
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

int Surface::getCaustic(CausticDiagram& causticData)
{
//    if(causticData.m_spots)
//        delete[] causticData.m_spots;

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


int Surface::getWavefrontData(SpotDiagramExt& WFdata, double distance)
{
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
        // pour être ensuite égalées au dérivées du front d'onde par rapport aux angles d'ouverture
        VectorType delta=pRay->projection(referencePoint)-referencePoint;
        WFmat(0,ip)= (delta(0)*pRay->direction()(2)- delta(2)*pRay->direction()(0) ) /sqrtl(1.L-pRay->direction()(1)*pRay->direction()(1));
        WFmat(1,ip)= (delta(1)*pRay->direction()(2)- delta(2)*pRay->direction()(1) ) /sqrtl(1.L-pRay->direction()(0)*pRay->direction()(0));
        WFmat.block<2,1>(2,ip)= pRay->direction().segment(0,2).cast<double>();
        WFmat(4,ip)=pRay->m_wavelength;
    }
    return ip;
}

EIGEN_DEVICE_FUNC MatrixXd Surface::getWavefontExpansion(double distance, Index Nx, Index Ny, Array22d& XYbounds)
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
        slopeMat.block<1,2>(ip,2)= pRay->direction().segment(0,2).cast<double>();
    }
    XYbounds.row(0)=slopeMat.block(0,2,ip,2).colwise().minCoeff(); // ici il est permis de faire min max sur les valeurs passées dans XY bounds
    XYbounds.row(1)=slopeMat.block(0,2,ip,2).colwise().maxCoeff();

    cout << "bounds\n" <<XYbounds.col(0).transpose() << endl<< XYbounds.col(1).transpose() << endl;

    LegendreCoefs=LegendreIntegrateSlopes(Nx,Ny, slopeMat, XYbounds.col(0), XYbounds.col(1));

    return LegendreCoefs;
}



