/**
 *************************************************************************
*   \file           ApertureStop.cpp
*
*   \brief             implementation of the ApertureStop class
*
*
*
*   \author             François Polack  <francois.polack@synchroton-soleil.fr>
*   \date               Creation: 2022-02-04
*   \date               Last update: 2022-02-04
 ***************************************************************************/

#include "ApertureStop.h"
#include "ellipse.h"  // needed to create regions from xml
#include "Polygon.h"


ApertureStop::~ApertureStop()
{
  vector<Region*>::iterator it;
  for (it=m_regions.begin(); it != m_regions.end(); ++it)
        delete *it;

  m_regions.clear();
}

size_t ApertureStop::addRegion(Region* pRegion)
{
    m_regions.push_back(pRegion);
    return m_regions.size()-1;
}

bool ApertureStop::insertRegion(size_t index, Region*pRegion)
{
    if(index <0 || index >= m_regions.size())
        return false;
    m_regions.insert(m_regions.begin()+index, pRegion);
    return true;
}

bool ApertureStop::removeRegion(size_t index)
{
    if(index <0 || index >= m_regions.size())
        return false;
    delete m_regions[index];
    m_regions.erase(m_regions.begin()+index);
    return true;
}

bool ApertureStop::replaceRegion(size_t index, Region*pRegion)
{
    if(index <0 || index >= m_regions.size())
        return false;
    delete m_regions[index] ;
    m_regions[index]=pRegion;
    return true;
}

bool ApertureStop::setRegionTransparency(size_t index, bool transparent)
{
        if(index <0 || index >= m_regions.size())
        return false;
        m_regions[index]->setTransparency(transparent);
        return true;
}


Region* ApertureStop::getRegion(size_t index)
{
    if(index <0 || index >= m_regions.size())
        return NULL;
    return m_regions[index];
}

double ApertureStop::getTransmissionAt(const Ref<Vector2d> &point)
{
    if(m_regions.size()==0)
        return 1.;

//    bool  transmission=true;
    vector<Region*>::reverse_iterator it;

    for (it=m_regions.rbegin(); it != m_regions.rend(); ++it)
    {
        if((*it)->locate(point) > 0) // region acts only when ray is inside
            return (*it)->isTransparent() ? 1. : 0;
////        if((*it)->isTransparent())
////            transmission = transmission || ((*it)->locate(point)> 0);
////        else
////            transmission = transmission && ((*it)->locate(point)< 0);
    }
//    if(transmission)
//       return 1.;
    // ray is outside all regions
    return m_regions[0]->isTransparent() ? 0 : 1. ;
}

#define XMLSTR (xmlChar*)
//xmlNodePtr operator<<(xmlNodePtr surfnode, const ApertureStop & aperture)
void ApertureStop::operator>>(xmlNodePtr surfnode)
{
    xmlChar* surfname=xmlGetProp(surfnode,XMLSTR "name");
    if(m_regions.size()==0)
    {
        std::cout << "no region found in " << (char*) surfname << " aperture \n";
        return;
    }
    else
        std::cout << m_regions.size() << " regions in " << (char*) surfname << " aperture \n";
    xmlNodePtr apernode=xmlNewTextChild(surfnode,NULL,XMLSTR "aperture", NULL); // no value
    xmlNewProp(apernode, XMLSTR "class", XMLSTR "ApertureStop");

    for (auto it=m_regions.begin(); it != m_regions.end(); ++it)
    {

        **it>>apernode;
    }
    xmlFree(surfname);
    std::cout << "aperture set\n";
}

void ApertureStop::operator<<(xmlNodePtr apernode)
{
    xmlNodePtr regnode=xmlFirstElementChild(apernode);
    if(!regnode)
        throw XmlFileException("Warning: ApertureStop defines no region", __FILE__, __func__, __LINE__);
    while(regnode)
    {
        xmlChar* classtype= xmlGetProp(regnode, XMLSTR "class");
        std::cout << "region class " << (char*)classtype << std::endl;
        if(xmlStrcmp(classtype, XMLSTR "Ellipse")==0)
        {
            Ellipse * pRegion= new Ellipse;
            *pRegion << regnode;
            Index reg=addRegion(pRegion);
            std::cout << "ellipse added as " << reg <<std::endl;
        }
        else if(xmlStrcmp(classtype, XMLSTR "Polygon")==0)
        {
            Polygon * pRegion= new Polygon;
            *pRegion << regnode;
            Index reg=addRegion(pRegion);
            std::cout << "polygon added as " << reg <<std::endl;
        }
        else
            std::cout << "unexpected region class " << classtype << " in aperture \n";
        if(classtype)
            xmlFree(classtype);
        regnode=xmlNextElementSibling(regnode);  // will return NULL if current regnode is the last one
    }
        std::cout << "no more region\n";
}


