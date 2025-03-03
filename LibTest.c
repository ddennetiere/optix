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
#include <string.h>

#include "interface.h"
#define _USE_MATH_DEFINES
#include <math.h>


#define ELEM_NAMELEN  32
#define PARAM_NAMELEN 48


#define  LIMITED 0

// this macro is used to keep track of element creation error in SolemioFile::get_element() function
#define Create_Element(Class, Name , newID) if(!CreateElement(Class, Name, newID )) \
    {   char * errstr;\
        GetOptiXError(&errstr);\
        printf("Failed to create element %s  %s  Reason:\n%s File %s, %s line %d\n",Class,Name,errstr , __FILE__, __func__, __LINE__);\
        return -1; }
#
int CassioTest();
int EllipticKB();
int KBtest();


int main()
{
   // return CassioTest();
   //return EllipticKB();
//   LoadConfigurationFile("../../Config.dat");
    return KBtest();
}

int CassioTest()
{

    printf( "Sizeof void* %d ; sizeof size_t %d \n", sizeof(void*), sizeof(size_t));

    size_t hSys=0, hParm=0, elemID=0; // Handles used to access internal objects
    char elname[ELEM_NAMELEN], elname2[ELEM_NAMELEN],classname[ELEM_NAMELEN],parmname[PARAM_NAMELEN],*errstr;  // String variables
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
        GetOptiXError( &errstr); // if an error occurs OptiXLast error will be set
        printf("ERROR: %s\n",errstr);
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
                    GetOptiXError( &errstr); //  message if error
                    printf("ERROR: %s\n",errstr);
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
        if(!FindElementID("S_ONDUL1",&elemID))
        {
            printf("element 'S_ONDUL1' was not found\n");
            return -1;
        }
        sourceID=elemID;  // Obtains the element handle

        // iterate on elements of the chain and displays their names and id
        while(elemID) // calling GetNextElement on the last element of the chain will bring-up NULL
        {
            GetElementName(elemID, elname,ELEM_NAMELEN);  // obtains the name from the current ID
            GetParameter(elemID,"distance", &param);
            printf("  %s   %llX  D=%g\n",elname, elemID,param.value );
           GetNextElement(elemID,&elemID);  // get next element ID (element ID should of course never be released
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

        if(!FindElementID("pupille",&elemID))
        {
            printf("element 'pupille' was not found\n");
            return -1;
        }
        size_t pupilleID=elemID;
        GetParameter(pupilleID,"distance",&param);
//        param.value-=.9;
        printf ("source-pupil distance %f \n\n" , param.value );
//        SetParameter(sourceID,"distance",param);


    //  Make sure the object EXP1 is recording impacts

        if(!FindElementID("EXP1",&elemID))
        {
            printf("element 'EXP1' was not found\n");
            return -1;
        }
        size_t targetID=elemID;
        SetRecording(targetID, RecordOutput); // possible values are RecordNone, RecordInput, and RecordOutput;
                                              //  For films there is no difference between the two recording modes
    }
    else
    {
        if(!FindElementID("S_ONDUL1",&elemID))
        {
            printf("element 'S_ONDUL1' was not found\n");
            return -1;
        }
        sourceID=elemID;  // Obtains the element handle

    }
    if(!Align(sourceID,2.5e-8)) // aligne le système à partir de la source pour la longueur d'on 25 nm (lambda utilisé seulement par les réseaux)
    {
        GetOptiXError( &errstr);
        printf("Alignment error: %s\n",errstr);
        return -1;
    }

    ClearImpacts(sourceID); //  Clears stored impacts in the source ans subsequent elements.
                            //  If not called, and elements are not clean, impacts will just add-up

    if(!Generate(sourceID, 2.5e-8, NULL))  // compute a set of rays at wavelength 25 nm, in the source space. These rays are stored in the impact vector.
    {
        GetOptiXError(&errstr);
        printf("Source generation error: %s\n",errstr);
        return -1;
    }

    clock_t start=clock();

    if(!Radiate(sourceID))  // This call performs the main computation. It propagate all the rays defined in the source to the end of the chain.
                            // Impacts are stores
    {
        GetOptiXError( &errstr);
        printf("Radiation error: %s\n",errstr);
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

            if(!FindElementID("f",&elemID))
            {
                printf("element 'f' was not found\n");
                return -1;
            }
            if(!GetSpotDiagram(elemID, &cdiagram, .510))
            {
                GetOptiXError( &errstr);
                printf("GetSpotDiagram failed: %s\n",errstr);
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

            if(!FindElementID("planfocH",&elemID))
            {
                printf("element 'planfocH' was not found\n");
                return -1;
            }
            if(!GetSpotDiagram(elemID, &cdiagram, -.0))
            {
                GetOptiXError( &errstr);
                printf("GetSpotDiagram failed: %s\n",errstr);
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
            GetOptiXError( &errstr);
            printf("Source generation error: %s\n",errstr);
            return -1;
        }

        Create_Element("PlaneFilm", "screen", &elemID )

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


        Create_Element("PlaneFilm", "screen2", &elemID )

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
    Version(NULL);
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
    Version(NULL);
    char *errstr;

    size_t sourceID, hfmID, vfmID, screenID;
    Create_Element("Source<Gaussian>", "source", &sourceID);
    Create_Element("Mirror<ConicBaseCylinder>", "hfm", &hfmID);
    Create_Element("Mirror<ConicBaseCylinder>", "vfm", &vfmID);
    Create_Element("Film<Plane>", "screen", &screenID);
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
       GetOptiXError( &errstr);
       printf( "Alignment error : %s \n" ,errstr );
       return -1;
    }

    if(!Generate(sourceID, lambda, NULL))
    {
       GetOptiXError( &errstr);
       printf("Source generation error :%s \n" ,errstr );
       return -1;
    }


    if(!Radiate(sourceID))
    {
       GetOptiXError( &errstr);
       printf("Radiation error :%s \n" ,errstr );
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
        GetOptiXError( &errstr);
        printf("GetSpotDiagram failed: %s\n",errstr);
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

int KBtest()
{
    char * errstr;
    if(!LoadSystemFromXml("../../TestSystems/KBtest.xml"))
    {
        GetOptiXError(&errstr);
        printf("XML load error :\n%s\n", errstr);
        return -1;
    }

    printf("file loaded OK\n") ;


    // we add surface error generators to M1 and M2
    size_t idP_in, idM1, idM2, idP_out, idScreen, sourceID;
    printf("getting surfaces \n");


    if(!FindElementID("M1",&idM1))
    {
        printf("element 'M1' was not found\n");
        return -1;
    }

    if(!FindElementID("M2",&idM2))
    {
        printf("element 'M2' was not found\n");
        return -1;
    }

    // These calls add the error generator parameters to the surface
    SetErrorGenerator(idM1);
   // SetErrorGenerator(idM2);

    printf("setting the parameters of the surface error generators\n");

    Parameter param;
    param.flags=0; // to be sure the array flag is not set

    // set the rms error level after zernike detrending

    if(GetParameter(idM1,"residual_sigma", &param))
    {
        param.value=3.e-9; // 3 nm
        SetParameter(idM1,"residual_sigma", param);
        // here we set identical errors for both surfeces change param.value if needed
        SetParameter(idM2,"residual_sigma", param);
        printf("residual sigma set:\n");
        DumpParameter(idM1,"residual_sigma");

    }
    else
    {
        GetOptiXError(&errstr);
        printf("parameter error :\n%s\n", errstr);
        return -1;
    }

    //prepare an array parameter variable
    Parameter a_param;
    ArrayParameter d_array;
    a_param.paramArray=&d_array;
    a_param.flags=NotOptimizable | ArrayData;

    //Set the size of the error map
    {
        d_array.dims[0]=d_array.dims[1]=2;
        double limits[]= {-0.055, 0.055, -0.005, 0.005};
        d_array.data=limits;
        SetParameter(idM1,"error_limits", a_param);
    }
    printf("\n Error_limits set;\n") ;
    DumpParameter(idM1, "error_limits");

    //Set the ssampling steps
    {
        d_array.dims[0]=2;
        d_array.dims[1]=1;
        double sampling[]= {5e-4, 1e-4};
        d_array.data=sampling;
        SetParameter(idM1,"sampling", a_param);
    }
    printf("\n Sampling set:\n") ;
    DumpParameter(idM1, "sampling");

    //Set the fractal exponents in x
    {
        d_array.dims[0]=2;
        d_array.dims[1]=1;
        double exp_x[]= {-1.5,-2.};
        d_array.data=exp_x;
        SetParameter(idM1,"fractal_exponent_x", a_param);
    }
    printf("\n fractal_exponent_x set:\n") ;
    DumpParameter(idM1, "fractal_exponent_x");

    //Set the fractal transition frequency in x  // only one value but must be an array
    {
        d_array.dims[0]=1;
        d_array.dims[1]=1;
        double freq_x[]= {500.};  // in m^-1
        d_array.data=freq_x;
        SetParameter(idM1,"fractal_frequency_x", a_param);
    }
    printf("\n fractal_frequency_x set:\n") ;
    DumpParameter(idM1, "fractal_frequency_x");

    // we keep default value of -1 for the Y fractal exponent
    // if not repeat the steps for fractal_exponent_y and fractal_frequency_y

    //Set the detrending mask ; x axis varying faster  // only one value but must be an array
    {
        d_array.dims[0]=d_array.dims[1]=3;
        double mask[]= {1.,1.,1.,   1.,1.,0,  1.,0,0};
        d_array.data=mask;
        SetParameter(idM1,"detrending", a_param);
    }
    printf("\n detrending mask set:\n") ;
    DumpParameter(idM1, "detrending");


    //Set the max sigma of Legendre fits  // only one value but must be an array
    {
        d_array.dims[0]=4;
        d_array.dims[1]=3;
        double n_legendre[]= {0,0,1.e-8,5.e-9,   0,2.e-9,0,0,    5e-9,1.e-9,2.e-9,0};
        d_array.data=n_legendre;
        SetParameter(idM1,"low_Zernike", a_param);
    }
    printf("\n low_Zernike set:\n") ;
    DumpParameter(idM1, "low_Zernike");

    // surface M1 error parameters are define we can generate a height error realization
    {
        int dims[]={4, 3}, mapDims[2];
        double total_sigma;
        double legendre_sigmas[12];
        if(!GenerateSurfaceErrors(idM1, mapDims, &total_sigma, dims, legendre_sigmas ))
        {
            GetOptiXError(&errstr);
            printf("Surface error generation failed :\n%s\n", errstr);
            return -1;
        }
        printf("SurfaceError map generated with size %d x %d \n",mapDims[0], mapDims[1]);
        // print the stats. Note they are dumped to the console by the generator
        printf("Total sigma= %8.3e\n Table of legendre sigmas:\n");
        for(int j=0; j<3; ++j)
        {
            for(int i=0; i <4; ++i)
                printf("%8.3e,", legendre_sigmas[4*j+i] );
            printf("\n");
        }
    }
    // set method to SimpleShift
    SetErrorMethod(idM1, SimpleShift);

 // Proceed the same way for surface M2

   // Now we prepare the ray tracing


    double lambda=1.e-9;

    printf("getting source \n");
    if(!FindElementID("source",&sourceID))
    {
        printf("element 'source' was not found\n");
        return -1;
    }

    printf("calling align on source\n");
    if(!Align(sourceID, lambda))
    {
       GetOptiXError( &errstr);
       printf("Alignment error:\n%s\n", errstr);
       return -1;
    }
    int numrays; //  numrays is defined in the xml file, but we need it to declare the spotdiag struct
    if(!Generate(sourceID, lambda, &numrays))
    {
        GetOptiXError( &errstr);
        printf("Source generation error:\n%s\n", errstr);
        return -1;
    }

    if(!FindElementID("screen",&idScreen))
    {
        printf("element 'screen' was not found\n");
        return -1;
    }


    // disable surface errors in ray tracing and launch a first ray tracing
    SurfaceErrorsEnable(false);

    printf("start ray tracing\n");
    if(!Radiate(sourceID))
    {
       GetOptiXError( &errstr);
       printf("Radiation error:\n%s\n", errstr);
       return -1;
    }
    printf("Ray tracing OK\n");

    //get the spot diagarm without errors and dump it to a file

    C_DiagramStruct cdiagram={5,numrays,0,0}; // defines and initialize a new C_DiagramStruct
                    // in order to record spot diagram the m_dim must be set to 5 and m_reserved should be at least the number of rays going through

    cdiagram.m_min=malloc(cdiagram.m_dim*sizeof(double)); // Use m_dim and m_reserve to be sure initialization is consistent
    cdiagram.m_max=malloc(cdiagram.m_dim*sizeof(double));
    cdiagram.m_mean=malloc(cdiagram.m_dim*sizeof(double));
    cdiagram.m_sigma=malloc(cdiagram.m_dim*sizeof(double));
    cdiagram.m_spots= malloc(cdiagram.m_dim*cdiagram.m_reserved*sizeof(double));

    if(!GetSpotDiagram(idScreen, &cdiagram, 0))
    {
        GetOptiXError( &errstr);
        printf("GetSpotDiagram failed: %s\n",errstr);
        return -1;
    }
    else
    {
        if(cdiagram.m_count)
        {
            DiagramToFile("KB_no_errors.sdg", &cdiagram);
            printf("\nSpot-diagram with %d impacts dumped to file 'KB_no_errors.sdg'\n", cdiagram.m_count);
            printf("     min         max        mean        sigma\n");
            for (int i=0; i<5 ; ++i)
                printf("%10.3e  %10.3e  %10.3e  %10.3e\n", cdiagram.m_min[i], cdiagram.m_max[i], cdiagram.m_mean[i], cdiagram.m_sigma[i] );
        }
        else
            printf("\npot-diagram contains no impact\n");

    }

    //  We now  ensable the error generation and ray trace again
    SurfaceErrorsEnable(true);
    // clear impacts in the system but not in the source
    uint64_t idFirst;
    GetNextElement(sourceID, &idFirst);
     if(!ClearImpacts(idFirst))
    {
        GetOptiXError( &errstr);
        printf("ClearImpacts failed: %s\n",errstr);
        return -1;
    }
    Radiate(sourceID);

    // we  get the spot diagram with errors
    if(!GetSpotDiagram(idScreen, &cdiagram, 0))
    {
        GetOptiXError( &errstr);
        printf("GetSpotDiagram failed: %s\n",errstr);
        return -1;
    }
    else
    {
        if(cdiagram.m_count)
        {
            DiagramToFile("KB_errors.sdg", &cdiagram);
            printf("\nSpot-diagram with %d impacts dumped to file 'KB_errors.sdg'\n", cdiagram.m_count);
            printf("     min         max        mean        sigma\n");
            for (int i=0; i<5 ; ++i)
                printf("%10.3e  %10.3e  %10.3e  %10.3e\n", cdiagram.m_min[i], cdiagram.m_max[i], cdiagram.m_mean[i], cdiagram.m_sigma[i] );
        }
        else
            printf("\nSpot-diagram contains no impact\n");

    }

    printf("foc diagram section\n");
    // Get the 3D focal diagram

    double  xlimit[2], ylimit[2], zlimit[2]={-0.002,0.002};
    int32_t dims[3]={201,201,101};   // le nombre de points en x, y et z  -  x (dims[0]) varie le plus vite
    int32_t focmapsize=dims[0]*dims[1]*dims[2];
    printf("reserving tensor space: size %d int\n", focmapsize);
    int32_t *focDiagram=malloc(focmapsize*sizeof(int32_t));

    printf("calling foc diagram\n");
    GetFocalDiagram(idScreen, dims, zlimit, focDiagram, xlimit, ylimit);   // Zlimit utilisé en input xlimit et ylimit sont calculés et donnés en output
    printf("limits X [%8.3e, %8.3e]  y [%8.3e, %8.3e] \n", xlimit[0], xlimit[1], ylimit[0], ylimit[1]) ;
    FILE * focfile= fopen("KB_focdiag.fdg", "w");
    if(focfile)
    {
        printf("writing header\n");
        fwrite(xlimit, sizeof(double), 2, focfile);
        fwrite(ylimit, sizeof(double), 2, focfile);
        fwrite(zlimit, sizeof(double), 2, focfile);
        fwrite(dims,  sizeof(int32_t), 3, focfile);
        printf("Header written\n");
        fwrite(focDiagram, sizeof(int32_t) ,dims[0]*dims[1]*dims[2],focfile );
        fclose(focfile);
        printf("Focal diagam with Surface errors written to KB_focdiag.fdg\n");
    }
    else
        printf("could not open KB_focdiag.fdg in output\n");

    free(focDiagram);


    return 0;
}
