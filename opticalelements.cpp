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
#include "sources.h"   /**< \todo create a source.h file for including all sources at once */

template class Mirror<Plane>;
template class Mirror<Sphere>;
template class Mirror<Cylinder>;
template class Mirror<Toroid>;

template class Film<Plane>;
template class Film<Sphere>;
template class Film<Cylinder>;
template class Film<Toroid>;

template class Grating<Holo,Plane>;
template class Grating<Holo,Sphere>;
template class Grating<Holo,Cylinder>;
template class Grating<Holo,Toroid>;
template class Grating<Poly1D,Plane>;
template class Grating<Poly1D,Sphere>;
template class Grating<Poly1D,Cylinder>;
template class Grating<Poly1D,Toroid>;


ElementBase* CreateElementObject(string s_type, string name)
{
    ElementBase*  elem=NULL;

    if (s_type=="Source<XYGrid>" || s_type=="XYGridSource")
    elem=new XYGridSource(name);

    else if (s_type=="Source<RadialGrid>" || s_type=="RadialGridSource")
        elem= new RadialGridSource(name);

    else if (s_type=="Source<Gaussian>" || s_type=="GaussianSource")
        elem= new GaussianSource(name);

    else if (s_type=="Mirror<Plane>" || s_type=="PlaneMirror")
        elem= new Mirror<Plane>(name);
    else if (s_type=="Mirror<Sphere>" || s_type=="SphericalMirror")
        elem= new Mirror<Sphere>(name);
    else if (s_type=="Mirror<Cylinder>" || s_type=="CylindricalMirror")
        elem= new Mirror<Cylinder>(name);
    else if (s_type=="Mirror<Toroid>" || s_type=="ToroidalMirror")
        elem= new Mirror<Toroid>(name);

    else if (s_type=="Film<Plane>" || s_type=="PlaneFilm")
        elem= new Film<Plane>(name);
    else if (s_type=="Film<Sphere>" || s_type=="SphericalFilm")
        elem= new Film<Sphere>(name);
    else if (s_type=="Film<Cylinder>" || s_type=="CylindricalFilm")
        elem= new Film<Cylinder>(name);
    else if (s_type=="Film<Toroid>" || s_type=="ToroidalFilm")
        elem= new Film<Toroid>(name);

    else if (s_type=="Grating<Holo,Plane>" || s_type=="PlaneHoloGrating")
        elem= new Grating<Holo,Plane>(name);
    else if (s_type=="Grating<Holo,Sphere>" || s_type=="SphericalHoloGrating")
        elem= new Grating<Holo,Sphere>(name);
    else if (s_type=="Grating<Holo,Cylinder>" || s_type=="CylindricalHoloGrating")
        elem= new Grating<Holo,Cylinder>(name);
    else if (s_type=="Grating<Holo,Toroid>" || s_type=="ToroidalHoloGrating")
        elem= new Grating<Holo,Toroid>(name);

    else if (s_type=="Grating<Poly1D,Plane>" || s_type=="PlanePoly1DGrating")
        elem= new Grating<Poly1D,Plane>(name);
    else if (s_type=="Grating<Poly1D,Sphere>" || s_type=="SphericalPoly1DGrating")
        elem= new Grating<Poly1D,Sphere>(name);
    else if (s_type=="Grating<Poly1D,Cylinder>" || s_type=="CylindricalPoly1DGrating")
        elem= new Grating<Poly1D,Cylinder>(name);
    else if (s_type=="Grating<Poly1D,Toroid>" || s_type=="ToroidalPoly1DGrating")
        elem= new Grating<Poly1D,Toroid>(name);

    else
       // cout << " Object not created:  invalid element class\n";
       throw ElementException("Invalid element class", __FILE__, __func__, __LINE__);

    return elem;
}
