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


ChainCopy::~ChainCopy()
{
    ElementBase *elem=First, *next=NULL;
    while (elem)
    {
        next=elem->getNext();
        delete elem;
        elem=next;
    }
}

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
ElementBase * ElementCopy(ElementBase* source)
{
    ElementBase* Copy=NULL;
    string s_type=source->getRuntimeClass();

    if (s_type=="Source<XYGrid>" || s_type=="XYGridSource")
        Copy=new XYGridSource(*dynamic_cast<XYGridSource*> (source));



    else if (s_type=="Source<RadialGrid>" || s_type=="RadialGridSource")
        Copy= new RadialGridSource(*dynamic_cast<RadialGridSource*>(source));

    else if (s_type=="Source<Gaussian>" || s_type=="GaussianSource")
        Copy= new GaussianSource(*dynamic_cast<GaussianSource*>(source));

    else if (s_type=="Mirror<Plane>" || s_type=="PlaneMirror")
        Copy= new Mirror<Plane>(*dynamic_cast<Mirror<Plane>*>(source));
    else if (s_type=="Mirror<Sphere>" || s_type=="SphericalMirror")
        Copy= new Mirror<Sphere>(*dynamic_cast<Mirror<Sphere>*>(source));
    else if (s_type=="Mirror<Cylinder>" || s_type=="CylindricalMirror")
        Copy= new Mirror<Cylinder>(*dynamic_cast<Mirror<Cylinder>*>(source));
    else if (s_type=="Mirror<Toroid>" || s_type=="ToroidalMirror")
        Copy= new Mirror<Toroid>(*dynamic_cast<Mirror<Toroid>*>(source));

    else if (s_type=="Film<Plane>" || s_type=="PlaneFilm")
        Copy= new Film<Plane>(*dynamic_cast<Film<Plane>*>(source));
    else if (s_type=="Film<Sphere>" || s_type=="SphericalFilm")
        Copy= new Film<Sphere>(*dynamic_cast<Film<Sphere>*>(source));
    else if (s_type=="Film<Cylinder>" || s_type=="CylindricalFilm")
        Copy= new Film<Cylinder>(*dynamic_cast<Film<Cylinder>*>(source));
    else if (s_type=="Film<Toroid>" || s_type=="ToroidalFilm")
        Copy= new Film<Toroid>(*dynamic_cast<Film<Toroid>*>(source));

    else if (s_type=="Grating<Holo,Plane>" || s_type=="PlaneHoloGrating")
        Copy= new Grating<Holo,Plane>(*dynamic_cast<Grating<Holo,Plane>*>(source));
    else if (s_type=="Grating<Holo,Sphere>" || s_type=="SphericalHoloGrating")
        Copy= new Grating<Holo,Sphere>(*dynamic_cast<Grating<Holo,Sphere>*>(source));
    else if (s_type=="Grating<Holo,Cylinder>" || s_type=="CylindricalHoloGrating")
        Copy= new Grating<Holo,Cylinder>(*dynamic_cast<Grating<Holo,Cylinder>*>(source));
    else if (s_type=="Grating<Holo,Toroid>" || s_type=="ToroidalHoloGrating")
        Copy= new Grating<Holo,Toroid>(*dynamic_cast<Grating<Holo,Toroid>*>(source));

    else if (s_type=="Grating<Poly1D,Plane>" || s_type=="PlanePoly1DGrating")
        Copy= new Grating<Poly1D,Plane>(*dynamic_cast<Grating<Poly1D,Plane>*>(source));
    else if (s_type=="Grating<Poly1D,Sphere>" || s_type=="SphericalPoly1DGrating")
        Copy= new Grating<Poly1D,Sphere>(*dynamic_cast<Grating<Poly1D,Sphere>*>(source));
    else if (s_type=="Grating<Poly1D,Cylinder>" || s_type=="CylindricalPoly1DGrating")
        Copy= new Grating<Poly1D,Cylinder>(*dynamic_cast<Grating<Poly1D,Cylinder>*>(source));
    else if (s_type=="Grating<Poly1D,Toroid>" || s_type=="ToroidalPoly1DGrating")
        Copy= new Grating<Poly1D,Toroid>(*dynamic_cast<Grating<Poly1D,Toroid>*>(source));

//    else
//        return NULL; // creation failed bad type
//
//    // reset the link pointers
    Copy->setPrevious(NULL);
    Copy->setNext(NULL);
    Copy->setParent(NULL);

    return Copy;
}

bool DuplicateChain(ElementBase * source, ChainCopy& newChain)
{
    typedef pair<ElementBase*, ElementBase*> ElemPair;
//    string stype=source->getRuntimeClass();
//    cout << "S: " << source << " < " << source->getPrevious() << ", " << source->getNext() << ">  " << stype <<endl;
    ElementBase* elemCopy=ElementCopy(source);
//    stype=elemCopy->getRuntimeClass();
    if(elemCopy==NULL)
        return false;   // Classtype invalid  -should not happen
    if(newChain.copyMap.empty())
    {
        newChain.First=elemCopy;
        elemCopy->setPrevious(NULL);  // normally useless
    }
    else
    {
        if(source->getPrevious()==NULL)
            return false; // should not occur either
        ElementCopyMap::iterator it=newChain.copyMap.find(source->getPrevious());
        if (it==newChain.copyMap.end() )
        {
            cout << "element not found\n";
            return false;  // again should never occur
        }
        cout << "It " <<it->first<< "  " << it->second <<endl;
        elemCopy->setPrevious(it->second); // obligé de faire un set car le chainage de source reste en place et chain irait rétroagir sur source
        it->second->setNext(elemCopy);
        cout << "S: " << source << " < " << source->getPrevious() << ", " << source->getNext() <<endl;
    }

    newChain.copyMap.insert(ElemPair(source,elemCopy));
//    cout << "S: " << source << " < " << source->getPrevious() << ", " << source->getNext() ;
//    cout << ">    C: " << elemCopy << " < " << elemCopy->getPrevious() << ", " << elemCopy->getNext() << ">\n";

    if(source->getNext()==NULL)
        return true;

    return DuplicateChain(source->getNext(),newChain);
}

ElementBase* ChangeElementType(ElementBase* elem, string newType)
{
    ElementBase* newElem=NULL;
    try
    {
        newElem=CreateElementObject(newType,elem->getName());
    }
    catch (ElementException& ex)
    {
        cout <<"invalid element type \n" ;
        return NULL;
    }

//    cout << newElem->getName() <<" created as " << newElem->getRuntimeClass() << endl;

    // copie les paramètres communs
    ElementBase::ParamIterator newElemIt;
    for(newElemIt=newElem->parameterBegin(); newElemIt!=newElem->parameterEnd(); ++newElemIt)
    {
        Parameter param;
        if(elem->getParameter(newElemIt->first,param))  // le même paramètre existe dans l'élément original
        {
           newElem->setParameter(newElemIt->first, param);
        }
    }
    cout << "A: " << elem << " < " << elem->getPrevious() << ", " << elem->getNext()  <<endl;

    //copie les liens
    newElem->chainNext(elem->getNext());
    newElem->chainPrevious(elem->getPrevious());
    newElem->setParent(elem->getParent());
//    // raz lien de elem pour éviter es à c^oté à la destruction
//    elem->setNext(NULL);
//    elem->setPrevious(NULL);
//    elem->setParent(NULL);
//    cout << "S: " << elem << " < " << elem->getPrevious() << ", " << elem->getNext() ;
//    cout << ">    C: " << newElem << " < " << newElem->getPrevious() << ", " << newElem->getNext() << ">\n";

    delete elem;
    return newElem;

}
