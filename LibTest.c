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


#include "ctypes.h"
#include <stdio.h>
#include "interface.h"


#define ELEM_NAMELEN  32
#define PARAM_NAMELEN 48
#define ERROR_BUFLEN 256

int main()
{

    size_t hSys=0, hParm=0, elemID=0; // Handles used to access internal objects
    char elname[ELEM_NAMELEN], elname2[ELEM_NAMELEN],parmname[PARAM_NAMELEN], errBuf[ERROR_BUFLEN];  // Sting variables
    struct Parameter param; // structure holding the definition of a parameter

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
        EnumerateElements(&hSys,&elemID, elname,32); // Get the next element in the system. To initializ the enumeration function is called with null sys and elem  handles
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
        GetElementName(elemID, elname,32);  // obtains the name from the current ID
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
    param.value=5000;               // modify value
    SetParameter(sourceID,"nRays",param); // set the parameter

//  Make sure the object EXP1 is recording impacts
    size_t targetID=GetElementID("EXP1");

    if(Align(sourceID,2.5e-8)) // aligne le système à partir de la source pour la longueur d'on 25 nm (lambda utilisé seulement par les réseaux)
    {
       GetOptiXLastError(errBuf,256);
        printf("Alignment error: %s\n",errBuf);
        return -1;
    }
    if(!Generate(sourceID, 2.5e-8))  // compute a set of rays at wavelength 25 nm, in the source space
    {
       GetOptiXLastError(errBuf,256);
        printf("Source generation error: %s\n",errBuf);
        return -1;
    }



    return 0;
}
