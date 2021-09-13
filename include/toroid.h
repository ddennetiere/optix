#ifndef HEADER_1E1A1F94451C1791
#define HEADER_1E1A1F94451C1791

////////////////////////////////////////////////////////////////////////////////
/**
*      \file           toroid.h
*
*      \brief         Spherical surface class definition
*
*      \author         François Polack <francois.polack@synchroton-soleil.fr>
*      \date        2020-12-11  Creation
*      \date         Last update
*

*/
///////////////////////////////////////////////////////////////////////////////////
//
//             REVISIONS
//
////////////////////////////////////////////////////////////////////////////////////
#ifndef TOROID_H
#define TOROID_H

#include "Surface.h"


/** \class Toroid
 *  \brief Implement a toroidal surface
 *    The class has two specific parameters belonging to the ShapeGroup
 *     -----------------------------------------
 *
 *   Name of parameter | UnitType | Description
 *   ----------------- | -------- | --------------
 *   \b minor_curvature | InverseDistance | Curvature (1/rc) of generator circle
 *   \b major_curvature | InverseDistance | Curvature (1/Rc) of the generated circle at apex
 */
class Toroid : virtual public Surface
{
    public:
        /** Default constructor */
        Toroid(string name="" ,Surface * previous=NULL); //:Surface("Toroid"){Surface::m_transmissive=false;}
        /** Default destructor */
        virtual ~Toroid(){}

        virtual inline string getOptixClass(){return "Toroid";}/**< \brief return the derived class name ie. Toroid */

        /** \brief Change parameters and recaculate the surface if needed

         * \param name parameter name
         * \n Toroid specific parameter are
         * * \b minor_curvature The inverse of the radius (r) of the generating circle (minor radius at apex)
         * * \b major_curvature The inverse of the major radius (R) at apex. The axis of the generating rotation is located at R from apex and oriented along Y.
         * \param param parameter parameter value and properties
         * \return true if parameter name is valid for a toroid bjecr and was set; false otherwise
         *
         *  Curvature can be positive (concave) or négative (convex). No assumption is made on which curvature is the largest in absolute value.
         */
        inline bool setParameter(string name, Parameter& param)
        {
            if(! Surface::setParameter(name, param))
                    return false;
            if(name=="minor_curvature" || (name=="major_curvature" ))
                createSurface();
            return true;
        }

         /** \brief Orients the toroidal surface (see important note)
         *
         * <b> Must be called after a call to the base class function </b> Surface::align() or Grating::align() otherwise  the space transforms have old or default values
         * \param wavelength  Only used by gratings
         * \return 0 if OK       */
        virtual inline int align(double wavelength=0)
        {
            Matrix<FloatType,5,5> toreTransform=Matrix<FloatType,5,5>::Identity();
            toreTransform.block(0,0,3,3)=  m_surfaceInverse.matrix().block(0,0,3,3);
            m_alignedMat1 = toreTransform.transpose() * m_toreMat1 * toreTransform;
            m_alignedMat2 = toreTransform.transpose() * m_toreMat2 * toreTransform;
//            cout << "aligned Mat1 \n" << m_alignedMat1 << endl;
//            cout << "aligned Mat2 \n" << m_alignedMat2 << endl;
            return 0;
        }

        /** \brief computes the intercept of ray with this quadric surface in the surface local absolute frame and sets the new origin at the intercept
        *
        *   \param  ray  on input : the ray in the previous surface frame. in output the ray positionned on the surface in this surface frame
        *   \param normal adress of a vector which, if not null,  will receive the surface normal (normalized) at the intercept point
        *   \return a reference to the modified ray
        */
        EIGEN_DEVICE_FUNC virtual VectorType intercept(RayType& ray, VectorType * normal=NULL);

        /**< \brief dump internal data to standard output */
        virtual void dumpData()
        {
            cout  <<  "m_alignedMat1" << endl << m_alignedMat1 << endl;
            cout  <<  "m_alignedMat2" << endl << m_alignedMat2 << endl;
            cout  <<  "m_toreMat1" << endl << m_toreMat1 << endl<<endl;
            cout  <<  "m_toreMat2" << endl << m_toreMat2 << endl<<endl;
            ElementBase::dumpData();
        }

    protected:
        void createSurface(); /**< \brief Initialize the local equation. Called when curvature is changed  */

        Matrix<FloatType,5,5> m_toreMat1, m_toreMat2;   /**< \brief definition matrices in the surface frame */
        Matrix<FloatType,5,5> m_alignedMat1, m_alignedMat2; /**< \brief definition matrices in the absolute frame */

    private:

};

#endif // TOROID_H
#endif // header guard

