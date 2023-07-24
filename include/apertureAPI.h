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
*   \date               API revision: 2023-06-28
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

    /** \brief **MODIFIED** *28/06/2023* - Retrieves the number of stop regions composing the ApertureStop of a given OpticalElement
     *
     *  An error is generated, if ID is not in the systeme dictionnary or doesn't refer to an optical Surface.
     * \param[in] element_ID the ID of the element to inquire for
     * \param[out] numStops A location to return the number of Region layers composing the Aperture Stop, -1 if an error occurred
     * \return true if successful; false is not and will set OptiXError
     */
    DLL_EXPORT bool GetStopNumber(size_t element_ID, int * numStops);

    /** \brief  **MODIFIED** *28/06/2023* -  gets the runtime class name of the Region object
     *
     *  The Region type  can be either Polygon either Ellipse. The return value is the number of sides in Polygon case
     * \param[in] element_ID The ID of the optical element to inquire for.
     * \param[in] index The index of the region in the aperture stack. An error is raised if invalid (<0 or > stop number)
     * \param[out] typeString A memory location to return a pointer to a **READ_ONLY** character string. Do not attempt to modify this sting.
     * \param[out] numSides If this location is valid (not NULL) and the region is a "Polygon", this integer location will receive
     *    he number of polygon sides if the region is a Polygon. If the region is an "Ellipse", 0 will be returned.
     *  \return true if successful, false otherwise. The function will set OptiXError
     */
    DLL_EXPORT  bool GetStopType(size_t element_ID, size_t index, char** typeString, int* numSides);


    /** \brief **MODIFIED** *28/06/2023* - Sets the activity status of the aperture stop of the optical element
     *
     * \param element_ID  The ID of the optical element to modify
     * \param status a boolean value for the new activity status
     * \return true if successful, false otherwise, reason in OptiXError
     */
    DLL_EXPORT bool SetApertureActivity(size_t element_ID, bool status);

    /** \brief **MODIFIED** *28/06/2023* - Gets the activity status of the aperture stop of the optical element
     *
     * \param element_ID The ID of the optical element to inquire for.
     * \param[out] status address of a boolean variable to receive the activity status
     * \return true if successful, false otherwise with reason in OptiXError
     */
    DLL_EXPORT bool GetApertureActivity(size_t element_ID, bool* status);

    /** \brief **MODIFIED** *28/06/2023* - Return the aperture transmission at a specified location
     *
     * \param element_ID The ID of the optical element to inquire for.
     * \param x X coordinate in the surface frame
     * \param y Y coordinate in the surface frame
     * \param T location to receive the transmission value (can be either 1. or 0)
     * \return true if successful, false otherwise with reason in OptiXError
     */
    DLL_EXPORT bool GetApertureTransmissionAt(size_t element_ID, double x, double y, double * T);


    /** \brief **MODIFIED** *28/06/2023* - Returns the parameters defining a polygonal Region
     *
     * Retrieves the array of vertex points and the opacity
     * \param element_ID The ID of the optical element to inquire for.
     * \param index The index of the region in the aperture stack. An error is raised if invalid (<0 or > num stops)
     * \param[in,out] arraySize *In input*: The number of vertex points which can be stored in the vertexArray.
     *      (must be at least able to store all points otherwise an error will be raised)\n
     *      *In output*: The number of vertex points returned in vertexArray.
     * \param[out] vertexArray  A pointer to a memory array of double s to be filled with the vertex coordinate in order \f$ [x,Y]_0, [x,Y]_1, .... [x,Y]_n \f$
     * \param[out] opacity address of a variable to return the opacity value of the Region
     * \return true if successful, false otherwise with reason in OptiXError
     *
     */
    DLL_EXPORT  bool GetPolygonParameters(size_t element_ID, int index, int * arraySize, double* vertexArray, bool *opacity);

    /** \brief **MODIFIED** *28/06/2023* - Append a new polygonal Region in the aperture stop list
     *
     * \param[in] element_ID The ID of the optical element to modify.
     * \param[in] numSides The number of sides of the polygon, and the number of vertex points which are stored in the vertexArray.
     * \param[out] vertexArray   A pointer to a memory array of double s to be filled with the vertex coordinates of the new polygon in order \f$ [x,Y]_0, [x,Y]_1, .... [x,Y]_n \f$
     * \param[in] opacity the requested opacity value
     * \param[out] regionIndex if the address is valid (not NULL), it will receive the index at which the element was added.
     * \return true if successful, false otherwise with reason in OptiXError
     */
    DLL_EXPORT bool AddPolygonalStop(size_t element_ID, int numSides, double* vertexArray, bool opacity, int *regionIndex);

    /** \brief **MODIFIED** *28/06/2023* - Insert a new polygonal region in the aperture stop list
     *
     * \param element_ID The ID of the optical element to modify.
     * \param index the position in the list where the new region will be inserted. The item previously at this position and all other above
     *  are moved up one position. An error is raised if invalid (<0 or > num stops)
     * \param numSides The number of sides of the polygon, and the number of vertex points which are stored in the vertexArray.
     * \param vertexArray   A pointer to a memory array of double s to be filled with the vertex coordinates of the new polygon in order \f$ [x,Y]_0, [x,Y]_1, .... [x,Y]_n \f$
     * \param opacity the requested opacity value
     * \return true if successful, false otherwise with reason in OptiXError
     */
    DLL_EXPORT bool InsertPolygonalStop(size_t element_ID, int index, int numSides, double* vertexArray, bool opacity);

    /** \brief **MODIFIED** *28/06/2023* - Replace a Region in the aperture stop list by a new Polygon
     *
     * \param element_ID The ID of the optical element to modify.
     * \param index the position in the list where the region will be replaced
     * \param numSides The number of sides of the polygon, and the number of vertex points which are stored in the vertexArray.
     * \param vertexArray   A pointer to a memory array of double s to be filled with the vertex coordinates of the new polygon in order \f$ [x,Y]_0, [x,Y]_1, .... [x,Y]_n \f$
     * \param opacity the requested opacity value
     * \return true if successful, false otherwise with reason in OptiXError
     */
    DLL_EXPORT bool ReplaceStopByPolygon(size_t element_ID, int index, int numSides, double* vertexArray, bool opacity);



    /** \brief **MODIFIED** *28/06/2023* - Add a new Rectangle at the end of the aperture stop list
     *
     * The rectangle is defined by its size, center and rotation angle around its center\n
     * \param element_ID The ID of the optical element to modify.
     * \param Xwidth size of the rectangle in the X direction before rotation
     * \param Ywidth size of the rectangle in the Y direction before rotation
     * \param opacity  the requested opacity value
     * \param Xcenter abscissa of the rectangle center
     * \param Ycenter ordinate of the rectangle center
     * \param angle of rotation from the on-axis orientation
     * \param regionIndex pointer to an integer where, if valid (not NULL), the index of the added region will be returned.
     * \return true if successful, false otherwise with reason in OptiXError
     */
     DLL_EXPORT bool AddRectangularStop(size_t element_ID,  double Xwidth, double Ywidth, bool opacity, double Xcenter, double Ycenter, double angle, int *regionIndex);

    /** \brief **MODIFIED** *28/06/2023* - Insert a new Rectangle at the given position in the aperture stop list
     *
     * The rectangle is defined by its size, center and rotation angle around its center\n
     * The Region which was at index position and all regions above are moved-up one place in the list.
     * \param element_ID The ID of the optical element to modify.
     * \param index of the position in the list where the region must be inserted
     * \param Xwidth size of the rectangle in the X direction before rotation
     * \param Ywidth size of the rectangle in the Y direction before rotation
     * \param opacity  the requested opacity value
     * \param Xcenter abscissa of the rectangle center
     * \param Ycenter ordinate of the rectangle center
     * \param angle of rotation from the on-axis orientation
     * \return true if successful, false otherwise with reason in OptiXError
     */
    DLL_EXPORT bool InsertRectangularStop(size_t element_ID, int index, double Xwidth, double Ywidth,
                                          bool opacity, double Xcenter, double Ycenter, double angle);

    /** \brief **MODIFIED** *28/06/2023* - Replace the Region at the given position in the aperture stop list by a new Rectangle
     *
     * The rectangle is defined by its size, center and rotation angle around its center
     * \param element_ID The ID of the optical element to modify.
     * \param index of the position in the list where the region will be replaced
     * \param Xwidth size of the rectangle in the X direction before rotation
     * \param Ywidth size of the rectangle in the Y direction before rotation
     * \param opacity  the requested opacity value
     * \param Xcenter abscissa of the rectangle center
     * \param Ycenter ordinate of the rectangle center
     * \param angle of rotation from the on-axis orientation
     * \return true if successful, false otherwise with reason in OptiXError
     */
    DLL_EXPORT bool ReplaceStopByRectangle(size_t element_ID, int index, double Xwidth, double Ywidth,
                                           bool opacity, double Xcenter, double Ycenter, double angle);



    /** \brief **MODIFIED** *28/06/2023* - Returns the parameters defining an elliptical Region
     *
     * The Ellipse is defined by its axis half-widths, center position,  and rotation angle around its center
     * \param element_ID The ID of the optical element to query.
     * \param index the position in the list of the queried region
     * \param[out] Xaxis address of a variable to return the half-length of the X axis before rotation
     * \param[out] Yaxis address of a variable to return the half-length of the X axis before rotation
     * \param[out] opacity address of a variable to return the opacity value of the Region
     * \param[out] Xcenter address of a variable to return the abscissa of the ellipse center
     * \param[out] Ycenter address of a variable to return the ordinate of the ellipse center
     * \param[out] angle address of a variable to return the rotation angle of the ellipse with respect to the on-axis definition
     * \return true if successful, false otherwise with reason in OptiXError
     */
    DLL_EXPORT bool GetEllipseParameters(size_t element_ID, int index, double *Xaxis, double *Yaxis,
                                           bool *opacity, double *Xcenter, double *Ycenter, double *angle);

    /** \brief **MODIFIED** *28/06/2023* - Adds a new elliptical Region at the end of the aperture stop list
     *
     * The Ellipse is defined by its axis half-widths, center position,  and rotation angle around its center
     * \param element_ID The ID of the optical element to add
     * \param Xaxis the half-length of the X axis before rotation
     * \param Yaxis the half-length of the Y axis before rotation
     * \param opacity  the requested opacity value
     * \param Xcenter abscissa of the ellipse center
     * \param Ycenter ordinate of the ellipse center
     * \param angle of rotation from the on-axis orientation
     * \param regionIndex Location where, if valid (not NULL) the index of the added region will be returned
     * \return true if successful, false otherwise with reason in OptiXError
     */
    DLL_EXPORT bool AddEllipticalStop(size_t element_ID, double Xaxis, double Yaxis, bool opacity,
                                  double Xcenter, double Ycenter, double angle, int *regionIndex);

    /** \brief **MODIFIED** *28/06/2023* - Inserts a new elliptical Region at the given position in the aperture stop list
     *
     * The Ellipse is defined by its axis half-widths, center position,  and rotation angle around its center
     * The Region which was at index position and all regions above are moved-up one place in the list
     * \param element_ID The ID of the optical element to modify
     * \param index the position in the list where the region must be inserted
     * \param Xaxis the half-length of the X axis before rotation
     * \param Yaxis the half-length of the Y axis before rotation
     * \param opacity  the requested opacity value
     * \param Xcenter abscissa of the ellipse center
     * \param Ycenter ordinate of the ellipse center
     * \param angle of rotation from the on-axis orientation
     * \return true if successful, false otherwise with reason in OptiXError
     */
    DLL_EXPORT bool InsertEllipticalStop(size_t element_ID, int index, double Xaxis, double Yaxis,
                                           bool opacity, double Xcenter, double Ycenter, double angle);

    /** \brief **MODIFIED** *28/06/2023* - Replace the Region at the given position in the aperture stop list by a new Ellipse
     *
     * The Ellipse is defined by its axis half-widths, center position,  and rotation angle around its center
     * \param element_ID The ID of the optical element to modify
     * \param index the position in the list of the region to replace
     * \param Xaxis the half-length of the X axis before rotation
     * \param Yaxis the half-length of the Y axis before rotation
     * \param opacity  the requested opacity value
     * \param Xcenter abscissa of the ellipse center
     * \param Ycenter ordinate of the ellipse center
     * \param angle of rotation from the on-axis orientation
     * \return true if successful, false otherwise with reason in OptiXError
     */
    DLL_EXPORT bool ReplaceStopByEllipse(size_t element_ID, int index, double Xaxis, double Yaxis,
                                           bool opacity, double Xcenter, double Ycenter, double angle);


    /** \brief **MODIFIED** *28/06/2023* - Adds a new circular Region at the end of the aperture stop list
     *
     * The circle is defined by its radius and center position
     * \param element_ID The ID of the optical element to add
     * \param radius The radius of the circle
     * \param opacity  the requested opacity value
     * \param Xcenter abscissa of the  center
     * \param Ycenter ordinate of the  center
     * \param regionIndex A location where, if valid (not NULL), the index of the added region will be returned
     * \return true if successful, false otherwise with reason in OptiXError
     */
    DLL_EXPORT bool AddCircularStop(size_t element_ID, double radius, bool opacity,
                                      double Xcenter, double Ycenter, int *regionIndex);

    /** \brief **MODIFIED** *28/06/2023* - Insert a new circular Region at the given position in the aperture stop list
     *
     * The circle is defined by its radius and center position
     * The Region which was at index position and all regions above are moved-up one place in the list
     * \param element_ID The ID of the optical element to modify
     * \param index the position in the list of the region to insert
     * \param radius The radius of the circle
     * \param opacity  the requested opacity value
     * \param Xcenter abscissa of the  center
     * \param Ycenter ordinate of the  center
     * \return true if successful, false otherwise with reason in OptiXError
     */
    DLL_EXPORT bool InsertCircularStop(size_t element_ID, int index, double radius,
                                       bool opacity, double Xcenter, double Ycenter);

    /** \brief **MODIFIED** *28/06/2023* - Replace the Region at the given position in the aperture stop list by a new circle
     *
     * The circle is defined by its radius and center position
     * \param element_ID The ID of the optical element to modify
     * \param index the position in the list of the region to replace
     * \param radius The radius of the circle
     * \param opacity  the requested opacity value
     * \param Xcenter abscissa of the  center
     * \param Ycenter ordinate of the  center
     * \return true if successful, false otherwise with reason in OptiXError
     */
    DLL_EXPORT bool ReplaceStopByCircle(size_t element_ID, int index, double radius,
                                    bool opacity, double Xcenter, double Ycenter);




/** \} */ // end of apertureAPI group
#ifdef __cplusplus
}
#endif



#endif // APERTUREAPI_H_INCLUDED
