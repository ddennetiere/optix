////////////////////////////////////////////////////////////////////////////////
/**
*      \file           test.cpp
*
*      \brief         Various tests on the OptiX library
*
*      \author         François Polack <francois.polack@synchroton-soleil.fr>
*      \date        2020-10-05  Creation
*      \date        2021-03-15  scission in 2 files
*
*/
///////////////////////////////////////////////////////////////////////////////////
//
//             REVISIONS
//
////////////////////////////////////////////////////////////////////////////////////
#
#include <iostream>
#include "opticalelements.h"
#include "sources.h"
#include <fstream>
#include <float.h>
#include "gratingbase.h"
#include "files.h"
#include <sstream>
#include "interface.h"
#include "wavefront.h"


//#define POSTFIX(X, P) X#P
//#define M_PIl POSTFIX(M_PI, L)



using namespace std;
int OriginalTest();
int SolemioTest();

int main()
{
    return SolemioTest();
//    return OriginalTest();
}
int OriginalTest()
{

//    double d_Pi=3.1415926535897932384626433832795;
//    long double ld_Pi=3.1415926535897932384626433832795L;

//  -----  MEMSET TEST   ---------------------------

    double dtest[]={1., 1., 1., 1., 1., 1., 1., 1.};
    for(int ii =0; ii <8; ++ii)
        cout << dtest[ii] << "  ";
    cout << endl;
    memset(dtest, 0, 8* sizeof(double));
    for(int ii =0; ii <8; ++ii)
        cout << dtest[ii] << "  ";
    cout << endl;



    Vector3d pos={0,0,0}, dir={1,0,-1}, pZ={0,0,1};
//    Matrix<long double,3,1> Trans={1,-1,3};

    ParametrizedLine<double,3> line(pZ,dir.normalized());
    cout << "projection " << line.projection(pos).transpose() << endl;


 //   dir.normalize();  normalization is forced
    RayType inray(RayBaseType(pos,dir,3.));


//    cout <<inray << endl << sizeof (inray) << "  "  << sizeof(Rayd::BaseLine) << endl;
//    inray+=Trans;
//    cout <<inray << endl;
//
//    cout <<(inray+=20. )<< endl;
//    cout << inray.position().transpose()<<endl;
//    cout << inray.origin() << endl;
//    inray.origin()=pos.cast<long double>();
//    cout  << "origin modified " <<  inray << endl;
//
//    cout <<(inray-=RayType::VectorType::UnitZ()*10.) << endl;

//    fstream file("test.data",ios_base::binary | ios_base::out);
//    file <<inray;
//    file.close();
//    cout << "Ecrit " <<inray << endl;
//    fstream infile("test.data", ios_base::binary | ios_base::in);
//    Rayld outray;
//    infile  >> outray;
//    infile.close();
//    cout << "relu   " << outray << endl;
//
//    cout << "digits ; double " <<DBL_DIG << " (" << sizeof(double) << ")  long double "  << LDBL_DIG << " (" << sizeof(long double) << ")\n";
//
//    printf("%28.24g\n",M_PI);
//    printf("%28.24Lg\n",ld_Pi);
//
//    RayType::VectorType ldpos ={-2.,0,0.004};
//    Transform<double, 3, Isometry> T(AngleAxisd(0.25*M_PI, Vector3d::UnitZ() ));
//
//    cout << T.matrix() << endl;

    Parameter param;

    pos << 0,-1.e-3,-1;
    dir << 0,0,1.;
    RayType rayin(RayBaseType(pos,dir,0.));
    RayType::VectorType normal;

    ToroidalMirror Tmir("TM1");
    Tmir.getParameter("minor_curvature",param);
    param.value=1./0.2;
    Tmir.setParameter("minor_curvature",param);
    param.value=1./80.;
    Tmir.setParameter("major_curvature",param);
    Tmir.getParameter("theta",param);
    param.value=0.08; // 0.707; // env 4°
    Tmir.setParameter("theta",param);

    Tmir.alignFromHere(0);

    Tmir.intercept(rayin, &normal);
    cout << endl;

//    inray=RayType (RayBaseType(ldpos,RayType::VectorType::UnitZ() ,3.));
//    cout << "new inray   " << inray << endl;
//

    XYGridSource source("Source") ;
    //PlaneMirror mirror("MP", &source);
    SphericalMirror mirror1("SM1",&source);
    PlaneFilm  film1("Film1",&mirror1);
    SphericalMirror mirror2("SM2",&film1);
    PlaneFilm  film2("Film2",&mirror2);
    mirror1.getParameter("curvature",param);
    param.value=sqrt(2.)/2.;
    mirror1.setParameter("curvature",param);
    mirror2.setParameter("curvature",param);

    mirror1.getParameter("distance",param);
    param.value=1.;
    mirror1.setParameter("distance", param);
    film1.setParameter("distance", param);
    mirror2.setParameter("distance", param);
    film2.setParameter("distance", param);

    mirror1.getParameter("theta",param);
    param.value=M_PI_4;
    mirror1.setParameter("theta",param);
    mirror2.setParameter("theta",param);
    param.value=M_PI;
    mirror2.setParameter("phi",param);
    source.getParameter("nXdiv",param);
    param.value=10;
    source.setParameter("nXdiv",param);
    source.setParameter("nYdiv",param);
  //  param.value=M_PI_2;
  //  mirror.setParameter("phi",param);
    mirror1.setRecording(RecordOutput);
    mirror2.setRecording(RecordOutput);
  //   Quadric quad;
  //  Grating grat; la classe grating n'est pas directement instanciable
  //

   // source.setParameter("nYdiv", param);
    double wavelength=1e-9;
    ElementBase* ps= film1.getSource();
    if(ps==NULL)
        cout << "No source found \n";
    else
        cout << "Source is " << ps->getName() << endl;

    source.alignFromHere(wavelength);
    source.generate(wavelength);
    source.radiate();

    if(0){
        cout << "\nIMPACTS\n";
        vector<RayType> impacts;
        int lost=film2.getImpacts(impacts, LocalAbsoluteFrame);       //       ());  AlignedLocalFrame
        vector<RayType>::iterator it;
        int ncount=0;
        cout << lost << " lost rays over" << (lost+impacts.size()) << endl;
        for(it=impacts.begin(); it!=impacts.end(); ++it,++ncount)
            cout << ncount <<*it << endl;
//        cout << ncount << "  " << it->origin().transpose() <<
//        "  \t" << it->direction().transpose() << endl;
    }

    {
        fstream spotfile("Spotdiag.sdg", ios::out | ios::binary);
        SpotDiagramExt spotDg;
       // PlaneFilm screenCopy(film2);
        film2.getSpotDiagram(spotDg,0.00);
        spotfile << spotDg;
        spotfile.close();
    }

    {
        fstream causticFile("diagram.cdg", ios::out | ios::binary);
        CausticDiagram caustic;
        int n= film2.getCaustic(caustic) ;
        cout << " caustic of " << n << " points\n";
        causticFile << caustic;
        causticFile.close();
    }
    if(0){
        fstream WfFile("wavederiv.sdg", ios::out | ios::binary);
        SpotDiagramExt WFdata;
        int n= film2.getWavefrontData(WFdata) ;
        cout << " WF deriv  of " << n << " points\n";
        WfFile << WFdata;
        WfFile.close();
    }
    {
        Array22d XYbounds;
        ArrayXXd WFlegendres=film2.getWavefontExpansion(0, 5,5, XYbounds);
        cout << "Wave front Legendre expansion coefficients \n";
        cout << WFlegendres<< endl;
        cout << "bounds:\n" << XYbounds << endl;
        VectorXd Xpos=VectorXd::LinSpaced(100, XYbounds(0,0), XYbounds(1,0));
        VectorXd Ypos=VectorXd::LinSpaced(100, XYbounds(0,1), XYbounds(1,1));
//        cout << "Interpolated Wavefront [nm]\n";
        ArrayXXd WFsurf=LegendreSurface(Xpos, Ypos, XYbounds, WFlegendres);
//        cout << WFsurf.transpose()*1.e9<< endl << endl;
        fstream WfFile("wavefront.swf", ios::out | ios::binary);
        int nx=WFsurf.rows(), ny=WFsurf.cols();
        WfFile.write((char*)&nx, sizeof(int));
        WfFile.write((char*)&ny, sizeof(int)) ;
        WfFile.write((char*)XYbounds.data(), 4*sizeof(double )) ;
        WfFile.write ((char*)WFsurf.data(), nx*ny*sizeof(double));
        WfFile.close();
    }

    cout << "     " << source.getName() << endl;
    cout << "     " << mirror1.getName() << endl;
    cout << "     " << film1.getName() << endl;
    cout << "     " << mirror2.getName() << endl;
    cout << "     " << film2.getName() << endl;



    if(1)
    {
        Parameter param;

        GaussianSource gSource("GaussSource") ;

        source.clearImpacts();
        source.chainPrevious(&gSource);

        cout << "  added " << gSource.getName() << endl;

        gSource.getParameter("nRays",param);
        param.value=5000.;
        gSource.setParameter("nRays", param);

        gSource.alignFromHere(wavelength);
        gSource.generate(wavelength);
        gSource.radiate();

        fstream spotfile("Spotdiag2.sdg", ios::out | ios::binary);
        SpotDiagramExt spotDg;
       // PlaneFilm screenCopy(film2);
        film2.getSpotDiagram(spotDg,0.00);
        spotfile << spotDg;
        spotfile.close();

        cout << "Spotdiag2.sdg saved \n";

        ChainCopy tempChain;
        if(DuplicateChain(&source, tempChain))
            cout << "chaine duplicated OK\n";
        else
            cout << "chain duplication failed\n";
        ElementBase *elem, *psrc=elem=tempChain.First;
        while(elem)
        {
            cout << elem->getName() << " " << elem->getOptixClass() <<endl;
            elem=elem->getNext();
        }

        cout <<psrc <<  " " << psrc->getOptixClass() << "  " << psrc->getNext() <<endl;

        elem=tempChain.First=ChangeElementType(psrc, "GaussianSource");
        if(elem)
            cout << "source type successfully changed to GaussianSource\n";
        else
            cout << "source could not be changed to GaussianSource\n";
        cout <<elem <<  " " << elem->getOptixClass() << "  " << elem->getNext() <<endl;

        while(elem)
        {
            cout << elem->getName() << " " << elem->getOptixClass() <<endl;

            for(ElementBase::ParamIterator it=elem->parameterBegin(); it!=elem->parameterEnd(); ++it)
            {

                cout << it->first << "  " << it->second.value <<" [" << it->second.bounds[0] <<", "<< it->second.bounds[1] <<"] x " << it->second.multiplier <<
                        " T:" << it->second.type << " G:" << it->second.group << " F:0x"<< hex << it->second.flags << dec << endl;
            }

            elem=elem->getNext();

        }
    }

    string strHelp, name;
    param.value=0.04;

//    size_t surfID=CreateSurface("XYGridSource", "Sys_Source");
//    size_t pSource=GetSurfaceID("Sys_Source");
//    if (pSource!=0)
//    {
//        size_t nextPtr=0;
//        char buffer[32];
//        Parameter pardata;
//        while( GetNextParameter(pSource, &nextPtr, buffer, sizeof(buffer), &pardata))
//        {
//            cout << buffer << "  " << pardata.value << " ["  << pardata.bounds[0] << ", " << pardata.bounds[1]<<
//                    "]  group " << pardata.group << endl;
//        }
//    }

    TextFile file;
    file.open("systemSave.data", ios::out);


    if(file.is_open())
    {

        file << source << mirror1 << film1<< mirror2 << film2;

      //  file<< mirror1 << film1<< mirror2 << film2;
        file.flush();
        file.close();

        cout << "SystemSave.data file saved\n";

        TextFile infile("systemSave.data", ios::in);

        string sClass, sName, sPrev,sNext;
        while(!infile.eof())
        {

            infile >> sClass;
    //        cout << sClass  <<endl;
            if(sClass.empty())
                break;
            infile >>sName >> sPrev >> sNext;

            cout  << "class: "<< sClass << "  " << sName <<  endl;
            cout  << "linked to : "<< sPrev << " & " << sNext <<  endl;
      //      infile.ignore('\n'); // ignore to next endl
            string paramName;
            Parameter param;
            infile >> paramName;
            while(!paramName.empty())
            {
                infile >> param;
                cout << paramName << "  " << param.value <<" [" << param.bounds[0] <<", "<< param.bounds[1] <<"] x " << param.multiplier <<
                        " T:" << param.type << " G:" << param.group << " F:0x"<< hex << param.flags << dec << endl;
               infile >> paramName;
            }
            infile.ignore('\n');
        };
    }
    else
        cout << "could not open file for output\n";

    if(1)
    {
      cout << "\n---------------------------------------------------------------------------\n\n";

 //   Parameter param;
//    ReadSolemioFile("R:\\Partage\\SOLEMIO\\DEIMOS-cpy.txt");
    ReadSolemioFile("R:\\Partage\\SOLEMIO\\CASSIOPEE");
//    ReadSolemioFile("R:\\Partage\\SOLEMIO\\HERMES");
//    ReadSolemioFile("R:\\Partage\\SOLEMIO\\ANTARES");
//    ReadSolemioFile("R:\\Partage\\SOLEMIO\\SEXTANTS");
//    ReadSolemioFile("R:\\Partage\\SOLEMIO\\DISCOdefintif");
//    ReadSolemioFile("R:\\Partage\\SOLEMIO\\TEMPOajoutReseauSLICING.dat");
//    ReadSolemioFile("R:\\Partage\\SOLEMIO\\DESIRSvraiM3coefAx3=62");
//    ReadSolemioFile("R:\\Partage\\SOLEMIO\\AILES7");

    }

    return 0;
}
