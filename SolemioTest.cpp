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
 #include <chrono>
using namespace std::chrono;

 #include "files.h"
 #include "interface.h"
 #include  "surface.h"  // temporaire avant de créer le fcts d'interface
 #include "gratingbase.h"
 #include "opticalelements.h"

#include "xmlfile.h"

 int SolemioTest()
 {
    //ReadSolemioFile("R:\\Partage\\SOLEMIO\\CASSIOPEE");
//    SolemioImport("D:\\projets\\projetsCB\\OptiX\\solemio\\CASSIOPEE");
    if(! SolemioImport("D:\\projets\\projetsCB\\OptiX\\solemio\\DESIRSvrai.sole"))
    {
        char buf[512];
        GetOptiXLastError(buf,511);
        cout << "Solemio import error:\n" << buf <<endl;
        exit(100);
    }
    size_t hSys=0, hParm=0, elemID=0;
    char elname[32], name2[32],parmname[48], errBuf[256];
    Parameter param;
    cout << "list of defined elements\n";
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
                        " T:" << param.type << " G:" << param.group << " F:0x"<< std::hex << param.flags << std::dec << endl;

        }while(hParm);

    }while(hSys);
    cout << "system END\n";
    size_t sourceID=elemID=GetElementID("S_ONDUL1");
//    size_t pupID=GetElementID("pupille");
//    ChainElement_byID(pupID,0);
//    ChainElement_byName("pupille","");

    while(elemID)
    {
        GetElementName(elemID, elname,32);
        cout << elname << "  " << std::hex << elemID << std::dec <<endl;
        elemID =GetNextElement(elemID);
    }

    GetParameter(sourceID,"nRays", &param);
    param.value=50000;
    SetParameter(sourceID,"nRays",param);

    if(!Align(sourceID,2.5e-8))
    {
       GetOptiXLastError(errBuf,256);
       cout << "Alignment error : " << errBuf << endl;
       return -1;
    }
    if(!Generate(sourceID, 2.5e-8))
    {
       GetOptiXLastError(errBuf,256);
       cout << "Source generation error : " << errBuf << endl;
       return -1;
    }
    cout << "source generated\n";
    {
        GratingBase* grating=dynamic_cast<GratingBase *> ((ElementBase*)GetElementID("reseaumiroir"));  // "Reseau_400H"
//        GratingBase* grating=dynamic_cast<GratingBase *> ((ElementBase*)GetElementID("Reseau_400H"));

//        Surface* screen=dynamic_cast<Surface*> ((ElementBase*)GetElementID("EXP1")); //"EXP1", S_ONDUL1, pupille, Reseau_400H, Fente, planfocH
        Surface* screen=dynamic_cast<Surface*> ((ElementBase*)GetElementID("fenteentree")); //"EXP1", S_ONDUL1, pupille, Reseau_400H, Fente, planfocH

        if(grating)
        {
                cout << "grating defined\n";
            for(double x=-2e-2; x  < 2.1e-2; x+=1e-2)
            {
                Surface::VectorType pos=Surface::VectorType::Zero();
                pos(0)=x;
                cout << x << "   " << grating->gratingVector(pos).transpose() << endl;
            }
        }
        else cout << "grating not defined\n";

        if(screen) cout << "screen defined\n";
        else
        {
            cout << "screen not defined\n";
            return 10;
        }

        screen->setRecording(RecordOutput);
        cout << "done\n";
        cout << "recording mode " << screen->getRecording() << endl;


        high_resolution_clock clock;
        high_resolution_clock::time_point start(clock.now());

        if(!Radiate(sourceID))
        {
           GetOptiXLastError(errBuf,256);
           cout << "Radiation error : " << errBuf << endl;
           return -1;
        }
        cout << "propagation computation time :" << duration_cast<milliseconds>(clock.now()-start).count() << " msec\n" ;

         cout << "\nIMPACTS\n";



        Diagram spotDg(5);

        int ncounts=screen->getSpotDiagram(spotDg,-0.002);
        if(ncounts)
        {
            for(int i=0; i<5 ; ++i)
               cout << spotDg.m_min[i] << " \t" << spotDg.m_spots[i] << endl;

            fstream spotfile("SMSpotdiag.sdg", ios::out | ios::binary);
            spotfile << spotDg;
            spotfile.close();

            {
               //  CausticDiagram caustic;
                Diagram caustic(4);
                int n= screen->getCaustic(caustic) ;
                cout << " caustic of " << n << " points\n";
                fstream causticFile("SMcaustic.cdg", ios::out | ios::binary);
                causticFile << caustic;
                causticFile.close();
            }

            cout << endl << endl;
            vector<RayType> impacts;
            screen->getImpacts(impacts, SurfaceFrame);
            cout << impacts[0].position().transpose() <<  "      " << impacts[0].direction().transpose() << endl<< endl;

            C_DiagramStruct cdiagram={5,50000,0,0};

            cdiagram.m_min=new double[cdiagram.m_dim];
            cdiagram.m_max=new double[cdiagram.m_dim];
            cdiagram.m_mean=new double[cdiagram.m_dim];
            cdiagram.m_sigma=new double[cdiagram.m_dim];
            cdiagram.m_spots= new double[cdiagram.m_dim*cdiagram.m_reserved];
            cout << "cdiag struct "<< std::hex << cdiagram.m_min << " " << cdiagram.m_max << " " << cdiagram.m_mean <<
                " " << cdiagram.m_sigma << std::dec << endl;

            if(!GetSpotDiagram(GetElementID("EXP1"), &cdiagram, 0))
            {
                char errbuf[256];
                GetOptiXLastError(errbuf, 256);
                cout << "GetSpotDiagram failed :  "<< errbuf << endl;
            }
            else
            {
                cout  <<  "GetSpotDiagram succeeded\n";
                DiagramToFile("cSpotDiag.sdg", &cdiagram);
            }

            delete [] cdiagram.m_min;
            delete [] cdiagram.m_max;
            delete [] cdiagram.m_mean;
            delete [] cdiagram.m_sigma;
            delete [] cdiagram.m_spots;
        }

        else cout <<" No spot on screen\n";
    }
     SaveSystemAsXml("system.xml");
     DumpXmlSys("system.xml");

     LoadSystemFromXml("system.xml");

    return 0;
 }

 int SphereTest()
 {
    //ReadSolemioFile("R:\\Partage\\SOLEMIO\\CASSIOPEE");
    if(!SolemioImport("D:\\projets\\projetsCB\\OptiX\\solemio\\SphereTest"))
    {
        cout << "An error occurred while loading Solemio file\n";
        return -1;
    }
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
                        " T:" << param.type << " G:" << param.group << " F:0x"<< std::hex << param.flags << std::dec << endl;

        }while(hParm);

    }while(hSys);

    size_t sourceID=elemID=GetElementID("source");

    while(elemID)
    {
        GetElementName(elemID, elname,32);
        cout << elname << "  " << std::hex << elemID << std::dec <<endl;
        elemID =GetNextElement(elemID);
    }

    GetParameter(sourceID,"nRays", &param);
    param.value=50000;
    SetParameter(sourceID,"nRays",param);

    if(!Align(sourceID,2.5e-8))
    {
       GetOptiXLastError(errBuf,256);
       cout << "Alignment error : " << errBuf << endl;
       return -1;
    }
    if(!Generate(sourceID, 2.5e-8))
    {
       GetOptiXLastError(errBuf,256);
       cout << "Source generation error : " << errBuf << endl;
       return -1;
    }

    cout << "getting mirror surface  \n";
    Surface* mir=dynamic_cast<Surface*> ((ElementBase*)GetElementID("M1")); //S_ONDUL1, pupille, Reseau_400H, Fente, planfocH
    cout << mir << endl;
    mir->setRecording(RecordOutput);
    cout << "recording mode " << mir->getRecording() << endl <<endl;
    mir->dumpData();
    cout << endl ;

    cout << "getting screen \n";
    Surface* screen=dynamic_cast<Surface*> ((ElementBase*)GetElementID("film-1")); //S_ONDUL1, pupille, Reseau_400H, Fente, planfocH
    cout << screen << endl;
    screen->setRecording(RecordOutput);
    cout << "recording mode " << screen->getRecording() << endl <<endl;
    screen->dumpData() ;
    cout << endl;

    high_resolution_clock clock;
    high_resolution_clock::time_point start(clock.now());

    if(!Radiate(sourceID))
    {
       GetOptiXLastError(errBuf,256);
       cout << "Radiation error : " << errBuf << endl;
       return -1;
    }
    cout << "propagation computation time :" << duration_cast<milliseconds>(clock.now()-start).count() << " msec\n" ;

    cout << "\nIMPACTS\n";


    Diagram spotDg(5);

    int ncounts=screen->getSpotDiagram(spotDg,0);
    if(ncounts)
    {
        for(int i=0; i<5 ; ++i)
           cout << spotDg.m_min[i] << " \t" << spotDg.m_max[i] << endl;

        fstream spotfile("SphTestScrSpotdiag.sdg", ios::out | ios::binary);
        spotfile << spotDg;
        spotfile.close();


        cout << endl << endl;
    }


    ncounts=mir->getSpotDiagram(spotDg,0);
    if(ncounts)
    {
        for(int i=0; i<5 ; ++i)
           cout << spotDg.m_min[i] << " \t" << spotDg.m_max[i] << endl;

        fstream spotfile("SphTestSphSpotdiag.sdg", ios::out | ios::binary);
        spotfile << spotDg;
        spotfile.close();


        cout << endl << endl;
    }

    Diagram impacts(7);

    ncounts=screen->getImpactData(impacts);
    if(ncounts)
    {
        for(int i=0; i<5 ; ++i)
           cout << impacts.m_min[i] << " \t" << impacts.m_max[i] << endl;

        fstream spotfile("SphTestScrImpacts.imp", ios::out | ios::binary);
        spotfile << impacts;
        spotfile.close();


        cout << endl << endl;
    }


    ncounts=mir->getImpactData(impacts);
    if(ncounts)
    {
        for(int i=0; i<5 ; ++i)
           cout << impacts.m_min[i] << " \t" << impacts.m_max[i] << endl;

        fstream spotfile("SphTestSphImpacts.imp", ios::out | ios::binary);
        spotfile << impacts;
        spotfile.close();


        cout << endl << endl;
    }
  return 0;
 }






 int QuickTest()
 {

    double lambdatest=6.e-9,  defoc=0;
    string sourceName="S_ONDUL_BE", mirrorName="M3tor", screenName="Fente";
    string gratingName="Reseau_450";


    if(!SolemioImport("D:\\projets\\projetsCB\\OptiX\\solemio\\Hermes-BEmono-c0.2"))
    {
        cout << "An error occurred while loading Solemio file\n";
        return -1;
    }
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
                        " T:" << param.type << " G:" << param.group << " F:0x"<< std::hex << param.flags << std::dec << endl;

        }while(hParm);

    }while(hSys);

    size_t sourceID=elemID=GetElementID(sourceName.c_str());

    while(elemID)
    {
        GetElementName(elemID, elname,32);
        cout << elname << "  " << std::hex << elemID << std::dec <<endl;
        elemID =GetNextElement(elemID);
    }


    if(!Align(sourceID, lambdatest))
    {
       GetOptiXLastError(errBuf,256);
       cout << "Alignment error : " << errBuf << endl;
       return -1;
    }
    if(!Generate(sourceID, lambdatest))
    {
       GetOptiXLastError(errBuf,256);
       cout << "Source generation error : " << errBuf << endl;
       return -1;
    }
  //  Generate(sourceID,lambdatest*1.002);

    cout << "getting mirror surface  \n";
    Surface* mir=dynamic_cast<Surface*> ((ElementBase*)GetElementID(mirrorName.c_str())); //S_ONDUL1, pupille, Reseau_400H, Fente, planfocH
    cout << mir << endl;
    mir->setRecording(RecordOutput);
    cout << "recording mode " << mir->getRecording() << endl <<endl;
    mir->dumpData();
    cout << endl ;

    cout << "getting screen \n";
    Surface* screen=dynamic_cast<Surface*> ((ElementBase*)GetElementID(screenName.c_str())); //S_ONDUL1, pupille, Reseau_400H, Fente, planfocH
    cout << screen << endl;
    screen->setRecording(RecordOutput);
    cout << "recording mode " << screen->getRecording() << endl <<endl;
    screen->dumpData() ;
    cout << endl;
    if(!gratingName.empty())
    {
        cout << "getting grating "<<gratingName <<"\n";
        size_t gratingID=GetElementID(gratingName.c_str());
        Grating<Holo,Plane> *grating=dynamic_cast<Grating<Holo,Plane>*> ((ElementBase*)gratingID); //S_ONDUL1, pupille, Reseau_400H, Fente, planfocH
//        GratingBase *grating=dynamic_cast<GratingBase*> ((ElementBase*)GetElementID(gratingName.c_str())); //S_ONDUL1, pupille, Reseau_400H, Fente, planfocH
        cout << grating << endl;
        cout<< "Direction1  " << grating->m_direction1.transpose()<<endl;
        cout<< "Direction2  " << grating->m_direction2.transpose()<<endl;
        cout << "line density " << grating->m_lineDensity << endl;
        cout << "Holo lambda " << grating->m_holoWavelength << endl;
        AlignGrating4Cff(gratingID,0.2,lambdatest );
        for(double x=-2e-2; x  < 2.1e-2; x+=1e-2)
        {
            Surface::VectorType pos=Surface::VectorType::Zero();
            pos(0)=x;
            cout << x << "   " << grating->gratingVector(pos,Surface::VectorType::UnitZ()).transpose() << endl;
        }

    }

    high_resolution_clock clock;
    high_resolution_clock::time_point start(clock.now());

    if(!Radiate(sourceID))
    {
       GetOptiXLastError(errBuf,256);
       cout << "Radiation error : " << errBuf << endl;
       return -1;
    }
    cout << "propagation computation time :" << duration_cast<milliseconds>(clock.now()-start).count() << " msec\n" ;

    cout << "\nIMPACTS\n";


    Diagram spotDg(5);

    int ncounts=screen->getSpotDiagram(spotDg,defoc);
    if(ncounts)
    {
        for(int i=0; i<5 ; ++i)
           cout << spotDg.m_min[i] << " \t" << spotDg.m_max[i] << endl;

        fstream spotfile("QTestScrSpotdiag.sdg", ios::out | ios::binary);
        spotfile << spotDg;
        spotfile.close();


        cout << endl << endl;
    }

    if(1) return 0;

    Diagram impacts(7);

    ncounts=screen->getImpactData(impacts);
    if(ncounts)
    {
        for(int i=0; i<5 ; ++i)
           cout << impacts.m_min[i] << " \t" << impacts.m_max[i] << endl;

        fstream spotfile("QTestScrImpacts.imp", ios::out | ios::binary);
        spotfile << impacts;
        spotfile.close();


        cout << endl << endl;
    }


  return 0;
 }
