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


ApertureStop::~ApertureStop()
{
  vector<Region*>::iterator it;
  for (it=m_regions.begin(); it != m_regions.end(); ++it)
        delete *it;

  m_regions.clear();
}


size_t ApertureStop::addRegion(Region* pRegion, bool transparent)
{
    pRegion->setTransparency(transparent);
    m_regions.push_back(pRegion);
    return m_regions.size()-1;
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

Region* ApertureStop::getRegion(size_t index)
{
    if(index <0 || index >= m_regions.size())
        return NULL;
    return m_regions[index];
}

double ApertureStop::getTransmissionAt(const Ref<Vector2d> &point)
{
    vector<Region*>::iterator it;
    bool  transmission=true;
    for (it=m_regions.begin(); it != m_regions.end(); ++it)
    {
        if((*it)->isTransparent())
            transmission = transmission || ((*it)->locate(point)> 0);
        else
            transmission = transmission && ((*it)->locate(point)< 0);
    }
    if(transmission)
       return 1.;
    return 0;
}
