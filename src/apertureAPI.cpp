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
#include "surface.h"

DLL_EXPORT bool GetStopNumber(size_t element_ID, int * numStops)
{
    if(!numStops)
    {
        SetOptiXLastError("invalid return location numStops", __FILE__, __func__);
        return false;
    }
    if(!System.isValidID(element_ID))
    {
        SetOptiXLastError("invalid element ID", __FILE__, __func__);
        return false;
    }
    Surface * psurf=dynamic_cast<Surface*>((ElementBase*)element_ID);
    if(! psurf )
    {
        SetOptiXLastError("element is not an OptiX Surface", __FILE__, __func__);
        return false;
    }
    *numStops=psurf->m_aperture.getRegionCount();
    return true;
}

DLL_EXPORT bool GetStopType(size_t element_ID, size_t index, char** typeString, int* numSides)
{
    static char className[32]="Unknown Class";
    if(!System.isValidID(element_ID))
    {
        SetOptiXLastError("invalid element ID", __FILE__, __func__);
        return false;
    }
    Surface * psurf=dynamic_cast<Surface*>((ElementBase*)element_ID);
    if(! psurf )
    {
        SetOptiXLastError("element is not an OptiX Surface", __FILE__, __func__);
        return false;
    }
    if(index <0 || index >= psurf->m_aperture.getRegionCount())
    {
        SetOptiXLastError("index is invalid", __FILE__, __func__);
        return false;
    }
    if(!typeString)
    {
        SetOptiXLastError("Invalid return location for type name", __FILE__, __func__);
        return false;
    }
    Region* pReg =psurf->m_aperture.getRegion(index);
    strncpy(className, pReg->getOptixClass().c_str(), 31);
    *typeString=className;
    if(numSides)
    {
        if(strcmp(className,"Polygon")==0 )
            *numSides = dynamic_cast<Polygon*>(pReg)->getNumSides();
        else
            *numSides=0;
    }
    return true;
}


DLL_EXPORT bool SetApertureActivity(size_t element_ID, bool status)
{
    if(!System.isValidID(element_ID))
    {
        SetOptiXLastError("invalid element ID", __FILE__, __func__);
        return false;
    }
    Surface * psurf=dynamic_cast<Surface*>((ElementBase*)element_ID);
    if(! psurf )
    {
        SetOptiXLastError("element is not an OptiX Surface", __FILE__, __func__);
        return false;
    }
    psurf->setApertureActive(status);
    return true;
}

DLL_EXPORT bool GetApertureActivity(size_t element_ID, bool * status)
{
    if(!System.isValidID(element_ID))
    {
        SetOptiXLastError("invalid element ID", __FILE__, __func__);
        return false;
    }
    Surface * psurf=dynamic_cast<Surface*>((ElementBase*)element_ID);
    if(! psurf )
    {
        SetOptiXLastError("element is not an OptiX Surface", __FILE__, __func__);
        return false;
    }
    if(!status)
    {
        SetOptiXLastError("invalid location to return the aperture status", __FILE__, __func__);
        return false;
    }
    *status=psurf->getApertureActive();
    return true;
}



DLL_EXPORT bool GetApertureTransmissionAt(size_t element_ID, double x, double y, double * T)
{
    if(!System.isValidID(element_ID))
    {
        SetOptiXLastError("invalid element ID", __FILE__, __func__);
        return false;
    }
    Surface * psurf=dynamic_cast<Surface*>((ElementBase*)element_ID);
    if(! psurf )
    {
        SetOptiXLastError("element is not an OptiX Surface", __FILE__, __func__);
        return false;
    }
    if(!T)
    {
        SetOptiXLastError("invalid location to return the aperture transmission", __FILE__, __func__);
        return false;
    }
    Vector2d point;
    point << x,y;
    *T=psurf->getApertureTransmissionAt(point);
    return true;
}



DLL_EXPORT bool GetPolygonParameters(size_t element_ID, int index, int * arraySize, double* vertexArray, bool *opacity)
{
    if(!System.isValidID(element_ID))
    {
        SetOptiXLastError("invalid element ID", __FILE__, __func__);
        return false;
    }
    Surface * psurf=dynamic_cast<Surface*>((ElementBase*)element_ID);
    if(! psurf )
    {
        SetOptiXLastError("element is not an OptiX Surface", __FILE__, __func__);
        return false;
    }
    if(index <0 || index >=(int) psurf->m_aperture.getRegionCount())
    {
        SetOptiXLastError("index is invalid", __FILE__, __func__);
        return false;
    }
    if(!vertexArray)
    {
        SetOptiXLastError("invalid array pointer vertexArray", __FILE__, __func__);
        return false;
    }
    if(!opacity)
    {
        SetOptiXLastError("invalid location to return opacity", __FILE__, __func__);
        return false;
    }

    Region* pReg =psurf->m_aperture.getRegion(index);
    string className=pReg->getOptixClass();

    if(className!="Polygon")
    {
        SetOptiXLastError("The region is not a Polygon", __FILE__, __func__);
        return false;
    }
    if(opacity)
        *opacity=!pReg->isTransparent();

    size_t dim= dynamic_cast<Polygon*>(pReg)->getNumSides();
    if(*arraySize < (int) dim)
    {
        SetOptiXLastError("The array is too small to contain all vertices", __FILE__, __func__);
        return false;
    }
    Map<Matrix2Xd> vertices(vertexArray,2,dim);
    vertices=dynamic_cast<Polygon*>(pReg)->getVertices();
    *arraySize=dim;
    return true;
}


DLL_EXPORT bool AddPolygonalStop(size_t element_ID, int numSides, double* vertexArray, bool opacity, int *regionIndex)
{
    if(!System.isValidID(element_ID))
    {
        SetOptiXLastError("invalid element ID", __FILE__, __func__);
        return false;
    }
    Surface * psurf=dynamic_cast<Surface*>((ElementBase*)element_ID);
    if(! psurf )
    {
        SetOptiXLastError("element is not an OptiX Surface", __FILE__, __func__);
        return false;
    }
    if(! vertexArray )
    {
        SetOptiXLastError("Invalid vertex array", __FILE__, __func__);
        return false;
    }
    Map<Array2Xd> vertices(vertexArray,2, numSides);
    Polygon * ppoly=new Polygon(!opacity);
    ppoly->setVertices(vertices);
    if(regionIndex)
        *regionIndex=psurf->m_aperture.addRegion(ppoly); // index of the added object
    else
        psurf->m_aperture.addRegion(ppoly);
    return true;

}

DLL_EXPORT bool InsertPolygonalStop(size_t element_ID, int index, int numSides, double* vertexArray, bool opacity)
{
    if(!System.isValidID(element_ID))
    {
        SetOptiXLastError("invalid element ID", __FILE__, __func__);
        return false;
    }
    if(! vertexArray )
    {
        SetOptiXLastError("Invalid vertex array", __FILE__, __func__);
        return false;
    }
    Surface * psurf=dynamic_cast<Surface*>((ElementBase*)element_ID);
    if(! psurf )
    {
        SetOptiXLastError("element is not an OptiX Surface", __FILE__, __func__);
        return false;
    }
    if(index <0 || index >= (int) psurf->m_aperture.getRegionCount())
            {
        SetOptiXLastError("index is invalid", __FILE__, __func__);
        return false;
    }
    Map<Array2Xd> vertices(vertexArray,2, numSides);
    Polygon * ppoly=new Polygon(!opacity);
    ppoly->setVertices(vertices);

    return psurf->m_aperture.insertRegion(index,ppoly);
}

DLL_EXPORT bool ReplaceStopByPolygon(size_t element_ID, int index, int numSides, double* vertexArray, bool opacity)
{
    if(!System.isValidID(element_ID))
    {
        SetOptiXLastError("invalid element ID", __FILE__, __func__);
        return false;
    }
    if(! vertexArray )
    {
        SetOptiXLastError("Invalid vertex array", __FILE__, __func__);
        return false;
    }
    Surface * psurf=dynamic_cast<Surface*>((ElementBase*)element_ID);
    if(! psurf )
    {
        SetOptiXLastError("element is not an OptiX Surface", __FILE__, __func__);
        return false;
    }
    if(index <0 || index >= (int)psurf->m_aperture.getRegionCount())
            {
        SetOptiXLastError("index is invalid", __FILE__, __func__);
        return false;
    }
    Map<Array2Xd> vertices(vertexArray,2, numSides);
    Polygon * ppoly=new Polygon(!opacity);
    ppoly->setVertices(vertices);

    return psurf->m_aperture.replaceRegion(index,ppoly);

}



DLL_EXPORT bool AddRectangularStop(size_t element_ID,  double Xwidth, double Ywidth, bool opacity,
                                     double Xcenter, double Ycenter, double angle, int *regionIndex)
{
    if(!System.isValidID(element_ID))
    {
        SetOptiXLastError("invalid element ID", __FILE__, __func__);
        return false;
    }
    Surface * psurf=dynamic_cast<Surface*>((ElementBase*)element_ID);
    if(! psurf )
    {
        SetOptiXLastError("element is not an OptiX Surface", __FILE__, __func__);
        return false;
    }
//    Vector2d center;
//    center <<  Xcenter,Ycenter;
    Polygon * prect=new Polygon(!opacity);
    prect->setRectangle(Xwidth, Ywidth, Xcenter,Ycenter);
//    prect->move(angle,center);
    if(regionIndex)
        *regionIndex=psurf->m_aperture.addRegion(prect); // index of the added object
    else
        psurf->m_aperture.addRegion(prect);
    return true;
}

DLL_EXPORT bool InsertRectangularStop(size_t element_ID, int index, double Xwidth, double Ywidth,
                                      bool opacity, double Xcenter, double Ycenter, double angle)
{
    if(!System.isValidID(element_ID))
    {
        SetOptiXLastError("invalid element ID", __FILE__, __func__);
        return false;
    }
    Surface * psurf=dynamic_cast<Surface*>((ElementBase*)element_ID);
    if(! psurf )
    {
        SetOptiXLastError("element is not an OptiX Surface", __FILE__, __func__);
        return false;
    }
    if(index <0 || index >= (int) psurf->m_aperture.getRegionCount())
            {
        SetOptiXLastError("index is invalid", __FILE__, __func__);
        return false;
    }
    Vector2d center;
    center <<  Xcenter,Ycenter;
    Polygon * prect=new Polygon(!opacity);
    prect->setRectangle(Xwidth, Ywidth, 0,0);
    prect->move(angle,center);
    return psurf->m_aperture.insertRegion(index,prect);
}

DLL_EXPORT bool ReplaceStopByRectangle(size_t element_ID, int index, double Xwidth, double Ywidth,
                                       bool opacity, double Xcenter, double Ycenter, double angle)
{
    if(!System.isValidID(element_ID))
    {
        SetOptiXLastError("invalid element ID", __FILE__, __func__);
        return false;
    }
    Surface * psurf=dynamic_cast<Surface*>((ElementBase*)element_ID);
    if(! psurf )
    {
        SetOptiXLastError("element is not an OptiX Surface", __FILE__, __func__);
        return false;
    }
    if(index <0 || index >=(int) psurf->m_aperture.getRegionCount())
            {
        SetOptiXLastError("index is invalid", __FILE__, __func__);
        return false;
    }
    Vector2d center;
    center <<  Xcenter,Ycenter;
    Polygon * prect=new Polygon(!opacity);
    prect->setRectangle(Xwidth, Ywidth, 0,0);
    prect->move(angle,center);

    return  psurf->m_aperture.replaceRegion(index,prect);
}

DLL_EXPORT bool GetEllipseParameters(size_t element_ID, int index, double *Xaxis, double *Yaxis, bool *opacity, double *Xcenter, double *Ycenter, double *angle)
{
    if(!System.isValidID(element_ID))
    {
        SetOptiXLastError("invalid element ID", __FILE__, __func__);
        return false;
    }
    Surface * psurf=dynamic_cast<Surface*>((ElementBase*)element_ID);
    if(! psurf )
    {
        SetOptiXLastError("element is not an OptiX Surface", __FILE__, __func__);
        return false;
    }
    if(index <0 || index >=(int) psurf->m_aperture.getRegionCount())
    {
        SetOptiXLastError("index is invalid", __FILE__, __func__);
        return false;
    }

    Region* pReg =psurf->m_aperture.getRegion(index);
    string className=pReg->getOptixClass();

    if(className!="Ellipse")
    {
        SetOptiXLastError("The region is not an Ellipse", __FILE__, __func__);
        return false;
    }
    *opacity=!pReg->isTransparent();
    dynamic_cast<Ellipse*>(pReg)->getParameters(Xaxis, Yaxis, Xcenter, Ycenter, angle);

    return true;

}


DLL_EXPORT  bool AddEllipticalStop(size_t element_ID, double Xaxis, double Yaxis, bool opacity,
                                  double Xcenter, double Ycenter, double angle, int *regionIndex)
{
    if(!System.isValidID(element_ID))
    {
        SetOptiXLastError("invalid element ID", __FILE__, __func__);
        return false;
    }
    Surface * psurf=dynamic_cast<Surface*>((ElementBase*)element_ID);
    if(! psurf )
    {
        SetOptiXLastError("element is not an OptiX Surface", __FILE__, __func__);
        return false;
    }

    Ellipse * pellipse=new Ellipse(Xaxis, Yaxis, Xcenter,Ycenter, angle);
    pellipse->setTransparency(!opacity);

    if(regionIndex)
        *regionIndex = psurf->m_aperture.addRegion(pellipse); // index of the added object
    else
        psurf->m_aperture.addRegion(pellipse);
    return true;
}

DLL_EXPORT bool InsertEllipticalStop(size_t element_ID, int index, double Xaxis, double Yaxis,
                                       bool opacity, double Xcenter, double Ycenter, double angle)
{
    if(!System.isValidID(element_ID))
    {
        SetOptiXLastError("invalid element ID", __FILE__, __func__);
        return false;
    }
    Surface * psurf=dynamic_cast<Surface*>((ElementBase*)element_ID);
    if(! psurf )
    {
        SetOptiXLastError("element is not an OptiX Surface", __FILE__, __func__);
        return false;
    }
    if(index <0 || index >=(int) psurf->m_aperture.getRegionCount())
            {
        SetOptiXLastError("index is invalid", __FILE__, __func__);
        return false;
    }

    Ellipse * pellipse=new Ellipse(Xaxis,Yaxis, Xcenter,Ycenter, angle);
    pellipse->setTransparency(!opacity);

    return psurf->m_aperture.insertRegion(index,pellipse);
}

DLL_EXPORT bool ReplaceStopByEllipse(size_t element_ID, int index, double Xaxis, double Yaxis,
                                     bool opacity, double Xcenter, double Ycenter, double angle)
{
    if(!System.isValidID(element_ID))
    {
        SetOptiXLastError("invalid element ID", __FILE__, __func__);
        return false;
    }
    Surface * psurf=dynamic_cast<Surface*>((ElementBase*)element_ID);
    if(! psurf )
    {
        SetOptiXLastError("element is not an OptiX Surface", __FILE__, __func__);
        return false;
    }
    if(index <0 || index >= (int) psurf->m_aperture.getRegionCount())
            {
        SetOptiXLastError("index is invalid", __FILE__, __func__);
        return false;
    }

    Ellipse * pellipse=new Ellipse(Xaxis,Yaxis, Xcenter,Ycenter, angle);
    pellipse->setTransparency(!opacity);

    return psurf->m_aperture.replaceRegion(index,pellipse);
}


DLL_EXPORT bool AddCircularStop(size_t element_ID, double radius, bool opacity,
                                double Xcenter, double Ycenter, int * regionIndex)
{
    if(!System.isValidID(element_ID))
    {
        SetOptiXLastError("invalid element ID", __FILE__, __func__);
        return false;
    }
    Surface * psurf=dynamic_cast<Surface*>((ElementBase*)element_ID);
    if(! psurf )
    {
        SetOptiXLastError("element is not an OptiX Surface", __FILE__, __func__);
        return false;
    }

    Ellipse * pellipse=new Ellipse(!opacity);
    pellipse->setCircle(radius, Xcenter, Ycenter);

    if(regionIndex)
        *regionIndex=psurf->m_aperture.addRegion(pellipse); // index of the added object
    else
        psurf->m_aperture.addRegion(pellipse);
    return true;
}

DLL_EXPORT bool InsertCircularStop(size_t element_ID, int index, double radius,
                                       bool opacity, double Xcenter, double Ycenter)
{
    if(!System.isValidID(element_ID))
    {
        SetOptiXLastError("invalid element ID", __FILE__, __func__);
        return false;
    }
    Surface * psurf=dynamic_cast<Surface*>((ElementBase*)element_ID);
    if(! psurf )
    {
        SetOptiXLastError("element is not an OptiX Surface", __FILE__, __func__);
        return false;
    }
    if(index <0 || index >=(int) psurf->m_aperture.getRegionCount())
            {
        SetOptiXLastError("index is invalid", __FILE__, __func__);
        return false;
    }

    Ellipse * pellipse=new Ellipse(!opacity);
    pellipse->setCircle(radius, Xcenter, Ycenter);

    return psurf->m_aperture.insertRegion(index,pellipse);
}

DLL_EXPORT bool ReplaceStopByCircle(size_t element_ID, int index, double radius,
                                    bool opacity, double Xcenter, double Ycenter)
{
    if(!System.isValidID(element_ID))
    {
        SetOptiXLastError("invalid element ID", __FILE__, __func__);
        return false;
    }
    Surface * psurf=dynamic_cast<Surface*>((ElementBase*)element_ID);
    if(! psurf )
    {
        SetOptiXLastError("element is not an OptiX Surface", __FILE__, __func__);
        return false;
    }
    if(index <0 || index >=(int) psurf->m_aperture.getRegionCount())
            {
        SetOptiXLastError("index is invalid", __FILE__, __func__);
        return false;
    }

    Ellipse * pellipse=new Ellipse(!opacity);
    pellipse->setCircle(radius, Xcenter, Ycenter);

    return psurf->m_aperture.replaceRegion(index,pellipse);

}


