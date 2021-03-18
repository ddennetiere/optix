/**
 *************************************************************************
*   \file           SolemioTest.cpp
*
*   \brief             implementation file
*
*
*
*   \author             François Polack  <francois.polack@synchroton-soleil.fr>
*   \date               Creation: 2021-03-15
*   \date               Last update: 2021-03-15
 ***************************************************************************/

 #include <iostream>
 #include "files.h"
 #include "interface.h"


 int SolemioTest()
 {
    //ReadSolemioFile("R:\\Partage\\SOLEMIO\\CASSIOPEE");
    SolemioImport("R:\\Partage\\SOLEMIO\\CASSIOPEE");
    size_t hSys=0, hParm=0, elemID=0;
    char elname[32], name2[32],parmname[48], errBuf[256];
    Parameter param;
    do
    {
        EnumerateElements(&hSys,&elemID, elname,32);
        GetElementName(elemID, name2,32);
        cout << endl << elname <<  "   (" << name2 <<")\n";
        hParm=0;
        do
        {
            if(!EnumerateParameters(elemID, &hParm, parmname, 48, &param))
            {
                GetOptiXLastError( errBuf,256);
                cout  << "ERROR : " << errBuf << endl;
            }
            cout << parmname << "  " << param.value <<" [" << param.bounds[0] <<", "<< param.bounds[1] <<"] x " << param.multiplier <<
                        " T:" << param.type << " G:" << param.group << " F:0x"<< hex << param.flags << dec << endl;

        }while(hParm);

    }while(hSys);

    elemID=GetElementID("S_ONDUL1");
    while(elemID)
    {
        GetElementName(elemID, elname,32);
        cout << elname <<endl;
        elemID =GetNextElement(elemID);
    }
    return 0;
 }


