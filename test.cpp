////////////////////////////////////////////////////////////////////////////////
/**
*      \file           test.cpp
*
*      \brief         TODO  fill in file purpose
*
*      \author         François Polack <francois.polack@synchroton-soleil.fr>
*      \date        2020-10-05  Creation
*      \date        Last update
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
#include "gridSource.h"
#include <fstream>
#include <float.h>
#include "gratingbase.h"
#include "files.h"
#include <sstream>
#include "interface.h"

//#define POSTFIX(X, P) X#P
//#define M_PIl POSTFIX(M_PI, L)

using namespace std;
int main()
{

    double d_Pi=3.1415926535897932384626433832795;
    long double ld_Pi=3.1415926535897932384626433832795L;

    Vector3d pos={0,0,0}, dir={1,0,-1}, pZ={0,0,1};
    Matrix<long double,3,1> Trans={1,-1,3};

    ParametrizedLine<double,3> line(pZ,dir.normalized());
    cout << "projection " << line.projection(pos).transpose() << endl;


 //   dir.normalize();  normalization is forced
    RayType inray(RayBaseType(pos,dir,3.));

    cout <<inray << endl << sizeof (inray) << "  "  << sizeof(Rayd::BaseLine) << endl;
    inray+=Trans;
    cout <<inray << endl;

    cout <<(inray+=20. )<< endl;
    cout << inray.position().transpose()<<endl;
    cout << inray.origin() << endl;
    inray.origin()=pos.cast<long double>();
    cout  << "origin modified " <<  inray << endl;

    cout <<(inray-=RayType::VectorType::UnitZ()*10.) << endl;

//    fstream file("test.data",ios_base::binary | ios_base::out);
//    file <<inray;
//    file.close();
//    cout << "Ecrit " <<inray << endl;
//    fstream infile("test.data", ios_base::binary | ios_base::in);
//    Rayld outray;
//    infile  >> outray;
//    infile.close();
//    cout << "relu   " << outray << endl;

    cout << "digits ; double " <<DBL_DIG << " (" << sizeof(double) << ")  long double "  << LDBL_DIG << " (" << sizeof(long double) << ")\n";

    printf("%28.24g\n",M_PI);
    printf("%28.24Lg\n",ld_Pi);

    RayType::VectorType ldpos ={-2.,0,0.004};
    Transform<double, 3, Isometry> T(AngleAxisd(0.25*M_PI, Vector3d::UnitZ() ));

    cout << T.matrix() << endl;

    Parameter param;

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

    source.alignFromHere(wavelength);
    source.generate(wavelength);
    source.radiate();

    cout << "\nIMPACTS\n";
    vector<RayType> impacts=move(film2.getImpacts(LocalAbsoluteFrame));       //       ());  AlignedLocalFrame
    vector<RayType>::iterator it;
    int ncount=0;
    for(it=impacts.begin(); it!=impacts.end(); ++it,++ncount)
        cout << ncount <<*it << endl;
//        cout << ncount << "  " << it->origin().transpose() <<
//        "  \t" << it->direction().transpose() << endl;

    {
        fstream spotfile("Spotdiag.sdg", ios::out | ios::binary);
        SpotDiagram spotDg;
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
    {
        fstream WfFile("wavederiv.sdg", ios::out | ios::binary);
        SpotDiagram WFdata;
        int n= film2.getWavefrontData(WFdata) ;
        cout << " WF deriv  of " << n << " points\n";
        WfFile << WFdata;
        WfFile.close();
    }

    cout << "     " << source.getName() << endl;
    cout << "     " << mirror1.getName() << endl;
    cout << "     " << film1.getName() << endl;
    cout << "     " << mirror2.getName() << endl;
    cout << "     " << film2.getName() << endl;


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

    file << source  << mirror1 << film1<< mirror2 << film2;
  //  file.flush();
    file.close();

    TextFile infile("systemSave.data", ios::in);

    string sClass, sName, sPrev,sNext;
    while(!infile.eof())
    {

        infile >> sClass;
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
                    " T:" << param.type << " G:" << param.group << " F:0x"<< cout.hex << param.flags << cout.dec << endl;
           infile >> paramName;
        }
        infile.ignore('\n');
    };

 //   Parameter param;



    return 0;
}
