/**
 *************************************************************************
*   \file           fractalsurface.cpp
*
*   \brief             auxiliary class and functions to generate random surfaces with prescribed fractal statistics
*
*
*
*   \author             François Polack  <francois.polack@synchroton-soleil.fr>
*   \date               Creation: 2024-04-29
*   \date               Last update: 2024-04-29
 ***************************************************************************/#include "fractalsurface.h"
#include "fractalsurface.h"
#include "wavefront.h"
#include <random>
#include <cstring>
#define NFFT_PRECISION_DOUBLE
#include <nfft3mp.h>

extern void Init_Threads();

using namespace Eigen;

const std::vector<int> FractalSurface::prim23={64,72,81,96,108,128,144,162,192,216,243,256,
             288,324,384,432,486,512,576,648,729,768,864,972,1024,1152,1296,1458,1536,1728,1944,2048,
             2187,2304,2592,2916,3072,3456,3888,4096,4374,4608,5184,5832,6144,6561,6912,7776,8192};

/** \brief helper functor for masking arrays
 *
 *  Note that Eigen requires (for vectorization sake) that mask and value have the same type
 */
template<typename Scalar_>  struct maskValid
{
  const Scalar_ operator()(const Scalar_& value, const Scalar_& mask) const { return mask==0 ? 0 : value; }
};


FractalSurface::FractalSurface()
{
  //  printf("prim 23 vector size %d  [%d, %d] \n", (int) prim23.size(), prim23[0], prim23[prim23.size()-1]);
    fracParms.nx=fracParms.ny=1;
    fracParms.exponent_x=new double[1];
    fracParms.exponent_y=new double[1];
    *fracParms.exponent_x=*fracParms.exponent_y=-1;
    fracParms.frequency_x=fracParms.frequency_y=NULL;
}

FractalSurface::~FractalSurface()
{
    delete [] fracParms.exponent_x;
    delete [] fracParms.exponent_y;
    if(fracParms.frequency_x) delete[] fracParms.frequency_x;
    if(fracParms.frequency_y) delete[] fracParms.frequency_y;
}



void FractalSurface::setXYfractalParams(const char* axe, const int N, const double *exponents, const double *frequencies)
{
    double *XYexp=0,  *XYfreq=0;
    if(stricmp(axe, "X")==0)
    {
//        std::cout << "Setting X fractal parameters\n";
        if(fracParms.nx != N)
        {
//            std::cout << "updating X size\n";
            delete [] fracParms.exponent_x;
            fracParms.exponent_x = new double[N];
            if(fracParms.frequency_x)
                delete[] fracParms.frequency_x;
            fracParms.frequency_x= new double[N-1];
            fracParms.nx=N;;
        }
        XYexp=fracParms.exponent_x;
        XYfreq=fracParms.frequency_x;

//        printf("Xfrequencies:  %p %p \n",XYfreq, fracParms.frequency_x);
    }
    else if(stricmp(axe, "Y")==0)
    {
//        std::cout << "Setting Y fractal parameters\n";
        if(fracParms.ny != N)
        {
            delete [] fracParms.exponent_y;
            fracParms.exponent_y = new double[N];
            if(fracParms.frequency_y)
                delete[] fracParms.frequency_y;
            fracParms.frequency_y= new double[N-1];
            fracParms.ny=N;;
        }
        XYexp=fracParms.exponent_y;
        XYfreq=fracParms.frequency_y;

//        printf("Yfrequencies:  %p %p \n",XYfreq, fracParms.frequency_y);
    }
    else
        throw ParameterException(string("Invalid axis name : ") +  axe + ", in " , __FILE__, __func__, __LINE__);

    if (N>1)
    {
//        std::cout << frequencies[0] << std::endl;
//        std::cout << XYfreq[0] << " \n ";
//        std::cout << "copying " << N-1 << "frequencies\n";
        memcpy(XYfreq, frequencies, sizeof(double)*(N-1));
//        std::cout << XYfreq[0] << std::endl;
    }
    bool wflag=false;
    for(int i=0; i < N; ++i)
    {
        if(exponents[i] > 0 ) wflag=true;
        XYexp[i]=exponents[i];
    }
    if(wflag)
        throw ParameterWarning("One at least of the exponents is  > 0 in ", __FILE__, __func__, __LINE__);
}

ArrayXXd FractalSurface::generate(int32_t xSize, double xStep, int32_t ySize, double yStep)
{

    //frequency steps unit =1 /distance unit= 1/(N*step)

    int ftNx=span(xSize+10);  // add some points to avoid the periodisation of FT
    int ftNy=span(ySize+10);
//    std::cout << "preparing a fourier transform of size " << ftNx << " x " << ftNy << std::endl;

    VectorXd xFilter=frequencyFilter(ftNx, xStep, fracParms.nx, fracParms.exponent_x, fracParms.frequency_x);
    RowVectorXd yFilter= frequencyFilter(ftNy, yStep, fracParms.ny, fracParms.exponent_y, fracParms.frequency_y);

    double sig=1./(xFilter.norm()*yFilter.norm()*sqrt(ftNx*ftNy));
    std::normal_distribution<double> normalrnd(0.,sig); //mean=0, sigma=1. après filtrage
    std::random_device rdsource;
    auto gauss = [&]() {return normalrnd(rdsource);};


    //NFFT a besoin de la matrice X des positions x,y des echantillons ( ici réguliers) dans [-1/2 ,  1/2 [
    ArrayXd xpos= ArrayXd::LinSpaced(ftNx+1,-0.5,0.5); // for convenience but the last point must not be used
    ArrayXd ypos= ArrayXd::LinSpaced(ftNy+1,-0.5,0.5);


    nfft_plan plan;
    nfft_init_2d(&plan,ftNy,ftNx,ftNx*ftNy);

    // we map here the x and y reduced coordinates of the datapoints
    Map<ArrayXXd,0, InnerStride<2> > mapX(plan.x, ftNx,ftNy) ;  //fixed inner stride=2 for the interlaced X and Y values
    Map<ArrayXXd,0, InnerStride<2> > mapY(plan.x+1, ftNx,ftNy) ;
    mapX.colwise()=xpos.segment(0,ftNx);
    mapY.rowwise()=ypos.segment(0,ftNy).transpose();

    // compute psi for the given x values
    if(plan.flags & PRE_ONE_PSI)
        nfft_precompute_one_psi(&plan);

    // then we map the surface and Fourier arrays
    Map<ArrayXXd, 0, InnerStride<2> > mapSurf((double*)plan.f, ftNx, ftNy);  //this is the real part of the complex f array
    Map<ArrayXXd, 0, InnerStride<2> > mapSurfIm((double*)plan.f +1, ftNx, ftNy);
    mapSurf=ArrayXXd::NullaryExpr(ftNx, ftNy, gauss); // initialise the real part with a random gaussian distribution
    mapSurfIm.setZero();        //set the imaginary part to 0

    Map<ArrayXXcd> mapFT((std::complex<double>*)plan.f_hat,ftNx,ftNy);  // map the complex Fourier array

    // compute the Fourier coefficient of the initial random surface
    nfft_adjoint(&plan);

    mapFT.colwise()*=xFilter.array();
    mapFT.rowwise()*=yFilter.array();

    nfft_trafo(&plan);

    // clip to requested size and return
    Index xorg=(ftNx-xSize)/2, yorg=(ftNy-ySize)/2;
    ArrayXXd surface=mapSurf.block(xorg,yorg, xSize, ySize);

    nfft_finalize(&plan);  // ceci desalloue toute la structure plan
//    std::cout << "sigma=" << sqrt(surface.square().matrix().sum()/(surface.rows()*surface.cols()) )<< std::endl;
//    std::cout << "sigma2=" << sqrt(surface.square().sum()/(surface.rows()*surface.cols()) )<< std::endl;
//    std::cout << "sigma=" << surface.matrix().norm()/sqrt(surface.rows()*surface.cols()) << std::endl;
    return surface;
}


ArrayXXd FractalSurface::detrend(ArrayXXd& surface, const Ref<ArrayXXd>& mask)
{
    int Nx=mask.rows(), Ny=mask.cols();
    if (Nx >= surface.rows() || Ny >= surface.cols() )
        throw ParameterException(string("Bad dimensions: the internal surface size doesn't allow the requested Legendre fit, in "),
                                        __FILE__, __func__, __LINE__);
    ArrayXXd Lcoeffs=LegendreFitGrid(Nx,Ny,surface);
//    std::cout << "legendre fit;\n" << Lcoeffs << std::endl;
    ArrayXXd maskedC=Lcoeffs.binaryExpr(mask, maskValid<double>());
//    std::cout << "maskedC applied;\n" << maskedC << std::endl;
    surface-= LegendreSurfaceGrid(surface.rows(), surface.cols(), maskedC);
    //return LegendreFitGrid(Nx,Ny,surface); // only for test otherwise it would be better to subtract the masked values
    return Lcoeffs-maskedC;
}


VectorXd FractalSurface::frequencyFilter(int N, double dstep, int nseg, double * exponent, double *ftrans)
{
    int l2=N/2;
    int n2=(N%2==1)?l2:l2-1;
    VectorXd filter(N);
//    Map<VectorXd, 0, InnerStride<> > negpart(filter.data()+n2, l2, InnerStride<>(-1) );
//    Map<VectorXd> pospart(filter.data()+n2, n2);
//    double coeff[nseg];
//    int ntrans[nseg];
//    coeff[0]=1.;
//    int i;
//    for (i=0; i<nseg; ++i)
//    {
//        ntrans[i]=N*dstep*ftrans[i];
//        coeff[i+1]=coeff[i]*pow(ntrans[i], exponent[i]-exponent[i+1]);
//    }
    double coeff=1.;

    double *ppos, *pneg=ppos=filter.data()+l2;
    *pneg=0;
    int n=0;
    for(int iseg=0; iseg< nseg; ++iseg)
    {
        double fmax=0;
        int nmax=n2;
        if(iseg <nseg -1)
        {
            fmax=N*dstep*ftrans[iseg];
            if(fmax < n2)
                nmax=int(fmax);
//            std::cout << iseg << "  " << fmax ;
        }

//        std::cout << "  " << nmax << std::endl;
        while (n <=nmax )
        {
            ++n, ++ppos, --pneg;
            *pneg=*ppos=coeff*pow(n,exponent[iseg]);

        }
        if(iseg < nseg-1)
            coeff*=pow(fmax, exponent[iseg]-exponent[iseg+1]);
//        std::cout << iseg << "  " << coeff << std::endl;
    }
    if(l2!=n2)
        filter[0]=coeff*pow(l2,exponent[nseg-1]);
    return filter;
}
