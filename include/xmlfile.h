#ifndef XMLFILE_H_INCLUDED
#define XMLFILE_H_INCLUDED

/**
*************************************************************************
*   \file       xmlfile.h

*
*   \brief     definition file
*
*
*
*
*   \author             François Polack  <francois.polack@synchroton-soleil.fr>
*   \date               Creation : 2021-05-04

*   \date               Last update: 2021-05-04

*
*
 ***************************************************************************/
#include "ctypes.h"
#include "collections.h"


bool SaveElementsAsXml(const char * filename, ElementCollection &system);
bool DumpXmlSys(const char* filename);
bool LoadElementsFromXml(const char * filename, ElementCollection &system);

#endif // XMLFILE_H_INCLUDED
