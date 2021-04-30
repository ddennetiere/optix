#ifndef CTYPES_H_INCLUDED
#define CTYPES_H_INCLUDED

/**
*************************************************************************
*   \file       ctypes.h

*
*   \brief     C types and structs defined in the OptiX interface
*
*
*
*
*   \author             François Polack  <francois.polack@synchroton-soleil.fr>
*   \date               Creation : 2021-04-30

*   \date               Last update: 2021-04-30

*
*
 ***************************************************************************/
//#include <cstdint>
#include <inttypes.h>

typedef long double FloatType ;/**< \brief the base type of all floating type ray tracing computation of the library */

/**
*   \ingroup enums
*   \brief Unit category
 */
enum UnitType/*:int32_t*/{
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
 */ /*  explicit underlying type is uint32_t */
enum ParameterGroup /*:uint32_t*/{
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
enum ParameterFlags/*:uint32_t*/{
    NotOptimizable=1, /**< The parameter cannnot be optimized */

//    Uniform=0x10,  /**< Uniform random generator (value=0)*/
//    Gaussian=0x20, /**< Gaussian random (value=sigma) */
//    Grided=0x80    /**< Grided (value=stepsize) */
};
#ifdef __cplusplus
/** \brief Keyed access class for the numeric parameters of an optical surface
 */
struct Parameter{
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
#else
struct Parameter{
    double value;
    double bounds[2];
    double multiplier;
    enum UnitType type;
    enum ParameterGroup group;
    uint32_t flags;
};
#endif // __cplusplus


/** \brief Structure designed to receive spot diagram
 *
 *  The calling program must define the m_dim member to the appropriate spot vector size,
 *  and m_reserved to indicate the maximum number of spots the storage area, m_spots, can accept
 */
struct C_DiagramStruct
{
    const int m_dim; /**< \brief number of components which are stored in each spot vector. Must be set by the calling program with appropriate value*/
    int m_reserved;   /**< \brief maximum number of spot vectors the m_spots storage area can store  */
    int m_count;    /**< \brief number of point in the returned spot diagram.  */
    int m_lost;  /**< \brief number of ray lost in propagation from source */
    double* m_min;    /**< \brief minimum values of the  components. This array must be allocated with a minimum size of m_dim  */
    double* m_max;    /**< \brief maximum values of the  components. This array must be allocated with a minimum size of m_dim  */
    double* m_mean;   /**< \brief average value of each of the components. This array must be allocated with a minimum size of m_dim */
    double* m_sigma;  /**< \brief RMS value of each of the components. This array must be allocated with a minimum size of m_dim */
    double* m_spots; /**<  \brief reference of an array the size of which must be larger than  m_dim* m_reserved, into which the spot vectors will be stored in component major order. */

};


/** \brief C equivalent structure to WavefrontData, holding a Wavefront map */
struct C_WFtype{
  double m_bounds[4];       /**< aray conntaining the X and Y bounds of the mapped area in order X min, X max, Ymin, Ymax */
  double * m_WFdata;        /**< pointer to the WF height data in memory */
  size_t m_dataSize[2];     /**< dimensions of the mapped array in order nX, nY */
};

#endif // CTYPES_H_INCLUDED
