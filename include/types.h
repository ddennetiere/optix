#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED

////////////////////////////////////////////////////////////////////////////////
/**
*      \file           types.h
*
*      \brief         basic types used thoughout OptiX
*                      Surface types are defined in opticalelements.h
*      \author         François Polack <francois.polack@synchroton-soleil.fr>
*      \date        2020-11-13  Creation
*      \date         Last update
*

*/
///////////////////////////////////////////////////////////////////////////////////
//
//             REVISIONS
//
////////////////////////////////////////////////////////////////////////////////////

#include "ray.h"

typedef long double FloatType ;/**< \brief the base type of all floating type ray tracing computation of the library */
typedef Ray<FloatType>  RayType;/**< \brief Complete ray with wavelength and metadata (photometric and polar weights) */
typedef RayBase<FloatType>  RayBaseType;  /**< \brief base  class of rays for intercept and propagation computations  */

/**
*   \ingroup enums
*   \brief Unit category
 */
enum UnitType:int32_t{
    Dimensionless =0,   /**< parameter is dimensionless  */
    Angle =1,           /**< parameter is an angle  */
    Distance=2,         /**< parameter is a distance  */
    InverseDistance=-1, /**< parameter is the inverse of a distance */
    DistanceMoins1=-1,  /**< parameter is the inverse of a distance (line density of poly gratings)*/
    DistanceMoins2=-2,  /**< parameter is the inverse of a squared distance (poly gratings) */
    DistanceMoins3=-3,  /**< parameter is the inverse of a cubed distance (poly gratings) */
    DistanceMoinsN=-4   /**< parameter is the inverse of the Nth power of a distance (poly gratings) */
};

/** \ingroup enums
 * \brief Impact recoording flag
 */
enum RecordMode{
    RecordNone=0,   /**< do not record impacts on this surface */
    RecordInput=1,  /**< record the inpacts in entrance space  */
    RecordOutput=2  /**< record the impacts in exit space */
};

/** \ingroup enums
 *  \brief Frame identification for space conversion
 *
 *   Absolute frames are oriented parallel to laboratory frame. Local frame have their origin at intersection of the considered surface with the chief ray (alignment ray)
 *   implicit type is int*/
enum FrameID{
    GeneralFrame=0,     /**< Absolute laboratory frame */
    LocalAbsoluteFrame=1, /**< Absolute frame with origin on the surface */
    AlignedLocalFrame=2, /**< Local rame, with origin on the surface, axe OZ is along the chief ray and OY is in the deviation palane of the last
                            *   preceeding reflective element. Transmissive elements do not change the AlignedLocalFrame */
    SurfaceFrame=3      /**< Local frame used to describe a surface. Origin is at surface intercept wit the chief ray. Oz is along the surface normal (at origin).
                        *   OX is the tangential axis for reflective elements. */
};

/** \ingroup enums
 * \brief parameter group indicator intended for property pages
 *
 *  explicit underlying type is uint32_t */
enum ParameterGroup:uint32_t{
    BasicGroup=0,   /**< The parameter belongs to the base group common to all surfaces */
    ShapeGroup=1,   /**< The parameter describes a surface shape */
    SourceGroup=2,  /**< The parameter describes a source*/
    GratingGroup=3  /**<  the parameter describes a grating*/
};

/** \ingroup enums
 * \brief modifier flags applicable to parameters
 * only NotPtimizable used presently
 *  Range   | applies to | description
 *  ------- | ---------- | ------------
 * 0 - 0xF  | any object |  optimization and computation modifiers
 * 0x10 - 0xF0 | Sources  | Type of ray generator associated with the parameter
 * flag > 0xF  | none     | reserved for future use
 */
enum ParameterFlags:uint32_t{
    NotOptimizable=1, /**< The parameter cannnot be optimized */

//    Uniform=0x10,  /**< Uniform random generator (value=0)*/
//    Gaussian=0x20, /**< Gaussian random (value=sigma) */
//    Grided=0x80    /**< Grided (value=stepsize) */
};

/** \brief Keyed access class for the numeric parameters of an optical surface
 */
class Parameter{
public:
    double value=0; /**< \brief the internal value for in internal unit (m, rad, etc)*/
    double bounds[2]={0,0};/**< \brief  boundary value for optimization */
    double multiplier=1.; /**< \brief multiplier for display */
    UnitType type=Dimensionless;  /**< \brief type of unit. This field is read-only outside Surface class*/
    ParameterGroup group=BasicGroup; /**< \brief parameter group. This field is read-only outside Surface class*/
    uint32_t flags=0; /**< \brief is this an optimization parameter ?*/
// methods:
    Parameter(){}
    inline Parameter(double newvalue, UnitType newtype, double newmultiplier=1.):/**<  \brief standard constructor sets optimization bounds to  parameter value */
        value(newvalue), multiplier(newmultiplier), type(newtype){bounds[0]=bounds[1]=value;}
};

struct C_LinkType
{
        string name;
        uint32_t parent=0;
        uint32_t prev=0;
        uint32_t next=0;
};

/** \brief Structure to hold spot and caustic diagrams
 *
 * For spot diagrams (\ref SpotDiagram) each spot records the ray position and direction with 4 numers X, Y, dX/dZ, dY/dZ
 * \n For caustic diagrams (\ref CausticDiagram), each spot records the position on the ray (X,Y,Z) which has the minimum distance to the alignment chief ray.
 * \n For caustic also, the sum of m_count and m_lost might be smaller than the number of rays emitted from the source, since some rays almost parallel
 * to the reference axes might be discarded.
 */
template<int Vsize>
class DiagramType{  // if this structure is changed, mind it is aligned on 8 byte boundaries
public:
    const int m_dim=Vsize; /**< \brief number of components stored in each spot vector */
    int m_reserved=0;   /**< \brief maximum number of spot vectors the m_spots storage area was allocated */
    int m_count=0;    /**< \brief number of point in returned the spot diagram */
    int m_lost=0;  /**< \brief number of ray lost in propagation from source */
    double* m_min;    /**< \brief minimum values of the (Vsize) components (X, Y, dX/dZ, dY/dZ  or X, Y Z) */
    double* m_max;    /**< \brief maximum values of the (Vsize) components  */
    double* m_mean;   /**< \brief average value of each of the (Vsize) components */
    double* m_sigma;  /**< \brief RMS value of each of the (Vsize) components */
    double* m_spots;   /**<  \brief The   (Vsize) x m_reserved  array of the spot vectors (in component major order) */
// methods
    DiagramType():m_count(0),m_spots(NULL)
    {
       m_min=new double[Vsize];
       m_max=new double[Vsize];
       m_mean=new double[Vsize];
       m_sigma=new double[Vsize];
    }
    DiagramType(int n):m_count(n), m_spots(new double[Vsize*n]){}
    ~DiagramType()
    {
        if(m_min) delete [] m_min;
        if(m_max) delete [] m_max;
        if(m_mean) delete [] m_mean;
        if(m_sigma) delete [] m_sigma;
        if(m_spots) delete [] m_spots;
    }
};

/** \brief Structure designed to receive spot diagram
 *
 *  The calling program must define the m_dim member to the appropriate spot vector size,
 *  and m_reserved to indicate the maximum number of spots the storage area, m_spots, can accept
 */
struct C_DiagramStruct
{
    const int m_dim; /**< \brief number of components which are stored in each spot vector. Must be set by the calling program with appropriate value*/
    int m_reserved=0;   /**< \brief maximum number of spot vectors the m_spots storage area can store  */
    int m_count=0;    /**< \brief number of point in the returned spot diagram.  */
    int m_lost=0;  /**< \brief number of ray lost in propagation from source */
    double* m_min;    /**< \brief minimum values of the  components. This array must be allocated with a minimum size of m_dim  */
    double* m_max;    /**< \brief maximum values of the  components. This array must be allocated with a minimum size of m_dim  */
    double* m_mean;   /**< \brief average value of each of the components. This array must be allocated with a minimum size of m_dim */
    double* m_sigma;  /**< \brief RMS value of each of the components. This array must be allocated with a minimum size of m_dim */
    double* m_spots; /**<  \brief reference of an array the size of which must be larger than  m_dim* m_reserved, into which the spot vectors will be stored in component major order. */

};


/** \brief Structure containing a wavefront map
 *  \see see also C_WFtype
 */
struct WavefrontData{
    Array22d m_bounds;/**< 2 X 2 array of doubles containing the bounds of the map */
    ArrayXXd m_WFdata;/**< Array of double holding the wavefront heights*/
};

/** \brief C equivalent structure to WavefrontData, holding a Wavefront map */
struct C_WFtype{
  double m_bounds[4];       /**< aray conntaining the X and Y bounds of the mapped area in order X min, X max, Ymin, Ymax */
  double * m_WFdata;        /**< pointer to the WF height data in memory */
  size_t m_dataSize[2];     /**< dimensions of the mapped array in order nX, nY */
};


typedef DiagramType<4> SpotDiagram;  /**< \brief Storage data structure of spot-diagrams */
typedef DiagramType<5> SpotDiagramExt;  /**< \brief Storage data structure of spot-diagrams + wavelength*/
typedef DiagramType<4> CausticDiagram; /**< \brief Storage data structure of caustic diagrams */

#endif // TYPES_H_INCLUDED
