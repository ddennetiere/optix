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
#include "CoatingTable.h"
extern map<string,CoatingTable> coatingTables;

// l'inclusion de la classe Tensor par le Header  <unsupported/Eigen/CXX11/Tensor> est incompatible avec les classes Polygon et ellipse de Aperture Api

//#include <unsupported\Eigen\CXX11\src\Tensor\TensorForwardDeclarations.h>




extern bool inhibitApertureLimit; // global flag defined in interfac.cpp

using namespace std;

class SourceBase;

//inline double oneMinusSqrt1MinusX(double x)
//{
//    if(::fabs(x) > 0.01) return 1.-sqrt(1. - x);
//    return  x*(0.5 + x*(0.125 + x*(0.0625 + x*(0.0390625 + x*(0.02734375 + x*(0.0205078125 + x*0.01611328125))))));
//}


/** \brief Abstract base class of all optical surfaces
*
*     Surface is the base class of all shape definition classes like Plane, Quadric, Toroid
*     and behaviour type classes like Transparency, Mirror, Grating.
*
*     Optical element classes might be derived from an agregates of these classes, but they MUST refer to the same surface object.
*      Hence these derived objects must declare the base Surface member as virtual
*
*     General parameters common to all surfaces
*     -----------------------------------------
*       \see Element
*/
class Surface: public  ElementBase
{

public:

    /** \brief default  constructor (Film) with explicit chaining to previous
    */
    Surface(bool transparent=true, string name="", Surface * previous=NULL):ElementBase(transparent,name,previous),m_recording(RecordNone),
             m_apertureActive(false), m_pCoating(NULL){}

    virtual ~Surface(){}    /**< \brief virtual destructor */

    virtual inline string getOptixClass(){return "Surface";}/**< \brief return the derived class name of this object */

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

    EIGEN_DEVICE_FUNC virtual VectorType intercept(RayType& ray, VectorType * normal=NULL )=0; /**< \brief Pure virtual function
    *
    *   Implemented in shape classes (Plane , Quadric, Toroid)
    *   \n All implementations <b> must move and rebase </b> the ray at its intersection with the surface and return this position
    *   \n if the ray is not alive the last active position is kept and expressed in local absolute frame cordinates
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

/** \brief  call transmit or reflect according to surface type ray and iterate to the next surface*/
    inline  void propagate(RayType& ray) /**< the propagated ray */
    {
        try{
            if(m_transmissive)
             {
//                 cout << m_name << " transmit\n";
                 transmit(ray);
             }
            else
            {
//                cout << m_name << " reflect\n";
                reflect(ray);
            }
            if(ray.m_alive)
            {
                if(m_next!=NULL )  // FP
                    m_next->propagate(ray);
            }
            else
                cout << m_name << " ray lost : " << "(" << ray.position().transpose() << ") (" << ray.direction().transpose() << ")\n";
        }
        catch( EigenException & excpt)
        {

            throw (EigenException(excpt.what()+ " in element " + getName() +"\nre-thrown from",__FILE__, __func__, __LINE__));
        }
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
      void computePSF(ndArray<complex<double>,3 > &PSF, Array2d &pixelSizes, double lambda, double oversampling=4, double distOffset=0);

      void computePSF(ndArray<complex<double>,4 > &PSF, Array2d &pixelSizes, ArrayXd &distOffset, double lambda, double oversampling=4);

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
     inline bool isOPDvalid(){return m_OPDvalid;}/**< \brief check validity  of OPD data before computing a PSF \return true if OPD data are valid*/

     /** \brief sets or replaces the Coating that will be used in reflectivity computations if enabled \see useReflectivity
      *
      * \param tableName name of te table where the coating is defined
      * \param coatingName name of the coating
      * \return true if the coating was set or replaced; false if coating was not found or element is a source. The OptiXLastError is set
      */
     bool setCoating(string tableName, string coatingName);
     void removeCoating();  /**< \brief Suppress the coating; a reflectivity (or transmittance) of 1. will be assumed for both poarizations*/
     inline string getCoatingName(){return m_pCoating->getParentTable()->getName()+':'+m_pCoating->getName() ;} /**< \brief returns the qualified name of the coating, that is CoatingTable name and Coating name separated by a colon ':' */
     inline Coating* getCoating(){return m_pCoating;} /**< \brief returns a pointer to the coating used by the optical element */
//    friend TextFile& operator<<(TextFile& file,  Surface& surface);  /**< \brief Duf this Surface object to a TextFile, in a human readable format  */
//
//    friend TextFile& operator >>(TextFile& file,  Surface& surface);  /**< \brief Retrieves a Surface object from a TextFile  */
public:
    ApertureStop m_aperture;  /**< \brief The active area of the surface   */


protected:
    vector<RayType> m_impacts; /**<  \brief the ray impacts on the surfaces in absolute local element space before or after reflection/transmission */
    RecordMode m_recording; /**<  \brief flag defining whether or not the ray impacts on this surface are recorded and before or after reflection/transmission   */
    bool m_apertureActive;  /**<  \brief boolean flag for taking the aperture active area into account */
    Coating *m_pCoating=NULL; /**< \brief a pointer to a instance of Coating class to be used in reflectivity (or to be done transmittance) computations */
    bool m_OPDvalid=false;  /**< \brief boolean flag for keeping track of the validity of the OPD of the rays stored in the m_impacts vector */
    int m_NxOPD=0;          /**< \brief degree of Legendre polynomials of the X variable used to interpolate the OPD*/
    int m_NyOPD=0;          /**< \brief degree of Legendre polynomials of the Y variable used to interpolate the OPD */
    Array22d m_XYbounds;    /**< \brief XYbounds the aperture limits in X and Y from the ray tracing impacts recorded on the surface \see getWavefontExpansion for details*/
    double m_OPDrefDist;    /**< \brief distance of the reference point used to compute the OPD from this surface  */
    ArrayX3d m_OPDdata;     /**< \brief The OPD data of each valid ray, expressed in the aligned local frame \ref AlignedLocalFrame ux, uy, and oOPD */
    ArrayX2cd m_amplitudes; /**< \brief S and P complex amplitudes for each valid ray */

};


#endif // header guard

