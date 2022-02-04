/**
 *************************************************************************
*   \file           Ellipse.cpp
*
*   \brief             implementation file of the Ellipse class
*
*
*
*   \author             François Polack  <francois.polack@synchroton-soleil.fr>
*   \date               Creation: 2022-02-02
*   \date               Last update: 2022-02-02
 ***************************************************************************/#include "Ellipse.h"
#include "Ellipse.h"


Ellipse::Ellipse()
{
    m_Mat=Matrix3d::Identity();
    m_Mat(2,2)=-1.;
}

Ellipse::Ellipse(double a, double b, double xcenter, double ycenter, double angle)
{
    Matrix3d rot=Matrix3d::Identity(),trans=Matrix3d::Identity();
    m_Mat(0,0)=1./(a*a);
    m_Mat(1,1)=1./(b*b);   // matrice sur ses axes*
    m_Mat(2,2)=-1.;
    rot(0,0)=rot(1,1)=cos(angle);
    rot(1,0)=-(rot(1,0)=sin(angle));
    trans(0,2)=-xcenter;
    trans(1,2)=-ycenter; // here trans contains the translation part
    trans=rot*trans; // here trans contains the full transform.
    m_Mat=trans.transpose()*m_Mat*trans; // doc eigen-3.3.9/doc/html/group__TopicAliasing.html say it is safe since dest mat keep its size
}

void Ellipse::setCircle(double radius,  double xcenter, double ycenter)
{
    Matrix3d trans=Matrix3d::Identity();
    m_Mat(0,0)=m_Mat(1,1)=1./(radius*radius);
    m_Mat(2,2)=-1.;
    trans(0,2)=-xcenter;
    trans(1,2)=-ycenter;
    m_Mat=trans.transpose()*m_Mat*trans;
}

Location Ellipse::locate(const Ref<Vector2d> &point)
{
    Vector3d homogenPoint;
    homogenPoint.head(2)=point;
    homogenPoint(2)=1.;
    double power=homogenPoint.transpose()*m_Mat *homogenPoint;
    if(abs(power)<=DBL_EPSILON)
        return border;
    if(power <0)
        return inside;
    return outside;
}

void Ellipse::move(double angle, const Ref<Vector2d> &translation)
{
    Matrix3d rot=Matrix3d::Identity(),trans=Matrix3d::Identity();
    rot(0,0)=rot(1,1)=cos(angle);
    rot(1,0)=-(rot(1,0)=sin(angle));
    trans(0,2)=-translation(0);
    trans(1,2)=-translation(1); // here trans contains the translation part
    trans=rot*trans; // here trans contains the full transform.
    m_Mat=trans.transpose()*m_Mat*trans;
}

void Ellipse::setSymmetric(const Ref<Vector2d> &point)
{
    Matrix3d trans= Matrix3d::Identity();
    trans(0,0)=trans(1,1)=-1.;
    trans.col(2).head(2)=2*point;
    m_Mat=trans.transpose()*m_Mat*trans;
}

void Ellipse::setSymmetric(const Ref<Vector2d> &point, const Ref<Vector2d> &dir)
{
    Vector2d n,u=dir.normalized();
    n << -u(1), u(0);
    Matrix3d trans= Matrix3d::Identity();
    trans(1,1)=-(trans(0,0)=u(0)*u(0)-u(1)*u(1));
    trans(1,0)=trans(0,1)=2*u(0)*u(1);
    trans.col(2).head(2)=2*point.dot(n)*n;
    m_Mat=trans.transpose()*m_Mat*trans;
}
