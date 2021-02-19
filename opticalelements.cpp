////////////////////////////////////////////////////////////////////////////////
/**
*      \file           opticalelements.cpp
*
*      \brief         Mirror template instanciation
*
*      \author         François Polack <francois.polack@synchroton-soleil.fr>
*      \date        2020-10-30  Creation
*      \date        Last update
*
*/
///////////////////////////////////////////////////////////////////////////////////
//
//             REVISIONS
//
////////////////////////////////////////////////////////////////////////////////////
#include "opticalelements.h"
#include "gridSource.h"   /**< \todo create a source.h file for including all sources at once */

template class Mirror<Plane>;
template class Mirror<Sphere>;
template class Mirror<Cylinder>;
template class Mirror<Toroid>;

template class Film<Plane>;
template class Film<Sphere>;
template class Film<Cylinder>;
template class Film<Toroid>;


ElementBase* CreateElementObject(string s_type, string name)
{
    ElementBase*  elem=NULL;
    if (s_type=="XYGridSource")
    elem=new XYGridSource(name);

    else if (s_type=="RadialGridSource")
        elem= new RadialGridSource(name);

    else if (s_type=="Mirror<Plane>" || s_type=="PlaneMirror")
        elem= new Mirror<Plane>(name);
    else if (s_type=="Mirror<Sphere>" || s_type=="SphericalMirror")
        elem= new Mirror<Sphere>(name);
    else if (s_type=="Mirror<Cylinder>" || s_type=="CylindricalMirror")
        elem= new Mirror<Cylinder>(name);

    else if (s_type=="Film<Plane>" || s_type=="PlaneFilm")
        elem= new Film<Plane>(name);
    else if (s_type=="Film<Sphere>" || s_type=="SphericalFilm")
        elem= new Film<Sphere>(name);
    else if (s_type=="Film<Cylinder>" || s_type=="CylindricalFilm")
        elem= new Film<Cylinder>(name);
    else
       // cout << " Object not created:  invalid element class\n";
       throw ElementException("Invalid element class", __FILE__, __func__, __LINE__);

    return elem;
}
