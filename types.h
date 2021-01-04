#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED

////////////////////////////////////////////////////////////////////////////////
/**
*      \file           types.h
*
*      \brief         TODO  fill in file purpose
*
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

typedef long double FloatType ;
typedef Ray<FloatType>  RayType;
typedef RayBase<FloatType>  RayBaseType;

/**
*   \ingroup enums
*   \brief Unit category
 */
enum UnitType:uint32_t{
    Dimensionless =0,   /**< dparameter is dimensionless  */
    Angle =1,           /**< parameter is an angle  */
    Distance=2,         /**< parameter is a distance  */
    InverseDistance=3   /**< parameter is the inverse of a distance */
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
 */
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
 */
enum ParameterGroup:uint32_t{
    BasicGroup=0,   /**< The parameter belongs to the base group common to all surfaces */
    ShapeGroup=1,   /**< The parameter describes a surface shape */
    SourceGroup=2,  /**< The parameter describes a source*/
    GratingGroup=3  /**<  the parameter describes a grating*/
};

/** \ingroup enums
 * \brief modifier flags applicable to parameters
 *
 *  Range   | applies to | description
 *  ------- | ---------- | ------------
 * 0 - 0xF  | any object |  optimization and computation modifiers
 * 0x10 - 0xF0 | Sources  | Type of ray generator associated with the parameter
 * flag > 0xF  | none     | reserved for future use
 */
enum ParameterFlags:uint32_t{
    Optimizable=1, /**< The parameter can be optimizes */
    Uniform=0x10,  /**< Uniform random generator (value=0)*/
    Gaussian=0x20, /**< Gaussian random (value=sigma) */
    Grided=0x80    /**< Grided (value=stepsize) */
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

/** \brief Structure to hold spot diagrams
 *
 * each spot records the ray position and dircetion with 4 numers X, Y, dX/dZ, dY/dZ*/
template<int Vsize>
class DiagramType{
public:
    int m_dim=Vsize;
    int m_count;    /**< number of point in the spot diagram */
    int m_lostCount=0;
    double m_min[Vsize];    /**< minimum values of the 4 components X, Y, dX/dZ, dY/dZ */
    double m_max[Vsize];    /**< maximum values of the 4 components  */
    double m_mean[Vsize];   /**< average value of each of the 4 components */
    double m_sigma[Vsize];  /**< RMS value of each of the 4 components */
    double* m_spots;    /**<  The  4 x m_count  array of the spot vectors (in component major order) */
// methods
    DiagramType():m_count(0),m_spots(NULL){}
    DiagramType(int n):m_count(n), m_spots(new double[Vsize*n]){}
    ~DiagramType(){if(m_spots) delete [] m_spots;}
};
struct WavefrontData{
    Array22d m_bounds;
    ArrayXXd m_WFdata;
};

struct C_WFtype{            /**< \brief C equivalebnt structure to WavefrontData, holding a Wavefront map */
  double m_bounds[4];       /**< aray conntaining the X and Y bounds of the mapped area in order X min, X max, Ymin, Ymax */
  double * m_WFdata;        /**< pointer to the WF height data in memory */
  size_t m_dataSize[2];     /**< dimensions of the mapped array in order nX, nY */
};

typedef DiagramType<4> SpotDiagram;
typedef DiagramType<3> CausticDiagram;

#endif // TYPES_H_INCLUDED
