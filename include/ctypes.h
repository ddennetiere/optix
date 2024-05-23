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
#ifdef __cplusplus
   #include "EigenSafeInclude.h"
   #include <iostream>
#else
    #include <stdalign.h>
#endif

typedef long double FloatType ;/**< \brief the base type of all floating type ray tracing computation of the library */

/**
*   \ingroup enums
*   \brief Unit category
 */
enum UnitType/*:int32_t*/{
    Dimensionless =0,   /**< parameter is dimensionless  */
    Angle =1,           /**< parameter is an angle in radians  */
    Distance=2,         /**< parameter is a distance  in meters */
    InverseDistance=-1, /**< parameter is the inverse of a distance \f$ in \ meter^{-1} \f$ */
    DistanceMoins1=-1,  /**< parameter is the inverse of a distance \f$ in \ meter^{-1} \f$  (line density of poly gratings)*/
    DistanceMoins2=-2,  /**< parameter is the inverse of a squared distance \f$ in \ meter^{-2} \f$ (poly gratings) */
    DistanceMoins3=-3,  /**< parameter is the inverse of a cubed distance \f$ in \ meter^{-3} \f$ (poly gratings) */
    DistanceMoinsN=-4   /**< parameter is the inverse of the Nth power of a distance \f$ in \ meter^{-N} \f$ (poly gratings) */
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
    AlignedLocalFrame=2, /**< Local frame, with origin on the surface, axe OZ is along the chief ray and OY is in the deviation plane of the last
                            *   preceding reflective element. Transmissive elements do not change the AlignedLocalFrame */
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
 *
 */
enum ParameterFlags/*:uint32_t*/{
    NotOptimizable=1, /**< The Parameter cannnot be optimized */
    ArrayData=0x8      /**< The Parameter contains an array of double \n pointed by the Parameter::paramArray member */
};

/** \brief C struct and C++ wrapper class to manipulate array type parameters. Companion struct of Parameter struct
 */
typedef struct  __ArrayParameter
{
    alignas(16) int64_t  dims[2]; /**< \brief The dimensions of the array*/
    double *data; /**< \brief a pointer to the first element of the array. The array is owned by the creator, who is in charge of its deletion*/
#ifdef __cplusplus
    // the following functions are only defined in C++
    /** \brief return the array data as a Matrix    */
    inline Eigen::Map<Eigen::MatrixXd> matrix() {return Eigen::Map<Eigen::MatrixXd>(data, dims[0], dims[1]);}
    /** \brief construct and reserve parameter array space
    *   \param rows number of rows of the array
    *   \param cols number of columns of the array
    */
    inline __ArrayParameter(int rows, int cols){dims[0]=rows; dims[1]= cols; data=new alignas(16) double[rows*cols];}

    inline __ArrayParameter(){}/**< \brief default empty constructor */

    /** \brief copy constructor with deep copy
    *   \param aparam the parameter array to copy
     */
    inline __ArrayParameter(const __ArrayParameter &aparam)
    {
        // std::cout <<"construct a copy of a " << aparam.dims[0] << " x " << aparam.dims[1] << " elements array\n";
        memcpy(dims, aparam.dims,2*sizeof(int64_t) );
        data=new alignas(16) double[dims[0]* dims[1]];
        memcpy(data, aparam.data, sizeof(double)*dims[0]*dims[1]);
    }

    /** \brief deep copy assignment
     * \param aparam the Arrayparameter to copy
     */
    inline __ArrayParameter& operator=(const __ArrayParameter & aparam)
    {
      //  std::cout <<"assign a copy of a " << aparam.dims[0] << " x " << aparam.dims[1] << " elements array\n";
        memcpy(dims, aparam.dims,2*sizeof(int64_t) );
        if(data)
            delete [] data;
    //    std::cout << "Array parameter data was re-assigned\n";
        data=new alignas(16) double[dims[0]* dims[1]];
        memcpy(data, aparam.data, sizeof(double)*dims[0]*dims[1]);
        return *this;
    }

    inline __ArrayParameter& operator=(const Eigen::Ref<Eigen::ArrayXXd> & dat )
    {
    //    std::cout << " reassigning array from eigen " << dat.rows() << " x " << dat.cols() << std::endl;
        dims[0]=dat.rows();
        dims[1]=dat.cols();
        if(data)
            delete [] data;
     //  std::cout << "Array parameter data was re-defined\n";
        data=new alignas(16) double[dims[0]* dims[1]];
        memcpy(data, dat.data(), sizeof(double)*dims[0]*dims[1]);
        return *this;
    }

    /** \brief destructor with memory cleaning
     */
    ~__ArrayParameter(){if(data) delete [] data;}

    void dump()
    {
        std::cout << "\t\t dims=[" << dims[0] << ", " << dims[1] << "]\n";
        std::cout << "\t\t data @ " << data << "\n";
        for(int i=0, k=0; i< dims[0]; ++i)
        {
            k=i;
            std::cout << "\t\t\t";
            for(int j=0; j< dims[1]; ++j,k+=dims[0])
                std::cout << data[k] << " ";
            std::cout << "\n";
        }
    }
#endif
} ArrayParameter; /**< \brief struct defining an Array type parameter for the interface functions and inside a \ref Parameter struct  */


/** \brief Keyed access class for the numeric parameters of an optical surface
 *
 *  The object contains either a single double value in member value, or a pointer to a ArrayParameter struct containing
 *  the array of double values, if the bit 0x08 of member flags is set
 */
typedef struct __Parameter{
#ifdef __cplusplus
    union {
        double value; /**< \brief if bit x08 of flags is unset, the internal value of the parameter in internal unit (m, rad, etc)*/
        ArrayParameter * paramArray=0; /**< \brief if bit x08 of flags is set, the address of the array parameter structure */
    };
    double bounds[2]={0,0};/**< \brief  boundary value for optimization, unused if ArrayData bit of flags is set*/
    double multiplier=1.; /**< \brief multiplier for display */
    UnitType type=Dimensionless;  /**< \brief type of unit. This field is read-only outside ElementBase class*/
    ParameterGroup group=BasicGroup; /**< \brief parameter group. This field is read-only outside ElementBase class*/
    uint32_t flags=0; /**< \brief non null if parameter is not optimizable. This field is read-only outside ElementBase class */
// methods: (only defined in C++)
    __Parameter(){}     /**< \brief default constructor */
    /** \brief constructor with value assignment
     * \param newvalue the  parameter value
     * \param newtype  UnitType of the parameter
     * \param newmultiplier=1. multiplier value
     */
    inline __Parameter(double newvalue, UnitType newtype, ParameterGroup newgroup, double newmultiplier=1.):/**<  \brief standard constructor sets optimization bounds to  parameter value */
        value(newvalue), multiplier(newmultiplier), type(newtype),group(newgroup), flags(0) {bounds[0]=bounds[1]=newvalue;}
    /** \brief constructor with array assignment
     * \param newparamArray the  parameter array of values
     * \param newtype  UnitType of the parameter values
     * \param newmultiplier=1. multiplier value
     */
    inline __Parameter(uint64_t Nx, uint64_t Ny, UnitType newtype, ParameterGroup newgroup, double newmultiplier=1.):/**<  \brief standard constructor sets optimization bounds to  parameter value */
        paramArray(new ArrayParameter(Nx,Ny)), multiplier(newmultiplier), type(newtype), group(newgroup), flags(ArrayData|NotOptimizable)
        {bounds[0]=bounds[1]=0;}

    /** \brief Copy constructor with internal array management
     *
     * \param param The parameter to deep copy
     *
     */
    inline __Parameter(const __Parameter & param)
    {
        if(param.flags & ArrayData)
        {
           // std::cout <<"Construct a copy of a parameter containing a " << param.paramArray->dims[0] << " x " << param.paramArray->dims[1] << " elements array\n";
            paramArray= new ArrayParameter(*param.paramArray);
        }
        else
            value=param.value;
       memcpy(bounds,param.bounds, 3*sizeof(double)) ;
       type=param.type;
       group=param.group;
       flags=param.flags;
    }
    /** \brief deep copy assignment operator
     * \param param the parameter to be copied
     */
    inline __Parameter& operator=(const __Parameter & param)
    {
        if( (flags & ArrayData) && paramArray )
           delete paramArray;
        if(param.flags & ArrayData)
        {
           // std::cout <<"assign a copy of a parameter containing a " << param.paramArray->dims[0] << " x " << param.paramArray->dims[1] << " elements array\n";
            paramArray= new ArrayParameter(*param.paramArray);
        }
        else
            value=param.value;
       memcpy(bounds,param.bounds, 3*sizeof(double)) ;
       type=param.type;
       group=param.group;
       flags=param.flags;
       return *this;
    }

    /** \brief Copy the Parameter object into the parameter struct passed in argument.
     * (This function makes limited checking of the destination structure validity and must be used with caution)
     *
     * \param destParam destination Parameter struct in which the object will be copied. NO Check is done on the available
     *      memory size of the destinationparamArray
     * \return 0 when successful;  1 if  if the flags members of both parameter don't match, 2 if the paramArray pointer
     *  of the destination struct is invalid, 3 if the data pointer is invalid or the product of dimensions of destination
     *  is smaller than the source one
     */
    inline int copy(__Parameter &destParam)
    {
        if((flags & ArrayData)!= (destParam.flags & ArrayData))
            return 1;

        if(flags & ArrayData)
        {

            if(!destParam.paramArray)
                return 2;
            int64_t Ndest=destParam.paramArray->dims[0]*destParam.paramArray->dims[1];
            int64_t Nsrc=paramArray->dims[0]*paramArray->dims[1];
            if((!destParam.paramArray->data)  || (Ndest < Nsrc ))
               return 3;
            memcpy(destParam.paramArray->dims,paramArray->dims,2*sizeof(int64_t));
            memcpy(destParam.paramArray->data,paramArray->data,paramArray->dims[0]*paramArray->dims[1]*sizeof(double) );
        }
        else
            destParam.value=value;
       memcpy(destParam.bounds,bounds, 3*sizeof(double)) ;
       destParam.type=type;
       destParam.group=group;
       destParam.flags=flags;
       return 0;
    }

    /** \brief destructor with memory cleaning*/
    inline ~__Parameter()
    {
        if((flags & ArrayData) && paramArray)
        {
            delete paramArray;
     //       std::cout <<"Array parameter was destroyed\n";
        }
    }

    void dump()
    {  if(flags & ArrayData)
       {
           std::cout << "\t ArrayParameter @ " <<  paramArray << "\n";
           paramArray->dump();
       }
        else
            std::cout << "\t value=" << value << "\n";

       std::cout << "\t bounds=[" << bounds[0] << ", " << bounds[1] << "]\n";
       std::cout << "\t multiplier=" << multiplier << "\n";
       std::cout << "\t type=" << type << "\n";
       std::cout << "\t group=" << group << "\n";
       std::cout << "\t flags=" <<std::hex << flags << std::dec << "\n";
    }

#else // the same structure viewed from C
    union {;
        double value;  /**< \brief if bit x08 of flags is unset, the internal value of the parameter in internal unit (m, rad, etc)*/
        ArrayParameter * paramArray; /**< \brief the address of the array parameter structure if bit x08 of flags is set*/
    };
    double bounds[2];  /**< \brief  boundary value for optimization, unused if ArrayData bit of flags is set */
    double multiplier;  /**< \brief multiplier for display */
    enum UnitType type;  /**< \brief type of unit. This field is read-only outside ElementBase class*/
    enum ParameterGroup group;  /**< \brief parameter group. This field is read-only outside ElementBase class*/
    uint32_t flags;  /**< \brief non null if parameter is not optimizable. This field is read-only outside ElementBase class */
#endif // __cplusplus

}Parameter; /**< \brief Struct defining a parameter for the interface functions */


/** \brief Structure designed to receive spot diagram
 *
 *  The calling program must define the m_dim member to the appropriate spot vector size,
 *  and m_reserved to indicate the maximum number of spots the storage area, m_spots, can accept
 */
typedef struct __C_DiagramStruct
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

}C_DiagramStruct; /**< \brief struct for returning a spot diagram through the interface functions */


/** \brief C equivalent structure to WavefrontData, holding a Wavefront map */
typedef struct __C_WFtype
{
  double m_bounds[4];       /**< aray conntaining the X and Y bounds of the mapped area in order X min, X max, Ymin, Ymax */
  double * m_WFdata;        /**< pointer to the WF height data in memory */
  size_t m_dataSize[2];     /**< dimensions of the mapped array in order nX, nY */
}C_WFtype; /**< \brief struct containing wavefrontdata returned by some  interface functions  */

/** \brief C structure to get pattern info from an holographic grating
 */
typedef struct __C_GratingPatternInfo{
    double AxialLineDensity[4];/**<array which will receive the coefficients of the line density approximation by a third degree polynomial.\n
                                * The term of degree 0 is the nominal line density at grating center (in lines/m)*/
    double lineTilt;    /**< angle in radian of the angle of the central line of the grating line with the Y axis (in rad)*/
    double lineCurvature; /**< approximate radius of curvature of the central line of the grating (in m)  */
}GratingPatternInfo;



/** \brief C struct to return PSF and other image stacks
*
*  The storage area contains the dimension vector followed by the data array
*  dimension vector dims  and data can be accessed as:\n
*  size_t dims[ndims];      in which dimensions are listed from inner (faster varying) to outer (lower varying) \n
*  < datatype >* data= storage+ ndims*sizeof(size_t)  */
typedef struct __C_ndArray
{
    size_t allocatedStorage; /**< \brief size of allocated storage in bytes. Must be properly filled by the creator */
    size_t ndims;            /**< \brief number of dimensions; will be set by the filling entity*/
    void* storage;           /**< \brief  address of storage area  containing the dimension vector followed by the data */
}C_ndArray;


/** \brief structure holding the parameters needed to radiate a wavefront
*
*  a wavefront is defined as a point source radiating a regular sampling grid in X and Y aperture angles */
typedef struct __WFemission
{
    double wavelength;  /**< \brief The wavelength in m*/
    double Xaperture;   /**< \brief The 1/2 aperture angle in the X direction (in radians) */
    double Yaperture;   /**< \brief The 1/2 aperture angle in the Y direction (in radians) */
    size_t Xsize;       /**< \brief  The number of point of the angular grid in X over the full X aperture*/
    size_t Ysize;       /**< \brief  The number of point of the angular grid in Y over the full Y aperture*/
#ifdef __cplusplus
    char polar='S';     /**< \brief The polarization of the generated rays - it can be: S (along X), P (along Y), R (right circular), L (left circular) are allowed*/
#else
    char polar;   /**< \brief The polarization of the generated rays - it can be: S (along X), P (along Y), R (right circular), L (left circular) are allowed*/
#endif
} WFemission;

/** \brief Parameters needed to integrate the output wavefront (on Legendre products) and compute PSF
 */
typedef struct __PSFparameters
{
#ifdef __cplusplus
    double opdRefDistance;  /**< \brief Distance from the observation plane of the image reference point used for OPD computation (it can be 0) */
    size_t legendreNx;      /**< \brief Degree of the Legendre polynomial base for interpolating the wavefront in the X direction */
    size_t legendreNy;      /**< \brief Degree of the Legendre polynomial base for interpolating the wavefront in the Y direction */

    double psfXpixel;       /**< \brief the X pixel size (in m) of PSF computation: \n  On input : the requested the sampling interval of PSF,
                            *   \n on output: The effectively used pixel size after application of the minimum oversampling factor.*/
    double psfYpixel;       /**< \brief the Y pixel size (in m) of PSF computation: \n  On input : the requested the sampling interval of PSF,
                            *   \n on output: The effectively used pixel size after application of the minimum oversampling factor.*/
    double oversampling=4;   /**< \brief The minimum oversampling of the PSF with respect to the exit aperture of the ray tracing (must be  >1.)
                            * this factor determines the maximum grid step of the computed PSF as lambda/ (ThetaMax-ThetaMin)/oversampling for each dimension */
    size_t xSamples;        /**< \brief Number sample points in X the PSF must be computed at */
    size_t ySamples;        /**< \brief Number sample points in X the PSF must be computed at */
    double zFirstOffset;    /**< \brief First image plane position the PSF must be computed at (Note it is relative to the OPD reference point */
    double zLastOffset;     /**< \brief Last image plane position the PSF must be computed at (Note it is relative to the OPD reference point */
    size_t numOffsetPlanes=1;  /**< \brief Number of planes where the PSF will be computed (if equal to 1, only \b Last plane will be computed */
#else
    double opdRefDistance;  /**< \brief Distance from the observation plane of the image reference point used for OPD computation (it can be 0) */

    size_t legendreNx;      /**< \brief Degree of the Legendre polynomial base for interpolating the wavefront in the X direction */
    size_t legendreNy;      /**< \brief Degree of the Legendre polynomial base for interpolating the wavefront in the Y direction */

    double psfXpixel;       /**< \brief the X pixel size (in m) of PSF computation: \n  On input : the requested the sampling interval of PSF,
                            *   \n on output: The effectively used pixel size after application of the minimum oversampling factor.*/
    double psfYpixel;       /**< \brief the Y pixel size (in m) of PSF computation: \n  On input : the requested the sampling interval of PSF,
                            *   \n on output: The effectively used pixel size after application of the minimum oversampling factor.*/
    double oversampling;   /**< \brief The minimum oversampling of the PSF with respect to the exit aperture of the ray tracing (must be  >1.)
                            * this factor determines the maximum grid step of the computed PSF as lambda/ (ThetaMax-ThetaMin)/oversampling for each dimension */
    size_t xSamples;        /**< \brief Number sample points in X the PSF must be computed at */
    size_t ySamples;        /**< \brief Number sample points in X the PSF must be computed at */
    double zFirstOffset;    /**< \brief First image plane position the PSF must be computed at (Note it is relative to the OPD reference point */
    double zLastOffset;     /**< \brief Last image plane position the PSF must be computed at (Note it is relative to the OPD reference point */
    size_t numOffsetPlanes;  /**< \brief Number of planes where the PSF will be computed (if equal to 1, only \b Last plane will be computed */
#endif
}PSFparameters;

/** \brief structure defining the statistical parameters of a fractal surface
 */
typedef struct __FractalParameters
{
    int32_t nx;     /**< \brief the number of fractal segments of the X PSD ( >=1)*/
    double * exponent_x;    /**< \brief the list of fractal exponents (in principle negative) of the X PSD model one per segment (size  >= nx)*/;
    double * frequency_x;   /**< \brief the transition frequencies between the X fractal segments i \f$ in \ meter^{-1} \f$ . Unused if nx=1 (size > nx-1) */
    int32_t ny;     /**< \brief the number of fractal segments of the Y PSD ( >=1)*/
    double * exponent_y;    /**< \brief the list of fractal exponents of the Y PSD model one per segment (size  >= ny)*/;
    double * frequency_y;   /**< \brief the transition frequencies between the Y fractal segments i \f$ in \ meter^{-1} \f$ . Unused if ny=1 (size > nx-1) */
}FractalParameters;

typedef struct __SurfaceStats
{
    double sigma;
    double sigmaPrimX;
    double sigmaPrimY;
} SurfaceStats;

#endif // CTYPES_H_INCLUDED

