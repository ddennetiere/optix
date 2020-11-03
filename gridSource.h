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

//template<class Grid>
//class GridSource: public Plane, public Grid
//{
//    public:
//        /** Default constructor */
//        GridSource(string name="" ):Surface(true,name){} // source surface must behave like a film
//        /** Default destructor */
//        virtual ~GridSource(){}
//        void CreateSource();
//    protected:
//    private:
//};

class XYGridSource: public virtual SourceBase
{
    /** Default constructor */
    XYGridSource(string name="" ,Surface * previous=NULL);
    /** Default destructor */
    virtual ~XYGridSource(){}
    virtual int generate(double wavelength);
    //public members
    int nYprim, nZprim,nY,nZ;
};

class RadialGridSource: public virtual SourceBase
{
    /** Default constructor */
    RadialGridSource(string name="" ,Surface * previous=NULL);
    /** Default destructor */
    virtual ~RadialGridSource(){}
    virtual int generate(double wavelength);
    // public members
    int nRprim, nTprim,nR,nT;
};

#endif // GRIDSOURCE_H
