/**
 *************************************************************************
*   \file           Polygon.cpp
*
*   \brief             implementation file of the Polygon class
*
*
*
*   \author             François Polack  <francois.polack@synchroton-soleil.fr>
*   \date               Creation: 2022-01-24
*   \date               Last update: 2022-01-24
 ***************************************************************************/#include "Polygon.h"

// using namespace std; no longer usable in recet C++ releases
using std::runtime_error;

Polygon::Polygon(bool transparent):Region(transparent)
{
   m_convex=false;
   m_size=0;
}



Polygon::Polygon(const Ref<Array2Xd> & vertices)
{
    m_size=vertices.cols();
    m_vertices.resize(2,m_size+1);
    m_vertices.leftCols(m_size)=vertices;
    m_vertices.col(m_size)=vertices.col(0);  // le vecteur corner est cyclique polygone fermé
    m_vects.resize(2,m_size+1);
    m_vects=m_vertices.rightCols(m_size)-m_vertices.leftCols(m_size);
    m_vects.col(m_size)=m_vects.col(0);
    checkConvex();
}

void Polygon::setVertices(const Ref<Array2Xd> & vertices)
{
    m_size=vertices.cols();
    m_vertices.resize(2,m_size+1);
    m_vertices.leftCols(m_size)=vertices;
    m_vertices.col(m_size)=vertices.col(0);  // le vecteur corner est cyclique polygone fermé
    m_vects.resize(2,m_size+1);
    m_vects.leftCols(m_size)=m_vertices.rightCols(m_size)-m_vertices.leftCols(m_size);
    m_vects.col(m_size)=m_vects.col(0);
    checkConvex();
}

void Polygon::setRectangle(double xsize, double ysize, double xcenter, double ycenter)
{
    Array<double,2,4> points;
    points << xcenter-xsize/2., xcenter+xsize/2., xcenter+xsize/2., xcenter-xsize/2.,
              ycenter-ysize/2., ycenter-ysize/2., ycenter+ysize/2., ycenter+ysize/2.;
    setVertices(points);
}

void Polygon::changeVertex(size_t n, double x, double y)
{
    if(n < 0 || n>= m_size)
        throw std::runtime_error("invalid point number");
    m_vertices(0,n)=x;
    m_vertices(1,n)=y;
    if(n==0)
        m_vertices.col(m_size)= m_vertices.col(0);
    m_vects.leftCols(m_size)=m_vertices.rightCols(m_size)-m_vertices.leftCols(m_size);
    m_vects.col(m_size)=m_vects.col(0);
    checkConvex();
}

void Polygon::deleteVertex(size_t n)
{
    if(n < 0 || n>= m_size)
        throw std::runtime_error("invalid point number");
    Matrix2Xd vertices=Matrix2Xd::Zero(2,m_size);
    if (n!=0)
        vertices.leftCols(n)=m_vertices.leftCols(n);
    vertices.rightCols(m_size-n)=m_vertices.rightCols(m_size-n);

    m_vertices.swap(vertices);
    m_vects.resize(2,m_size);
    m_size-=1;
    m_vects.leftCols(m_size)=m_vertices.rightCols(m_size)-m_vertices.leftCols(m_size);
    m_vects.col(m_size)=m_vects.col(0);
    checkConvex();
}

void Polygon::insertVertex(size_t pos, double x, double y)
{
    if(pos < 0 || pos > m_size)
        throw std::runtime_error("invalid point number");
    Matrix2Xd vertices(2,m_size+2); //=Matrix2Xd::Zero

    if(pos!=0)
        vertices.leftCols(pos)=m_vertices.leftCols(pos);
    vertices(0,pos)=x;
    vertices(1,pos)=y;
    size_t n=m_size+1-pos;
    vertices.rightCols(n)=m_vertices.rightCols(n);

    m_vertices.swap(vertices);
    m_size+=1;
    m_vects.resize(2,m_size+1);
    m_vects.leftCols(m_size)=m_vertices.rightCols(m_size)-m_vertices.leftCols(m_size);
    m_vects.col(m_size)=m_vects.col(0);
    checkConvex();
}
void Polygon::move(double angle, const Ref<Vector2d> &translation)
{
    if(m_size==0)
        return;
//    Matrix2d rot;
//    rot << cos(angle), -sin(angle), sin(angle), cos(angle);
//    Matrix2Xd mtemp;        // TODO remplacer mtemp par un eval() cf setSymmetric
//    mtemp=(rot*m_vertices).colwise() +translation;
//    m_vertices.swap(mtemp);
//    mtemp=(rot*m_vects).colwise() +translation;
//    m_vects.swap(mtemp);
//    mtemp=(rot*m_sides.topRows(2)).colwise() +translation;
//    m_sides.topRows(2)=mtemp;
//    mtemp.array()*=m_vertices.leftCols(m_size).array();
//    m_sides.row(2)=-mtemp.colwise().sum();
//    mtemp=rot*m_refPoint+translation;
//    m_refPoint.swap(mtemp);
    Transform<double,2, Isometry> isoTransform;
    isoTransform=Rotation2Dd(angle) ;
    isoTransform.pretranslate(translation);

    m_vertices=isoTransform*m_vertices;
    m_vects=isoTransform*m_vects;
    m_sides=isoTransform.inverse().matrix().transpose()*m_sides;
    m_refPoint=isoTransform*m_refPoint;
}

void Polygon::setSymmetric(const Ref<Vector2d> &point, const Ref<Vector2d> &dir)
{
    if(m_size==0)
        return;
    Vector2d n,u=dir.normalized();
    n << -u(1), u(0);
    double k=2*point.dot(n);
    Matrix2d rot;
    rot(1,1)=-(rot(0,0)=u(0)*u(0)-u(1)*u(1));
    rot(1,0)=rot(0,1)=2*u(0)*u(1);

    m_vertices=(rot*m_vertices).eval().colwise()+k*n;
    m_vects=(rot*m_vects).eval().colwise()+k*n;
    m_sides.topRows(2)=(rot*m_sides.topRows(2)).eval().colwise()+k*n;
    m_sides.row(2)=(m_sides.topRows(2).array()*m_vertices.leftCols(m_size).array()).colwise().sum();
    m_refPoint=(rot*m_refPoint).eval().colwise()+k*n;
}

void Polygon::setSymmetric(const Ref<Vector2d> &point)
{
    if(m_size==0)
        return;
    Matrix2d rot= -Matrix2d::Identity();

    m_vertices=(rot*m_vertices).eval().colwise()+2*point;
    m_vects=(rot*m_vects).eval().colwise()+2*point;
    m_sides.topRows(2)=(rot*m_sides.topRows(2)).eval().colwise()+2*point;
    m_sides.row(2)=(m_sides.topRows(2).array()*m_vertices.leftCols(m_size).array()).colwise().sum();
    m_refPoint=(rot*m_refPoint).eval().colwise()+2*point;
}

bool Polygon::checkConvex()
{
    if(m_size<3)
    {
        m_convex=false;
        return m_convex;
    }
    m_convex=true;
    m_sides.resize(3,m_size);
    Vector2d u;
    // calcul du centroide
    m_refPoint=m_vects.leftCols(m_size).rowwise().mean();

    double det=m_vects.rightCols(2).determinant();
    if (det ==0)
        throw std::runtime_error("invalid polygon");
    bool direct= !std::signbit(det);
    for (size_t i=0 ; i< m_size; ++i)
    {
//       if(signbit(det)) // rotation inverse
//        {
//            m_sides(0,i)= m_vects(1,i);
//            m_sides(1,i)=-m_vects(0,i);
//        }
//        else
//        {
            m_sides(0,i)=-m_vects(1,i);
            m_sides(1,i)= m_vects(0,i);
//        }
        // le vecteur directeur est simplement le vecteur côté tourné de +90°
        m_sides(2,i)=-m_sides.block<2,1>(0,i).dot(m_vertices.col(i));

        det=m_vects.block<2,2>(0,i).determinant();
        if (det ==0)
            throw std::runtime_error("invalid polygon");

        if(std::signbit(det)==direct)
            m_convex=false;
    }
    return m_convex;
}

Location Polygon::locate(const Ref<Vector2d> &point)
{
    if(m_size <3)
    {
       // return undetermined;
       throw std::runtime_error("Polygon does not define an area");

    }
    if(m_convex)
    {
        Vector3d homogenPt;
        homogenPt.head(2)=point;
        homogenPt(2)=1.;
        double  d=(m_sides.transpose()*homogenPt).minCoeff();
       // cout << homogenPt.transpose() *m_sides << endl << d <<endl;
        if(abs(d)<=DBL_EPSILON)
            return border;
        if(d > 0)
            return inside;
        else
            return outside;
    }
    else
    {
        Matrix2d M;
        Vector2d V, U= (m_refPoint- point).normalized(); // si l'opération n'est pas possible U n'est pas normalisé
        if(U.norm() < 1.e-3 )
            U << 0,1.;
        M(0,0)=-U(1);
        M(0,1)= U(0);

        double  det;
        int numInter=0;
        for(size_t i=0; i <m_size; ++i)
        {
            M.row(1)=m_sides.col(i).head(2).transpose();
            det=M.determinant();
            V=M*(point-m_vertices.col(i))/det;
            if(abs(V(1)) <= DBL_EPSILON)
                return border;
            if(V(1) >0 && (V(0)>=0 ||V(0)<1.))
                ++numInter;
        }
        if(numInter==0)
            throw std::runtime_error("Failure of the Polygon::locate algorithm");
        if(numInter%2==0)
            return outside;
        else
            return inside;

    }
    //return undetermined; // ne passe jamais ici
}
#define XMLSTR (xmlChar*)
//xmlNodePtr operator<<(xmlNodePtr apernode, const Polygon & polygon)
void Polygon::operator>>(xmlNodePtr apernode)
{
    char buf[34*m_size];  // max length per number in %.8g =15 => max length 34 char/point
    std::cout << "m_vertices(" <<m_vertices.rows() << " x " <<m_vertices.cols() << ")\n";

    xmlNodePtr regnode=xmlNewTextChild(apernode,NULL,XMLSTR "region", NULL); // no value
    xmlNewProp(regnode, XMLSTR "class", XMLSTR "Polygon");
    xmlNewProp(regnode, XMLSTR "transparent", XMLSTR (m_transparent? "1" : "0"));

    char *pbuf=buf;
    pbuf+=sprintf(pbuf,"%.8g, %.8g", m_vertices(0,0), m_vertices(1,0));
    for(int i=1; i< (int) m_size; ++i)
        pbuf+=sprintf(pbuf,"; %.8g, %.8g", m_vertices(0,i), m_vertices(1,i));
//    xmlNewProp (parmnode, XMLSTR "positions", XMLSTR buf);
    xmlNodePtr parmnode=xmlNewTextChild(regnode,NULL,XMLSTR "vertices", XMLSTR buf);
    sprintf(buf,"%Ld", m_size);
    xmlNewProp (parmnode, XMLSTR "size", XMLSTR buf);
}

void Polygon::operator<<(xmlNodePtr regnode)
{
    xmlNodePtr curnode= xmlFirstElementChild(regnode);
    if(xmlStrEqual(curnode->name, XMLSTR "vertices"))
    {
        xmlChar* strbuf = xmlGetProp(curnode, XMLSTR "size");
        sscanf((char*)strbuf,"%Ld",&m_size);
        xmlFree(strbuf);
        m_vertices.resize(2,m_size);
    //    strbuf = xmlGetProp(regnode, XMLSTR "positions");
        strbuf = xmlNodeGetContent(curnode);
        char* pbuf=(char*) strbuf;
        for(int i=0; i< (int) m_size; ++i)
        {
            int n=sscanf(pbuf,"%lf, %lf", &m_vertices(0,i), &m_vertices(1,i));
            if(n!=2)
                std::cout <<"format error in polygon <<, number of item read "<< n << " instead of 2\n";
            pbuf=strchr(pbuf,';')+1;
        }
        xmlFree(strbuf);
        strbuf = xmlGetProp(regnode, XMLSTR "transparent");
        setTransparency(strcmp( (char*) strbuf,"1")==0 ? true: false);
        xmlFree(strbuf);
    }
}
