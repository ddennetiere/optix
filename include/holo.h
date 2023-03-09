#ifndef HEADER_D3A0933725DE927
#define HEADER_D3A0933725DE927

/**
*************************************************************************
*   \file       holo.h

*
*   \brief     definition file
*
*
*
*
*   \author             François Polack  <francois.polack@synchroton-soleil.fr>
*   \date               Creation : 2021-02-23

*   \date               Last update: 2021-02-23

*
*
 ***************************************************************************/

#ifndef HOLO_H
#define HOLO_H
#define HOLO_EULERIAN
//#define HOLO_CARTESIAN

#include "gratingbase.h"
#include "surface.h"

#ifdef HOLO_CARTESIAN
/** \class Holo
 *  \brief  this class define a holographic line pattern and implements function gratingVector() of the abstract base class Pattern
 *
 *    The class has seven specific parameters belonging to the SourceGroup
 *     -----------------------------------------
 *   Name of parameter | UnitType | Description
 *   ----------------- | -------- | --------------
 *   \b recordingWavelength | Distance | Recording wavelength of the holographic pattern
 *   \b constructionP\e n _\e c  | Distance  | \e c  coordinate of construction point P\e n
 * <em> with  n  = 1 or 2, and  c = X, Y or Z;  example:</em> constructionP1_Z
 */
#endif // HOLO_CARTESIAN
#ifdef HOLO_EULERIAN
/** \class Holo
 *  \brief  this class define a holographic line pattern and implements function gratingVector() of the abstract base class Pattern
 *
 *    The class has seven specific parameters belonging to the GratingGroup
 *   Name of parameter | UnitType | Description
 *   ----------------- | -------- | --------------
 *   \b recordingWavelength | Distance | Recording wavelength of the holographic pattern
 *   \b lineDensity     | Inverse Distance  | Central line density of the grating
 *   \b inverseDist\e n  | Inverse Distance  | reciprocal distance of construction point P\e n (a signed value see diagram below)
 *   \b azimuthAngle\e n  | Angle  | azimuth angle ( \f$ \psi \f$) of construction point P\e n  [\f$ - \pi/2, \pi/2 \f$]
 *   \b elevationAngle1  | Angle | elevation angle ( \f$ \theta \f$) of the first construction point P1  [\f$ - \pi/2, \pi/2 \f$]
 * <em> with  n  = 1 or 2,  example:</em> inverseDist1
 *  \n <b> WARNING: elevationAngle2 is not defined and invalid </b> \n
 *  "elevationAngle2" is defined from elevationAngle1 and lineDensity and calculated as sign(\e elevationAngle1 ) * arcos{ cos( \e elevationAngle1 ) - \e lineDensity * \e recordingWavelength  }
 *   \n If lineDensity > 0 the construction direction 1 is more grazing than direction 2  (irrespective of the signs)
 *
 *  \image html  Holo.png "Definition of holographic construction points"
 */
#endif // HOLO_EULERIAN
class Holo :   virtual public Surface, virtual public Pattern
{
    public:
        Holo();
        virtual ~Holo(){}
        virtual  inline string getOptixClass(){return "Holo";}/**< return the derived class name ie. Holo */

        /** \brief record pattern definition parameters. This function \b must be overridden in derived class and call after the Sureface::SetParameter() function was called.
         * \param name parameter name
         * \param param Parameter object
         * \return true if the parameter is active on the Poly1D Pattern  and successfully applied, or is inactive and ignored.
         * False if an error occured while trying to apply the value
         */
        virtual  bool setParameter(string name, Parameter& param); // cette fonction est susceptible de créer une ambiguité avec cele des classes de forme elle doit donc être réimplémenté dans les classes dérivées

        /** \brief compute the local line density vector
         *
         * \param[in]  position position where the line density vector must be computed
         * \param[in] normal  vector normal to the surface at position (usually position and normal are provided by the Surface::intercept() function)
         * \return the line density vector at position. it is perpendicular to normal
         */
        virtual Surface::VectorType gratingVector(const Surface::VectorType &position, const
                                Surface::VectorType &normal);

        /** \brief Computes the orientation and curvature of the grating central line and a third order polynomial approximation
         *  of the line density function along the meridional axis (X)
         *
         * \param halfLength 1/2 length of the grating (X direction) used for computing the polynomial approximation
         * \param halfWidth 1/2 width of the grating (Y) used to compute the central line curvature
         * \param patInfo a pointer to a GratingPatternInfo structure to be filled in return
         */
        void getPatternInfo(double halfLength, double halfWidth, GratingPatternInfo *patInfo);

        double lineNumber(const Surface::VectorType &position);
 //   protected:
        double m_holoWavelength; /**< Holographic recording wavelength*/

#ifdef HOLO_EULERIAN
        bool defineDirection2();
        double m_inverseDistance1;      /**< reciprocal distance of the 1st holographic source point */
        double m_inverseDistance2;      /**< reciprocal distance of the 2nd holographic source point */
        double m_lineDensity;           /**< Grating line density at center */ // this is redundant with m_direction2[3] but useful to fix this value if construction P2 is iterated
        Surface::VectorType m_direction1; /**< direction of the 1st holographic source point */
        Surface::VectorType m_direction2; /**< direction of the 2nd holographic source point */
#else
  #ifdef HOLO_CARTESIAN
        Surface::VectorType C1; /**< Position of the 1st holographic source point */
        Surface::VectorType C2; /**< Position of the 2nd holographic source point */
  #else
        #error HOLOGRAPHIC MODEL NOT DEFINED
  #endif // HOLOE_EULERIAN
#endif // HOLO_CARTESIAN
    private:
};

#endif // HOLO_H
#endif // header guard

