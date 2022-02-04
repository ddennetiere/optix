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

 using namespace std;

Polygon::Polygon()
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
        throw runtime_error("invalid point number");
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
        throw runtime_error("invalid point number");
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

void Polygon::insertVertex(size_t nprevious, double x, double y)
{
    if(nprevious < 0 || nprevious>= m_size)
        throw runtime_error("invalid point number");
    Matrix2Xd vertices(2,m_size+2); //=Matrix2Xd::Zero
    size_t n=nprevious+1;
    vertices.leftCols(n)=m_vertices.leftCols(n);
    vertices(0,n)=x;
    vertices(1,n)=y;
    n=m_size-nprevious;
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
    Matrix2d rot;
    rot << cos(angle), -sin(angle), sin(angle), cos(angle);
    Matrix2Xd mtemp;        // TODO remplacer mtemp par un eval() cf setSymmetric
    mtemp=(rot*m_vertices).colwise() +translation;
    m_vertices.swap(mtemp);
    mtemp=(rot*m_vects).colwise() +translation;
    m_vects.swap(mtemp);
    mtemp=(rot*m_sides.topRows(2)).colwise() +translation;
    m_sides.topRows(2)=mtemp;
    mtemp.array()*=m_vertices.leftCols(m_size).array();
    m_sides.row(2)=-mtemp.colwise().sum();
    mtemp=rot*m_refPoint+translation;
    m_refPoint.swap(mtemp);
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
        throw runtime_error("invalid polygon");
    bool direct= !signbit(det);
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
            throw runtime_error("invalid polygon");

        if(signbit(det)==direct)
            m_convex=false;
    }
    return m_convex;
}

Location Polygon::locate(const Ref<Vector2d> &point)
{
    if(m_size <3)
    {
        return undetermined;
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
            throw runtime_error("Failure of the Polygon::locate algorithm");
        if(numInter%2==0)
            return outside;
        else
            return inside;

    }
    return undetermined; // ne passe jamais ici
}

