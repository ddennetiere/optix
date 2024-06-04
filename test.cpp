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



#include <iostream>
#include <fstream>
#include <chrono>

#include "opticalelements.h"
#include "sources.h"
#include "gratingbase.h"
#include "holo.h"
#include "files.h"
#include <sstream>
#include "interface.h"
#include "wavefront.h"

//#include "heightmap.h"
#include "fractalsurface.h"
#include "bidimspline.h"
#include "surfacegenerator.h"

#include "Polygon.h"
#include "Ellipse.h"
#include "ApertureAPI.h"

using HR_clock = std::chrono::high_resolution_clock ;
//#define POSTFIX(X, P) X#P
//#define M_PIl POSTFIX(M_PI, L)
//#include <unsupported\Eigen\CXX11\src\Tensor\TensorBase.h>
//#include <unsupported\Eigen\CXX11\src\Tensor\Tensor.h>




/** \brief helper functor to extract the coefficient-wise minimum value of two arrays
 */
template<typename Scalar_>  struct minOf2Op
{
  const Scalar_ operator()(const Scalar_& x, const Scalar_& y) const { return x < y ? x : y; }
};


using namespace std;
int OriginalTest();
int SolemioTest();
int Solemio2Xml(string filename);
int DiscoTest();
int SphereTest();
int QuickTest();
int TestEllipse();
int XmlTest();
int TstMistral();

int main()
{
    char * version_string;
    Version(&version_string);
    cout << version_string<<endl;
    char* strerr=NULL; // pointeur à la chaine source
    bool erreur=GetOptiXError(&strerr);
    size_t msglen=strlen(strerr);
    char message[msglen+1]; // create a copy buffer with space for 0 ant end
    strncpy(message,(char*)strerr, msglen ); //copy the message  (mandatory; strerr cannot be printed directly)
    cout << "Error level  " << erreur << " message: "<< message << endl << endl;

    cout <<"size of bool " << sizeof(bool) <<endl;
    cout << "size of int " << sizeof(int) << endl;
    cout << "true " << true << "  false " << false <<endl;
    cout << "starting \n";

    FractalSurface fracSurf;

   // int n=6145;

   // cout << n << " span=" << fracSurf.span(n) << endl;

//    double fracFrequencies[]={200.};
//    double fracExponents[]={-1., -1.5 };
//    VectorXd Filter=fracSurf.frequencyFilter(45,0.001,2,fracExponents,fracFrequencies);
//    cout << endl << Filter.transpose() << endl;
//    fstream fracfile("fractalout.dat", ios::out | ios::binary);
//    if(!fracfile.is_open())
//    {
//        SetOptiXLastError("Can't open the file for writing the fractal filter",__FILE__,__func__);
//        return -1;
//    }
//
    double xexp[]={-1.5,-2.};
    double yexp[]={-1.};
    double xfl[]={100.};
    cout << "initiating surface\n";

    fracSurf.setXYfractalParams("X",2,xexp,xfl);
//    cout << "fractX set\n";
    fracSurf.setXYfractalParams("Y",1,yexp,NULL);

//    cout << "fractal parameters set\n";
    ArrayXXd surface = fracSurf.generate(501,0.001, 201, 2e-4);

    SurfaceToFile(surface, "mapTest.bin");

    ArrayXXd Lcoeffs=LegendreFitGrid(6,5, surface);
    ArrayXXd LNcoeffs=LegendreNormalize(Lcoeffs);
    ArrayXXd recoeffs=LegendreFromNormal(LNcoeffs);

    cout << endl <<Lcoeffs << endl;
    cout << endl << LNcoeffs  << endl;
    double sigma2= surface.matrix().squaredNorm()/surface.rows()/surface.cols();
    cout << "sigma =" << sqrt(sigma2) << "   sigma corr=" << sqrt(sigma2-LNcoeffs(0,0)*LNcoeffs(0,0)-LNcoeffs(1,0)*LNcoeffs(1,0)-LNcoeffs(0,1)*LNcoeffs(0,1)) << endl;

    ArrayXXd detCoeffs=Lcoeffs.topLeftCorner(2,2);
    detCoeffs(1,1)=0;

    ArrayXXd detrended= surface- LegendreSurfaceGrid(surface.rows(), surface.cols(), detCoeffs);
    cout << "detrended sigma=" << sqrt(detrended.matrix().squaredNorm()/detrended.rows()/detrended.cols()) << endl;

    ArrayXXd mask=ArrayXXd::Zero(Lcoeffs.rows(), Lcoeffs.cols());
    mask(0,0)=mask(1,0)=mask(0,1)=1.;

    Lcoeffs= fracSurf.detrend(surface, mask);
    LNcoeffs=LegendreNormalize(Lcoeffs);

    SurfaceToFile(surface,"detrendMapTest.bin");
    cout << endl <<Lcoeffs << endl;
    cout << endl << LNcoeffs  << endl;

    cout << "Fractalsurf memory size " << sizeof(fracSurf) << endl;

    Array22d limits;
    limits << -250, -10, 250, 10;
    BidimSpline Sspline ;

    cout << "size after initializing " << Sspline.controlValues().size() <<endl;
    cout <<"Memorysize " << sizeof(Sspline) << endl;;

    Sspline.setFromGridData(limits,surface);
    cout << "\nSpline interpolator computed\n limits are\n" << Sspline.getLimits() <<endl;
    cout << Sspline(-250,-10) << endl;
    MatrixXd derivx, derivy;
    {
        ArrayXd xval(10), yval(10);
        xval << 241,242,243,244, 245,246,247,248,249,250;
        yval << 9.1, 9.2, 9.3, 9.4, 9.5,9.6,9.7,9.8,9.9,10;

        cout << "input values\n";
        cout << surface.bottomRightCorner(10,10) << endl;
        cout << "interpolated values\n";
        cout << Sspline.interpolator(X, xval, derivx).transpose()* Sspline.controlValues()*Sspline.interpolator(Y, yval, derivy ) <<endl;
    }
    ArrayXd xval=ArrayXd::LinSpaced(501, -250,250);
    ArrayXd yval=ArrayXd::LinSpaced(201, -10,10);
    cout << "new x,y values set\n ";
    MatrixXd interx=Sspline.interpolator(X, xval, derivx);
    MatrixXd intery=Sspline.interpolator(Y, yval, derivy );

    surface= interx.transpose()* Sspline.controlValues()*intery;
    SurfaceToFile(surface,"interpol.bin");
    surface= derivx.transpose()* Sspline.controlValues()*intery;
    SurfaceToFile(surface,"derivX.bin");
    interx=intery=MatrixXd();
    cout <<"size after reinitializing " << interx.size() << "  " << intery.size() << endl << endl;

    SurfaceErrorGenerator generator;
//    return 0;
     // heightmap was removed from optics project 22/05/2024
//    HeightMap hmp;
//
//    hmp.fromModel("D:\\projets\\projetsCB\\OptiX\\surfaceDB\\SESO_ALBA-fit_1,7.crms", 0.05, 0.04, 0.001);
//    return (0);

  //  return Solemio2Xml("D:\\Documents SOLEIL\\Dossiers-Lignes\\Disco\\Solemio\\DISCOdefinitif.sole");
//    return Solemio2Xml("D:\\Documents SOLEIL\\Lignes externes\\APE\\Solemio-LE\\APELE-R1 700T VREF");
//    return Solemio2Xml("D:\\Documents SOLEIL\\Lignes externes\\APE\\APE-New\\APELE-R2 1200T  Gauss-aligned");

//    return Solemio2Xml("D:\\projets\\projetsCB\\OptiX\\solemio\\CASSIOPEE");

  // return DiscoTest();
  //  return TstMistral();

    bool result=LoadSystemFromXml("polytestcopy.xml");
    cout << "file loaded " << (result ? "OK\n" : "ERROR\n");
//    LoadSystemFromXml("Beamline_test_polynomial.xml");
//    size_t idM1;
//    FindElementID("M1",&idM1);
//    int index;
//    AddRectangularStop(idM1,0.1, 0.03,0,0,0,0,&index);
//    cout << "Rectangular region " << index << " successfully added\n";
//    AddEllipticalStop(idM1,0.15, 0.03,0, 0,0,0,&index);
//    cout << "Elliptical region " << index << " successfully added\n";


    string  outfile = "polytestcopy2.xml";
    cout << "saving to " << outfile << endl;
    result=  SaveSystemAsXml(outfile.c_str());
    cout << "file saved " << (result ?  "OK\n" : " ERROR\n");

    return 0;


    ArrayXd v1(7), v2(7), v3;
    v1 << 4,8,5.2,8.5, 9,14,18 ;
    v2 << 3,9.5,7, 13.2, 6, 15, 19;
    cout << v1.transpose() << endl <<v2.transpose()<< endl;

    v1= v1.binaryExpr(v2, minOf2Op<double>());

    cout << v1.transpose() << endl <<endl;
    cout << "sizeof(UnitType) " << sizeof(UnitType) << endl ;
    cout << "sizeof(ParameterGroup) " <<  sizeof(ParameterGroup) << endl << endl;

//    return SolemioTest();
//    return OriginalTest();
//    return SphereTest();
//    return QuickTest();
//    return TestEllipse();   // modifié pour test grating
    cout << "sizeof(bool) " << sizeof(bool) << endl ;
     Polygon rectangle;
     bool cvx;
     Array2d point, trans,vect;
     point << 0.,3.1;
     trans << 0.5, 0;

     rectangle.setRectangle(1., 1., 0, 0);
     rectangle.move(0,trans);
     cvx=rectangle.isConvex();
     rectangle.dump();
     if(cvx)
        cout << "shape is convex; is origin inside ? :" << rectangle.locate(point)<<endl;
     else
        cout << "shape is NOT convex;; is origin inside ? :" << rectangle.locate(point)<<endl;
    for(point(1)=-0.1; point(1) <0.1; point(1)+= 0.01)
    {
        for(point(0)=-0.1; point(0) < 0.1 ; point(0)+= 0.01)
            cout << rectangle.locate(point) << "  ";

        cout << endl;
    }


     rectangle.insertVertex(2, 0,1);
     cvx=rectangle.isConvex();
     rectangle.dump();
     if(cvx)
        cout << "shape is convex; is origin inside ? :" << rectangle.locate(point)<<endl;
     else
        cout << "shape is NOT convex; is origin inside ? :" << rectangle.locate(point)<<endl;

     rectangle.changeVertex(3, 0,4);
     cvx=rectangle.isConvex();
     rectangle.dump();
     if(cvx)
        cout << "shape is convex; is origin inside ? :" << rectangle.locate(point)<<endl;
     else
        cout << "shape is NOT convex; is origin inside ? :" << rectangle.locate(point)<<endl;

     rectangle.deleteVertex(3);
     cvx=rectangle.isConvex();
     rectangle.dump();
     if(cvx)
        cout << "shape is convex; is origin inside ? :" << rectangle.locate(point)<<endl;
     else
        cout << "shape is NOT convex; is origin inside ? :" << rectangle.locate(point)<<endl;

     rectangle.move(M_PI_2, trans);
     cvx=rectangle.isConvex();
     rectangle.dump();
     if(cvx)
        cout <<endl << "shape is convex; is origin inside ? :" << rectangle.locate(point)<<endl;
     else
        cout << "shape is NOT convex; is origin inside ? :" << rectangle.locate(point)<<endl;

    Polygon rect2=rectangle;
    cout <<endl;
    rect2.dump();
    cout << "\nellipse\n";

    Ellipse ellipse(0.2, 0.5, 0.3, 0.2,  M_PI/6.); // 0); //

    double a,b, x0, y0, theta;
    ellipse.getParameters(&a, &b, &x0, &y0, &theta);
    ellipse.dump();

    cout << "\ndemi axes:" <<a << ", " << b << endl;
    cout << "centre:   " <<x0 <<", "<< y0 << endl;
    cout << "angle:    " << theta*180/M_PI << endl;

    point <<0,0;
    vect << -1,-1;
    ellipse.move(-M_PI/18.,vect);
    ellipse.getParameters(&a, &b, &x0, &y0, &theta);
    ellipse.dump();

    cout << "\ndemi axes:" <<a << ", " << b << endl;
    cout << "centre:   " <<x0 <<", "<< y0 << endl;
    cout << "angle:    " << theta*180/M_PI << endl;


}

int OriginalTest()
{
//
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
    LegendrePolynomialMirror lpMirror("LPM1");

    cout <<"LPM1: surface class " << lpMirror.getSurfaceClass() << endl;
    size_t paramsize=0;
    lpMirror.getParameterArraySize("surfaceLimits", &paramsize );
    cout << "size of limits array =" << paramsize << endl;
    lpMirror.getParameterArraySize("coefficients", &paramsize );
    cout << "size of coefficients array =" << paramsize << endl;

    Parameter aparam(2,2, Distance, ShapeGroup); // this creates an array parameter of size 2,2
//    double * pdata=aparam.paramArray->data;
//    lpMirror.dumpParameter("surfaceLimits", aparam);
    cout << "size of Parameter struct " << sizeof(Parameter) <<endl;
    MemoryDump((void*) &aparam, sizeof(Parameter));

    double data[4];
    data[0]=-.1;
    data[1]=+.1;
    data[2]=-.1;
    data[3]=+.1;

    cout << "aligment(double)=" << alignof(double) <<endl;
    cout << "aligment(size_t)=" << alignof(size_t) <<endl;
    cout << "aligment(Maxtrix4d)=" << alignof(Matrix4d) << endl;
    cout << "aligment(MaxtrixXd)=" << alignof(MatrixXd) << endl;

    char *errstr;
    size_t lpmID;
    if(!CreateElement("LegendrePolynomialMirror", "LPM2", &lpmID))
    {
        GetOptiXError(&errstr);
        cout <<"could not create LegendrePolynomialMirror LPM2\n" << errstr <<endl;
        return 0;
    }
    if(!DumpParameter(lpmID,"surfaceLimits"))
    {
        GetOptiXError(&errstr);
        cout <<"could not first dump parameter reason\n" <<errstr << endl;
    }
    cout <<" get surfaceLimits parameter\n";
    Parameter lparam;
    uint32_t flags;
    GetParameterFlags(lpmID,"surfaceLimits",&flags);
    if( flags & 0X08)
    {
        int64_t dims[2];
        GetParameterArrayDims(lpmID,"surfaceLimits",&dims);
        lparam.paramArray=new ArrayParameter;
        lparam.paramArray->dims[0]=dims[0];
        lparam.paramArray->dims[1]=dims[1];
        lparam.paramArray->data=new double[dims[0]*dims[1]];
        lparam.flags=9;
        cout << dims[0] << " x " <<dims[1] <<endl;
        DumpArgParameter(&lparam);
        if( !GetArrayParameter(lpmID,"surfaceLimits",&lparam, dims[0]*dims[1]))
        {
            GetOptiXError(&errstr);
            cout <<"cannot get parameter reason\n" <<errstr << endl;
        }

    }
    else
        cout << "surfaceLimits is not an array\n";
    cout << "surfaceLimits is\n";
    DumpArgParameter(&lparam);

    delete [] lparam.paramArray->data;
    lparam.paramArray->data=data;

    cout << "setting new surfaceLimits\n";
    SetParameter(lpmID,"surfaceLimits", lparam);


//    SetArray Parameter was removed from the C API
//    if(!SetArrayParameter(lpmID, "surfaceLimits", 2,2, data ))
//    {
//        GetOptiXError( &errstr);
//        cout <<"could not set array parameter reason\n" <<errstr << endl;
//    }
     if(!DumpParameter(lpmID,"surfaceLimits"))
    {
        GetOptiXError(&errstr);
        cout <<"could not last dump parameter reason\n" <<errstr << endl;
    }
     if(!DumpParameter(lpmID,"coefficients"))
    {
        GetOptiXError(&errstr);
        cout <<"could not =dump parameter 'coefficients';  reason\n" <<errstr << endl;
    }
 //   lpMirror.dumpParameter("surfaceLimits", aparam);



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
    param.value=500;
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
    {
        HR_clock clock;
        HR_clock::time_point start(clock.now());

        source.alignFromHere(wavelength);
        source.generate(wavelength);
        source.radiate();

        cout << "Ray tracing; duration " << chrono::duration_cast<chrono::milliseconds>(clock.now()-start).count() << " msec\n" ;
    }
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
        Diagram spotDg(5);
       // PlaneFilm screenCopy(film2);
        film2.getSpotDiagram(spotDg,0.00);
        spotfile << spotDg;
        spotfile.close();
    }

    {
        fstream causticFile("diagram.cdg", ios::out | ios::binary);
        Diagram caustic(4);
        int n= film2.getCaustic(caustic) ;
        cout << " caustic of " << n << " points\n";
        causticFile << caustic;
        causticFile.close();
    }
    if(0){
        fstream WfFile("wavederiv.sdg", ios::out | ios::binary);
        Diagram WFdata(5); // dim à revoir
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

        HR_clock clock;
        HR_clock::time_point start(clock.now());


        film2.computeOPD(0, 7,7);
        HR_clock::time_point midtime(clock.now());
        cout << "OPD computed; duration " << chrono::duration_cast<chrono::milliseconds>(midtime-start).count() << " msec\n" ;

//        ArrayXXcd Psf;
        nx=ny=500;
        int nplane=51;
        Array2d pixelSize;
        pixelSize <<  2e-7, 2e-7;
        // la profondeur de Rayley à 1 nm ouverture 1 mead est env 0.5 mm
        ArrayXd zOffsets=ArrayXd::LinSpaced(nplane, -2e-2,2e-2);
        ndArray<complex<double>,4> Psf((size_t)nx,(size_t)ny,(size_t)nplane,2ull);
        film2.computePSF(Psf, pixelSize, zOffsets, wavelength);
       // ndArray<complex<double>,3> Psf(nx,ny,2);
      //  film2.computePSF(Psf,pixelSize, wavelength); //, nx,ny);
        cout << "PSF computed; duration " << chrono::duration_cast<chrono::milliseconds>(clock.now()-midtime).count() << " msec\n" ;

        WfFile.open("film2.psf", ios::out | ios::binary);
        WfFile.write((char*)&nx, sizeof(int));
        WfFile.write((char*)&ny, sizeof(int)) ;
        WfFile.write((char*)&nplane,sizeof(int));
        WfFile.write((char*)pixelSize.data(), 2*sizeof(double )) ;
        WfFile.write ((char*)Psf.data(), nx*ny*nplane*4*sizeof(double));
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
        PlanePoly1DGrating testGrating("testGrating");

        testGrating.getParameter("degree", param);
        cout << "Default degree for test grating is " << param.value << endl ;
        param.value=2;
        testGrating.setParameter("degree", param);
        cout << "Default degree for test grating is " << param.value << endl ;
        testGrating.getParameter("degree", param);
        cout << "Default degree for test grating is " << param.value << endl ;

        testGrating.getParameter("lineDensity", param);
        cout << endl << "Default line density for test grating is " << param.value << endl ;
        param.value=2e6;
        testGrating.setParameter("lineDensity",param);
        cout << endl << "Line density test grating set to "<< param.value << "m^-1 \n\n";
        testGrating.getParameter("lineDensity", param);
        cout << endl << "Default line density for test grating is " << param.value << endl ;
    }

    if(1)
    {
        Parameter param;
        PlaneHoloGrating hGrating("holoGrating", & film2);
        hGrating.getParameter("distance", param);
        cout << "distance of Holo grating from film2" << param.value << "m\n";

        hGrating.getParameter("recordingWavelength", param);
        cout << "Default recording wavelength for holo grating is " << param.value << endl ;
        // left unchanged = 351.1

        hGrating.getParameter("lineDensity", param);
        cout << endl << "Default line density for holo grating is " << param.value << endl ;
        param.value=6e5;
        hGrating.setParameter("lineDensity",param);
        cout <<  "Line density of holo grating set to "<< param.value << "m^-1 \n\n" <<endl;
        hGrating.getParameter("lineDensity", param);
        cout <<  "Actual line density for holo grating is " << param.value << endl ;

        hGrating.getParameter("inverseDist1", param);
        param.value=-0.2002547;
        hGrating.setParameter("inverseDist1", param);
        cout <<  "inverseDist1 of holo grating set to "<< param.value << "m^-1 \n\n" <<endl;

        hGrating.getParameter("inverseDist2", param);
        param.value=-0.11111109999999999;
        hGrating.setParameter("inverseDist2", param);
        cout <<  "inverseDist2 of holo grating set to "<< param.value << "m^-1 \n\n" <<endl;

        hGrating.getParameter("elevationAngle1", param);
        param.value=-0.6457718399328866;
        hGrating.setParameter("elevationAngle1", param);
        cout <<  "elevationAngle1 of holo grating set to "<< param.value << "m^-1 \n\n" <<endl;

        source.alignFromHere(wavelength);

        GratingPatternInfo gratInfo;
        hGrating.getPatternInfo(0.06, 0.01, &gratInfo);

        cout << "\n HOLO GRATING \n    central line density " << gratInfo.AxialLineDensity[0] << " m^-1\n";
        cout  << "     linear   VGD : " << gratInfo.AxialLineDensity[1] << " m^-2\n";
        cout  << "     2nd order VGD: " << gratInfo.AxialLineDensity[2] << " m^-3\n";
        cout  << "     3rd order VGD: " << gratInfo.AxialLineDensity[3] << " m^-4\n";

        cout << "      central line angle: " << gratInfo.lineTilt << " rd\n";
        cout << "      central line radius: " << gratInfo.lineCurvature << " m\n";


    }

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
        Diagram spotDg(5);
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
        cout << "could not open file 'SystemSave.data'  for output\n";

    if(0)
    {
      cout << "\n---------------------------------------------------------------------------\n\n";

 //   Parameter param;
//    ReadSolemioFile("R:\\Partage\\SOLEMIO\\DEIMOS-cpy.txt");
//    ReadSolemioFile("R:\\Partage\\SOLEMIO\\CASSIOPEE");
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

inline void SetParamValue(size_t ID,string parmName, double value)
{
    Parameter parm;
    if(!GetParameter(ID, parmName.c_str(),&parm) )
        throw std::runtime_error("invalid object ID or parameter");
    parm.value=value;
    SetParameter(ID,parmName.c_str(),parm);
}

int TestEllipse()
{
    char *errstr;

    size_t sourceID, hfmID, vfmID, screenID; //, gratID;
    CreateElement("Source<Gaussian>", "source", &sourceID);
    CreateElement("Mirror<ConicBaseCylinder>", "hfm", &hfmID);
    CreateElement("Mirror<ConicBaseCylinder>", "vfm", &vfmID);
//    gratID=CreateElement("Grating<Poly1D,Plane>", "grating");  //just for alignment test
    CreateElement("Film<Plane>", "screen", &screenID);
    ChainElement_byID(sourceID, hfmID);
    ChainElement_byID(hfmID,vfmID);
 //   ChainElement_byID(vfmID,gratID);
    ChainElement_byID(vfmID,screenID);


    SetParamValue(sourceID,"nRays", 50000.);
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

//    SetParamValue(gratID, "distance", .3);
//    SetParamValue(gratID, "lineDensity", 1.e+6);
//    SetParamValue(gratID, "order_align",  1);
//    SetParamValue(gratID, "order_use", 1);
//    SetParamValue(gratID, "theta", 1.5*M_PI/180.);
//

    SetParamValue(screenID, "distance", 1.5);

    double lambda=6.e-9;
    cout << "calling align on source\n";
    if(!Align(sourceID, lambda))
    {
       GetOptiXError( &errstr);
       cout << "Alignment error : " << errstr << endl;
       return -1;
    }

    if(!Generate(sourceID, lambda))
    {
       GetOptiXError( &errstr);
       cout << "Source generation error : " << errstr << endl;
       return -1;
    }
    size_t elemID;
    cout << "getting screen \n";
    if(!FindElementID("screen",&elemID))
    {
        cout << "element 'screen' was not found\n";
        return -1;
    }
    Surface* screen=dynamic_cast<Surface*> ((ElementBase*)elemID);
    cout << screen << endl;

    if(!Radiate(sourceID))
    {
       GetOptiXError( &errstr);
       cout << "Radiation error : " << errstr << endl;
       return -1;
    }

    Diagram spotDg(5);

    cout << "\nIMPACTS\n";
    int ncounts=screen->getSpotDiagram(spotDg,0);
    if(ncounts)
    {
        for(int i=0; i<5 ; ++i)
           cout << spotDg.m_min[i] << " \t" << spotDg.m_max[i] << " \t" << spotDg.m_sigma[i] << endl;

        fstream spotfile("EllipseSpotdiag.sdg", ios::out | ios::binary);
        spotfile << spotDg;
        spotfile.close();


        cout << endl << endl;
    }




    return 0;
}
