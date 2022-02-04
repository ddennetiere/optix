#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED

////////////////////////////////////////////////////////////////////////////////
/**
*      \file           types.h
*
*      \brief         basic types used throughout OptiX
*                     Surface types are defined in opticalelements.h
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
#include "ctypes.h"
#include "ray.h"

typedef Ray<FloatType>  RayType;/**< \brief Complete ray with wavelength and metadata (photometric and polar weights) */
typedef RayBase<FloatType>  RayBaseType;  /**< \brief base  class of rays for intercept and propagation computations  */


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

/** \brief Structure containing a wavefront map
 *  \see see also C_WFtype
 */
struct WavefrontData{
    Array22d m_bounds;/**< 2 X 2 array of doubles containing the bounds of the map */
    ArrayXXd m_WFdata;/**< Array of double holding the wavefront heights*/
};

/** \brief Structure to register the parameters of a link between two elements in Solemio
 */
struct SolemioLinkType
{
        string name;        /**< Name of the linked element */
        uint32_t parent=0;  /**< Currently unused. Might be removed in future */
        uint32_t prev=0;    /**< reference ID of the previous element in Solemio element chain. */
        uint32_t next=0;    /**< reference ID of the next element in Solemio element chain.  */
};


typedef DiagramType<4> SpotDiagram;  /**< \brief Storage data structure of spot-diagrams */
typedef DiagramType<5> SpotDiagramExt;  /**< \brief Storage data structure of spot-diagrams + wavelength*/
typedef DiagramType<4> CausticDiagram; /**< \brief Storage data structure of caustic diagrams */
typedef DiagramType<7> ImpactData;/**< \brief Storage data structure to get full available impact information  */

#endif // TYPES_H_INCLUDED
