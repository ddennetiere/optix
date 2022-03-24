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
#include <cstdarg>
//#include <stdexcept>

typedef Ray<FloatType>  RayType;/**< \brief Complete ray with wavelength and metadata (photometric and polar weights) */
typedef RayBase<FloatType>  RayBaseType;  /**< \brief base  class of rays for intercept and propagation computations  */

//
// /** \brief Structure to hold spot and caustic diagrams
// *
// * For spot diagrams (\ref SpotDiagram) each spot records the ray position and direction with 4 numers X, Y, dX/dZ, dY/dZ
// * \n For caustic diagrams (\ref CausticDiagram), each spot records the position on the ray (X,Y,Z) which has the minimum distance to the alignment chief ray.
// * \n For caustic also, the sum of m_count and m_lost might be smaller than the number of rays emitted from the source, since some rays almost parallel
// * to the reference axes might be discarded.
// */
//template<int Vsize>
//class DiagramType{  // if this structure is changed, mind it is aligned on 8 byte boundaries
//public:
//    const int m_dim=Vsize; /**< \brief number of components stored in each spot vector */
//    int m_reserved=0;   /**< \brief maximum number of spot vectors the m_spots storage area was allocated */
//    int m_count=0;    /**< \brief number of point in returned the spot diagram */
//    int m_lost=0;  /**< \brief number of ray lost in propagation from source */
//    double* m_min;    /**< \brief minimum values of the (Vsize) components (X, Y, dX/dZ, dY/dZ  or X, Y Z) */
//    double* m_max;    /**< \brief maximum values of the (Vsize) components  */
//    double* m_mean;   /**< \brief average value of each of the (Vsize) components */
//    double* m_sigma;  /**< \brief RMS value of each of the (Vsize) components */
//    double* m_spots;   /**<  \brief The   (Vsize) x m_reserved  array of the spot vectors (in component major order) */
//// methods
//    DiagramType():m_count(0),m_spots(NULL)
//    {
//       m_min=new double[Vsize];
//       m_max=new double[Vsize];
//       m_mean=new double[Vsize];
//       m_sigma=new double[Vsize];
//    }
//  //  DiagramType(int n):m_count(n), m_spots(new double[Vsize*n]){}  Obsolete
//    ~DiagramType()
//    {
//        if(m_min) delete [] m_min;
//        if(m_max) delete [] m_max;
//        if(m_mean) delete [] m_mean;
//        if(m_sigma) delete [] m_sigma;
//        if(m_spots) delete [] m_spots;
//    }
//};

/** \brief Structure to hold spot and caustic diagrams
 *
 * For spot diagrams (\ref Surface::getSpotDiagram) each spot records the 2D ray position and direction with  X, Y, dX/dZ, dY/dZ
 * then  the wavelength and up to 4 Stokes parameters
 *
 * \n For impactData each spots records the 3D position and direction , then the wavelength and up to 4 stokes parameters (according to the structure size m_dim
 * \n For caustic diagrams (\ref Surface::getCaustic), each spot records the position on the ray (X,Y,Z) which has the minimum distance to the alignment chief ray, and the wavelength
 * \n For caustic also, the sum of m_count and m_lost might be smaller than the number of rays emitted from the source, since some rays almost parallel
 * to the reference axes might be discarded.
 *
 */
class Diagram{  // if this structure is changed, mind it is aligned on 8 byte boundaries
public:
    const int m_dim; /**< \brief number of components stored in each spot vector */
    int m_reserved=0;   /**< \brief maximum number of spot vectors the m_spots storage area was allocated */
    int m_count=0;    /**< \brief number of point in returned the spot diagram */
    int m_lost=0;  /**< \brief number of ray lost in propagation from source */
    double* m_min;    /**< \brief minimum values of the (m_dim) components (X, Y, dX/dZ, dY/dZ  or X, Y Z, dirX, dirY, dirZ) */
    double* m_max;    /**< \brief maximum values of the (m_dim) components  */
    double* m_mean;   /**< \brief average value of each of the (m_dim) components */
    double* m_sigma;  /**< \brief RMS value of each of the (m_dim) components */
    double* m_spots;   /**<  \brief The   (m_dim) x m_reserved  array of the spot vectors (in component major order) */
// methods
    Diagram(int Vsize):m_dim(Vsize), m_count(0), m_spots(NULL)
    {
       m_min=new double[Vsize];
       m_max=new double[Vsize];
       m_mean=new double[Vsize];
       m_sigma=new double[Vsize];
    }
  //  DiagramType(int n):m_count(n), m_spots(new double[Vsize*n]){}  Obsolete
    ~Diagram()
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



/** \brief Multidimensional array class
 */
template <typename scalar_, size_t ndims_>
class ndArray
{
  public:

    /** \brief default constructor; create a multidimensional array of size zero (empty storage)
     */
    ndArray():storage(NULL){dims.fill(0); }

    /** \brief create a multidimensional array of specifies size and allocate the corresponding storage
     *
     * \param N0 size of the internal (faster varying index) dimension
     * \param ...  one size_t value for the size of the corresponding dimension, ordered from internal to external.
     *  Dimensions equal or above ndims_ are ignored,; missing dimensions are set to 1
     */
    ndArray(size_t N0, ...)
    {
        dims.fill(1);
        dims[0]=N0;
        va_list args;
        va_start(args, N0);
        size_t length=N0;
        for (size_t i = 1; i < ndims_; ++i)
        {
            dims[i]=va_arg(args, size_t);
            length*=dims[i];
        }
        va_end(args);
        storage=new scalar_[length];
        ownStorage=true;
    }

    ndArray(C_ndArray & cndArray)
    {
        if(cndArray.ndims != ndims_)
            throw std::invalid_argument("The number of required dimension does't match the C_ndArray setting");
        size_t length=1;
        size_t* idim=dims.data(), *jdim=(size_t*)cndArray.storage;
        for (int i =0; i< ndims_; ++i, ++idim, ++jdim )
        {
            *idim=*jdim;
            length*=*jdim;
        }
        if(length*sizeof(scalar_)+ ndims_*sizeof(size_t) < cndArray.allocatedStorage)
            throw std::length_error("The allocated storage in the C_ndArray is too small to receive the requested array");
        storage=cndArray.storage+ndims_*sizeof(size_t);
    }
    /** \brief change the dimension sizes of a multidimensional array of specifies size
     *
     *  If the new size (product of the dimensions) is equal to the old size, no storage réassignation is performed and the stored data are preserved.
     *  If storage is not owned the change in dimension should be reflected in the storage owning entity
     *  \n In all other case the old storage is destroyed and a new one is reallocated. All stored data are lost.
     * \param N0 size of the internal (faster varying index) dimension
     * \param ...  one size_t value for the size of the corresponding dimension, ordered from internal to external.
     *  Dimensions equal or above ndims_ are ignored,; missing dimensions are set to 1
     */
    void resize(size_t N0, ...)
    {
        std::array<size_t,ndims_> newdims;
        newdims.fill(1);
        newdims[0]=N0;
        va_list args;
        va_start(args, N0);
        size_t oldlength=dims[0],newlength=N0;
        for (size_t i = 1; i < ndims_; ++i)
        {
            newdims[i]=va_arg(args, size_t);
            oldlength*=dims[i];
            newlength*=newdims[i];
        }
        va_end(args);
        if(oldlength!= newlength)
        {
            if(!ownStorage)
                throw runtime_error("Cannot realloc not owned storage");
            delete[] storage;
            storage=new scalar_[newlength];
        }
        dims=newdims;

    }
    /** \brief destructor: release the allocated storage and release the object
     *
     */
    ~ndArray(){delete[] storage;}
    /** \brief return the size of all dimensions of the multidimensional array
     *
     * \return a std::array containing the dimensions sizes
     *
     */
    std::array<size_t,ndims_> dimensions(){return dims;}
    /** \brief Gets a pointer to the storage area
     *
     * \return a pointer to the data storage
     *
     */
    scalar_* data() {return storage;}
  protected:
    bool ownStorage=false;
    std::array<size_t,ndims_> dims; /**< \brief dimension vector */
    scalar_ * storage;/**< pointer to the stored data */
};



//typedef DiagramType<4> SpotDiagram;  /**< \brief Storage data structure of spot-diagrams */
//typedef DiagramType<5> SpotDiagramExt;  /**< \brief Storage data structure of spot-diagrams + wavelength*/
// typedef DiagramType<4> CausticDiagram; /**< \brief Storage data structure of caustic diagrams */
//typedef DiagramType<7> ImpactData;/**< \brief Storage data structure to get full available impact information  */

#endif // TYPES_H_INCLUDED
