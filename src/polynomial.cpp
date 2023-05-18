/**
 *************************************************************************
*   \file           polynomial.cpp
*
*   \brief             implementation file
*
*
*
*   \author             François Polack  <francois.polack@synchroton-soleil.fr>
*   \date               Creation: 2023-03-12
*   \date               Last update: 2023-03-12
*
*   \todo Cleaning of exception catching
 ***************************************************************************/

#include "polynomial.h"
#include <sstream>

//#define VERBOSE

RayBaseType::VectorType Polynomial::intercept(RayBaseType& ray,  RayBaseType::VectorType * normal )
{
    std::stringstream errmsg;
    ray.moveTo(-ray.direction().dot(ray.origin()) ).rebase();   // move and rebase at the nearest of surface origin
    Vector<FloatType,3> gradient;
    Matrix<FloatType,2,2> curvature;
    gradient << 0,0,-1.;
    // iteration loop
    int i, maxiter=20;
#ifdef VERBOSE
    ArrayXXd trace(2, maxiter);
#endif // VERBOSE
    FloatType ztol=1e-11,  t=0, dt=0;
    for(i=0; i < maxiter; ++i, t+=dt)
    {
        VectorXType Px,Py, d1x, d1y, d2x, d2y;
        int curline=0;
        try {
            Px=getBaseValues(m_coeffs.rows(), m_Kx*(ray.position(t)(0)-m_X0), d1x, d2x);curline=__LINE__;
            Py=getBaseValues(m_coeffs.cols(), m_Ky*(ray.position(t)(1)-m_Y0), d1y, d2y);curline=__LINE__;
            FloatType DZ=ray.position(t)(2) - Px.transpose()*m_coeffs*Py;curline=__LINE__;
            if(!isnormal(DZ))
            {

                errmsg << "invalid surface distance at t="<< t<< "   position " << ray.position(t).transpose()
                          << "   direction: " << ray.direction().transpose() << std::endl;
#ifdef VERBOSE
                std::cout << "X Poly values: "  << Px.transpose() <<std::endl;
                std::cout << "Y Poly values: "  << Py.transpose() <<std::endl;
                std::cout << "coeff dump:\n" << m_coeffs <<std::endl;
#endif // VERBOSE
                throw RayException(errmsg.str(), __FILE__, __func__, __LINE__);
            }
            if(abs(DZ) <ztol)
                break;
            gradient(0)= m_Kx*d1x.transpose()*m_coeffs* Py;curline=__LINE__;
            gradient(1)=m_Ky*Px.transpose()*m_coeffs*d1y;curline=__LINE__;
            curvature(0,0)=m_Kx*m_Kx*d2x.transpose()*m_coeffs*Py;curline=__LINE__;
            curvature(1,1)=m_Ky*m_Ky*Px.transpose()*m_coeffs*d2y;curline=__LINE__;
            curvature(0,1)=curvature(1,0)=m_Kx*m_Ky*d1x.transpose()*m_coeffs*d1y;curline=__LINE__;
            FloatType g=gradient.dot(ray.direction());curline=__LINE__;
            FloatType c=ray.direction().head(2).transpose()*curvature*ray.direction().head(2);curline=__LINE__;
            FloatType tau=DZ/g;
            FloatType gamma=c*tau/g;
            if (abs(gamma) >0.5)
            {
                std::cout << "Local curvature is too large\n";
                dt=tau;
            }
            else
                dt=tau*(1.-gamma*(1.-gamma*(2.-5.*gamma)));
#ifdef VERBOSE
            trace(0,i)=dt;
            trace(1,i)=DZ;
#endif // VERBOSE
        } catch (EigenException  & excpt ) {
            throw( EigenException(string("Rethrowing exception from ")+excpt.what(), __FILE__, __func__, curline));
        }
    }
//    std::cout <<"iter=" << i << std::endl;
    if(i == maxiter)
    {
        errmsg << "Intercept tolerance " << ztol << " not achieved in " << maxiter << "  iterations"<<std::endl;
#ifdef VERBOSE
        std::cout << "Ray input: " << ray.position().transpose() << "   direction: " << ray.direction().transpose() << std::endl;
        std::cout << "Ray output:" << ray.position(t).transpose()  << std::endl;
        std::cout << "t=" << t << " dt=" << dt << "     trace:\n";
        std::cout  << trace <<std::endl;

#endif // VERBOSE
        throw RayException(errmsg.str(), __FILE__, __func__, __LINE__);
    }
    if(normal)
        *normal=-gradient.normalized();
    ray.moveTo(t).rebase();
    return ray.position();
}

std::pair<double,double> Polynomial::fitSlopes(Index Nx, Index Ny, const Ref<ArrayX4d>& slopedata)
{
    Index numData=slopedata.rows(), nvars=Nx*Ny-1;
    Index nlines=2*numData, i=0,j=0, k=0;
    double sigmax=0, sigmay=0;

    m_coeffs=MatrixXType::Zero(Nx,Ny);
    ArrayXXType Px, Py, dPx, dPy;

    ArrayXType tmparray=Xnormalize(slopedata.col(0));
    Px=getBaseValues(Nx,tmparray, dPx);
    tmparray=Ynormalize(slopedata.col(1));
    Py=getBaseValues(Ny, tmparray, dPy);

    MatrixXType Mat(nlines, Nx*Ny), A;
    VectorXType Vprim(nlines), Rhs;
    for(j=0, k=0; j < Ny; ++j)
        for(i=0; i < Nx; ++i, ++k)
        {
            Mat.block(0, k, numData, 1)=m_Kx*dPx.col(i)*Py.col(j);
            Mat.block(numData, k, numData, 1)=m_Ky*Px.col(i)*dPy.col(j);
        }
    Vprim.head(numData)=slopedata.col(2).cast<FloatType>();
    Vprim.tail(numData)=slopedata.col(3).cast<FloatType>();
    // On construit la matrice en supprimant la colonne 0 (facteur constant qui n'a pas d'incidence pentes à fitter)
    A=Mat.block(0,1,nlines,nvars).transpose()*Mat.block(0,1,nlines,nvars);
    Rhs=Mat.block(0,1,nlines,nvars).transpose()*Vprim;

    m_coeffs(0,0)=0;
    Map<VectorXType> Cmap(m_coeffs.data()+1, nvars);
    Cmap=A.lu().solve(Rhs);

    Vprim-= A*Cmap;
    sigmax= sqrt(Vprim.head(numData).array().square().sum()/numData);
    sigmay= sqrt(Vprim.tail(numData).array().square().sum()/numData);
    return std::make_pair(sigmax,sigmay);
}

double Polynomial::fitHeights(Index Nx, Index Ny, const Ref<ArrayX3d>& heightdata)
{
    Index numData=heightdata.rows(), nvars=Nx*Ny;
    Index i=0,j=0, k=0;
    double sigma=0;

    m_coeffs=MatrixXType::Zero(Nx,Ny);
    ArrayXXType Px, Py, dPx, dPy;

    ArrayXType tmparray=Xnormalize(heightdata.col(0));
    Px=getBaseValues(Nx,tmparray, dPx);
    tmparray=Ynormalize(heightdata.col(1));
    Py=getBaseValues(Ny, tmparray, dPy);

    MatrixXType Mat(numData, nvars), A;
    VectorXType Vheight(numData), Rhs;
    for(j=0, k=0; j < Ny; ++j)
        for(i=0; i < Nx; ++i, ++k)
            Mat.col(k)=Px.col(i)*Py.col(j);

    Vheight=heightdata.col(2).cast<FloatType>();

    A=Mat.transpose()*Mat;
    Rhs=Mat.transpose()*Vheight;

    Map<VectorXType> Cmap(m_coeffs.data(), nvars);
    Cmap=A.lu().solve(Rhs);

    Vheight-= A*Cmap;
    sigma= sqrt(Vheight.array().square().sum()/numData);
    return sigma;
}


ArrayXXd Polynomial::surfaceHeight(const Ref<ArrayXd>& Xpos, const Ref<ArrayXd>& Ypos )
{
    Index Nx= Xpos.size(), Ny=Ypos.size();
    ArrayXXType  dPx, dPy;
    MatrixXType Px, Py;

    ArrayXType tmparray=Xnormalize(Xpos);
    Px=getBaseValues(Nx,tmparray, dPx);
    tmparray=Ynormalize(Ypos);
    Py=getBaseValues(Ny, tmparray, dPy);

    // Px(Xpos.size(), NXorder) et Py(Xpos.size(), NYorder)

    MatrixXType Height= Px*m_coeffs*Py.transpose();
    return Height.cast<double>().array();
}

Tensor<double,3> Polynomial::surfaceSlopes(const Ref<ArrayXd>& Xpos, const Ref<ArrayXd>& Ypos )
{
    Index Nx= Xpos.size(), Ny=Ypos.size();
    ArrayXXType  dPx, dPy;
    MatrixXType Px, Py;

    ArrayXType tmparray=Xnormalize(Xpos);;
    Px=getBaseValues(Nx,tmparray, dPx);
    tmparray=Ynormalize(Ypos);;
    Py=getBaseValues(Ny, tmparray, dPy);

    MatrixXType slopeX =dPx.matrix()*m_coeffs*Py.transpose();
    MatrixXType slopeY =Px*m_coeffs*dPy.matrix().transpose();

    Tensor<double,3> Tres(Nx, Ny, 2);
    Map<MatrixXd>(Tres.data(), Nx,Ny)= slopeX.cast<double>();
    Map<MatrixXd>(Tres.data()+Nx*Ny, Nx,Ny)= slopeY.cast<double>();
    return Tres;
}

