#ifndef APERTUREAPI_H_INCLUDED
#define APERTUREAPI_H_INCLUDED

/**
*************************************************************************
*   \file       apertureAPI.h

*
*   \brief     Aperture API  definition file
*
*
*
*
*   \author             François Polack  <francois.polack@synchroton-soleil.fr>
*   \date               Creation : 2022-02-07
*
*   \date               Last update: 2022-02-07
*
*
*
 ***************************************************************************/


#ifdef BUILD_DLL
    #define DLL_EXPORT __declspec(dllexport)
#else
    #ifdef NO_DLL
        #define DLL_EXPORT
    #else
        #define DLL_EXPORT __declspec(dllimport)
    #endif
#endif

#include "ctypes.h"
#include "surface.h"
#include "collections.h"
#include "Polygon.h"
#include "Ellipse.h"

/** \ingroup apertureAPI
*   \{
*/


extern ElementCollection System;


#ifdef __cplusplus
extern "C"
{
#else
#include <stdbool.h>
#endif


    DLL_EXPORT size_t AddRectangularStop(size_t element_ID,  double Xwidth, double Ywidth, bool opacity, double Xcenter, double Ycenter, double angle);

    DLL_EXPORT size_t InsertRectangularStop(size_t element_ID, size_t index, double Xwidth, double Ywidth, bool opacity, double Xcenter, double Ycenter, double angle);

    DLL_EXPORT size_t ReplaceStopByRectangle(size_t element_ID, size_t index, double Xwidth, double Ywidth, bool opacity, double Xcenter, double Ycenter, double angle);


    DLL_EXPORT size_t AddEllipticalStop(size_t element_ID, double mainAxis, double minorAxis, bool opacity, double Xcenter, double Ycenter, double angle);

    DLL_EXPORT size_t InsertEllipticalStop(size_t element_ID, size_t index, double mainAxis, double minorAxis, bool opacity, double Xcenter, double Ycenter, double angle);

    DLL_EXPORT size_t ReplaceStopByEllipse(size_t element_ID, size_t index, double mainAxis, double minorAxis, bool opacity, double Xcenter, double Ycenter, double angle);


    DLL_EXPORT size_t AddCircularStop(size_t element_ID, double radius, bool opacity, double Xcenter, double Ycenter);

    DLL_EXPORT size_t InsertCircularStop(size_t element_ID, size_t index, double radius, bool opacity, double Xcenter, double Ycenter);

    DLL_EXPORT size_t ReplaceStopByCircle(size_t element_ID, size_t index, double radius, bool opacity, double Xcenter, double Ycenter);


    DLL_EXPORT size_t AddPolygonalStop(size_t element_ID, size_t numSides, double* vertexArray, bool opacity);

    DLL_EXPORT size_t InsertPolygonalStop(size_t element_ID, size_t index, size_t numSides, double* vertexArray, bool opacity);





/** \} */ // end of apertureAPI group
#ifdef __cplusplus
}
#endif



#endif // APERTUREAPI_H_INCLUDED
