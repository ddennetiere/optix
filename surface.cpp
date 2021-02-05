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
#include "sourcebase.h"
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
        dynamic_cast<Surface*>(m_next)->clearImpacts();
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

int Surface::getSpotDiagram(SpotDiagram& spotDiagram, double distance)
{
    if(spotDiagram.m_spots)
        delete[] spotDiagram.m_spots;

    vector<RayType> impacts;
    spotDiagram.m_lostCount=getImpacts(impacts,AlignedLocalFrame);
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
    for(ip=0, pRay=impacts.begin(); pRay!=impacts.end(); ++pRay)  /// \todo Gérer la validité des rayons dans getSpotDiagram()
    {
        if(pRay->m_alive)
        {
            pRay->moveToPlane(obsPlane);
            spotMat.block<2,1>(0,ip)=pRay->position().segment(0,2).cast<double>();
            spotMat.block<2,1>(2,ip)=pRay->direction().segment(0,2).cast<double>();
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
    if(causticData.m_spots)
    delete[] causticData.m_spots;

    vector<RayType> impacts;
    causticData.m_lostCount=getImpacts(impacts,AlignedLocalFrame);
    if(impacts.size()==0)
    {
        causticData.m_spots=NULL;
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

    vector<RayType> impacts;
    WFdata.m_lostCount=getImpacts(impacts,AlignedLocalFrame);

    if(impacts.size()==0)
    {
        WFdata.m_spots=NULL;
        return 0;
    }

    VectorType referencePoint= VectorType::UnitZ()*distance;
    Array4Xd WFmat(4,impacts.size() );

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
    // on charge dans le tableau slope mat
    // dans les colonnes 0 et 1, l'aberration transverse selon x et y (dans le plan perpendiculaire au rayon)
    // dans les colonnes 2 et 4 les coefficient directeur de la direction du rayon
    for(ip=0, pRay=impacts.begin(); pRay!=impacts.end(); ++pRay, ++ip)
    {

        VectorType delta=pRay->projection(referencePoint)-referencePoint;
        slopeMat(ip,0)= (delta(0)*pRay->direction()(2)- delta(2)*pRay->direction()(0) ) /sqrtl(1.L-pRay->direction()(1)*pRay->direction()(1));
        slopeMat(ip,1)= (delta(1)*pRay->direction()(2)- delta(2)*pRay->direction()(1) ) /sqrtl(1.L-pRay->direction()(0)*pRay->direction()(0));
        slopeMat.block<1,2>(ip,2)= pRay->direction().segment(0,2).cast<double>();
    }
    XYbounds.row(0)=slopeMat.block(0,2,ip,2).colwise().minCoeff(); // ici il est permis de faire min max sur les bvaleurs passées dans XY bounds
    XYbounds.row(1)=slopeMat.block(0,2,ip,2).colwise().maxCoeff();

    cout << "bounds\n" <<XYbounds.col(0).transpose() << endl<< XYbounds.col(1).transpose() << endl;

    LegendreCoefs=LegendreIntegrateSlopes(Nx,Ny, slopeMat, XYbounds.col(0), XYbounds.col(1));

    return LegendreCoefs;
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
