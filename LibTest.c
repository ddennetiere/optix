/**
 *************************************************************************
*   \file           LibTest.c
*
*   \brief             Test of library functions
*
*
*
*   \author             François Polack  <francois.polack@synchroton-soleil.fr>
*   \date               Creation: 2021-04-30
*   \date               Last update: 2021-04-30
 ***************************************************************************/

#include <stdio.h>
#include <time.h>
#include <stdlib.h>


#include "interface.h"
#include <math.h>


#define ELEM_NAMELEN  32
#define PARAM_NAMELEN 48
#define ERROR_BUFLEN 256

#define  LIMITED 0
int CassioTest();
int EllipticKB();


int main()
{
   // return CassioTest();
   return EllipticKB();
}

int CassioTest()
{

    printf( "Sizeof void* %d ; sizeof size_t %d \n", sizeof(void*), sizeof(size_t));

    size_t hSys=0, hParm=0, elemID=0; // Handles used to access internal objects
    char elname[ELEM_NAMELEN], elname2[ELEM_NAMELEN],classname[ELEM_NAMELEN],parmname[PARAM_NAMELEN], errBuf[ERROR_BUFLEN];  // Sting variables
    Parameter param; // structure holding the definition of a parameter
    int numrays=5000;  // number of random rays to issued by the source
    char * title[5]={"X   ", "Y   ", "X'  ", "Y'  ", "lmda" };

/* **********************************************************************************
*   Load a new system from a Solemio file
*   This function lists the properties of the loaded elements to the console as defined inside SOLEMIO
*   It is a bit verbose since Solemio parameter have some redundancy.
*/
    if(!LoadSolemioFile("D:\\projets\\projetsCB\\OptiX\\solemio\\CASSIOPEE"))
    {
        GetOptiXLastError( errBuf,ERROR_BUFLEN); // if an error occurs OptiXLast error will be set
        printf("ERROR: %s\n",errBuf);
        return -1;
    }
    printf("\n\n");

    size_t sourceID;



/* *********************************************************************************
*    Enumerates the elements of the loaded system and list their OptiX properties
*    A system is a collection of optical elements
*    Active elements can linked to one another to form double linked chains
*    An active chain must start with a source element
*/
    if(!LIMITED)
    {

        do
        {
            EnumerateElements(&hSys,&elemID, elname,ELEM_NAMELEN); // Get the next element in the system. To initializ the enumeration function is called with null sys and elem  handles
            GetElementName(elemID, elname2,ELEM_NAMELEN); // another means to retrieve the element name from an elem ID

            printf("%s  (%s)\n", elname, elname2);
            hParm=0;
            do
            {
                if(!EnumerateParameters(elemID, &hParm, parmname, PARAM_NAMELEN, &param)) // get the parameter list for element elemID. Enumeration is initialized with hParm = NULL
                {
                    GetOptiXLastError( errBuf,ERROR_BUFLEN); //  message if error
                    printf("ERROR: %s\n",errBuf);
                    ReleaseElementEnumHandle(hParm);     // Release the handle since the enumeration must be terminated early
                    break;
                }
                printf("%s  %g [%g, %g] x %g T:%d G%d F:%X\n", parmname, param.value, param.bounds[0], param.bounds[1],
                       param.multiplier , param.type, param.group, param.flags);

            }while(hParm);  // Terminating the enumeration with a non null handle value will result in memory leaks, unless ReleaseElementEnumHandle is call on the handle

        }while(hSys);  // Release the handle if the enumeration must be terminated early
        printf("\n\n");
    if(0)
        return 0;

    /* ******************************************************************************
    *        Displays the active chain from source "S_ONDUL1"
    */
        sourceID=elemID=GetElementID("S_ONDUL1");  // Obtains the element handle

        // iterate on elements of the chain and displays their names and id
        while(elemID) // calling GetNextElement on the last element of the chain will bring-up NULL
        {
            GetElementName(elemID, elname,ELEM_NAMELEN);  // obtains the name from the current ID
            GetParameter(elemID,"distance", &param);
            printf("  %s   %llX  D=%g\n",elname, elemID,param.value );
            elemID =GetNextElement(elemID);  // get next element ID (element ID should of course never be released
        }
        printf("\n\n");

    /* ************************************************************************
    *       Here we will initiate some ray tracing from S_ONDUL1 and get the spot diagram generated on EXP1
    *
    *       First set or update the required element parameters
    */

        GetParameter(sourceID,"nRays", &param); // initialize the parameter struct to be properly configured for the requested property
        printf ("ORIGINAL nRays VALUE %f \n\n" , param.value );
        param.value=numrays;               // modify value
        SetParameter(sourceID,"nRays",param); // set the parameter

        size_t pupilleID=GetElementID("pupille");
        GetParameter(pupilleID,"distance",&param);
//        param.value-=.9;
        printf ("source-pupil distance %f \n\n" , param.value );
//        SetParameter(sourceID,"distance",param);


    //  Make sure the object EXP1 is recording impacts
        size_t targetID=GetElementID("EXP1");
        SetRecording(targetID, RecordOutput); // possible values are RecordNone, RecordInput, and RecordOutput;
                                              //  For films there is no difference between the two recording modes
    }
    else
    {
        sourceID=elemID=GetElementID("S_ONDUL1");  // Obtains the element handle

    }
    if(!Align(sourceID,2.5e-8)) // aligne le système à partir de la source pour la longueur d'on 25 nm (lambda utilisé seulement par les réseaux)
    {
       GetOptiXLastError(errBuf,ERROR_BUFLEN);
        printf("Alignment error: %s\n",errBuf);
        return -1;
    }

    ClearImpacts(sourceID); //  Clears stored impacts in the source ans subsequent elements.
                            //  If not called, and elements are not clean, impacts will just add-up

    if(!Generate(sourceID, 2.5e-8))  // compute a set of rays at wavelength 25 nm, in the source space. These rays are stored in the impact vector.
    {
        GetOptiXLastError(errBuf,ERROR_BUFLEN);
        printf("Source generation error: %s\n",errBuf);
        return -1;
    }

    clock_t start=clock();

    if(!Radiate(sourceID))  // This call performs the main computation. It propagate all the rays defined in the source to the end of the chain.
                            // Impacts are stores
    {
        GetOptiXLastError(errBuf,ERROR_BUFLEN);
        printf("Radiation error: %s\n",errBuf);
        return -1;
    }
    printf("propagation computation time : %f ms\n", 1000.*(clock()-start)/ CLOCKS_PER_SEC);

    if(!LIMITED)
    {

        {
            C_DiagramStruct cdiagram={5,numrays,0,0}; // defines and initialize a new C_DiagramStruct
                            // in order to record spot diagram the m_dim must be set to 5 and m_reserved should be at least the number of rays going through

            cdiagram.m_min=malloc(cdiagram.m_dim*sizeof(double)); // Use m_dim and m_reserve to be sure initialization is consistent
            cdiagram.m_max=malloc(cdiagram.m_dim*sizeof(double));
            cdiagram.m_mean=malloc(cdiagram.m_dim*sizeof(double));
            cdiagram.m_sigma=malloc(cdiagram.m_dim*sizeof(double));
            cdiagram.m_spots= malloc(cdiagram.m_dim*cdiagram.m_reserved*sizeof(double));

            if(!GetSpotDiagram(GetElementID("f"), &cdiagram, .510))
            {
                GetOptiXLastError(errBuf, ERROR_BUFLEN);
                printf("GetSpotDiagram failed: %s\n",errBuf);
            }
            else
            {
                if(cdiagram.m_count)
                {
                    DiagramToFile("fSpotDiag.sdg", &cdiagram);
                    printf("Spot-diagram with %d impacts dumped to file\n", cdiagram.m_count);
                    printf("        min         max        mean        sigma\n");
                    for (int i=0; i<5 ; ++i)
                        printf("%10.3e  %10.3e  %10.3e  %10.3e\n", cdiagram.m_min[i], cdiagram.m_max[i], cdiagram.m_mean[i], cdiagram.m_sigma[i] );
                }
                else
                    printf("Spot-diagram contains no impact\n");

            }

            if(!GetSpotDiagram(GetElementID("planfocH"), &cdiagram, -.0))
            {
                GetOptiXLastError(errBuf, ERROR_BUFLEN);
                printf("GetSpotDiagram failed: %s\n",errBuf);
            }
            else
            {
                if(cdiagram.m_count)
                {
                    DiagramToFile("focH_SpotDiag.sdg", &cdiagram);
                    printf("Spot-diagram H with %d impacts dumped to file\n", cdiagram.m_count);
                    printf("        min         max        mean        sigma\n");
                    for (int i=0; i<5 ; ++i)
                        printf("%s %10.3e  %10.3e  %10.3e  %10.3e\n",title[i], cdiagram.m_min[i], cdiagram.m_max[i], cdiagram.m_mean[i], cdiagram.m_sigma[i] );
                }
                else
                    printf("Spot-diagram contains no impact\n");

            }
            free(cdiagram.m_min);   // Clean the structure before deletion since it belongs to the caller
            free(cdiagram.m_max);
            free(cdiagram.m_mean);
            free(cdiagram.m_sigma);
            free(cdiagram.m_spots);
        }

        SaveSystem("Cassioptix.dat");  // Save the  system in the compact text forma

        SaveSystemAsXml("Cassiosys.xml");
      //  DumpXML("Cassiosys.xml");

        if(!LoadSystemFromXml("Cassiosys.xml"))
        {
            GetOptiXLastError(errBuf,ERROR_BUFLEN);
            printf("Source generation error: %s\n",errBuf);
            return -1;
        }

        elemID= CreateElement("PlaneFilm", "screen" );
        if(!elemID)
        {
            printf("\nfailed to create element \"screen\" \n");
            return -1;
        }
        GetElementName(elemID, elname,ELEM_NAMELEN);
        printf("\n new element %llX created as %s\n", elemID, elname);


        printf("Dump of the new system\n");
        hSys=0;
        do
        {
            EnumerateElements(&hSys,&elemID, elname,ELEM_NAMELEN); // Get the next element in the system. To initializ the enumeration function is called with null sys and elem  handles
            GetElementName(elemID, elname2,ELEM_NAMELEN); // another means to retrieve the element name from an elem ID
            GetElementType(elemID,classname,ELEM_NAMELEN);

            printf("#%llX: %s [%s] (%s)\n", elemID, elname, classname, elname2);

        }while(hSys);  // Release the handle if the enumeration must be terminated early
        printf("\n\n");


        elemID= CreateElement("PlaneFilm", "screen2" );
        if(!elemID)
        {
            printf("\nfailed to create element \"screen2\" \n");
            return -1;
        }
        GetElementName(elemID, elname,ELEM_NAMELEN);
        printf("\n new element %llX created as %s\n", elemID, elname);


        printf("Dump of the new system\n");
        hSys=0;
        do
        {
            EnumerateElements(&hSys,&elemID, elname,ELEM_NAMELEN); // Get the next element in the system. To initializ the enumeration function is called with null sys and elem  handles
            GetElementName(elemID, elname2,ELEM_NAMELEN); // another means to retrieve the element name from an elem ID
            GetElementType(elemID,classname,ELEM_NAMELEN);

            printf("#%llX: %s [%s] (%s)\n", elemID, elname, classname, elname2);

        }while(hSys);  // Release the handle if the enumeration must be terminated early
        printf("\n\n");
        return 0;
    }
    Version();
}

 void SetParamValue(size_t ID,char* parmName, double value)
{
    Parameter parm;
    if(!GetParameter(ID, parmName,&parm) )
    {
        printf("invalid object ID %d or invalid parameter %s\n", ID, parmName);
        exit -1;
    }
    parm.value=value;
    SetParameter(ID,parmName,parm);
}

int EllipticKB()
{
    Version();
    char errBuf[256];

    size_t sourceID, hfmID, vfmID, screenID;
    sourceID=CreateElement("Source<Gaussian>", "source");
    hfmID=CreateElement("Mirror<ConicBaseCylinder>", "hfm");
    vfmID=CreateElement("Mirror<ConicBaseCylinder>", "vfm");
    screenID=CreateElement("Film<Plane>", "screen");
    ChainElement_byID(sourceID, hfmID);
    ChainElement_byID(hfmID,vfmID);
    ChainElement_byID(vfmID,screenID);

    double numrays=50000.;
    SetParamValue(sourceID,"nRays", numrays);
    SetParamValue(sourceID,"sigmaX", 5.e-6);
    SetParamValue(sourceID, "sigmaY", 5.e-6);
    SetParamValue(sourceID, "sigmaXdiv", 1.e-3);
    SetParamValue(sourceID, "sigmaYdiv", 1.e-3);

    SetParamValue(hfmID, "distance", 5.);
    SetParamValue(hfmID, "invp", -1./5.);
    SetParamValue(hfmID, "invq",  1./2.3);
    SetParamValue(hfmID, "theta0", 1.75*M_PI/180.);
    SetParamValue(hfmID, "theta", 1.75*M_PI/180.);
    SetParamValue(hfmID, "phi", - M_PI/2.);

    SetParamValue(vfmID, "distance", .5);
    SetParamValue(vfmID, "invp", -1./5.5);
    SetParamValue(vfmID, "invq",  1./1.8);
    SetParamValue(vfmID, "theta0", 1.75*M_PI/180.);
    SetParamValue(vfmID, "theta", 1.75*M_PI/180.);
    SetParamValue(vfmID, "phi", - M_PI/2.);

    SetParamValue(screenID, "distance", 1.8);

    double lambda=6.e-9;
    if(!Align(sourceID, lambda))
    {
       GetOptiXLastError(errBuf,256);
       printf( "Alignment error : %s \n" ,errBuf );
       return -1;
    }

    if(!Generate(sourceID, lambda))
    {
       GetOptiXLastError(errBuf,256);
       printf("Source generation error :%s \n" ,errBuf );
       return -1;
    }


    if(!Radiate(sourceID))
    {
       GetOptiXLastError(errBuf,256);
       printf("Radiation error :%s \n" ,errBuf );
       return -1;
    }


    C_DiagramStruct cdiagram={5,numrays,0,0}; // defines and initialize a new C_DiagramStruct
                    // in order to record spot diagram the m_dim must be set to 5 and m_reserved should be at least the number of rays going through

    cdiagram.m_min=malloc(cdiagram.m_dim*sizeof(double)); // Use m_dim and m_reserve to be sure initialization is consistent
    cdiagram.m_max=malloc(cdiagram.m_dim*sizeof(double));
    cdiagram.m_mean=malloc(cdiagram.m_dim*sizeof(double));
    cdiagram.m_sigma=malloc(cdiagram.m_dim*sizeof(double));
    cdiagram.m_spots= malloc(cdiagram.m_dim*cdiagram.m_reserved*sizeof(double));

    if(!GetSpotDiagram(screenID, &cdiagram, 0))
    {
        GetOptiXLastError(errBuf, ERROR_BUFLEN);
        printf("GetSpotDiagram failed: %s\n",errBuf);
    }
    else
    {
        if(cdiagram.m_count)
        {
            DiagramToFile("EllipticKB.sdg", &cdiagram);
            printf("Spot-diagram with %d impacts dumped to file\n", cdiagram.m_count);
            printf("     min         max        mean        sigma\n");
            for (int i=0; i<5 ; ++i)
                printf("%10.3e  %10.3e  %10.3e  %10.3e\n", cdiagram.m_min[i], cdiagram.m_max[i], cdiagram.m_mean[i], cdiagram.m_sigma[i] );
        }
        else
            printf("Spot-diagram contains no impact\n");

    }





    return 0;
}
