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

#include "gratingbase.h"
#include "surface.h"


/** \class Holo
 *  \brief  this class define a holographic line pattern and implements function gratingVector() of the abstract base class Pattern
 *
 *    The class has seven specific parameters belonging to the SourceGroup
 *     -----------------------------------------
 *
 *   Name of parameter | UnitType | Description
 *   ----------------- | -------- | --------------
 *   \b recordingWavelength | Distance | Recording wavelength of the holographic pattern
 *   \b constructionP\e n _\e c  | Distance  | \e c  coordinate of construction point P\e n
 * <em> with  n  = 1 or 2, and  c = X, Y or Z;  example:</em> constructionP1_Z
 */
class Holo :   virtual public Surface, virtual public Pattern
{
    public:
        Holo();
        virtual ~Holo(){}
        virtual  inline string getRuntimeClass(){return "Holo";}/**< return the derived class name ie. Holo */

        /** \brief record pattern definition parameters. This function \b must be overridden in derived class and call after the Sureface::SetParameter() function was called.
         * \param name parameter name
         * \param param Parameter object
         * \return true if the parameter was recognized and properly set, false otherwise
         */
        virtual  bool setParameter(string name, Parameter& param); // cette fonction est susceptible de créer une ambiguité avec cele des classes de forme elle doit donc être réimplémenté dans les classes dérivées

        /** \brief compute the local line density vector
         *
         * \param[in]  position position where the line density vector must be computed
         * \param[in] normal  vector normal to the surface at position (usually position and normal are provided by the Surface::intercept() function)
         * \return the line density vector at position. it is perpendicular to normal
         */
        virtual EIGEN_DEVICE_FUNC Surface::VectorType gratingVector(Surface::VectorType position,
                                Surface::VectorType normal=Surface::VectorType::UnitZ());

    protected:
        double m_holoWavelength; /**< Holographic recording wavelength*/
        Surface::VectorType C1; /**< Position of the 1st holographic source point */
        Surface::VectorType C2; /**< Position of the 2nd holographic source point */

    private:
};

#endif // HOLO_H
#endif // header guard

