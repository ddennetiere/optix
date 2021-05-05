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


/** \brief Save the element collection in memory to a text file in XML format
 * \ingroup GlobalCpp
 * \param filename the name of the output file
 * \param system the collection of optical elements
 * \return a success flag
 */
bool SaveElementsAsXml(const char * filename, ElementCollection &system);

/** \brief Prints Optix meaningful part of a XML file to stdout
 * \ingroup GlobalCpp
 * \param filename the file to print
 * \return a success flag
 */
bool DumpXmlSys(const char* filename);

/** \brief Scans an XML file and imports the contained optical elements to a new system
 * \ingroup GlobalCpp
 * \param filename the XML file to import
 * \param system the ElementCollection of newly created elements <i>(usually the global collection System)</i>
 * \return a success flag
 */
bool LoadElementsFromXml(const char * filename, ElementCollection &system);

#endif // XMLFILE_H_INCLUDED
