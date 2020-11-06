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

    cout <<(inray-=RayType::VectorType::UnitX()*10.) << endl;

    fstream file("test.data",ios_base::binary | ios_base::out);
    file <<inray;
    file.close();
    cout << "Ecrit " <<inray << endl;
    fstream infile("test.data", ios_base::binary | ios_base::in);
    Rayld outray;
    infile  >> outray;
    infile.close();
    cout << "relu   " << outray << endl;

    cout << "digits ; double " <<DBL_DIG << " (" << sizeof(double) << ")  long double "  << LDBL_DIG << " (" << sizeof(long double) << ")\n";

    printf("%28.24g\n",M_PI);
    printf("%28.24Lg\n",ld_Pi);

    RayType::VectorType ldpos ={-2.,0,0.004};
    Transform<double, 3, Isometry> T(AngleAxisd(0.25*M_PI, Vector3d::UnitX() ));

    cout << T.matrix() << endl;

    Parameter param;

    inray=RayType (RayBaseType(ldpos,RayType::VectorType::UnitX() ,3.));
    cout << "new inray   " << inray << endl;
    XYGridSource source("Source") ;
    PlaneFilm  film("Film",&source);
    film.getParameter("distance",param);
    param.value=1.;
    film.setParameter("distance", param);
    film.getParameter("theta",param);
    param.value=M_PI_4;
    film.getParameter("theta",param);
    film.setRecording(RecordInput);
  //   Quadric quad;
  //  Grating grat; la classe grating n'est pas directement instanciable
  //  PlaneMirror planeMir("MP", &film);

   // source.setParameter("nYdiv", param);
    double wavelength=1e-9;
    source.generate(wavelength);
    source.alignFromHere(wavelength);
    source.radiate();

    cout << "\nIMPACTS\n";
    vector<RayType>::iterator it;
    int ncount=0;
    for(it=film.m_impacts.begin(); it!=film.m_impacts.end(); ++it,++ncount)
        cout << ncount << "  " << it->position().transpose() << endl;
        //"  " << it->direction().transpose() << "  " << it->parameter() << endl;

    cout << "     " << source.getName() << endl;
    string strHelp, name;
    param.value=0.04;

    film.setParameter(name="theta", param);
//    quad.setParameter(name="theta", param);
//    quad.align();

    param.value=M_PI_4;
    film.setParameter("phi", param);
    film.alignFromHere(wavelength);
    cout << film.intercept(inray).transpose()<< endl;
    cout << inray.rebase()<< endl;

    film.getParameter(name , param);
    film.getHelpstring(name, strHelp);
    cout << param.value << " " << param.multiplier << "  " <<  name+"  "+strHelp << endl;
    film.getParameter(name="phi" , param);
    film.getHelpstring(name, strHelp);
    cout << param.value << " " << param.multiplier << "  " <<  name+"  "+strHelp << endl;
    film.getParameter(name="psi" , param);
    film.getHelpstring(name, strHelp);
    cout << param.value << " " << param.multiplier << "  " <<  name+"  "+strHelp << endl;

    return 0;
}
