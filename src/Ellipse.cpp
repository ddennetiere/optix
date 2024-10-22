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


Ellipse::Ellipse(bool transparent):Region(transparent)
{
    m_Mat=Matrix3d::Identity();
    m_Mat(2,2)=-1.;
}

Ellipse::Ellipse(double a, double b, double xcenter, double ycenter, double angle)
{
    setParameters(a, b, xcenter, ycenter, angle);
}

void Ellipse::setParameters(double a, double b, double xcenter, double ycenter, double angle)
{
    Matrix3d rot=Matrix3d::Identity(),trans=Matrix3d::Identity();
    m_Mat.setZero();
    m_Mat(0,0)=1./(a*a);
    m_Mat(1,1)=1./(b*b);   // matrice sur ses axes*
    m_Mat(2,2)=-1.;
    rot(0,0)=rot(1,1)=cos(angle);
    rot(1,0)=sin(angle), rot(0,1)=-rot(1,0);
    trans(0,2)=-xcenter;
    trans(1,2)=-ycenter; // here trans contains the translation part
    trans=rot*trans; // here trans contains the full transform.
    m_Mat=trans.transpose()*m_Mat*trans; // doc eigen-3.3.9/doc/html/group__TopicAliasing.html say it is safe since dest mat keep its size
}

void Ellipse::setCircle(double radius,  double xcenter, double ycenter)
{
    Matrix3d trans=Matrix3d::Identity();
    m_Mat=Matrix3d::Identity();
    m_Mat(0,0)=m_Mat(1,1)=1./(radius*radius);
    m_Mat(2,2)=-1.;
    trans(0,2)=-xcenter;
    trans(1,2)=-ycenter;
   // std::cout <<"\n mat_axes:\n" << m_Mat << std::endl <<"\n trans:\n" << trans << std::endl;
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
    rot(0,1)=-(rot(1,0)=sin(angle));
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

void Ellipse::getParameters(double* a, double* b, double* xcenter, double* ycenter, double *angle)
{
    Vector2d trans= m_Mat.topLeftCorner(2,2).inverse()* m_Mat.col(2).head(2);
    double dif=m_Mat(1,1)-m_Mat(0,0), sum1=m_Mat(1,1)+m_Mat(0,0), sum2=m_Mat(0,1)+m_Mat(1,0);
    *xcenter=-trans(0);
    *ycenter=-trans(1);
    if(dif==0)
    {
        *a=1./sqrt(m_Mat(0,0));
        *b=1./sqrt(m_Mat(1,1));
        *angle=0;  // en fait indéfini
        return;
    }
    *angle=atan(sum2/dif)/2.;
    double delta=copysign(sqrt(dif*dif+sum2*sum2), *angle);
  //  std::cout << "\ndelta=" << delta << std::endl;
    *b=1./sqrt((sum1+delta)/2.);
    *a=1./sqrt((sum1-delta)/2.);

}
#define XMLSTR (xmlChar*)
//xmlNodePtr operator<<(xmlNodePtr apernode, const Ellipse & ellipse)
void Ellipse::operator>>(xmlNodePtr apernode)
{
    char buf[128];
    double a, b, xcenter, ycenter, angle;
    getParameters(&a, &b, &xcenter, &ycenter, &angle);
    xmlNodePtr regnode=xmlNewTextChild(apernode,NULL,XMLSTR "region", NULL); // no value
    xmlNewProp(regnode, XMLSTR "class", XMLSTR "Ellipse");
    xmlNewProp(regnode, XMLSTR "transparent", XMLSTR (m_transparent? "1" : "0"));
   // xmlNodePtr parmnode=xmlNewTextChild(apernode,NULL,XMLSTR "parameters", NULL); //useless
    sprintf(buf,"%.8g",a);
    xmlNewProp(regnode, XMLSTR "a", XMLSTR buf);
    sprintf(buf,"%.8g",b);
    xmlNewProp(regnode, XMLSTR "b", XMLSTR buf);
    sprintf(buf,"%.8g",xcenter);
    xmlNewProp(regnode, XMLSTR "xcenter", XMLSTR buf);
    sprintf(buf,"%.8g",ycenter);
    xmlNewProp(regnode, XMLSTR "ycenter",XMLSTR buf);
    sprintf(buf,"%.8g",angle);
    xmlNewProp(regnode, XMLSTR "angle", XMLSTR buf);
}

void Ellipse::operator<<(xmlNodePtr regnode)
{
    double a, b, xcenter, ycenter, angle;
    xmlChar* strbuf = xmlGetProp(regnode, XMLSTR "a");

    sscanf((char*) strbuf,"%lf", &a);
    xmlFree(strbuf);
    strbuf = xmlGetProp(regnode, XMLSTR "b");
    sscanf((char*) strbuf,"%lf", &b);
    xmlFree(strbuf);
    strbuf = xmlGetProp(regnode, XMLSTR "xcenter");
    sscanf((char*) strbuf,"%lf", &xcenter);
    xmlFree(strbuf);
    strbuf = xmlGetProp(regnode, XMLSTR "ycenter");
    sscanf((char*) strbuf,"%lf", &ycenter);
    xmlFree(strbuf);
    strbuf = xmlGetProp(regnode, XMLSTR "angle");
    sscanf((char*) strbuf,"%lf", &angle);
    xmlFree(strbuf);
    setParameters(a, b, xcenter, ycenter, angle);
    strbuf = xmlGetProp(regnode, XMLSTR "transparent");
    setTransparency(strcmp( (char*) strbuf,"1")==0 ? true: false);
    xmlFree(strbuf);
}
