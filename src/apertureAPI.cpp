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

DLL_EXPORT size_t GetStopNumber(size_t element_ID)
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
    return psurf->m_aperture.getRegionCount();
}

DLL_EXPORT size_t GetStopType(size_t element_ID, size_t index, char* buffer, int bufferSize)
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
    if(index <0 || index >= psurf->m_aperture.getRegionCount())
            {
        SetOptiXLastError("index is invalid", __FILE__, __func__);
        return -1;
    }

    Region* pReg =psurf->m_aperture.getRegion(index);
    string className=pReg->getOptixClass();
    strncpy(buffer, className.c_str(), bufferSize);
    if(bufferSize <(int)(className.size())+1)
    {
        SetOptiXLastError("Buffer too small, name was truncated", __FILE__, __func__);
        return -1;
    }
    if(className=="Polygon")
        return  dynamic_cast<Polygon*>(pReg)->getNumSides();
    else
        return 0;

}


DLL_EXPORT size_t SetApertureActivity(size_t element_ID, bool status)
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
    psurf->setApertureActive(status);
    return 0;
}

DLL_EXPORT size_t GetApertureActivity(size_t element_ID, bool * status)
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
    *status=psurf->getApertureActive();
    return 0;
}


DLL_EXPORT size_t GetPolygonParameters(size_t element_ID, size_t index, size_t arrayWidth, double* vertexArray, bool *opacity)
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
    if(index <0 || index >= psurf->m_aperture.getRegionCount())
    {
        SetOptiXLastError("index is invalid", __FILE__, __func__);
        return -1;
    }

    Region* pReg =psurf->m_aperture.getRegion(index);
    string className=pReg->getOptixClass();

    if(className!="Polygon")
    {
        SetOptiXLastError("The region is not a Polygon", __FILE__, __func__);
        return -1;
    }
    *opacity=!pReg->isTransparent();

    size_t dim= dynamic_cast<Polygon*>(pReg)->getNumSides();
    if(arrayWidth < dim)
    {
        SetOptiXLastError("The array is too small to contain all vertices", __FILE__, __func__);
        return -1;
    }
    Map<Matrix2Xd> vertices(vertexArray,2,dim);
    vertices=dynamic_cast<Polygon*>(pReg)->getVertices();
    return dim;

}


DLL_EXPORT size_t AddPolygonalStop(size_t element_ID, size_t numSides, double* vertexArray, bool opacity)
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
    Map<Array2Xd> vertices(vertexArray,2, numSides);
    Polygon * ppoly=new Polygon(!opacity);
    ppoly->setVertices(vertices);

    return psurf->m_aperture.addRegion(ppoly); // index of the added object

}

DLL_EXPORT size_t InsertPolygonalStop(size_t element_ID, size_t index, size_t numSides, double* vertexArray, bool opacity)
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
    if(index <0 || index >= psurf->m_aperture.getRegionCount())
            {
        SetOptiXLastError("index is invalid", __FILE__, __func__);
        return -1;
    }
    Map<Array2Xd> vertices(vertexArray,2, numSides);
    Polygon * ppoly=new Polygon(!opacity);
    ppoly->setVertices(vertices);

    if(! psurf->m_aperture.insertRegion(index,ppoly))
        return -1;
    return index;
}

DLL_EXPORT size_t ReplaceStopByPolygon(size_t element_ID, size_t index, size_t numSides, double* vertexArray, bool opacity)
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
    if(index <0 || index >= psurf->m_aperture.getRegionCount())
            {
        SetOptiXLastError("index is invalid", __FILE__, __func__);
        return -1;
    }
    Map<Array2Xd> vertices(vertexArray,2, numSides);
    Polygon * ppoly=new Polygon(!opacity);
    ppoly->setVertices(vertices);

    if(! psurf->m_aperture.replaceRegion(index,ppoly))
        return -1;
    return index;  // index of the inserted object

}



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
    Polygon * prect=new Polygon(!opacity);
    prect->setRectangle(Xwidth, Ywidth, 0,0);
    prect->move(angle,center);
    return psurf->m_aperture.addRegion(prect); // index of the added object
}

DLL_EXPORT size_t InsertRectangularStop(size_t element_ID, size_t index, double Xwidth, double Ywidth, bool opacity, double Xcenter, double Ycenter, double angle)
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
    if(index <0 || index >= psurf->m_aperture.getRegionCount())
            {
        SetOptiXLastError("index is invalid", __FILE__, __func__);
        return -1;
    }
    Vector2d center;
    center <<  Xcenter,Ycenter;
    Polygon * prect=new Polygon(!opacity);
    prect->setRectangle(Xwidth, Ywidth, 0,0);
    prect->move(angle,center);
    if(! psurf->m_aperture.insertRegion(index,prect))
        return -1;
    return index;
}

DLL_EXPORT size_t ReplaceStopByRectangle(size_t element_ID, size_t index, double Xwidth, double Ywidth, bool opacity, double Xcenter, double Ycenter, double angle)
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
    if(index <0 || index >= psurf->m_aperture.getRegionCount())
            {
        SetOptiXLastError("index is invalid", __FILE__, __func__);
        return -1;
    }
    Vector2d center;
    center <<  Xcenter,Ycenter;
    Polygon * prect=new Polygon(!opacity);
    prect->setRectangle(Xwidth, Ywidth, 0,0);
    prect->move(angle,center);

    if(! psurf->m_aperture.replaceRegion(index,prect))
        return -1;
    return index;  // index of the inserted object

}

DLL_EXPORT size_t GetEllipseParameters(size_t element_ID, size_t index, double *Xaxis, double *Yaxis, bool *opacity, double *Xcenter, double *Ycenter, double *angle)
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
    if(index <0 || index >= psurf->m_aperture.getRegionCount())
    {
        SetOptiXLastError("index is invalid", __FILE__, __func__);
        return -1;
    }

    Region* pReg =psurf->m_aperture.getRegion(index);
    string className=pReg->getOptixClass();

    if(className!="Ellipse")
    {
        SetOptiXLastError("The region is not an Ellipse", __FILE__, __func__);
        return -1;
    }
    *opacity=!pReg->isTransparent();
    dynamic_cast<Ellipse*>(pReg)->getParameters(Xaxis, Yaxis, Xcenter, Ycenter, angle);

    return 0;

}


DLL_EXPORT size_t AddEllipticalStop(size_t element_ID, double Xaxis, double Yaxis, bool opacity, double Xcenter, double Ycenter, double angle)
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

    Ellipse * pellipse=new Ellipse(Xaxis, Yaxis, Xcenter,Ycenter, angle);
    pellipse->setTransparency(!opacity);

    return psurf->m_aperture.addRegion(pellipse); // index of the added object

}

DLL_EXPORT size_t InsertEllipticalStop(size_t element_ID, size_t index, double Xaxis, double Yaxis, bool opacity, double Xcenter, double Ycenter, double angle)
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
    if(index <0 || index >= psurf->m_aperture.getRegionCount())
            {
        SetOptiXLastError("index is invalid", __FILE__, __func__);
        return -1;
    }

    Ellipse * pellipse=new Ellipse(Xaxis,Yaxis, Xcenter,Ycenter, angle);
    pellipse->setTransparency(!opacity);

    if(! psurf->m_aperture.insertRegion(index,pellipse))
    return -1;
    return index;
}

DLL_EXPORT size_t ReplaceStopByEllipse(size_t element_ID, size_t index, double Xaxis, double Yaxis, bool opacity, double Xcenter, double Ycenter, double angle)
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
    if(index <0 || index >= psurf->m_aperture.getRegionCount())
            {
        SetOptiXLastError("index is invalid", __FILE__, __func__);
        return -1;
    }

    Ellipse * pellipse=new Ellipse(Xaxis,Yaxis, Xcenter,Ycenter, angle);
    pellipse->setTransparency(!opacity);

    if(! psurf->m_aperture.replaceRegion(index,pellipse))
        return -1;
    return index;  // index of the inserted object
}


DLL_EXPORT size_t AddCircularStop(size_t element_ID, double radius, bool opacity, double Xcenter, double Ycenter)
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

    Ellipse * pellipse=new Ellipse(!opacity);
    pellipse->setCircle(radius, Xcenter, Ycenter);

    return psurf->m_aperture.addRegion(pellipse); // index of the added object
}

DLL_EXPORT size_t InsertCircularStop(size_t element_ID, size_t index, double radius, bool opacity, double Xcenter, double Ycenter)
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
    if(index <0 || index >= psurf->m_aperture.getRegionCount())
            {
        SetOptiXLastError("index is invalid", __FILE__, __func__);
        return -1;
    }

    Ellipse * pellipse=new Ellipse(!opacity);
    pellipse->setCircle(radius, Xcenter, Ycenter);

    if(! psurf->m_aperture.insertRegion(index,pellipse))
    return -1;
    return index;
}

DLL_EXPORT size_t ReplaceStopByCircle(size_t element_ID, size_t index, double radius, bool opacity, double Xcenter, double Ycenter)
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
    if(index <0 || index >= psurf->m_aperture.getRegionCount())
            {
        SetOptiXLastError("index is invalid", __FILE__, __func__);
        return -1;
    }

    Ellipse * pellipse=new Ellipse(!opacity);
    pellipse->setCircle(radius, Xcenter, Ycenter);

    if(! psurf->m_aperture.replaceRegion(index,pellipse))
        return -1;
    return index;  // index of the inserted object
}


