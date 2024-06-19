#ifndef _SURFACE_H
#define   _SURFACE_H

/**
*************************************************************************
*   \file       surface.h
*
*   \brief Definition of Surface base class and surface parameter class definition file
*
*
*
*
*   \author             François Polack  <francois.polack@synchroton-soleil.fr>
*   \date               Creation : 2020-10-05
*   \date               Last update: 2021-02-26
**********************************************************************************/



#include "elementbase.h"  // include many basic headers
#include "ApertureStop.h"
#include "bidimspline.h"  // Needed for surface errors
//#include "surfacegenerator.h"

#ifdef HAS_REFLEX
    #include "CoatingTable.h"
    extern map<string,CoatingTable> coatingTables;
#endif //HAS_REFLEX

using std::cout, std::endl, std::complex ;

// Dans la version Eigen3.3.9 l'inclusion de la classe Tensor par le Header  <unsupported/Eigen/CXX11/Tensor> était
// incompatible avec les classes Polygon et ellipse de Aperture Api
// raison pour laquelle le calculs defront d'onde et de PSF utilisent une classe speciale ndArray (et C_ndArray)

// le bug est désormais résolu en Eingen-3.4.0

// #include <unsupported/Eigen/CXX11/Tensor>



extern bool enableApertureLimit; // global flag defined in interfac.cpp
extern bool enableSurfaceErrors;

// using namespace std; no longer valid in recent c++ releases

class SourceBase;

//inline double oneMinusSqrt1MinusX(double x)
//{
//    if(::fabs(x) > 0.01) return 1.-sqrt(1. - x);
//    return  x*(0.5 + x*(0.125 + x*(0.0625 + x*(0.0390625 + x*(0.02734375 + x*(0.0205078125 + x*0.01611328125))))));
//}


/** \brief Abstract base class of all optical surfaces \n
* (includes a \link generel surface error generator \endlink )\n
*
*
*     Surface is the base class of all shape definition classes like Plane, Quadric, Toroid
*     and behaviour type classes like Transparency, Mirror, Grating.
*
*     Optical element classes might be derived from an agregates of these classes, but they MUST refer to the same surface object.
*      Hence these derived objects must declare the base Surface member as virtual
*
*     General parameters common to all surfaces
*     -----------------------------------------
*     <b> are defined in class  <b href="class_element_base.html#details">ElementBase </b></b>
*
*/
class Surface: public  ElementBase
{
public:

#ifdef HAS_REFLEX

    /** \brief default  constructor (Film) with explicit chaining to previous
    */
    Surface(bool transparent=true, string name="", Surface * previous=NULL):ElementBase(transparent,name,previous),m_recording(RecordNone),
             , m_pCoating(NULL), m_lostCount(0), m_apertureActive(false){}
#else
    Surface(bool transparent=true, string name="", Surface * previous=NULL):ElementBase(transparent,name,previous),m_recording(RecordNone),
              m_lostCount(0), m_apertureActive(false){}
#endif // HAS_REFLEX
    /** \brief virtual destructor
     *   clean the surface error  generator id any
     */
    virtual ~Surface()
    {

        if(m_errorMap)
            delete m_errorMap;
    }

    virtual inline string getOptixClass(){return "Surface";}/**< \brief return the most derived class name of this object */

    virtual inline string getSurfaceClass(){return "Surface";}/**< \brief return the most derived shape class name of this object */

    /** \brief pure virtual defined in derived class Align this surface with respect to the main incident ray according to the parameters,
    *        and defines the related geometric space transforms
    *
    *       \param wavelength the alignment wavelength (used by chromatic elements only)
    *       \return  0 if alignment  is OK ; -1 if a grating can't be aligned
    *
    *       alignement defines the transformations matrices needed for ray propagation computations
    *       This is a default implementation which will be overridden for gratings and maybe other elements
    *       \n If the surface is reflective \b theta is the half-deflection angle of the chief ray,
    *       if it is transmissive, the chief ray direction is not modified and theta represents the surface tilt angle of normal to chief ray)
    *       \ In all cases \b Phi defines the frame rotation auround the incident chief ray from entry to exit.
    *       When the surface is reflective the incidence plane is the YZ plane of the rotated frame and XZ is tangent to the surface
    *       \n \b Psi is always the rotation around the surface normal (in case of transmissive a
    */
    virtual int align(double wavelength)=0;

    virtual VectorType intercept(RayBaseType& ray, VectorType * normal=NULL )=0; /**< \brief Pure virtual function
    *
    *   Re-implemented in shape classes (Plane , Quadric, Toroid, PolynomialSurface and SourceBase)
    *   These functions must work in the Surface AbsoluteLocalFrame only and must not apply the translationFromPrevious
    *   \n if the ray is not alive the last active position is kept and expressed in local absolute frame coordinates
    */

    virtual RayType& transmit(RayType& ray);       /**<  \brief  ray transmission and impact storage. \n To be reimplemented in derived class
            *   \param ray the input ray in previous element exit space \return the transmitted ray in exit space of this element
            *
            *   This implementation moves the ray to the surface and rebase it there
            *   \n All implementations <b> must take care of recording impacts </b> in entrance or exit plane as required */

    virtual RayType& reflect(RayType& ray);    /**< \brief Ray reflection and impact storage. \n To be reimplemented in derived class
            *
            *    \param ray the input ray inprevious element exit space
            *    \return the reflected ray in exit space of this element
            *
            *    This implementation simply reflect the ray on the tangent plane at intercept position.
            *   \n All implementations <b> must take care of recording impacts </b> in entrance or exit plane as required. */

    /** \brief  call transmit or reflect according to surface type ray and iterate to the next surface
     *
     *  If no intercept can be found with the surface the ray will be marked as dead (m_alive=false)
     *  The ray is propagated even though it could be dead in order to keep the same ray index in all impact vectors
     *  \throw an intercept or ray exception is thrown in case of computation error only
     */
    inline virtual void propagate(RayType& ray) /**< the propagated ray */
    {
        try {
            if(m_transmissive)
                 transmit(ray);
            else
                reflect(ray);
        } // propagation stopped by rethrowing the exception
        catch (InterceptException & excpt){
            throw excpt;
        } catch(RayException &excpt) {
            throw excpt;
        }
        catch(...){
            throw RayException(string("Unexpected exception in surface ")+ m_name, __FILE__, __func__, __LINE__);
        }

        // the ray is propagated even though it could be dead in order to keep the same ray index in all impact vectors
        if(m_next!=NULL )  // FP
            m_next->propagate(ray);
        if(! ray.m_alive)
            ++m_lostCount;
           // cout << m_name << " ray lost : " << "(" << ray.position().transpose() << ") (" << ray.direction().transpose() << ")\n";

    }

    inline void setRecording(RecordMode rflag){m_recording=rflag;} /**< \brief Sets the impact recording mode for the surface */
    inline RecordMode getRecording(){return m_recording;} /**< \brief Gets the impact recording mode of the surface */


    void clearImpacts();    /**< \brief  clear the impact vector of all elements of the surface chain starting from this element*/

    void reserveImpacts(int n); /**< \brief  reserve space for élements in the impact vector of all elements of the surface chain starting from this element to avoid reallocations
                                *    \param  n the  number of elements to reserve t*/
    inline int sizeImpacts(){return m_impacts.size();}  /**< \brief returns the number of recorded impacts  */

    /** \brief get the impacts and directions of the set of of rays propagated from the source
     *
     * \param impacts a vector of  propagated rays rays in this object recording space. Ray position is the ray intercept with the surface
     * \param frame  The type of frame where impacts must be referred to. Can be: AlignedLocalFrame, SurfaceFrame, GeneralFrame or LocalAbsoluteFrame
     * \return   The number of lost rays in propagation from source
     */
    int getImpacts(vector<RayType> &impacts, FrameID frame);

    /** \brief get the 3D impacts as internally stored in a convenient shape for file output
     *
     * \param impactData a DiagramType object to fill with the internally stored data
     * \param frame  The type of frame where impacts must be referred to. Can be: AlignedLocalFrame, SurfaceFrame, GeneralFrame or LocalAbsoluteFrame
     * \return the number of stored impacts     */
    int getImpactData(Diagram &impactData, FrameID frame=LocalAbsoluteFrame);

   // int getNewImpactData(int n, DiagramType<n> &impactData, FrameID frame=LocalAbsoluteFrame);

    /** \brief Computes and fills-up a SpotDiagram object from the internally stored impact collection
     * \param spotDiagram a SpotDiagram object reference which wiill be uptated on return
     * \param distance the distance from this surface along the alignment ray where the observation plane is located
     * \return the number of stored impacts
     */
    int getSpotDiagram(Diagram & spotDiagram, double distance=0);

    /** \brief Computes and fills-up a CausticDiagram object from the internally stored impact collection
     *
     * The CausticDiagram consists in the point of each ray wich is the closest to the central alignment ray
     * \param causticData a CausticDiagram object reference which will be uptated on return
     * * \n  ATENTION the Diagram spotDiagram.m_spots array might be reallocated by this function. Do not pass a C struct allocated by malloc
     * \return the number of stored impacts
     */    int getCaustic(Diagram& causticData);

     /** \brief return in a SpotDiagram structure the transverse aberrant distances from a given reference point and the direction coefficients of each ray
      *
      *  The transverse aberrant distances can be identified as the derivative of the wavefront aberration  with respect to the aperture angles
      * \param[out] WFdata a SpotDiagram structure to return the transverse aberrant distances in the X and Y planes and the X and Y components of the normalized direction vector.
      * \n  ATENTION the Diagram WFdata.m_spots array might be reallocated by this function. Do not pass a C struct allocated by malloc
      * \param[in] distance the distance of the reference point from the recording plane of the rays
      * \return the number of rays in the wFdata structure
      */
     int getWavefrontData(Diagram & WFdata, double distance=0);

    /** \brief Get the wavefront expansion in Legendre Polynomial of aperture angle
     *
     * The surface must be recording impacts and the beam should be converging close to the specified position determined by distance
     * \param[in] distance of the surface where the reference is taken. This has an influence on the 2<sup>nd</sup> order terms (curvature) only
     * \param[in] Nx max order of Legendres in X direction
     * \param[in] Ny max order of Legendres in Y direction
     * \param[out] XYbounds the aperture limits in X and Y from the ray tracing impacts recorded on the surface. (mins in the first row and maxs in the second; X in first column and Y in the second)
     * \return The Nx x Ny (row,col) array of coefficients of Legendre polynomials describing the wavefront error to the given degrees and best fitting  the transverse aberration data
     */
    /*EIGEN_DEVICE_FUNC*/
    MatrixXd getWavefontExpansion(double distance, Index Nx, Index Ny, Array22d& XYbounds);

    /** \brief compute the Optical Path difference between the wavefront computed on this surface and a spherical wavefront converging at a given distance on the chief ray.
     *
     *    The wavefront is determined as the  best fit of the ray tracing with a polynomial wavefront surface expressed as a Legendre polynomial product in X and Y
     *    \n The computed OPD is stored in the local array m_OPDdata, and remains available for PSF computation at different distance while the impacts data remain.
     * \param distance  Distance from this surface of the reference image point on the chief ray
     * \param Nx Degree of the legendre polynomial expansion in X
     * \param Ny Degree of the legendre polynomial expansion in X
     * \throw an instance of runtime_error if the impact number is too small to compute a valid OPD
     */
    void computeOPD(double distance, Index Nx, Index Ny);

    /** \brief Compute the amplitude PSF of the S(//X) and P(//Y) component of the transmitted wavefront
     *
     * \param[out] PSF A 3 dimension array where the PSFs must be returned. This ndArray must be initialized.  The slowest varying dimension (last one) should be at least 2,
     *  so that the array will host the 2 polarization PSFs as 2 consecutive images, S being first and P second.. The first dimension (fastest varying dimension)
     *  corresponds to the X direction in the surface frame and the second one correspond to the Y direction. The two first dimensions must be initialised with the image to compute.
     * \param[in,out] pixelSizes  On input : the requested the sampling interval of PSF, in a 2 element Eigen Array (X first).
     *        \n on output: The effectively used pixel size after application of the minimum oversampling factor.
     *        \n if the given sampling interval is to loose to respect the oversampling value, the pixel size is adapted to match it
     * \param[in] lambda the wavelength of computation (the wavelength stored in each ray is not considered)
     * \param[in] oversampling this factor determines the maximum grid step of the computed PSF as lambda/ (ThetaMax-ThetaMin)/oversampling for each dimension
     *        Oversampling must be >1. The default oversampling is 4. Theta Max and Min  are stored in the array m_XYbounds
     * \param[in] distOffset Displacement of the plane of PSF determination with respect the the reference point used to determine the OPD
     */
    void computePSF(ndArray<std::complex<double>,3> &PSF, Array2d &pixelSizes, double lambda, double oversampling=4, double distOffset=0);

    /** \brief Compute the amplitude PSF of the S(//X) and P(//Y) component of the transmitted wavefront at a series of distance offsets
     *
     * \param[out] PSF A 4 dimension array where the PSFs must be returned. This ndArray must be initialized.  The slowest varying dimension (last one) should be at least 2,
     *  so that the array will host the 2 polarization PSFs as 2 consecutive image stacks, S being first and P second.. The first dimension (fastest varying dimension)
     *  corresponds to the X direction in the surface frame and the second one correspond to the Y direction. The two first dimensions must be initialised with the image to compute.
     * \param[in,out] pixelSizes  On input : the requested the sampling interval of PSF, in a 2 element Eigen Array (X first).
     *        \n on output: The effectively used pixel size after application of the minimum oversampling factor.
     *        \n if the given sampling interval is to loose to respect the oversampling value, the pixel size is adapted to match it
     * \param[in] distOffset an Eigen 1D array containing the offsets to the distance  where the OPD was computed, where the PSF will be evaluated
     * \param[in] lambda the wavelength of computation (the wavelength stored in each ray is not considered)
     * \param[in] oversampling this factor determines the maximum grid step of the computed PSF as lambda/ (ThetaMax-ThetaMin)/oversampling for each dimension
     *        Oversampling must be >1. The default oversampling is 4. Theta Max and Min  are stored in the array m_XYbounds
     */
    void computePSF(ndArray<std::complex<double>,4> &PSF, Array2d &pixelSizes, ArrayXd &distOffset, double lambda, double oversampling=4);

    /** \brief Defines whether or not the aperture limitations of this surface are taken into account in the tray tracing
     *
     * \param activity the level of activity to set-up
     */
    inline void setApertureActive(bool activity=true){m_apertureActive=activity;}

    /** \brief retrieves whether or not the aperture limitations of this surface are taken into account in the tray tracing
     *
     * \return the aperture activity setting of this surface
     */
    inline bool getApertureActive(){return m_apertureActive;}

    /** \brief Retrieves the transmission at a given point of the surface
     *
     * \param point A Vector of fixed size 2 containing the coordinates (X,Y) in the surface plane of the point to be checked
     * \return 1. or 0 according to the aperture  transmission at the given point
     *
     */
    inline double getApertureTransmissionAt(const Ref<Vector2d> & point)
    {
     if(m_apertureActive)
        return m_aperture.getTransmissionAt(point);
     else
        return 1.;
    }

    inline bool isOPDvalid(){return m_OPDvalid;}/**< \brief check validity  of OPD data before computing a PSF \return true if OPD data are valid*/

    void operator>>(xmlNodePtr elemnode);
    void operator<<(xmlNodePtr surfnode);




 /**
 */

/**
 *  \name Surface error and  height error generator group
 *
 *  \anchor generel
 *
 *
 *
 *   Functions associated to the surface errors generation and handling
 *
 *   These function uses additional parameters, which are defined by a call to setErrorGenerator and removed by
 *   unsetErrorGenerator.
 *
 *
 *   Name of parameter | UnitType | Data type\n and dims | Description
 *   ----------------- | -------- | --------- | -----------
 *   \b fractal_exponent_x | Dimensionless | Array[n,1] | List of the fractal exponents of each frequency segments of\n the generated PSD in the X direction
 *   \b fractal_frequency_x | InverseDistance | Array[n-1,1] | list of transition frequencies between PSD segments.in X  \f$ ^1 \f$
 *   \b fractal_exponent_y | Dimensionless | Array[m,1] | List of the fractal exponents of each frequency segments of\n the generated PSD in the Y direction
 *   \b fractal_frequency_y | InverseDistance | Array[m-1,1] | list of transition frequencies between PSD segments.in Y \f$ ^1 \f$
 *   \b error_limits       | Distance  | Array[2,2]  | Limits of the surface error interpolation (storage order: Xmin, Xmax, Ymin, Ymax
 *   \b sampling      |    Distance   | Array[2,1]   |  Approximate sampling steps (step_x, step_y) \f$ ^2 \f$
 *   \b detrending    |  Dimensionless  | Array[p,q]  | Detrending  Mask of zero-constrained *Zernike*. Legendre polynomials \f$ L_{i,j} \f$ corresponding to non-zero \n values are forced to 0 \f$ ^3 \f$
 *   \b low_Zernike   | Distance  |  Array[p',q']   | Upper bound of RMS values of the first non zero Legendre polynomials.\n Here a zero value means the corresponding \f$ L_{i,j} \f$  is not constrained, not that it is forced to 0\f$ ^3 \f$
 *   \b residual_sigma  |  Distance | Scalar  |  specifies the RMS value of  surface height fluctuations, constrained Zernike removed
 *
 *   All these parameters belong to the special SurfErrorGroup and their NotOptimizable flag is raised.
 *   Units are  \f$ [m] \f$ or \f$ [m^{-1}] \f$ for frequencies
 *
 *   \note
 *      1 - the first dimension n of fracta_exponent_X defines the number of segments. Size of fractal_Frequency_X
 *      must be  at least equal to n-1 \n otherwise Generate will raise an error
 *   \note
 *      2 - Sampling step is an approximate value which is use to determine the number of definition point in the
 *     surface error array; \n since this number is an integer, the actual step will be adjusted to the nearest value.
 *   \note
 *      3 - for detrending and low_Zernike arrays, the first index *p* is the fast varying dimension and corresponds to X, \n
 *      the second index *q* is the slow dimension and corresponds to Y
 *
 * \{ */



    /** \brief Adds a random surface error generator to the Surface by defining the nine related parameters (cf \link generel supra \endlink )
     *
     *  Nine parameters are used to define the fractal/legendre model used to generate random surface errors:
     *
     *  *fractal_exponent_x*, *fractal_frequency_x*, *fractal_exponent_y*, *fractal_frequency_y*, *error_limits,
     *  *sampling*, *detrending*, *low_Zernike*, *residual_sigma*.
     *
     *  A program should normally set all nine parameters. However fractal parameters for X and Y,\n *detrending* and *low_Zernike" are given default values at construction time.\n
     *  *fractal_exponent_x* and *fractal_exponent_y* are given a unique value of -1, and *fractal_frequency_x*  and *fractal_frequency_y*
     *  are defined as zero size array.\n
     *  *low_Zernike* is set as zero size array, meaning no Zernike is constrained.\n
     *  *detrending* is set to the 2,2 matrix \f$  \left[ {\begin{array}{cc}     1 & 1 \\  1 & 1 \\  \end{array} } \right]  \f$ to remove piston and tilt.
     *  The 3 other parameters must be defined before calling GenerateSurfaceErrors otherwise it will raise an error.
     */
    void setErrorGenerator();

    /** \brief Remove the  ability to generate a surface error
     *
     *   It deletes the 9  error defining  parameters and invalidate the error map interpolator
    */
    void unsetErrorGenerator();

    /** \brief Specialisation of the  ElementBase::setParameter function inherited from ElementBase to handle the error defining parameters
     *
     *  It provides complimentary checks of parameter validity before setting it, and turns-off the generator validation flag \see validateErrorGenerator
     * \param name name of the parameter
     * \param param new value of the parameter in a Parameter struct
     * \return true if the parameter was set; false otherwise, and OptiXerror will document the error
     *
     */
    virtual  bool setParameter(string name, Parameter& param);


    /** \brief  Generate a new instance of error map and activate the error interpolator
    *
    *
    * \return true if the surface errors were generated; false in case of invalid configuration. The OptiXError describes the issue.
    */
    virtual bool generateSurfaceErrors(double* total_sigma, MatrixXd& Legendre_sigmas );  //

    /** \brief Checks the set of error defining parameters and signals configuration errors
     *
     * This function is called by the SetErrorGenerator function whenever an error defining parameter was modified
     * \return true if the parameter provide a valid set, False otherwise and the error is documented by OptiXError
     */
    bool validateErrorGenerator();

    /** \brief Directly sets sets the bidim spline interpolator of the surface heights errors.\n
     *  *The function is automatically called by the GenerateSurfaceErrors function, but can be independently called to install a fixed error map*
     *
     * Since the interpolator is only valid in the given limits, activating computation with surface errors will be equivalent to
     *  inserting a rectangular aperture of identical size at the bottom of the aperture stop region stack.\n
     * This fictitious aperture is not active when the surface error computations are inactivated
     *
     * \param xmin aperture low limit in X; [in m units]
     * \param xmax aperture high limit in X; [in m units]
     * \param ymin aperture low limit in Y; [in m units]
     * \param ymax aperture high limit in Y; [in m units]
     * \param heights Array containing the height data. These data will be considered equispaced on the given aperture.
     *
     */
    inline void setSurfaceErrors(double xmin, double xmax, double ymin, double ymax, const Ref<ArrayXXd>& heights)
    {
        if(!m_errorMap)
            m_errorMap=new BidimSpline(3);
        Array22d limits;
        limits << xmin, ymin, xmax,ymax;
        m_errorMap->setFromGridData(limits, heights);
    }

    /** \brief Destroy the SurfaceError spline inerpolator and invalidate the associated m_errorMap pointer
     *
     *   This call should be reserved to remove a fixed error map installed by setSurfaceErrors \n
     *  Since the m_errorMap is deleted, the surface errors are temporarily suppressed, but the error defining parameters
     * are not deleted so that a next call to generate SurfaceErrors will create a new errorMap  and enable again surface error computations
     */
    inline void unsetSurfaceErrors()
    {
        if(m_errorMap)
            delete m_errorMap;
        m_errorMap=NULL;
    }

    /** \brief  retrieve the array of height errors either imposed either defined by the surface error generator
     *
     * \return A 2D array of the height errors interpolated by the <a href="#pro-attribs">m_errorMap</a>  member.\n
     *  A Null Array is returned if the error interpolator is not set ( <a href="#pro-attribs">m_errorMap</a>  = NULL)
     *  \note This function could easily return the slope errors if needed
     */
    ArrayXXd getSurfaceErrors();

    /** \brief set the method used to take this surface errors into account in the ray tracing
     *
     *  *The "method flag" is defined whenever no surfaceError generator  and no spline interpolator are set*
     * \param meth  The method to be used for this element. Must be a member of the theErrorMethod enumeration
     */
    inline void setErrorMethod(ErrorMethod meth){m_errorMethod=meth;}

    /** \brief returns the method used to take this surface errors into account in the ray tracing
     *
     * * The "method flag" is defined whenever no surfaceError generator  and no spline interpolator are set*
     * \return the method being used for this element. It is a member of the theErrorMethod enumeration
     *
     */
    inline ErrorMethod getErrorMethod(){return m_errorMethod;}


/** \} */

#ifdef HAS_REFLEX
    /** \brief sets or replaces the Coating that will be used in reflectivity computations if enabled \see enableReflectivity
     *
     * \param tableName name of te table where the coating is defined
     * \param coatingName name of the coating
     * \return true if the coating was set or replaced; false if coating was not found or element is a source. The OptiXLastError is set
     */
    bool setCoating(string tableName, string coatingName);
    void removeCoating();  /**< \brief Suppress the coating; a reflectivity (or transmittance) of 1. will be assumed for both poarizations*/
    inline string getCoatingName(){return m_pCoating->getParentTable()->getName()+':'+m_pCoating->getName() ;} /**< \brief returns the qualified name of the coating, that is CoatingTable name and Coating name separated by a colon ':' */
    inline Coating* getCoating(){return m_pCoating;} /**< \brief returns a pointer to the coating used by the optical element */
#endif // HAS_REFLEX
//    friend TextFile& operator<<(TextFile& file,  Surface& surface);  /**< \brief Duf this Surface object to a TextFile, in a human readable format  */
//
//    friend TextFile& operator >>(TextFile& file,  Surface& surface);  /**< \brief Retrieves a Surface object from a TextFile  */
protected:
      /** \brief Helper function used by Reflect & transmit. Modifies the intercept (ray position) and normal according to the surface error model
       *
       * \param[in,out] spos the 2D position of the intercept in the surface frame. In input the unperturbed position, in output the corrected one
       * \param[in,out] ray  input :The ray moved to the surface after a call to intercept; output: the modified ray positionned at new intercept
       * \param[in,out] normal input: The normal after a call to intercept; output: the modified normal
       */
      void applyPerturbation(Vector2d& spos, RayType& ray, VectorType& normal);
public:
    ApertureStop m_aperture;  /**< \brief The active area of the surface   */

protected:
    vector<RayType> m_impacts; /**<  \brief the ray impacts on the surfaces in absolute local element space before or after reflection/transmission */
    RecordMode m_recording; /**<  \brief flag defining whether or not the ray impacts on this surface are recorded and before or after reflection/transmission   */
#ifdef HAS_REFLEX
    Coating *m_pCoating=NULL; /**< \brief a pointer to a instance of Coating class to be used in reflectivity (or to be done transmittance) computations */
#endif // HAS_REFLEX
    int m_lostCount=0;        /**<  \brief Count of rays lost in this surface at transmission or reflexion */
    bool m_apertureActive;  /**<  \brief boolean flag for taking the aperture active area into account */
    BidimSpline* m_errorMap=NULL; /**< \brief A bidimensionnal spline interpolator of local height and slope deviation from ideal surface */
    ErrorMethod  m_errorMethod=None;     /**< \brief Indicates the way the surface errors are taken in computation  None=0 means surface errors ignored, LocalSlope=1 use slope errors without changing the intercept (other cases later)*/
    bool m_ErrorGeneratorValid=false;
//    SurfaceErrorGenerator* m_errorGenerator=NULL;/**< \brief if non null and validly initialized a call to GenerateError() will set up aspline error map associated to the surface  */

    bool m_OPDvalid=false;  /**< \brief boolean flag for keeping track of the validity of the OPD of the rays stored in the m_impacts vector */
    int m_NxOPD=0;          /**< \brief degree of Legendre polynomials of the X variable used to interpolate the OPD*/
    int m_NyOPD=0;          /**< \brief degree of Legendre polynomials of the Y variable used to interpolate the OPD */
    Array22d m_XYbounds;    /**< \brief XYbounds the aperture limits in X and Y from the ray tracing impacts recorded on the surface \see getWavefontExpansion for details*/
    double m_OPDrefDist;    /**< \brief distance of the reference point used to compute the OPD from this surface  */
    ArrayX3d m_OPDdata;     /**< \brief The OPD data of each valid ray, expressed in the aligned local frame \ref AlignedLocalFrame ux, uy, and oOPD */
    ArrayX2cd m_amplitudes; /**< \brief S and P complex amplitudes for each valid ray */

};


#endif // header guard

