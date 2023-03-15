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
 ***************************************************************************/

#include "polynomial.h"

RayBaseType::VectorType Polynomial::intercept(RayBaseType& ray,  RayBaseType::VectorType * normal )
{
    ray.moveTo(-ray.direction().dot(ray.origin()) ).rebase();   // move and rebase at the nearest of surface origin
    Vector<FloatType,3> gradient;
    Matrix<FloatType,2,2> curvature;
    gradient << 0,0,-1.;
    // iteration loop
    int i, maxiter=20;
    FloatType ztol=1e-9,  t=0, dt=0;
    for(i=0; i < maxiter; ++i, t+=dt)
    {
        VectorXType Px,Py, d1x, d1y, d2x, d2y;
        Px=getBaseValues(m_coeffs.rows(), ray.position(t)(0), d1x, d2x);
        Py=getBaseValues(m_coeffs.cols(), ray.position(t)(1), d1x, d2x);
        FloatType DZ=ray.position(t)(3) - Px.transpose()*m_coeffs*Py;
        if(abs(DZ) <ztol)
            break;
        gradient(0)= d1x.transpose()*m_coeffs* Py;
        gradient(1)=Px.transpose()*m_coeffs*d1y;
        curvature(0,0)=d2x.transpose()*m_coeffs*Py;
        curvature(1,1)=Px.transpose()*m_coeffs*d2y;
        curvature(0,1)=curvature(1,0)=d1x.transpose()*m_coeffs*d1y;
        FloatType g=gradient.dot(ray.direction());
        FloatType c=ray.direction().head(2).transpose()*curvature*ray.direction().head(2);
        FloatType tau=DZ/g;
        FloatType gamma=c*tau/g;
        if (abs(gamma) >0.5)
        {
            std::cout << "Local curvature is too large\n";
            dt=tau;
        }
        else
            dt=tau*(1.-gamma*(1.-gamma*(2.-5.*gamma)));
    }
    if(i == maxiter)
    {
        char msg[80];
            sprintf(msg, "Intercept tolerance %Lg not achieved in %d iterations", ztol, maxiter);
        throw RayException(msg, __FILE__, __func__, __LINE__);
    }
    ray.moveTo(t).rebase();
    return ray.position();
}

std::pair<double,double> Polynomial::fitSlopes(int Nx, int Ny, const Ref<ArrayX4d>& slopedata)
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
            Mat.block(0, k, numData, 1)=dPx.col(i)*Py.col(j);
            Mat.block(numData, k, numData, 1)=Px.col(i)*dPy.col(j);
        }
    Vprim.head(numData)=slopedata.col(2).cast<FloatType>();
    Vprim.tail(numData)=slopedata.col(3).cast<FloatType>();
    // On construit la matrice en supprimant la colonne 0 (facteur constant qui n'a pas d'incidence pentes à fitter)
    A=Mat.block(0,1,nlines,nvars).transpose()*Mat.block(0,1,nlines,nvars);
    Rhs=Mat.block(0,1,nlines,nvars).transpose()*Vprim;

    Map<VectorXType> Cmap(m_coeffs.data()+1, nvars);
    Cmap=A.lu().solve(Rhs);

    Vprim-= A*Cmap;
    sigmax= sqrt(Vprim.head(numData).array().square().sum()/numData);
    sigmay= sqrt(Vprim.tail(numData).array().square().sum()/numData);
    return std::make_pair(sigmax,sigmay);
}

double Polynomial::fitHeights(int Nx, int Ny, const Ref<ArrayX3d>& heightdata)
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

    Map<VectorXType> Cmap(m_coeffs.data()+1, nvars);
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

