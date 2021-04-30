/**
 *************************************************************************
*   \file           LibTest.c
*
*   \brief             Test of library functions
*
*
*
*   \author             Fran�ois Polack  <francois.polack@synchroton-soleil.fr>
*   \date               Creation: 2021-04-30
*   \date               Last update: 2021-04-30
 ***************************************************************************/

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "ctypes.h"
#include "interface.h"


#define ELEM_NAMELEN  32
#define PARAM_NAMELEN 48
#define ERROR_BUFLEN 256


int main()
{

    size_t hSys=0, hParm=0, elemID=0; // Handles used to access internal objects
    char elname[ELEM_NAMELEN], elname2[ELEM_NAMELEN],parmname[PARAM_NAMELEN], errBuf[ERROR_BUFLEN];  // Sting variables
    struct Parameter param; // structure holding the definition of a parameter
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

/* *********************************************************************************
*    Enumerates the elements of the loaded system and list their OptiX properties
*    A system is a collection of optical elements
*    Active elements can linked to one another to form double linked chains
*    An active chain must start with a source element
*/
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
            printf("%s  %f [%f, %f] x %f T:%d G%d F:%X\n", parmname, param.value, param.bounds[0], param.bounds[1],
                   param.multiplier , param.type, param.group, param.flags);

        }while(hParm);  // Terminating the enumeration with a non null handle value will result in memory leaks, unless ReleaseElementEnumHandle is call on the handle

    }while(hSys);  // Release the handle if the enumeration must be terminated early
    printf("\n\n");

/* ******************************************************************************
*        Displays the active chain from source "S_ONDUL1"
*/
    size_t sourceID=elemID=GetElementID("S_ONDUL1");  // Obtains the element handle

    // iterate on elements of the chain and displays their names and id
    while(elemID) // calling GetNextElement on the last element of the chain will bring-up NULL
    {
        GetElementName(elemID, elname,ELEM_NAMELEN);  // obtains the name from the current ID
        printf("  %s   %X\n",elname, elemID );
        elemID =GetNextElement(elemID);  // get next element ID (element ID should of course never be released
    }
    printf("\n\n");

/* ************************************************************************
*       Here we will initiate some ray tracing from S_ONDUL1 and get the spot diagram generated on EXP1
*
*       First set or update the required element parameters
*/

    GetParameter(sourceID,"nRays", &param); // initialize the parameter struct to be properly configured for the requested property
    param.value=numrays;               // modify value
    SetParameter(sourceID,"nRays",param); // set the parameter

//  Make sure the object EXP1 is recording impacts
    size_t targetID=GetElementID("EXP1");
    SetRecording(targetID, RecordOutput); // possible values are RecordNone, RecordInput, and RecordOutput;
                                          //  For films there is no difference between the two recording modes


    if(Align(sourceID,2.5e-8)) // aligne le syst�me � partir de la source pour la longueur d'on 25 nm (lambda utilis� seulement par les r�seaux)
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

    {
        struct C_DiagramStruct cdiagram={5,numrays,0,0}; // defines and initialize a new C_DiagramStruct
                        // in order to record spot diagram the m_dim must be set to 5 and m_reserved should be at least the number of rays going through

        cdiagram.m_min=malloc(cdiagram.m_dim*sizeof(double)); // Use m_dim and m_reserve to be sure initialization is consistent
        cdiagram.m_max=malloc(cdiagram.m_dim*sizeof(double));
        cdiagram.m_mean=malloc(cdiagram.m_dim*sizeof(double));
        cdiagram.m_sigma=malloc(cdiagram.m_dim*sizeof(double));
        cdiagram.m_spots= malloc(cdiagram.m_dim*cdiagram.m_reserved*sizeof(double));

        if(!GetSpotDiagram(GetElementID("EXP1"), &cdiagram, 0))
        {
            GetOptiXLastError(errBuf, ERROR_BUFLEN);
            printf("GetSpotDiagram failed: %s\n",errBuf);
        }
        else
        {
            if(cdiagram.m_count)
            {
                DiagramToFile("cSpotDiag.sdg", &cdiagram);
                printf("Spot-diagram with %d impacts dumped to file\n", cdiagram.m_count);
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


    return 0;
}