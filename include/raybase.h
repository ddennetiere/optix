#ifndef HEADER_E50CE6B62C615030
#define HEADER_E50CE6B62C615030

////////////////////////////////////////////////////////////////////////////////
/**
*      \file           raybase.h
*
*      \brief         RayBase class template declarataion
*
*      \author         François Polack <francois.polack@synchroton-soleil.fr>
*      \date        2020-10-01  Creation
*      \date         Last update
*

*/
///////////////////////////////////////////////////////////////////////////////////
//
//             REVISIONS
//
////////////////////////////////////////////////////////////////////////////////////
#ifndef RAYBASE_H
#define RAYBASE_H

#include "EigenSafeInclude.h"

// if the message show-up in compilation comment the pragma and uncomment M_PI and M_PI_2 definitions
#ifndef M_PI
    #pragma message "you need to define M_PI and M_PI_2 in raybase.h line 37"
    //#define M_PI		3.14159265358979323846
    //#define M_PI_2		1.57079632679489661923
    //#define M_PI_4		0.78539816339744830962
    //#define M_1_PI		0.31830988618379067154
    //#define M_2_PI		0.63661977236758134308
#endif // M_PI not defined

#include <fstream>
#include <iostream>


using namespace Eigen;


/** \brief series developpement of \f$ 1-\sqrt{1-x} \f$ for small \f$ x \f$
 *  \ingroup GlobalCpp
 *
 * \param x  the value of x
 * \return \f$ 1-\sqrt{1-x} \f$
 */
inline double oneMinusSqrt1MinusX(double x)
{
    if(::fabs(x) > 0.01) return 1.-sqrt(1. - x);
    return  x*(0.5 + x*(0.125 + x*(0.0625 + x*(0.0390625 + x*(0.02734375 + x*(0.0205078125 + x*0.01611328125))))));
}


/** \brief  base class for rays and ray propagation
 *
 *  This is a specialization of Eigen::Parametrizedline defining an origin and a currentpoint */
template <typename scalar>
class RayBase : protected ParametrizedLine<scalar,3>
{
public:
    typedef ParametrizedLine<scalar, 3> BaseLine;
    typedef Matrix<scalar,3,1> VectorType;
    typedef Matrix<scalar,4,1> HomogeneousType;
    typedef Hyperplane<scalar,3> PlaneType;
    typedef Matrix<scalar,4,4> QuadricType;

    /** Default constructor */
    RayBase():m_alive(true) {} //All rays are created alive
    /** Default destructor */
    virtual ~RayBase() {}

    /** Copy constructor with scalar type conversion (m_alive flag is preserved)
     *  \param other Object to copy from
     */
     template<typename otherScalar>
    inline RayBase(const RayBase<otherScalar>& other):BaseLine( other),
            m_distance(other.m_distance),m_alive(other.m_alive) {}


     /** \brief Construction from origin and direction , sets  position at origin
      *
      * \param origin  origin vector
      * \param direction  direction vector ; it will be normalized if not
      * \param distance  the distance of ray position from origin
      */
     inline explicit RayBase(const VectorType& origin, const VectorType& direction, const scalar distance=0):
        BaseLine(origin, direction.normalized()), m_distance(distance),m_alive(true){ } // force la normalisation

    /** \brief Construction from origin and direction , sets  position at some distance from origin
     *
     *
     * \param origin The origin vector
     * \param direction the direction vector; it will be normalized if not
     * \param distance the relative distance of position to origin
     */
      template<typename otherScalar>
     inline explicit RayBase(const Matrix<otherScalar,3,1> &origin,
                             const Matrix<otherScalar,3,1>  &direction,
                             otherScalar distance=0):
                BaseLine( ParametrizedLine<otherScalar,3>( origin, direction) ),
                m_distance(distance),m_alive(true)
                {BaseLine::m_direction.normalize(); } ;// force la normalisation dans la précision finale



    /** \brief  Normalize the direction vector
     *
     * \return A reference to *this
     */
    inline RayBase&  normalized(){BaseLine::m_direction.normalize(); return *this ;}


    /** \brief sets the origin at curent position without changing it
     *
     * The internal distance is set to zero
     * \return a reference to this object
     */
    inline RayBase& rebase(){
        BaseLine::m_origin=BaseLine::pointAt(m_distance);
        m_distance=0;
        return *this;
    }

    /** \brief in place origin shift by vT
     *
     * \param vT   the origin translation vector
     * \return  a reference to this \sa operator+()
     */
    inline RayBase&  operator+=(Matrix<scalar,3,1>& vT){
        origin()+=vT;
        return *this;
    }

    /** \brief in place origin shift by -vT
     *
     * \param vT   the origin translation vector
     * \return  a reference to this \sa operator-()
     */
    inline RayBase&  operator-=(Matrix<scalar,3,1>& vT){
        BaseLine::m_origin-=vT;
        return *this;
    }
    /** \brief in place origin shift by vT
     *
     * \param vT   the origin translation vector
     * \return  a reference to this \sa operator+()
     */
    inline RayBase&  operator+=(Matrix<scalar,3,1>&& vT){
        origin()+=vT;
        return *this;
    }

    /** \brief in place origin shift by -vT
     *
     * \param vT   the origin translation vector
     * \return  a reference to this \sa operator-()
     */
    inline RayBase&  operator-=(Matrix<scalar,3,1>&& vT){
        BaseLine::m_origin-=vT;
        return *this;
    }

    /** \brief shifts the position by a given distence along the ray
    *        \param  offset the requested  shift
    */
  // template <typename otherScalar>
   inline  RayBase& operator+=(scalar offset)
   {
       m_distance+=offset;
       return *this;
   }

    /** \brief origin shift by vT
     *
     * \param vT   the origin translation vector
     * \return  a new ray with same position expressed in a new frame translated by -vT
     */
    inline  RayBase operator+(Matrix<scalar,3,1> vT){
        RayBase sum(*this);
        sum.BaseLine::m_origin+=vT;
        return sum;
    }



    /** \brief origin shift by -vT
     *
     * \param vT   the origin translation vector
     * \return  a new ray with same position expressed in a new frame translated by vT
     */
    inline  RayBase operator-(Matrix<scalar,3,1> vT){
        RayBase dif(*this);
        dif.BaseLine::m_origin-=vT;
        return dif;
    }

    /** \brief get the position at a given offset from the current one
     *
     * \param  offset
     * \return VectorType  return the position at a given offset from current. Current position is not changed
     */
    //inline VectorType position(scalar offset=0){return pointAt(m_distance+offset) ;   }
    EIGEN_DEVICE_FUNC inline VectorType position(scalar offset=0){return BaseLine::pointAt(m_distance+offset) ;   }

    EIGEN_DEVICE_FUNC inline VectorType& origin(){return BaseLine::m_origin;}   /**< \brief gets a reference to the origin vector*/
    EIGEN_DEVICE_FUNC inline VectorType& direction(){return BaseLine::m_direction;}   /**< \brief gets a reference to the direction vector*/
    EIGEN_DEVICE_FUNC inline VectorType projection(const VectorType& p){return BaseLine::projection(p);}   /**< \brief gets a reference projection of point p on this*/
    inline scalar& parameter(){return m_distance;}

    inline RayBase& transform(const Transform<scalar, 3, Affine> & trans )
    {
       ParametrizedLine<scalar,3>::transform(trans);
       return *this;
    }

    /** \brief move the internal position at the given distance from origin
    *   \param distance the distance from orignin of the  new position*/
    inline RayBase& moveTo(scalar distance)
    {
        m_distance=distance;
        return *this;
    }

    /** \brief move internal position to the intersection with the given plane
     *
     * \param plane The plane to intersect with
     * \return a reference to *this
     */
    inline RayBase& moveToPlane(const PlaneType &plane)
    {
         m_distance=BaseLine::intersectionParameter(plane);
         return *this;
    }

    /** \brief Move internal position to the insection with the given plane and returns it
     *
     * \param plane The plane to intersect with
     * \return The intersection point
     */
    EIGEN_DEVICE_FUNC inline VectorType getToPlane(const PlaneType &plane)
    {
        m_distance=BaseLine::intersectionParameter(plane);
        return BaseLine::pointAt(m_distance);
    }

    /** \brief returns the intersection point with the given plane without changing the internal position
     *
     * \param plane The plane to intersect with
     * \return The intersection point
     */
    inline VectorType getPlaneIntersection(const PlaneType &plane)
    {
        return BaseLine::pointAt(BaseLine::intersectionParameter(plane));
    }


    /** \brief Move internal position to the nearest intersection with the given quadric and returns it
     *
     * \param quadric The quadric to intersect with
     * \return The intersection point
     */
    EIGEN_DEVICE_FUNC inline VectorType getToQuadric(const QuadricType &quadric)
    {
       // m_distance=BaseLine::intersectionParameter(plane);
        m_distance=-direction().dot(origin());   // move and rebase close to the nearest of quadric apex  (0 point)
        rebase();
        HomogeneousType MX=quadric*origin().homogeneous();
        HomogeneousType MU=quadric*direction().homogeneous();
        scalar b=direction().homogeneous().dot(MX);
        scalar delta=b*b - (direction().homogeneous().transpose()*MU).dot((origin().homogeneous().transpose()*MX));
        if(delta < 0)
            throw RayException("No intercept found", __FILE__, __func__, __LINE__);
        m_distance= b>0 ? b-sqrt(delta) : b+sqrt(delta); // return the closest to 0

        return BaseLine::pointAt(m_distance);
    }

     EIGEN_DEVICE_FUNC  VectorType getMinDistance(RayBase & ray, scalar* param=NULL, scalar* rayParam=NULL )
     {
        VectorType vDist, rhs, vParams, diff0=origin()-ray.origin();

        scalar scal=direction().dot(ray.direction());
        if(1.L-scal < 1.e-10L)
        {
            if(param)
                *param=std::numeric_limits<scalar>::infinity();
            if(rayParam)
               *rayParam=std::numeric_limits<scalar>::infinity();
            return diff0-diff0.dot(direction())*direction() ;
        }

         Matrix<scalar,2,2> M=Matrix<scalar,2,2>::Identity()/(scal*scal);
         M(0,1)= M(1,0)= 1.L/scal;
         rhs(0)=diff0.dot(direction());
         rhs(1)=diff0.dot(ray.direction());
         vParams=M*rhs;
         vDist=ray.pointAt(vParams(1))-pointAt(vParams(0));

         if(param)
            *param=vParams(0);
         if(rayParam)
            *rayParam=vParams(1);
         return vDist;
     }

    /** \brief returns the symmetric of the input ray with respect to the plane
     *
     * \param plane  the reflection plane. Normalisation is assumed
     * \return a reflected ray originating from the plane
     */
     inline RayBase reflectOnPlane(const PlaneType &plane)
     {
        RayBase reflected(*this);
        reflected.moveToPlane(plane).rebase();
        /*(BaseLine)*/ reflected.m_direction-=2.*BaseLine::m_direction.dot(plane.normal())*plane.normal();
        return reflected;
     }


     /** \brief output the parameters to a text stream
      */
     friend std::ostream & operator << (std::ostream & s, const RayBase& ray)
     {
        IOFormat CleanFmt(4, 0, ", ", "\n", "[", "]");
        s << ray.m_origin.transpose().format(CleanFmt) << "  "  << ray.m_direction.transpose().format(CleanFmt) << "  d=" << ray.m_distance;
       return s;
     }


     /** \brief save the ray into a binary output stream
     */
     friend std::fstream & operator << (std::fstream & s, const RayBase& ray)
     {
   //     std::cout << "output vector length ="  << sizeof(ray.m_origin) << std::endl;
        s.write((char*)&ray.m_origin, sizeof(ray.m_origin));
        s.write((char*)&ray.m_direction, sizeof(ray.m_direction));
        s.write((char*)&ray.m_distance, sizeof(ray.m_distance));
        s.write((char*)&ray.m_alive, sizeof(ray.m_alive));
        return s;
     }

     /** \brief retrieves a ray from a binary input stream
     */
     friend std::fstream & operator >> (std::fstream & s, const RayBase& ray)
     {
   //     std::cout << "input vector length ="  << sizeof(ray.m_origin) << std::endl;
        s.read((char*)&ray.m_origin, sizeof(ray.m_origin));
        s.read((char*)&ray.m_direction, sizeof(ray.m_direction));
        s.read((char*)&ray.m_distance, sizeof(ray.m_distance));
        s.read((char*)&ray.m_alive, sizeof(ray.m_alive));
        return s;
     }

//     inline static RayBase OX(){ RayBase ox; ox.m_direction(0)=1.; return ox;}
     inline static RayBase OX(){ return RayBase(VectorType::Zero(), VectorType::UnitX());} /**< \brief ray along X with origin and position set at zero */
     inline static RayBase OY(){ return RayBase(VectorType::Zero(), VectorType::UnitY());}  /**< \brief ray along Y with origin and position set at zero */
     inline static RayBase OZ(){ return RayBase(VectorType::Zero(), VectorType::UnitZ());}  /**< \brief ray along X with origin and position set at zero */
protected:
    scalar m_distance;/**< \brief distance from origin of the current position */

public:  // public variables
    bool m_alive;  /**<  \brief  boolean variable which defines if aray should be propagated further or not
          *
          *   new rays are always created alive. m_alive is set to false when no intercept is found with or no diffraction is possible from a surface)
          */

};


typedef RayBase<long double> Rayld;
typedef RayBase<double> Rayd;

#endif // RAYBASE_H
#endif // header guard

