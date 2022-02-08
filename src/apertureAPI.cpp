/**
 *************************************************************************
*   \file           apertureAPI.cpp
*
*   \brief          Aperture API  implementation file
*
*
*
*   \author             François Polack  <francois.polack@synchroton-soleil.fr>
*   \date               Creation: 2022-02-07
*   \date               Last update: 2022-02-07
 ***************************************************************************/

#include "apertureAPI.h"

    DLL_EXPORT size_t AddRectangularStop(size_t element_ID,  double Xwidth, double Ywidth, bool opacity, double Xcenter, double Ycenter, double angle)
    {
        if(!System.isValidID(element_ID))
        {
            SetOptiXLastError("invalid element ID", __FILE__, __func__);
            return -1;
        }
        Surface * psurf=dynamic_cast<Surface*>((ElementBase*)element_ID);
        if(! psurf )
        {
            SetOptiXLastError("element is not an OptiX Surface", __FILE__, __func__);
            return -1;
        }
        Vector2d center;
        center <<  Xcenter,Ycenter;
        Polygon * rect(0);
        rect->setRectangle(Xwidth, Ywidth, 0,0);
        rect->move(angle,center);
        return psurf->m_aperture.addRegion(rect,!opacity); // index of the added object
    }
