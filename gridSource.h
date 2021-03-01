#ifndef HEADER_29A4858F3C0BB3C5
#define HEADER_29A4858F3C0BB3C5

////////////////////////////////////////////////////////////////////////////////
/**
*      \file           gridSource.h
*
*      \brief         sources emitting a limited number of drays on a prededined grid
*
*      \author         François Polack <francois.polack@synchroton-soleil.fr>
*      \date        2020-10-30  Creation
*      \date         Last update
*

*/
///////////////////////////////////////////////////////////////////////////////////
//
//             REVISIONS
//
////////////////////////////////////////////////////////////////////////////////////
#ifndef GRIDSOURCE_H
#define GRIDSOURCE_H

#include "sourcebase.h"


/** \brief a point source radiating regularly spaced rays on a cartesian defined angular grid
 *
 *    The class has eight specific parameters belonging to the SourceGroup
 *     -----------------------------------------
 *
 *   Name of parameter | UnitType | Description
 *   ----------------- | -------- | --------------
 *   \b divX | Angle | 1/2 divergence in the horizontal plane
 *   \b divY | Angle | 1/2 divergence in the vertical plane
 *   \b nXdiv | Dimensionless | Number of steps in horizontal 1/2 divergence
 *   \b nYdiv | Dimensionless | Number of steps in vertical 1/2 divergence
 *   \b sizeX | Distance | 1/2 source size in the horizontal plane
 *   \b sizeY | Distance | 1/2 source size in the vertical plane
 *   \b nXsize | Dimensionless | Number of steps in horizontal 1/2 size
 *   \b nYsize | Dimensionless | Number of steps in vertical 1/2 size
 */
class XYGridSource: public virtual SourceBase
{
public:
    /** Default constructor */
    XYGridSource(string name="" ,Surface * previous=NULL);
    /** Default destructor */
    virtual ~XYGridSource(){}
    virtual inline string getRuntimeClass(){return "XYGridSource";}/**< return the derived class name ie. XYGridSource */
    virtual int generate(double wavelength);    /**< implementation of SourceBase::generate for XYGridSource() */
    //public members
    int nXprim, nYprim,nX,nY;
};

/** \brief a point source radiating regularly spaced rays on a  polar defined angular grid
 *
 *    The class has six specific parameters belonging to the SourceGroup
 *     -----------------------------------------
 *
 *   Name of parameter | UnitType | Description
 *   ----------------- | -------- | --------------
 *   \b divR | Angle | 1/2 divergence in the radial direction
 *   \b nRdiv | Dimensionless | Number of radial steps in 1/2 divergence
 *   \b nTheta_div | Dimensionless | Number of divergence azimuth steps in 2 Pi
 *   \b sizeR | Distance | Radius of the  source
 *   \b nRsize | Dimensionless | Number of radial steps in source radius
 *   \b nTheta_size | Dimensionless | Number of  source azimuth steps in 2 Pi
 */
class RadialGridSource: public virtual SourceBase
{
public:
    /** Default constructor */
    RadialGridSource(string name="" ,Surface * previous=NULL);
    /** Default destructor */
    virtual ~RadialGridSource(){}
    virtual inline string getRuntimeClass(){return "RadialGridSource";}/**< return the derived class name ie. RadialGridSource */
    virtual int generate(double wavelength);    /**< implementation of SourceBase::generate for RadialGridSource() */
    // public members
    int nRprim, nTprim,nR,nT;
};

#endif // GRIDSOURCE_H
#endif // header guard

