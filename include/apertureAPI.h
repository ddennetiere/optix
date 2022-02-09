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

extern ElementCollection System;


/** \ingroup apertureAPI
*   \{
*/


#ifdef __cplusplus
extern "C"
{
#else
#include <stdbool.h>
#endif

    /** \brief Retrieves the number of stop regions composing the ApertureStop of a given OpticalElement
     *
     *  An error is generated, if ID is not in the systeme dictionnary or doesn't refer to an optical Surface.
     * \param element_ID the ID of the element to inquire for
     * \return the number of Region layers composing the Aperture Stop, -1 if an error occurred
     *
     */
    DLL_EXPORT size_t GetStopNumber(size_t element_ID);

    /** \brief  gets the runtime class name of the Region object
     *
     *  The Region type  can be either Polygon either Ellipse. The return value is the number of sides in Polygon case
     * \param element_ID The ID of the optical element to inquire for.
     * \param index The index of the region in the aperture stack. An error is raised if invalid (<0 or > num stops)
     * \param[out] buffer a character buffer to store the returned type name. It bust be at least 8 bytes long.
     * \param[in] bufferSize The actual length of the buffer handled to the function
     * \return The number of polygon sides if the region is a Polygon; 0 if the region is an Ellipse; -1 if an error occurred.
     */
    DLL_EXPORT size_t GetStopType(size_t element_ID, size_t index, char* buffer, int bufferSize);


    /** \brief Sets the activity status of the aperture stop of the optical element
     *
     * \param element_ID  The ID of the optical element to modify
     * \param status a boolean value for the new activity status
     * \return -1 if an error occurred ; 0 otherwise
     */
    DLL_EXPORT size_t SetApertureActivity(size_t element_ID, bool status);

    /** \brief Gets the activity status of the aperture stop of the optical element
     *
     * \param element_ID The ID of the optical element to inquire for.
     * \param[out] status address of a boolean variable to receive the activity status
     * \return -1 if an error occurred, 0 otherwise
     */
    DLL_EXPORT size_t GetApertureActivity(size_t element_ID, bool* status);


    /** \brief Returns the parameters defining a polygonal Region
     *
     * Retrieves the array of vertex points and the opacity
     * \param element_ID The ID of the optical element to inquire for.
     * \param index The index of the region in the aperture stack. An error is raised if invalid (<0 or > num stops)
     * \param[in] arrayWidth The number of vertex points which can be stored in the vertexArray. (must be at least able to store all points otherwise an error will be raised)
     * \param[out] vertexArray  A pointer to a memory array of double s to be filled with the vertex coordinate in order \f$ [x,Y]_0, [x,Y]_1, .... [x,Y]_n \f$
     * \param[out] opacity The opacity value of the Region
     * \return The number of vertex points returned in vertexArray; -1 if array is too small or another error occurred.
     *
     */
    DLL_EXPORT size_t GetPolygonParameters(size_t element_ID, size_t index, size_t arrayWidth, double* vertexArray, bool *opacity);

    /** \brief Append a new polygonal Region in the aperture stop list
     *
     * \param element_ID The ID of the optical element to modify.
     * \param numSides The number of sides of the polygon, and the number of vertex points which are stored in the vertexArray.
     * \param vertexArray   A pointer to a memory array of double s to be filled with the vertex coordinates of the new polygon in order \f$ [x,Y]_0, [x,Y]_1, .... [x,Y]_n \f$
     * \param opacity the requested opacity value
     * \return The index at which the element was added
     */
    DLL_EXPORT size_t AddPolygonalStop(size_t element_ID, size_t numSides, double* vertexArray, bool opacity);

    /** \brief Insert a new polygonal region in the aperture stop list
     *
     * \param element_ID The ID of the optical element to modify.
     * \param index the position in the list where the new region will be inserted. The item previously at this position and all other above
     *  are moved up one position. An error is raised if invalid (<0 or > num stops)
     * \param numSides The number of sides of the polygon, and the number of vertex points which are stored in the vertexArray.
     * \param vertexArray   A pointer to a memory array of double s to be filled with the vertex coordinates of the new polygon in order \f$ [x,Y]_0, [x,Y]_1, .... [x,Y]_n \f$
     * \param opacity the requested opacity value
     * \return The index at which the polygonal region was inserted in the Region list
     */
    DLL_EXPORT size_t InsertPolygonalStop(size_t element_ID, size_t index, size_t numSides, double* vertexArray, bool opacity);

    /** \brief Replace a Region in the aperture stop list by a new Polygon
     *
     * \param element_ID The ID of the optical element to modify.
     * \param index the position in the list where the region will be replaced
     * \param numSides The number of sides of the polygon, and the number of vertex points which are stored in the vertexArray.
     * \param vertexArray   A pointer to a memory array of double s to be filled with the vertex coordinates of the new polygon in order \f$ [x,Y]_0, [x,Y]_1, .... [x,Y]_n \f$
     * \param opacity the requested opacity value
     * \return The index at which the Region list was modified
     */
    DLL_EXPORT size_t ReplaceStopByPolygon(size_t element_ID, size_t index, size_t numSides, double* vertexArray, bool opacity);


    DLL_EXPORT size_t AddRectangularStop(size_t element_ID,  double Xwidth, double Ywidth, bool opacity, double Xcenter, double Ycenter, double angle);

    DLL_EXPORT size_t InsertRectangularStop(size_t element_ID, size_t index, double Xwidth, double Ywidth, bool opacity, double Xcenter, double Ycenter, double angle);

    DLL_EXPORT size_t ReplaceStopByRectangle(size_t element_ID, size_t index, double Xwidth, double Ywidth, bool opacity, double Xcenter, double Ycenter, double angle);



    /** \brief Returns the parameters defining an elliptical Region
     *
     * \param element_ID size_t
     * \param index size_t
     * \param mainAxis double*
     * \param minorAxis double*
     * \param opacity bool*
     * \param Xcenter double*
     * \param Ycenter double*
     * \param angle double*
     * \return DLL_EXPORT size_t
     */
    DLL_EXPORT size_t GetEllipseParameters(size_t element_ID, size_t index, double *mainAxis, double *minorAxis, bool *opacity, double *Xcenter, double *Ycenter, double *angle);

    DLL_EXPORT size_t AddEllipticalStop(size_t element_ID, double mainAxis, double minorAxis, bool opacity, double Xcenter, double Ycenter, double angle);

    DLL_EXPORT size_t InsertEllipticalStop(size_t element_ID, size_t index, double mainAxis, double minorAxis, bool opacity, double Xcenter, double Ycenter, double angle);

    DLL_EXPORT size_t ReplaceStopByEllipse(size_t element_ID, size_t index, double mainAxis, double minorAxis, bool opacity, double Xcenter, double Ycenter, double angle);


    DLL_EXPORT size_t AddCircularStop(size_t element_ID, double radius, bool opacity, double Xcenter, double Ycenter);

    DLL_EXPORT size_t InsertCircularStop(size_t element_ID, size_t index, double radius, bool opacity, double Xcenter, double Ycenter);

    DLL_EXPORT size_t ReplaceStopByCircle(size_t element_ID, size_t index, double radius, bool opacity, double Xcenter, double Ycenter);





/** \} */ // end of apertureAPI group
#ifdef __cplusplus
}
#endif



#endif // APERTUREAPI_H_INCLUDED
