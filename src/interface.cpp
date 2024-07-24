////////////////////////////////////////////////////////////////////////////////
/**
*      \file           interface.cpp
*
*      \brief         interface  C functions implementation
*
*      \author         François Polack <francois.polack@synchroton-soleil.fr>
*      \date        2020-11-12  Creation
*      \date        Last update
*
*/
///////////////////////////////////////////////////////////////////////////////////
//
//             REVISIONS
//
////////////////////////////////////////////////////////////////////////////////////

#include <map>
#include <set>
#include <vector>
#include <iostream>
#include "interface.h"
#include "sources.h"
#include "opticalelements.h"
#include "files.h"
#include "xmlfile.h"
#include "version.h"
#include <limits>  // pour epsilon

#define NFFT_PRECISION_DOUBLE
#include <nfft3mp.h>

/** \ingroup InternalVar
*   \{
*/

#ifdef HAS_REFLEX  // if libRefleX added to linklist
#include "ReflectivityAPI.h"

map<string, XDatabase> dataBases;   /**< \brief a set of DABAX data bases indexed for fast access  */
map<string, MaterialTable> indexTables; /**< \brief a set of tables of material index. Material tables interpolate
                                        *  the index of the registered materials on a common energy grid with minimum number of steps*/
map<string,CoatingTable> coatingTables;  /**< \brief a set of CoatingTable. A coating table enables interpolating the reflectivity
                                        *   of the registered coatings on a unique energy and angle grid */

#endif // HAS_REFLEX

ElementCollection System;   /**< \brief dictionary of all elements created through this interface  */
                           // system, hence all optical elements, will be deleted

bool enableApertureLimit=true; /**< \brief Global flag to take into account or not the apertures stops in the ray tracing
                                 *  inhibitApertureLimit replaced by enableApertureLimit of complementary value  should be user transparent*/
bool enableReflectivity=false;     /**<  \brief Global flag to switch on or off the computation of reflectivity  in the ray tracing computation*/
bool enableSurfaceErrors=false; /**< \brief Global flag indicating whether the surface errors are taken into account in propagation */
bool threadInitialized=false;   /**< \brief Global flag to keep track of Open_MP  initialization */

/** \} */ // end of InternalVar

/** \brief Initialize Multi-thread mode for NFFT if openmp is available (set the -fopenmp compiler flag and link  with libgomp
 *  \ingroup GlobalCpp
 */
void Init_Threads()
{
    if(threadInitialized)
        return;


//#ifdef _OPENMP  NFFT is already compiled with OpenMP
//    cout << "openmp is defined\n";
//#ifndef _DEBUG   // we dont want debuggin in MT mode
// #ifdef MAXTHREADS  // num threads is limited
//     int maxthreads=omp_get_max_threads();
//    cout << "Available processors " << available << endl;
//    maxthreads= available > MAXTHREADS ? MAXTHREADS : available;
//  #else
//    maxthreads=omp_get_max_threads();
//  #endif // threads unlimited
//    //double time0=omp_get_wtime();  // initialise l'horloge
//    omp_set_num_threads(maxthreads);
   fftw_init_threads();
//#endif  // _DEBUG
//#else
//    cout << "openmp is not defined\n";
//#endif // _OPENMP
//  //  Eigen::setNbThreads(0); // use default thread number in Eigen
//    cout << "number of active threads " << maxthreads <<endl;
    threadInitialized=true;
}



#ifdef __cplusplus
extern "C"
{
#endif

    DLL_EXPORT bool Version(char** version){
        static char versionString[128];
        sprintf(versionString,"SR_Source library %s release %s build %s-%s-%s\n", AutoVersion::STATUS, AutoVersion::FULLVERSION_STRING,
                    AutoVersion::YEAR, AutoVersion::MONTH, AutoVersion::DATE);
        if(version==NULL)
            cout << versionString <<endl;
        else
            *version=versionString;
        return true;
//        printf( " FloatType size  %lld  Epsilon value= %Lg \n",sizeof(FloatType) , numeric_limits<FloatType>::epsilon() );
    }


    DLL_EXPORT bool IsElementValid(size_t  ID, bool *valid)
    {
        *valid = System.isValidID(ID);
        return true;
    }

    DLL_EXPORT bool GetOptiXLastError(char* buffer, int bufferSize)
    {
        if(OptiXError )
        {
            if(buffer)
                strncpy(buffer,LastError.c_str(), bufferSize);
            OptiXError=false;
            return true;
        }

        if(buffer)
                strncpy(buffer,"No Error", bufferSize);
        return false;

    }

    DLL_EXPORT bool GetOptiXError( char** errstring_ptr)
    {
        if(OptiXError )
        {
            *errstring_ptr=(char*)LastError.c_str();
            OptiXError=false;
            return true;
        }
        *errstring_ptr=noerror;
        return  false;
    }


    DLL_EXPORT bool CreateElement(const char* type, const char* name, size_t* elementID)
    {
        ClearOptiXError();
        if(!elementID)
        {
            SetOptiXLastError("invalid location for returning elementID", __FILE__, __func__);
            return false;
        }
        *elementID=(size_t) System.createElement(type,name);
        if(*elementID)
            return true;
        else
            return false;
    }

//    {
//        ClearOptiXError();
//        if(System.find(name)!=System.end())
//        {
//            SetOptiXLastError("Name already exists in the current system", __FILE__, __func__);
//            return 0;
//        }
//        ElementBase * elem=0;
//        try {
//                elem=(ElementBase*)CreateElementObject(type,name);
//            }
//            catch(ElementException& ex)
//            {
//                 SetOptiXLastError( "Invalid element type", __FILE__,__func__);
//                 return 0;
//            }
//
//        System.insert(pair<string, ElementBase*>(name, elem) );
//        System.ValidIDs.insert((size_t) elem);
//        return (size_t)elem;
//    }

    DLL_EXPORT bool EnumerateElements(size_t * pHandle, size_t* elemID, char * nameBuffer, const int bufSize)
    {
        ClearOptiXError();
        map<string,ElementBase*>::iterator* pit;
        if(*pHandle==0)
            pit=new map<string,ElementBase*>::iterator(System.begin());
        else
            pit=(map<string,ElementBase*>::iterator* )*pHandle;

        strncpy(nameBuffer,(*pit)->first.c_str(), bufSize);
        *elemID=(size_t) (*pit)->second;
        if(bufSize < (int) (*pit)->first.size()+1)
        {
            SetOptiXLastError("nameBuffer is too small",__FILE__,__func__);
            delete pit;
            *pHandle=0;
            return false;
        }

        if(++(*pit)== System.end())
        {
             delete pit;
             *pHandle=0;
        }
        else
            *pHandle=(size_t) pit;

        return true;
    }

    DLL_EXPORT bool ReleaseElementEnumHandle(size_t handle)
    {
        ClearOptiXError();
        if(handle)
        {
            delete (map<string, ElementBase*>::iterator*) handle;
            return true;
        }
        else
        {
            SetOptiXLastError("invalid handle", __FILE__, __func__);
            return false;
        }
    }


    DLL_EXPORT bool FitSurfaceToHeights(size_t elementID, int64_t Nx, int64_t Ny, const double *limits, const ArrayParameter *heights, double* sigmah)
    {
        ClearOptiXError();
        if(!System.isValidID(elementID))
        {
            SetOptiXLastError("invalid element ID", __FILE__, __func__);
            return false;
        }
        string surfaceClass=((Surface*)elementID)->getSurfaceClass();
        if(surfaceClass!="NaturalPolynomialSurface" || surfaceClass!="LegendrePolynomialSurface")
        {
            SetOptiXLastError(string("Function not supported by the element base class ")+surfaceClass, __FILE__, __func__);
            return false;
        }
        Parameter param;
        ((Surface*)elementID)->getParameter("surfaceLimits", param);
        if(!(param.flags&ArrayData) || (param.paramArray->dims[0]!=2) ||(param.paramArray->dims[1]!=2) )
        {
            SetOptiXLastError("invalid surfaceLimits parameter", __FILE__, __func__);
            return false;
        }
        memcpy(param.paramArray->data,limits,4*sizeof(double));
        double s=0;
        try
        {
            if(surfaceClass=="NaturalPolynomialSurface")
            {
                NaturalPolynomialSurface* polysurf=(NaturalPolynomialSurface*)elementID;
                polysurf->setParameter("surfaceLimits",param);
                s= polysurf->fitHeightData(Nx,Ny, *heights);
            }
            else if(surfaceClass=="LegendrePolynomialSurface")
            {
                LegendrePolynomialSurface* polysurf=(LegendrePolynomialSurface*)elementID;
                polysurf->setParameter("surfaceLimits",param);
                s= polysurf->fitHeightData(Nx,Ny, *heights);

            }


        }
        catch(ParameterException & excpt)
        {
            SetOptiXLastError(excpt.what(), __FILE__, __func__);
            return false;
        }
        if(sigmah)
            *sigmah=s;

        return true;
    }


    DLL_EXPORT bool FitSurfaceToSlopes(size_t elementID, int64_t Nx, int64_t Ny, const double *limits, const ArrayParameter *slopes, double* sigmasx, double* sigmasy)
    {
        ClearOptiXError();
        if(!System.isValidID(elementID))
        {
            SetOptiXLastError("invalid element ID", __FILE__, __func__);
            return false;
        }
        string surfaceClass=((Surface*)elementID)->getSurfaceClass();
        if(surfaceClass!="NaturalPolynomialSurface" || surfaceClass!="LegendrePolynomialSurface")
        {
            SetOptiXLastError(string("Function not supported by the element base class ")+surfaceClass, __FILE__, __func__);
            return false;
        }
        Parameter param;
        ((Surface*)elementID)->getParameter("surfaceLimits", param);
        if(!(param.flags&ArrayData) || (param.paramArray->dims[0]!=2) ||(param.paramArray->dims[1]!=2) )
        {
            SetOptiXLastError("invalid surfaceLimits parameter", __FILE__, __func__);
            return false;
        }
        memcpy(param.paramArray->data,limits,4*sizeof(double));

        pair<double,double> sigmas;

        try
        {
            if(surfaceClass=="NaturalPolynomialSurface")
            {
                NaturalPolynomialSurface* polysurf=(NaturalPolynomialSurface*)elementID;
                polysurf->setParameter("surfaceLimits",param);
                sigmas= polysurf->fitSlopeData(Nx,Ny, *slopes);
            }
            else if(surfaceClass=="LegendrePolynomialSurface")
            {
                LegendrePolynomialSurface* polysurf=(LegendrePolynomialSurface*)elementID;
                polysurf->setParameter("surfaceLimits",param);
                sigmas= polysurf->fitSlopeData(Nx,Ny, *slopes);
            }


        }
        catch(ParameterException & excpt)
        {
            SetOptiXLastError(excpt.what(), __FILE__, __func__);
            return false;
        }
        if(sigmasx)
            *sigmasx=sigmas.first;
        if(sigmasy)
            *sigmasy=sigmas.second;
        return true;
    }



//    DLL_EXPORT size_t GetElementID(const char* elementName)
//    {
//
//        map<string,ElementBase*>:: iterator it= System.find(elementName);
//        if(it==System.end())
//            return 0;
//        return (size_t) it->second;
//    }

    DLL_EXPORT bool FindElementID(const char* elementName, size_t * elemID)
    {
        ClearOptiXError();
        if(!elemID)
        {
            SetOptiXLastError("invalid location for returning elemID", __FILE__, __func__);
            return false;
        }

        map<string,ElementBase*>:: iterator it= System.find(elementName);
        if(it==System.end())
        {
            SetOptiXLastError(string("Element of name  ")+elementName+" doesn't exist", __FILE__, __func__);
            *elemID=0;
            return false;
        }
        *elemID=(size_t) it->second;
        return true;

    }

    DLL_EXPORT bool GetElementName(size_t elementID, char* strBuffer, int bufSize)
    {
        ClearOptiXError();
        if(!System.isValidID(elementID))
        {
            SetOptiXLastError("invalid element ID", __FILE__, __func__);
            return false;
        }
        strncpy(strBuffer,((ElementBase*)elementID)->getName().c_str(), bufSize);
        if(bufSize <(int)((ElementBase*)elementID)->getName().size()+1)
        {
            SetOptiXLastError("Buffer too small, name was truncated", __FILE__, __func__);
            return false;
        }
        return true;
    }

    DLL_EXPORT bool GetElementType(size_t elementID, char* strBuffer, int bufSize)
    {
        ClearOptiXError();
        if(!System.isValidID(elementID))
        {
            SetOptiXLastError("invalid element ID", __FILE__, __func__);
            return false;
        }
        strncpy(strBuffer,((ElementBase*)elementID)->getOptixClass().c_str(), bufSize);
        if(bufSize <(int)((ElementBase*)elementID)->getOptixClass().size()+1)
        {
            SetOptiXLastError("Buffer too small, type was truncated", __FILE__, __func__);
            return false;
        }
       return true;
    }

    DLL_EXPORT bool DeleteElement_byName(const char* name)
    {
        ClearOptiXError();
        map<string,ElementBase*>:: iterator it= System.find(name);
        if(it==System.end())
            return false ;

        //ValidIDs.erase( (size_t) it->second );
        System.erase(it);
        return true;
    }

    DLL_EXPORT bool DeleteElement_byID(size_t elementID)
    {
        ClearOptiXError();
        set<size_t>::iterator it=System.ValidIDs.find(elementID);
        if(it==System.ValidIDs.end())
            return false;
        string name= ((ElementBase*)elementID)->getName();
        return DeleteElement_byName(name.c_str());
    }

    DLL_EXPORT bool ChainElement_byName(const char* previous, const char* next)
    {
        ClearOptiXError();
        map<string, ElementBase*>::iterator it;
        ElementBase* elprev=NULL;
        ElementBase* elnext=NULL;

        if(previous[0]!=0)
        {
            it==System.find(previous);
            if(it==System.end())
                return false;
            elprev=it->second;
        }
        if (next[0]!=0)
        {
            it=System.find(next);
            if(it==System.end())
                return false;
            elnext=it->second;
        }
        if(!elprev)
        {
            if(elnext)
                elnext->chainPrevious(NULL);
            else
                return false;
        }
        else
            elprev->chainNext(elnext);
        return true;
    }

    DLL_EXPORT bool ChainElement_byID(size_t prevID, size_t nextID)
    {
        ClearOptiXError();
        if(prevID==0)
        {
            if(nextID!=0 && System.isValidID(nextID))
            {
                ((ElementBase*)nextID)->chainPrevious(NULL);
                return true;
            }
            else
                return false;
        }
        else if(!System.isValidID(prevID))
            return false;

        if(nextID==0)
            ((ElementBase*)prevID)->chainNext(NULL);
        else
            ((ElementBase*)prevID)->chainNext((ElementBase*)nextID);

        return true;

    }

    DLL_EXPORT bool FindPreviousElement(const char * elementName, size_t * previousID)
    {
        ClearOptiXError();
        if(!previousID)
        {
            SetOptiXLastError("invalid location for returning previousID", __FILE__, __func__);
            return false;
        }
        map<string,ElementBase*>:: iterator it= System.find(elementName);
        if(it==System.end())
        {
            SetOptiXLastError(string("Element of name  ")+elementName+" doesn't exist", __FILE__, __func__);
            *previousID=0;
            return false;
        }

        *previousID=(size_t) it->second->getPrevious();
        return true;
            return 0;
   }


    DLL_EXPORT bool FindNextElement(const char * elementName, size_t * nextID)
    {
        ClearOptiXError();
        if(!nextID)
        {
            SetOptiXLastError("invalid location for returning nextID", __FILE__, __func__);
            return false;
        }
        map<string,ElementBase*>:: iterator it= System.find(elementName);
        if(it==System.end())
        {
            SetOptiXLastError(string("Element of name  ")+elementName+" doesn't exist", __FILE__, __func__);
            *nextID=0;
            return false;
        }

        *nextID=(size_t) it->second->getNext();
        return true;
            return 0;
   }

    DLL_EXPORT bool GetPreviousElement(size_t elementID, size_t * previousID )
    {
        ClearOptiXError();
        if(System.isValidID(elementID))
        {
            *previousID = (size_t) ((ElementBase*)elementID)->getPrevious();
            return true;
        }
        *previousID = 0;
        SetOptiXLastError("invalid element ID", __FILE__, __func__);
        return false;
    }

    DLL_EXPORT bool GetNextElement(size_t elementID, size_t * nextID)
    {
        ClearOptiXError();
        if(!System.isValidID(elementID))
        {
            *nextID=0;
            SetOptiXLastError("invalid element ID", __FILE__, __func__);
            return false;
        }
        *nextID = (size_t) ((ElementBase*)elementID)->getNext();
        return true;
    }

    DLL_EXPORT bool GetTransmissive(size_t elementID, bool * transmissionMode)
    {
        ClearOptiXError();
        if(!System.isValidID(elementID))
        {
            SetOptiXLastError("invalid element ID", __FILE__, __func__);
            return false;
        }
        *transmissionMode = ((ElementBase*) elementID)->getTransmissive();
        return true;
    }

    DLL_EXPORT bool SetTransmissive(size_t elementID, bool transmit)
    {
        ClearOptiXError();
        if(!dynamic_cast<GratingBase*>((ElementBase*) elementID))
            return false;

        ((ElementBase*) elementID)->setTransmissive(transmit);
        return true;
    }

    DLL_EXPORT bool GetRecording(size_t elementID, enum RecordMode *recordingMode)
    {
        ClearOptiXError();
        if(!dynamic_cast<Surface*>((ElementBase*)elementID) )
        {
            SetOptiXLastError("The pointed element is not an optical surface",__FILE__, __func__);
            return false ;
        }
        else
        {
            *recordingMode=dynamic_cast<Surface*>((ElementBase*)elementID)->getRecording();
            return true;
        }
    }

    DLL_EXPORT bool SetRecording(size_t elementID, enum RecordMode recordingMode)
    {
        ClearOptiXError();
        if(recordingMode <RecordNone || recordingMode > RecordOutput)
        {
            SetOptiXLastError("Invalid recording mode",__FILE__, __func__);
            return false ;
        }
        if(!dynamic_cast<Surface*>((ElementBase*)elementID) )
        {
            char what[256];
            sprintf(what,"The element %llX is not an optical surface", elementID);
            SetOptiXLastError(what,__FILE__, __func__);
            return false;
        }

        dynamic_cast<Surface*>((ElementBase*)elementID)->setRecording( (RecordMode) recordingMode);
        return true;
    }

    DLL_EXPORT bool SetParameter(size_t elementID, const char* paramTag,  Parameter paramData)
    {
        ClearOptiXError();
        if(System.isValidID(elementID))
            return ((ElementBase*)elementID)->setParameter(paramTag, paramData);
        else
        {
             SetOptiXLastError("The given elementID is invalid ", __FILE__, __func__);
             return false;
        }
    }


//    DLL_EXPORT bool SetArrayParameter(size_t elementID, const char* paramTag, size_t fastindex, size_t slowindex, double *data)
//    {
//        if(!System.isValidID(elementID))
//        {
//             SetOptiXLastError("The given elementID is invalid ", __FILE__, __func__);
//             return false;
//        }
//        Parameter aparam;
//        if(!((ElementBase*)elementID)->getParameter(paramTag, aparam ))
//        {
//             SetOptiXLastError( string("The element doen't have a parammeter named ")+paramTag, __FILE__, __func__);
//             return false;
//        }
//        aparam.paramArray->dims[0]=fastindex;
//        aparam.paramArray->dims[1]=slowindex;
//        aparam.paramArray->data=data;
//        return ((ElementBase*)elementID)->setParameter(paramTag, aparam );
//    }


    DLL_EXPORT bool DumpParameter(size_t elementID, const char* paramTag)
    {
        ClearOptiXError();
        if(System.isValidID(elementID))
            return ((ElementBase*)elementID)->dumpParameter(paramTag);
        else
        {
             SetOptiXLastError("The given elementID is invalid ", __FILE__, __func__);
             return false;
        }
    }

    DLL_EXPORT void DumpArgParameter(Parameter* param)
    {
        param->dump();
    }

    DLL_EXPORT void MemoryDump(void* address, uint64_t size)
    {
        memoryDump(address, size);
    }

    DLL_EXPORT bool GetParameterFlags(size_t elementID, const char* paramTag, uint32_t *flags)
    {
        ClearOptiXError();
        if(System.isValidID(elementID))
        {
            if(!((ElementBase*)elementID)->getParameterFlags(paramTag,*flags))
                return false; //if name is invalid last error is already set
        }
        else
        {
             SetOptiXLastError("The given elementID is invalid ", __FILE__, __func__);
             return false;
        }
        return true;
    }

    DLL_EXPORT bool GetParameter(size_t elementID, const char* paramTag, Parameter* paramData)
    {
        ClearOptiXError();
        if(System.isValidID(elementID))
        {
            uint32_t flags;
            if(!((ElementBase*)elementID)->getParameterFlags(paramTag,flags))
                return false; //if name is invalid last error is already set
            if (flags & ArrayData)
            {
                SetOptiXLastError(string("Array parameter  ") + paramTag  +
                                " should be retrieved with the GetArrayParameter function"  , __FILE__, __func__);
                return false;
            }
            else
            {
                Parameter param;
                if(! ((ElementBase*)elementID)->getParameter(paramTag, param))
                    return false ;
                switch(param.copy(*paramData))
                {
                case 0:
                    return true;
                case 1:
                    char msg[256];
                    sprintf(msg, "In Parameter::copy, the array flags of the source struct(%X) and destination struct (%X) do not match",
                             paramData->flags, param.flags);
                    SetOptiXLastError(msg,__FILE__, __func__);
                    return false;
                case 2:
                    SetOptiXLastError("In Parameter::copy, the array pointer of the destination Parameter is invalid",__FILE__, __func__);
                    return false;
                case 3:
                    SetOptiXLastError("In Parameter::copy, the data memory pointer of destination is invalid or points to insufficient memory",__FILE__, __func__);
                    return false;
                default:
                    SetOptiXLastError("Invalid return value or Parameter::copy function ",__FILE__, __func__);
                    return false;
                }

            }
        }
        else
        {
             SetOptiXLastError("The given elementID is invalid ", __FILE__, __func__);
             return false;
        }
    }

     DLL_EXPORT bool GetParameterArraySize(size_t elementID, const char* paramTag, size_t * size)
     {
        ClearOptiXError();
        if(!System.isValidID(elementID))
        {
             SetOptiXLastError("The given elementID is invalid ", __FILE__, __func__);
             return false;
        }

        uint32_t flags;
        if(!((ElementBase*)elementID)->getParameterFlags(paramTag,flags))
            return false; //if name is invalid last error is already set
        if (flags & ArrayData)
        {
            if( ((ElementBase*)elementID)->getParameterArraySize(paramTag, size))
                return true;
            else
                return false;  // OptixLastError is already set
        }
        else
        {
            SetOptiXLastError(string("Parameter  ") + paramTag  +
                            " should be retrieved with the GetParameter function"  , __FILE__, __func__);
            return false;
        }
     }


    DLL_EXPORT bool GetParameterArrayDims(size_t elementID, const char* paramTag, int64_t (*dims)[2])
    {
        ClearOptiXError();
       // cout << "in GetParameterArrayDims  sizeof i =" << sizeof(*dims)/sizeof (int64_t) <<endl;
        if(!System.isValidID(elementID))
        {
             SetOptiXLastError("The given elementID is invalid ", __FILE__, __func__);
             return false;
        }
        if(!dims)
        {
             SetOptiXLastError("The dims parameter is an invalid pointer ", __FILE__, __func__);
             return false;
        }
        uint32_t flags;
        if(!((ElementBase*)elementID)->getParameterFlags(paramTag,flags))
            return false; //if name is invalid last error is already set
        if (flags & ArrayData)
        {
            int64_t (*dimptr)[2]=0;
            if( ((ElementBase*)elementID)->getParameterArrayDims(paramTag, &dimptr))
            {
                memcpy(*dims,*dimptr, 2*sizeof(int64_t));
                return true;
            }
            else
                return false;  // OptixLastError is already set
        }
        else
        {
            SetOptiXLastError(string("Parameter  ") + paramTag  +
                            " should be retrieved with the GetParameter function"  , __FILE__, __func__);
            return false;
        }
     }

    DLL_EXPORT bool GetArrayParameter(size_t elementID, const char* paramTag, Parameter* paramData, size_t maxsize)
    {
        ClearOptiXError();
        if( ! System.isValidID(elementID))
        {
            SetOptiXLastError("The given elementID is invalid ", __FILE__, __func__);
            return false;
        }

        size_t paramSize;
        uint32_t flags;
        if(!((ElementBase*)elementID)->getParameterFlags(paramTag,flags))
            return false; // parameter not found OptixLastError already set
        if(flags & ArrayData)
        {
            if(!((ElementBase*)elementID)->getParameterArraySize(paramTag,&paramSize))
                return false; //if anaything is invalid last error is already set

            if(paramSize > maxsize)
            {
                SetOptiXLastError(string("Size of array parameter  ") + paramTag +
                            " is " + std::to_string(paramSize) + ", which is larger than the buffer size "  , __FILE__, __func__);
                return false;
            }
//            return ((ElementBase*)elementID)->getParameter(paramTag, *paramData);
            Parameter param;
            if(! ((ElementBase*)elementID)->getParameter(paramTag, param))
                return false ;
            switch(param.copy(*paramData))
            {
            case 0:
                return true;
            case 1:
                char msg[256];
                sprintf(msg, "In Parameter::copy, the array flags of the source struct(%X) and destination struct (%X) do not match",
                        param.flags, paramData->flags);
                SetOptiXLastError(msg,__FILE__, __func__);
                return false;
            case 2:
                SetOptiXLastError("In Parameter::copy, the array pointer of the destination Parameter is invalid",__FILE__, __func__);
                return false;
            case 3:
                SetOptiXLastError("In Parameter::copy, the data memory pointer of destination is invalid or points to insufficient memory",__FILE__, __func__);
                return false;
            default:
                SetOptiXLastError("Invalid return value or Parameter::copy function ",__FILE__, __func__);
                return false;
            }
        }
        SetOptiXLastError(string("Single value parameter  ") + paramTag  +
            " should be retrieved with the GetParameter function"  , __FILE__, __func__);
        return false;



    }

    DLL_EXPORT bool EnumerateParameters(size_t elementID, size_t * pHandle, char* tagBuffer, const int bufSize ,  Parameter* paramData)
    {
        ClearOptiXError();
        if(!System.isValidID(elementID))
        {
            SetOptiXLastError("Invalid element ID", __FILE__, __func__);
            return false;
        }

        map<string, Parameter>::iterator* pRef;
        if (*pHandle==0)
            pRef= new map<string, Parameter>::iterator( ((ElementBase*)elementID)->parameterBegin() );
        else
            pRef= (map<string, Parameter>::iterator*) *pHandle;

        strncpy(tagBuffer, (char*)((*pRef)->first).c_str(), bufSize);
        *paramData=(*pRef)->second;
        if(bufSize <(int) (*pRef)->first.size()+1)
        {
            SetOptiXLastError("Buffer too small", __FILE__, __func__);
            delete pRef;
            *pHandle=0;
            if(paramData->flags & ArrayData) // clear Array data if any
            {
                if(paramData->paramArray)
                {
                    delete paramData->paramArray;
                    paramData->flags &= ~ArrayData;
                    paramData->value=0;
                }
            }
            return false;
        }

        if(++(*pRef) ==((ElementBase*)elementID)->parameterEnd() )
        {
            ClearOptiXError();
            delete pRef;
            *pHandle=0;
            if(paramData->flags & ArrayData) // clear Array data if any
            {
                if(paramData->paramArray)
                {
                    delete paramData->paramArray;
                    paramData->flags &= ~ArrayData;
                    paramData->value=0;
                }
            }
            return true;
        }
        * pHandle=(size_t) pRef;
        return true;
    }


    DLL_EXPORT  bool ReleaseParameterEnumHandle(size_t handle, Parameter *paramData)
    {
        ClearOptiXError();
        if(!handle)
        {
            SetOptiXLastError("Invalid handle passed", __FILE__, __func__);
            return false;
        }

        delete (map<string, Parameter>::iterator*) handle;

        if(paramData)
            if(paramData->flags & ArrayData) // clear Array data if any
            {
                if(paramData->paramArray)
                {
                    delete paramData->paramArray;
                    paramData->flags &= ~ArrayData;
                    paramData->value=0;
                }
            }
        return true;
    }


    DLL_EXPORT bool Align(size_t elementID, double wavelength)
    {
        ClearOptiXError();
        if(wavelength <0)
        {
            SetOptiXLastError("Invalid wavelength", __FILE__, __func__);
            return false;
        }
        if(System.isValidID(elementID))
        { //  returns 0 if alignment is OK ; -1 if a grating can't be aligned and OptiXLastError is set with the grating name
//            printf("aligning from %s  at WL %g \n", ((ElementBase*)elementID)->getName().c_str(),wavelength );
            if(((ElementBase*)elementID)->alignFromHere(wavelength))
                return false;  // last error will be set by grating align
            else
                return true;
        }
        else
        {
            SetOptiXLastError("Invalid element ID", __FILE__, __func__);
            return false;
        }

    }

    DLL_EXPORT bool AlignGrating4Cff(size_t elementID, double Cff, double wavelength)
    {
        ClearOptiXError();
        if(wavelength <0)
        {
            SetOptiXLastError("Invalid wavelength", __FILE__, __func__);
            return false;
        }
        if (Cff <=0)
                    {
            SetOptiXLastError("Invalid negative or null Cff value", __FILE__, __func__);
            return false;
        }
        if(!System.isValidID(elementID))
        {
            SetOptiXLastError("Invalid element ID", __FILE__, __func__);
            return false;
        }
        GratingBase *pGrating= dynamic_cast<GratingBase*>((ElementBase*) elementID);
        if( !pGrating)
        {
            SetOptiXLastError("Element is not a grating", __FILE__, __func__);
            return false;
        }


        double lineDensity=pGrating->gratingVector(Surface::VectorType::Zero()).norm();
        cout << "Central line density =" << lineDensity << endl;
        Parameter theta;
        pGrating->getParameter("theta", theta);
        double alpha, beta, LLD=wavelength*lineDensity, C2=Cff*Cff;
        double X,DC2=1-C2;
        X=(sqrt(LLD*LLD*C2+DC2*DC2) -LLD)/DC2;
        alpha=acos(X);
       // beta=asin(Cff*sin(alpha));
        beta=acos(X+LLD);
        theta.value=(alpha+beta)/2.;
        cout << "Half deviation angle " << theta.value << endl;
        pGrating->setParameter("theta", theta);

        return true;
    }

    DLL_EXPORT bool EmulateUndulator(size_t elementID, double sigmaX, double sigmaY, double sigmaprimX, double sigmaprimY,
                                     double undulatorLength,  double SD_UndulatorDistance, double wavelength, double detuning=1.4)
    {
        ClearOptiXError();
        if(wavelength <0)
        {
            SetOptiXLastError("Invalid wavelength", __FILE__, __func__);
            return false;
        }
        if(!System.isValidID(elementID))
        {
            SetOptiXLastError("Invalid element ID", __FILE__, __func__);
            return false;
        }
        if(undulatorLength <=0 ||detuning <=0)
        {
                SetOptiXLastError("Unduator length and detuning must be strictly positive numbers", __FILE__, __func__);
                return false  ;
        }
        ElementBase * element=(ElementBase*) elementID;
        if( element->getOptixClass()!="Source<Astigmatic,Gaussian>")
        {
            if(element->getOptixClass()!="Source<Gaussian>")
            {
                SetOptiXLastError("Element is not an astigmatic gaussian source", __FILE__, __func__);
                return false  ;
            }
            else if(SD_UndulatorDistance!=0)
            {
                SetOptiXLastError("Element is a gaussian source but undulator centers are not in the same plane", __FILE__, __func__);
                return false  ;
            }
        }
        double sigmaprim2und =wavelength *detuning / (2. * undulatorLength);
        double sigma2und= wavelength * undulatorLength / detuning/ (8 *  M_PI*M_PI);
        double sigmaprim2=sigmaprimX*sigmaprimX;
        double sigmaprimX2total=sigmaprim2 +sigmaprim2und;
        double WaistX=sigmaprim2und/sigmaprimX2total*SD_UndulatorDistance;
        double sigmaX2total=sigmaX*sigmaX+sigma2und+sigmaprim2*sigmaprim2und*SD_UndulatorDistance*SD_UndulatorDistance/sigmaprimX2total;

        sigmaprim2=sigmaprimY*sigmaprimY;
        double sigmaprimY2total=sigmaprim2 +sigmaprim2und;
        double WaistY=sigmaprim2und/sigmaprimY2total*SD_UndulatorDistance;
        double sigmaY2total=sigmaY*sigmaY+sigma2und+sigmaprim2*sigmaprim2und*SD_UndulatorDistance*SD_UndulatorDistance/sigmaprimY2total;
        Parameter param;
        element->getParameter("sigmaX", param);
        param.value=sqrt(sigmaX2total);
        element->setParameter("sigmaX", param);
        element->getParameter("sigmaY", param);
        param.value=sqrt(sigmaY2total);
        element->setParameter("sigmaY", param);
        element->getParameter("sigmaXdiv", param);
        param.value=sqrt(sigmaprimX2total);
        element->setParameter("sigmaXdiv", param);
        element->getParameter("sigmaYdiv", param);
        param.value=sqrt(sigmaprimY2total);
        element->setParameter("sigmaYdiv", param);
        if( element->getOptixClass()=="Source<Astigmatic,Gaussian>")
        {
            element->getParameter("waistX", param);
            param.value=WaistX;
            element->setParameter("waistX",param);
            element->getParameter("waistY", param);
            param.value=WaistY;
            element->setParameter("waistY",param);

        }
        return true ;
    }


    DLL_EXPORT bool Generate(size_t elementID, double wavelength, int *numRays)
    {
        ClearOptiXError();
        if(numRays)
            *numRays=0;
        if(!System.isValidID(elementID))
        {
            SetOptiXLastError("Invalid element ID", __FILE__, __func__);
            return false;
        }
        if( !((ElementBase*)elementID)->isSource())
        {
            SetOptiXLastError("Element is not a source", __FILE__, __func__);
            return false;
        }
        if(wavelength <0)
        {
            SetOptiXLastError("Invalid wavelength", __FILE__, __func__);
            return false;
        }
//        printf("generating rays in %s  at WL %g \n", ((ElementBase*)elementID)->getName().c_str(),wavelength );
        if(numRays)
            *numRays=dynamic_cast<SourceBase*>((ElementBase*)elementID)->generate(wavelength );
        else
            dynamic_cast<SourceBase*>((ElementBase*)elementID)->generate(wavelength );
        return true;
    }

    DLL_EXPORT bool GeneratePol(size_t elementID, double wavelength, char polar, int * numRays)
    {
        ClearOptiXError();
        if(numRays)
            *numRays=0;
        if(!System.isValidID(elementID))
        {
            SetOptiXLastError("Invalid element ID", __FILE__, __func__);
            return false;
        }
        if( !((ElementBase*)elementID)->isSource())
        {
            SetOptiXLastError("Element is not a source", __FILE__, __func__);
            return false;
        }
        if(wavelength <0)
        {
            SetOptiXLastError("Invalid wavelength", __FILE__, __func__);
            return false;
        }

        if(polar!='S'&& polar!='P' &&  polar!='R' &&  polar!='L')
        {
            SetOptiXLastError(string("Invalid Polarization: '") +polar +"'; 'S', 'P', 'R', 'L' only are valid", __FILE__, __func__);
            return false;
        }
//             printf("generating rays in %s  at WL %g \n", ((ElementBase*)elementID)->getName().c_str(),wavelength );
        if(numRays)
           *numRays=dynamic_cast<SourceBase*>((ElementBase*)elementID)->generate(wavelength,polar );
        else
           dynamic_cast<SourceBase*>((ElementBase*)elementID)->generate(wavelength,polar );
        return true;
    }

    DLL_EXPORT bool Radiate(size_t elementID)
    {
        ClearOptiXError();
        if(!System.isValidID(elementID))
        {
            SetOptiXLastError("Invalid element ID", __FILE__, __func__, __LINE__);
            return false;
        }
        if( !((ElementBase*)elementID)->isSource())
        {
            SetOptiXLastError("Element is not a source", __FILE__, __func__, __LINE__);
            return false;
        }
        int losses;
        try {
            losses=dynamic_cast<SourceBase*>((ElementBase*)elementID)->radiate();
        }
        catch(RayException &excpt) {
            SetOptiXLastError(excpt.what()+"\nPropagation interrupted", __FILE__, __func__, __LINE__);
            return false;
        }catch (InterceptException &excpt){
            SetOptiXLastError(excpt.what()+"\nPropagation interrupted", __FILE__, __func__, __LINE__);
            return false;
        }
        if(losses==0)
            return true;
        else
        {
            char buffer[80];
            sprintf(buffer,"WARNING: %d rays lost in propagation", losses);
            SetOptiXLastError(buffer, __FILE__, __func__,__LINE__);
            return false;
        }
    }

    DLL_EXPORT bool RadiateAt(size_t elementID, double wavelength)
    {
        ClearOptiXError();
        if(!System.isValidID(elementID))
        {
            SetOptiXLastError("Invalid element ID", __FILE__, __func__, __LINE__);
            return false;
        }
        if( !((ElementBase*)elementID)->isSource())
        {
            SetOptiXLastError("Element is not a source", __FILE__, __func__, __LINE__);
            return false;
        }
        if(wavelength <0)
        {
            SetOptiXLastError("Invalid wavelength", __FILE__, __func__, __LINE__);
            return false;
        }
        dynamic_cast<SourceBase*>((ElementBase*)elementID)->setWavelength(wavelength);
        int losses;
        try {
            losses=dynamic_cast<SourceBase*>((ElementBase*)elementID)->radiate();
        }
        catch(RayException &excpt) {
            SetOptiXLastError(excpt.what()+"\nPropagation interrupted", __FILE__, __func__, __LINE__);
            return false;
        }catch (InterceptException &excpt){
            SetOptiXLastError(excpt.what()+"\nPropagation interrupted", __FILE__, __func__, __LINE__);
            return false;
        }
        if(losses==0)
            return true;
        else
        {
            char buffer[80];
            sprintf(buffer,"WARNING: %d rays lost in propagation", losses);
            SetOptiXLastError(buffer, __FILE__, __func__,__LINE__);
            return false;
        }
    }

    DLL_EXPORT bool GetSpotDiagram(size_t elementID,  C_DiagramStruct * diagram, double distance)
    {
        ClearOptiXError();
        if(diagram->m_dim <4)
        {
            SetOptiXLastError("m_dim must be at least 4 for spotdiagrams", __FILE__, __func__);
            return false;
        }

        char errstr[64];
        Diagram* spotdiag= (Diagram*)diagram;
        Surface * surf=dynamic_cast<Surface*>((ElementBase*)elementID);

        if(surf)
        {
            if(surf->getRecording()==0)
            {
                SetOptiXLastError("Element is not recording impacts", __FILE__, __func__);
                return false;
            }

            if(diagram->m_reserved < surf->sizeImpacts())
            {
                sprintf(errstr,"Diagram storage space is too small to host %d impacts", surf->sizeImpacts());
                SetOptiXLastError(errstr, __FILE__, __func__);
                return false;
            }
            surf->getSpotDiagram(*spotdiag, distance);
            spotdiag=NULL;
            return true;
        }
        SetOptiXLastError("Element cannot record impacts (not a surface) ", __FILE__, __func__);
        return false;
    }

    DLL_EXPORT bool GetImpactsData(size_t elementID,  C_DiagramStruct * diagram, enum FrameID frame)
    {
        ClearOptiXError();
        if(diagram->m_dim <= 6)
        {
            SetOptiXLastError("m_dim must be at least  6 for impact data", __FILE__, __func__);
            return false;
        }

        char errstr[64];
        Diagram* impactData= (Diagram*)diagram;
        Surface * surf=dynamic_cast<Surface*>((ElementBase*)elementID);

        if(surf)
        {
            if(surf->getRecording()==0)
            {
                SetOptiXLastError("Element is not recording impacts", __FILE__, __func__);
                return false;
            }

            if(diagram->m_reserved < surf->sizeImpacts())
            {
                sprintf(errstr,"Diagram storage space is too small to host %d impacts", surf->sizeImpacts());
                SetOptiXLastError(errstr, __FILE__, __func__);
                return false;
            }
            surf->getImpactData(*impactData, frame);
            impactData=NULL;
            return true;
        }
        SetOptiXLastError("Element cannot record impacts (not a surface) ", __FILE__, __func__, __LINE__);
        return false;
    }

    DLL_EXPORT bool GetFocalDiagram(size_t elementID, const int dims[3], const double zbound[2], int32_t *focdg,
                                    double* xbound, double * ybound)
    {
        ClearOptiXError();

        Surface * surf=dynamic_cast<Surface*>((ElementBase*)elementID);
        if(!surf)
        {
            SetOptiXLastError("Element is not a surface) ", __FILE__, __func__, __LINE__);
            return false;
        }

        if(surf->getRecording()==0)
        {
            SetOptiXLastError("Element is not recording impacts", __FILE__, __func__);
            return false;
        }
        cout << "getting focal digram \n";
        Tensor<int,3> focdiag=surf->getFocalDiagram(dims, zbound, xbound, ybound);
        cout << "copying diagram to return array\n";
        memcpy(focdg, focdiag.data(), dims[0]*dims[1]*dims[2]*sizeof(int));
        cout << "returning \n";
        return true;
    }



    DLL_EXPORT bool GetExitFrame(size_t elementID, double* frame_vectors)
    {
        ClearOptiXError();
        if(!System.isValidID(elementID))
        {
            SetOptiXLastError("Invalid element ID", __FILE__, __func__);
            return false;
        }
        Map<MatrixXd> mat(frame_vectors,3,4);
        mat=((ElementBase*)elementID)->exitFrame().matrix().topRows(3).cast<double>();
        return true;
    }


    DLL_EXPORT bool GetSurfaceFrame(size_t elementID, double* frame_vectors)
    {
        ClearOptiXError();
        if(!System.isValidID(elementID))
        {
            SetOptiXLastError("Invalid element ID", __FILE__, __func__);
            return false;
        }
        Map<MatrixXd> mat(frame_vectors,3,4);
        mat=((ElementBase*)elementID)->surfaceFrame().matrix().topRows(3).cast<double>();
        return true;
    }



    DLL_EXPORT bool ClearImpacts(size_t elementID)
    {
        ClearOptiXError();
        if(!System.isValidID(elementID))
        {
            SetOptiXLastError("Invalid element ID", __FILE__, __func__);
            return false;
        }
        Surface* psurf=dynamic_cast<Surface*>((ElementBase*)elementID);
        if(psurf)
        {
            psurf->clearImpacts();
            return true;
        }
           // else this is a group
        SetOptiXLastError("This is not a valid ElementBase object", __FILE__,__func__);
        return false;
    }


    DLL_EXPORT bool SaveSystem(const char* filename)
    {
        TextFile file(filename, ios::out);
        if(!file.is_open())
        {
            SetOptiXLastError("Can't open the file for writing",__FILE__,__func__);
            return false;
        }
        map<string,ElementBase*>::iterator it;
        for (it=System.begin(); it!=System.end(); ++it)
        {
            file << *(it->second);
            if(file.fail())
            {
                SetOptiXLastError("Text file write error",__FILE__,__func__);
                return false;
            }
        }
        file.close();
        return true;
    }

    DLL_EXPORT bool LoadSystem(const char* filename)
    {
        ClearOptiXError();
        TextFile file(filename, ios::in);
        if(!file.is_open())
        {
            SetOptiXLastError("Can't open the file for reading",__FILE__,__func__);
            return false;
        }
        string sClass, sName, sPrev,sNext, paramName;
        Parameter param;
        size_t ElementBaseID;

        System.clear(); // destroys all elements
       // ValidIDs.clear();
        while(!file.eof()) // loop of ElementBase creation
        {
            file >> sClass;
            if(sClass.empty())
                break;
            file >> sName >> sNext >> sPrev >> paramName;
            if(file.fail())
                { SetOptiXLastError("File reading error",__FILE__,__func__);  return false; }

            if(!CreateElement(sClass.c_str(), sName.c_str(), &ElementBaseID))
            {
                char errstr[128], *opterr;
                GetOptiXError(&opterr);
                sprintf(errstr, "Cannot create element %s of type %s\n", sName.c_str(), sClass.c_str());
                SetOptiXLastError(string("File reading error: ")+errstr+opterr,__FILE__,__func__, __LINE__);
                return false;
            }
            while(!paramName.empty())
            {
               file >> param;
               if(file.fail())
                    { SetOptiXLastError("File reading error",__FILE__,__func__);  return false; }
               if(!SetParameter(ElementBaseID, paramName.c_str(), param))
               {
                    char errstr[128];
                    sprintf(errstr, "Cannot create element %s of type %s", sName.c_str(), sClass.c_str());
                    SetOptiXLastError("File reading error",__FILE__,__func__, __LINE__);
                    return false;
               }
               file >> paramName;
               if(file.fail())
                    { SetOptiXLastError("File reading error",__FILE__,__func__, __LINE__ );  return false; }
            }

            file.ignore('\n');
        }               // end ElementBase  creation

        file.seekg(0);
                while(!file.eof()) // loop of ElementBase creation
        {
            file >> sClass;
            if(file.fail())
                { SetOptiXLastError("File reading error",__FILE__,__func__, __LINE__);  return false; }
            if(sClass.empty())
                break;
            file >> sName >> sNext ; //>> sPrev ;   // On peut se contenter d'appeler seulement set next qui se chargera de mettre à jour les 2 elements connectés
            if(file.fail())
                { SetOptiXLastError("File reading error",__FILE__,__func__, __LINE__);  return false; }
            if(!sNext.empty() )   // inutile d'agir si sNext empty; les nouvelles surfaces ont tous leurs liens nuls
                System.find(sName)->second->chainNext(System.find(sNext)->second); // ces deux ElementBases existent; elles viennent d'être créées.

            file.ignore('\n'); // skip prameters
        }
        return true;
    }

    DLL_EXPORT bool LoadSolemioFile(char * filename)
    {
        ClearOptiXError();
        System.clear();
        return SolemioImport(filename);

    }


    DLL_EXPORT bool DiagramToFile(const char* filename,  C_DiagramStruct* cdiagram)
    {
        ClearOptiXError();
        fstream spotfile(filename, ios::out | ios::binary);
        if(!spotfile.is_open())
        {
            SetOptiXLastError("Can't open the file for writing",__FILE__,__func__);
            return false;
        }
        spotfile << *cdiagram;
        spotfile.close();
        return true;
    }

    DLL_EXPORT bool SaveSystemAsXml(const char * filename){ return SaveElementsAsXml(filename, System);}

    DLL_EXPORT bool LoadSystemFromXml(const char * filename)
    {

        System.clear(); // destroys all elements
       // ValidIDs.clear();
        return LoadElementsFromXml(filename,System);
    }

    DLL_EXPORT bool DumpXML(const char* filename) {return DumpXmlSys(filename);}

    DLL_EXPORT bool GetHologramPatternInfo(size_t elementID, GratingPatternInfo *gratInfo, double halfLength, double halfWidth )
    {
        ClearOptiXError();
        if(!System.isValidID(elementID))
        {
            SetOptiXLastError("Invalid element ID", __FILE__, __func__);
            return false;
        }
        Holo *pHologram= dynamic_cast<Holo*>((ElementBase*) elementID);
        if( !pHologram)
        {
            SetOptiXLastError("Element is not a holographic grating", __FILE__, __func__);
            return false;
        }
        if(! gratInfo)
        {
            SetOptiXLastError("Second parameter is not a valid GratingPatternInfo pointer", __FILE__, __func__);
            return false;
        }
        pHologram->getPatternInfo(halfLength, halfWidth, gratInfo);

        return true;
    }
// ------------------------------------------------------------------------
// |           Surface error related functions                             |
// ------------------------------------------------------------------------

    DLL_EXPORT bool SetErrorGenerator(size_t elementID )
    {
        ClearOptiXError();
        if(!System.isValidID(elementID))
        {
            SetOptiXLastError("Invalid element ID", __FILE__, __func__, __LINE__);
            return false;
        }
        Surface* psurf=dynamic_cast<Surface*>((ElementBase*) elementID);
        if(!psurf)
        {
            SetOptiXLastError(string("Element ")+((ElementBase*) elementID)->getName()+" is not a Surface", __FILE__, __func__, __LINE__);
            return false;
        }

        psurf->setErrorGenerator();
        return true;
    }

    DLL_EXPORT bool UnsetErrorGenerator(size_t elementID )
    {
        ClearOptiXError();
        if(!System.isValidID(elementID))
        {
            SetOptiXLastError("Invalid element ID", __FILE__, __func__, __LINE__);
            return false;
        }
        Surface* psurf=dynamic_cast<Surface*>((ElementBase*) elementID);
        if(!psurf)
        {
            SetOptiXLastError(string("Element ")+((ElementBase*) elementID)->getName()+" is not a Surface", __FILE__, __func__, __LINE__);
            return false;
        }
        if(!psurf->hasParameter("error_limits"))
        {
            SetOptiXLastError(string("Surface ")+psurf->getName()+" doesn't have an active surface error generator",
                               __FILE__, __func__, __LINE__);
            return false;
        }
        psurf->unsetErrorGenerator();
        return true;
    }

    DLL_EXPORT bool GenerateSurfaceErrors(size_t elementID, int32_t mapDims[2],
                                          double* total_sigma, int32_t LegendreDims[2], double *normalizedLegendre)

    {
        ClearOptiXError();
        if(!System.isValidID(elementID))
        {
            SetOptiXLastError("Invalid element ID", __FILE__, __func__, __LINE__);
            return false;
        }
        Surface* psurf=dynamic_cast<Surface*>((ElementBase*) elementID);
        if(!psurf)
        {
            SetOptiXLastError(string("Element ")+((ElementBase*) elementID)->getName()+" is not a Surface", __FILE__, __func__, __LINE__);
            return false;
        }
        if(!psurf->hasParameter("error_limits"))
        {
            SetOptiXLastError(string("Surface ")+psurf->getName()+" doesn't have an active surface error generator",
                               __FILE__, __func__, __LINE__);
            return false;
        }
        MatrixXd legendreMat;
        if( ! psurf->generateSurfaceErrors(mapDims, total_sigma, legendreMat))
            return false;

        if(LegendreDims && normalizedLegendre)
        {
            int32_t matsize=legendreMat.rows()* legendreMat.cols();
            if( LegendreDims[0]*LegendreDims[1] < matsize)
            {
                SetOptiXLastError(string("The allocated size of normalizedLegendre array is too small for the actual ")+
                                  std::to_string(legendreMat.rows())+"*"+std::to_string(legendreMat.cols())+" size, in element "+
                                  ((ElementBase*) elementID)->getName(), __FILE__, __func__, __LINE__);
                return false;
            }
            LegendreDims[0]=legendreMat.rows();
            LegendreDims[1]=legendreMat.cols();
            memcpy(normalizedLegendre, legendreMat.data(), matsize*sizeof(double));
        }
        return true;
    }

    DLL_EXPORT bool SetSurfaceErrors(size_t elementID,double xmin, double xmax, double ymin, double ymax,
                                     int32_t xsize, int32_t ysize, const double* heightErrors)
    {
        ClearOptiXError();
        if(!System.isValidID(elementID))
        {
            SetOptiXLastError("Invalid element ID", __FILE__, __func__, __LINE__);
            return false;
        }
        Surface* psurf=dynamic_cast<Surface*>((ElementBase*) elementID);
        if(!psurf)
        {
            SetOptiXLastError(string("Element ")+((ElementBase*) elementID)->getName()+" is not a Surface", __FILE__, __func__, __LINE__);
            return false;
        }
        const Map<ArrayXXd> errmap((double*)heightErrors, xsize, ysize);
        psurf->setSurfaceErrors(xmin, xmax, ymin, ymax, errmap);
        return true;
    }


     DLL_EXPORT bool UnsetSurfaceErrors(size_t elementID )
     {
        ClearOptiXError();
        if(!System.isValidID(elementID))
        {
            SetOptiXLastError("Invalid element ID", __FILE__, __func__, __LINE__);
            return false;
        }
        Surface* psurf=dynamic_cast<Surface*>((ElementBase*) elementID);
        if(!psurf)
        {
            SetOptiXLastError(string("Element ")+((ElementBase*) elementID)->getName()+" is not a Surface", __FILE__, __func__, __LINE__);
            return false;
        }
        psurf->unsetSurfaceErrors();
        return true;
     }

    DLL_EXPORT bool GetSurfaceErrors(size_t elementID, int* dims, double* height_errors )
    {
        ClearOptiXError();
        if(!System.isValidID(elementID))
        {
            SetOptiXLastError("Invalid element ID", __FILE__, __func__, __LINE__);
            return false;
        }
        Surface* psurf=dynamic_cast<Surface*>((ElementBase*) elementID);
        if(!psurf)
        {
            SetOptiXLastError(string("Element ")+((ElementBase*) elementID)->getName()+" is not a Surface", __FILE__, __func__, __LINE__);
            return false;
        }
        if(!dims || !height_errors)
        {
            SetOptiXLastError(string("Element ")+((ElementBase*) elementID)->getName()+" invalid dims or height_errors pointer", __FILE__, __func__, __LINE__);
            return false;
        }
        int heightsize=dims[0]*dims[1];
        ArrayXXd errmap=psurf->getSurfaceErrors() ;
        int mapsize=errmap.rows()*errmap.cols();
        if(!mapsize)
        {
            SetOptiXLastError(string("Thereis no error map associated to Surface ")+
                            ((ElementBase*) elementID)->getName(), __FILE__, __func__, __LINE__);
            return false;
        }
        if(heightsize < mapsize)
        {
            SetOptiXLastError(string("The allocated size of height_errors array is too small for the actual ")+
                            std::to_string(errmap.rows())+"*"+std::to_string(errmap.cols())+" size, in element "+
                            ((ElementBase*) elementID)->getName(), __FILE__, __func__, __LINE__);
            return false;
        }
        dims[0]=errmap.rows();
        dims[1]=errmap.cols();
        memcpy(height_errors, errmap.data(), mapsize*sizeof(double)) ;
        return true;
    }

    DLL_EXPORT bool SetErrorMethod(size_t elementID, ErrorMethod meth)
    {
        ClearOptiXError();
        if(!System.isValidID(elementID))
        {
            SetOptiXLastError("Invalid element ID", __FILE__, __func__, __LINE__);
            return false;
        }
        Surface* psurf=dynamic_cast<Surface*>((ElementBase*) elementID);
        if(!psurf)
        {
            SetOptiXLastError(string("Element ")+((ElementBase*) elementID)->getName()+" is not a Surface", __FILE__, __func__, __LINE__);
            return false;
        }
        psurf->setErrorMethod(meth);
        return true;
    }

    DLL_EXPORT bool GetErrorMethod(size_t elementID, ErrorMethod *meth)
    {
        ClearOptiXError();
        if(!System.isValidID(elementID))
        {
            SetOptiXLastError("Invalid element ID", __FILE__, __func__, __LINE__);
            return false;
        }
        Surface* psurf=dynamic_cast<Surface*>((ElementBase*) elementID);
        if(!psurf)
        {
            SetOptiXLastError(string("Element ")+((ElementBase*) elementID)->getName()+" is not a Surface", __FILE__, __func__, __LINE__);
            return false;
        }
        *meth=psurf->getErrorMethod();
        return true;
    }

    DLL_EXPORT bool SurfaceErrorsEnable(const bool activity)
    {
        enableSurfaceErrors=activity;
        return true;
    }


    DLL_EXPORT bool SurfaceErrorsGetState(bool *activityFlag)
    {
        ClearOptiXError();
        if(!activityFlag)
        {
            SetOptiXLastError("Invalid reference to activityFlag", __FILE__, __func__);
            return false;
        }
        *activityFlag= enableSurfaceErrors;
        return true;
    }

// -----------------------------------------------------------------
// |           Aperture and wavefront related functions            |
// -----------------------------------------------------------------

    DLL_EXPORT void SetAperturesActive(const bool activity){enableApertureLimit=activity;}

    DLL_EXPORT bool GetAperturesActive(bool *activityFlag)
    {
        if(!activityFlag)
        {
            SetOptiXLastError("Invalid reference to activityFlag", __FILE__, __func__);
            return false;
        }
        *activityFlag= enableApertureLimit;
        return true;
    }

    DLL_EXPORT bool GetSource(size_t elementID, size_t* sourceID)
    {
        ClearOptiXError();
        if(!System.isValidID(elementID))
        {
            SetOptiXLastError("Invalid element ID", __FILE__, __func__);
            return false;
        }
        if(!sourceID)
        {
            SetOptiXLastError("Invalid reference to sourceID", __FILE__, __func__);
            return false;
        }
        *sourceID=(size_t)((ElementBase*)elementID)->getSource();
        return true;

    }

    DLL_EXPORT bool WaveRadiate(size_t elementID, WFemission WFemitParams)
    {
        ClearOptiXError();
        if(!System.isValidID(elementID))
        {
            SetOptiXLastError("Invalid element ID", __FILE__, __func__);
            return false;
        }
        SourceBase* source;
        if(((ElementBase*)elementID)->isSource())
            source=dynamic_cast<SourceBase*>((ElementBase*)elementID);
        else
            source=dynamic_cast<SourceBase*>(((ElementBase*)elementID)->getSource());
        if(!source)
        {
            SetOptiXLastError("No source found upstream the given element", __FILE__, __func__);
            return false;
        }
        source->alignFromHere(WFemitParams.wavelength);
        source->waveRadiate(WFemitParams.wavelength, WFemitParams.Xaperture, WFemitParams.Yaperture,
                            WFemitParams.Xsize, WFemitParams.Xsize, WFemitParams.polar);

        return true;
    }


    DLL_EXPORT bool GetPsf(size_t elementID, double wavelength, PSFparameters *psfParams, C_ndArray * psfData)
    {
        ClearOptiXError();
        if(!System.isValidID(elementID))
        {
            SetOptiXLastError("Invalid element ID", __FILE__, __func__);
            return false;
        }
        Surface* surface= dynamic_cast<Surface*>((ElementBase*)elementID);

        size_t psfStorage=psfParams->xSamples*psfParams->ySamples*psfParams->numOffsetPlanes*4*sizeof(double)+4*sizeof(size_t);

        if(psfData->allocatedStorage < psfStorage)
        {
            SetOptiXLastError("Allocated storage for psfData is to small for the size requested by parameters ", __FILE__, __func__);
            return false;
        }
        psfData->ndims=4;
        size_t *dims=(size_t*)psfData->storage;
        dims[0]=psfParams->xSamples;
        dims[1]=psfParams->ySamples;
        dims[2]=psfParams->numOffsetPlanes;
        dims[3]=2;


        ndArray<complex<double>, 4> PSF(*psfData);
        Array2d pixelSizes;
        pixelSizes << psfParams->psfXpixel, psfParams->psfYpixel;
        ArrayXd distoffsets=ArrayXd::LinSpaced(psfParams->numOffsetPlanes, psfParams->zFirstOffset, psfParams->zLastOffset);

        if(! surface->isOPDvalid())
        {
            try {
                surface->computeOPD(psfParams->opdRefDistance, psfParams->legendreNx, psfParams->legendreNy);
            }
            catch (runtime_error &rte)
            {
                SetOptiXLastError("The number of recorded impacts is too small to get the requested OPD Legendre development", __FILE__, __func__);
                return false;
            }
        }

        surface->computePSF(PSF, pixelSizes, distoffsets, wavelength, psfParams->oversampling );

        psfParams->psfXpixel=pixelSizes(0);
        psfParams->psfYpixel=pixelSizes(1);
        return true;
    }

    DLL_EXPORT bool LoadConfigurationFile(const char* filename)
    {
        System.clear();
        return LoadConfiguration(filename);

    }

#ifdef HAS_REFLEX

    DLL_EXPORT bool SetCoating(size_t elementID,const char* coatingTable, const char* coatingName)
    {
        ClearOptiXError();
        if(!System.isValidID(elementID))
        {
            SetOptiXLastError("invalid element ID", __FILE__, __func__);
            return false;
        }
        Surface * psurf=dynamic_cast<Surface*>((ElementBase*)elementID);
        if(! psurf )
        {
            SetOptiXLastError("element is not an OptiX Surface", __FILE__, __func__);
            return false;
        }
        if(psurf->isSource())
        {
            SetOptiXLastError("Cannot define a coating on a source", __FILE__, __func__);
            return false;
        }

        return psurf->setCoating(coatingTable, coatingName);
    }
#endif // HAS_REFLEX

#ifdef __cplusplus
} // extern C
#endif
