#ifndef SOURCES_H
#define SOURCES_H

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


#include "sourcebase.h"


/** \brief a source composed of points distributed on a regular Cartesian grid, radiating regularly spaced rays on a Cartesian angular grids
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
 *   All parameters are defined and store as double. nXsize, nYsize,  nXdiv, nYdiv  will be rounded to the nearest integer
 */
class XYGridSource: public virtual SourceBase
{
public:
    /** Default constructor */
    XYGridSource(string name="" ,Surface * previous=NULL);
    /** Default destructor */
    virtual ~XYGridSource(){}
    virtual inline string getRuntimeClass(){return "Source<XYGrid>";}/**< return the derived class name ie. Source<XYGrid> */
    virtual int generate(double wavelength);    /**< implementation of SourceBase::generate for XYGridSource() */
    //public members
    int nXprim, nYprim,nX,nY;
};

/** \brief a source composed of points distributed on a regular polar grid, radiating regularly spaced rays on a polar angular grids
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
 *   All parameters are defined and store as double. nRsize, nTheta_size,  nRdiv, nTheta_div  will be rounded to the nearest integer
 */
class RadialGridSource: public virtual SourceBase
{
public:
    /** Default constructor */
    RadialGridSource(string name="" ,Surface * previous=NULL);
    /** Default destructor */
    virtual ~RadialGridSource(){}
    virtual inline string getRuntimeClass(){return "Source<RadialGrid>";}/**< return the derived class name ie. Source<RadialGrid> */
    virtual int generate(double wavelength);    /**< implementation of SourceBase::generate for RadialGridSource() */
    // public members
    int nRprim, nTprim,nR,nT;
};





/** \brief an extended source radiating gaussian distributed ray in source size and aperture
 *
 *    The class has five specific parameters belonging to the SourceGroup
 *     -----------------------------------------
 *
 *   Name of parameter | UnitType | Description
 *   ----------------- | -------- | --------------
 *   \b nRays | Dimensionless | number of rays to be generated
 *   \b sigmaX | Distance | RMS source size in X direction
 *   \b sigmaY | Distance | RMS source size in Y direction
 *   \b sigmaXdiv | Angle | RMS source divergence in X direction
 *   \b sigmaYdiv | Angle | RMS source divergence in Y direction
 *  \note
 *  All parameters are defined and store as double. nRays will be rounded to the nearest integer
 */
class GaussianSource: public virtual SourceBase
{
public:
    /** Default constructor */
    GaussianSource(string name="" ,Surface * previous=NULL);
    /** Default destructor */
    virtual ~GaussianSource(){}
    virtual inline string getRuntimeClass(){return "Source<Gaussian>";}/**< return the derived class name ie. Source<XYGrid> */
    virtual int generate(double wavelength);    /**< implementation of SourceBase::generate for GaussianSource() */
    //public members

};



#endif // SOURCES_H


