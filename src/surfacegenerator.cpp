/**
 *************************************************************************
*   \file           surfacegenerator.cpp
*
*   \brief             implementation file
*
*
*
*   \author             François Polack  <francois.polack@synchroton-soleil.fr>
*   \date               Creation: 2024-05-21
*   \date               Last update: 2024-05-23
 ***************************************************************************/

#include "surface.h"
#include "surfacegenerator.h"

ArrayXXd SurfaceErrorGenerator::generate()
{
    if (m_nonZsigma <=0 || (m_limits.matrix()==Matrix2d::Zero()) || (m_sampling.matrix()==Vector2d::Zero()) )
        throw ParameterException(string("The surface error generator is not properly initialized, at "),
                                            __FILE__, __func__, __LINE__);

    Index xpoints(round((m_limits(0,1)-m_limits(0,0))/m_sampling(0)));
    Index ypoints(round((m_limits(1,1)-m_limits(1,0))/m_sampling(1)));
    double xstep=(m_limits(0,1)-m_limits(0,0))/xpoints++;
    double ystep=(m_limits(1,1)-m_limits(1,0))/ypoints++;

    int Nx=Legendre_ubound.rows() > m_detrendMask.rows() ? Legendre_ubound.rows() : m_detrendMask.rows();
    int Ny=Legendre_ubound.cols() > m_detrendMask.cols() ? Legendre_ubound.cols() : m_detrendMask.cols();
        if (Nx >= xpoints || Ny >= ypoints )
            throw ParameterException(string("Bad dimensions: the internal surface size doesn't allow the requested Legendre fit, in "),
                                            __FILE__, __func__, __LINE__);

    ArrayXXd surface=m_fractalSurf.generate(xpoints, xstep, ypoints, ystep);

    if(Nx*Ny==0)
        return surface;

    ArrayXXd mask=ArrayXXd::Zero(Nx,Ny);
    if(Legendre_ubound.size())
        mask.topLeftCorner(Legendre_ubound.rows(), Legendre_ubound.cols())=Legendre_ubound;
    if(m_detrendMask.size())
        mask.topLeftCorner(m_detrendMask.rows(),m_detrendMask.cols())+=m_detrendMask;

    m_fractalSurf.detrend(surface,m_detrendMask);
    //scale the detrended surface to match the given sigma value
    surface *= m_nonZsigma/surface.matrix().norm();

    if(Legendre_ubound.size())
    {
        MatrixXd  Lcoeffs= Legendre_ubound * ArrayXXd::Random(Legendre_ubound.rows(), Legendre_ubound.cols());
        surface+= LegendreSurfaceGrid(surface.rows(), surface.cols(), Lcoeffs);
    }
    return surface;
}

#define XMLSTR (xmlChar*)
#define MAX(a,b) (a>b ? a:b)

void SurfaceErrorGenerator::operator>>(xmlNodePtr surfnode)
{
    const FractalParameters& fracparms=m_fractalSurf.getFractalParameters();
    int maxval=MAX(4,MAX(fracparms.nx, fracparms.ny));
    maxval=MAX(MAX(maxval, Legendre_ubound.size()),m_detrendMask.size());

    char buf[17*maxval], *pbuf;  // max length per number in %.8g =15 => max length 17 char/value

    xmlNodePtr errnode=xmlNewTextChild(surfnode,NULL,XMLSTR "error_generator", NULL); // no value
    xmlNewProp(errnode, XMLSTR "class", XMLSTR "SurfaceErrorGenerator");
    sprintf(buf,"%.8g, %.8g, %.8g, %.8g", m_limits(0,0), m_limits(1,0), m_limits(0,1), m_limits(1,1));
    xmlNewProp(errnode, XMLSTR "limits", XMLSTR buf);
    sprintf(buf,"%.8g, %.8g", m_sampling(0), m_sampling(1));
    xmlNewProp(errnode, XMLSTR "sampling", XMLSTR buf);
    sprintf(buf,"%.8g",  m_nonZsigma);
    xmlNewProp(errnode, XMLSTR "sigma", XMLSTR buf);

    xmlNodePtr parmnode=xmlNewTextChild(errnode,NULL,XMLSTR "detrend", NULL);
    sprintf(buf,"%Ld, %Ld", m_detrendMask.rows(), m_detrendMask.cols());
    xmlNewProp (parmnode, XMLSTR "dims", XMLSTR buf);
    pbuf=buf;
    pbuf+=sprintf(pbuf,"%.8g", m_detrendMask(0));
    for(Index i=1; i<  m_detrendMask.size(); ++i)
        pbuf+=sprintf(pbuf,", %.8g", m_detrendMask(i));
    xmlNewProp (parmnode, XMLSTR "values", XMLSTR buf);

    parmnode=xmlNewTextChild(errnode,NULL,XMLSTR "Legendre_max", NULL);
    sprintf(buf,"%Ld, %Ld", Legendre_ubound.rows(), Legendre_ubound.cols());
    xmlNewProp (parmnode, XMLSTR "dims", XMLSTR buf);
    pbuf=buf;
    pbuf+=sprintf(pbuf,"%.8g", Legendre_ubound(0));
    for(Index i=1; i<  Legendre_ubound.size(); ++i)
        pbuf+=sprintf(pbuf,", %.8g", Legendre_ubound(i));
    xmlNewProp (parmnode, XMLSTR "values", XMLSTR buf);

    parmnode=xmlNewTextChild(errnode,NULL,XMLSTR "fractal_parameters", NULL);

        xmlNodePtr fracnode=xmlNewTextChild(parmnode,NULL,XMLSTR "X_fractal", NULL);
        sprintf(buf,"%d",  fracparms.nx);
        xmlNewProp (fracnode, XMLSTR "n_segments", XMLSTR buf);
        pbuf=buf;
        pbuf+=sprintf(pbuf,"%.8g", fracparms.exponent_x[0]);
        for(Index i=1; i< fracparms.nx; ++i)
            pbuf+=sprintf(pbuf,", %.8g",fracparms.exponent_x[i]);
        xmlNewProp (fracnode, XMLSTR "exponents", XMLSTR buf);
        pbuf=buf;
        pbuf+=sprintf(pbuf,"%.8g", fracparms.frequency_x[0]);
        for(Index i=1; i< fracparms.nx-1; ++i)
            pbuf+=sprintf(pbuf,", %.8g",fracparms.frequency_x[i]);
        xmlNewProp (fracnode, XMLSTR "frequencies", XMLSTR buf);

        fracnode=xmlNewTextChild(parmnode,NULL,XMLSTR "Y_fractal", NULL);
        sprintf(buf,"%d",  fracparms.ny);
        xmlNewProp (fracnode, XMLSTR "n_segments", XMLSTR buf);
        pbuf=buf;
        pbuf+=sprintf(pbuf,"%.8g", fracparms.exponent_y[0]);
        for(Index i=1; i< fracparms.ny; ++i)
            pbuf+=sprintf(pbuf,", %.8g",fracparms.exponent_y[i]);
        xmlNewProp (fracnode, XMLSTR "exponents", XMLSTR buf);
        pbuf=buf;
        pbuf+=sprintf(pbuf,"%.8g", fracparms.frequency_y[0]);
        for(Index i=1; i< fracparms.ny-1; ++i)
            pbuf+=sprintf(pbuf,", %.8g",fracparms.frequency_y[i]);
        xmlNewProp (fracnode, XMLSTR "frequencies", XMLSTR buf);

}

void SurfaceErrorGenerator::operator<<(xmlNodePtr generatornode)
{
    const xmlChar* parentname=generatornode->parent->name;
    xmlChar* strbuf = xmlGetProp(generatornode, XMLSTR "class");
    if(xmlStrcmp(strbuf, XMLSTR "SurfaceErrorGenerator"))  // wern if class is not this one
        std::cout << "Error in Surface:" << parentname << " Generator class is " << strbuf << " while SurfaceErrorGenerator was expected \n";
    xmlFree(strbuf);

    strbuf = xmlGetProp(generatornode, XMLSTR "limits");
    if(!strbuf) //not found
        throw XmlFileException(string("In ") +(char*) parentname + " surface generator has no limits",
                               __FILE__, __func__, __LINE__);
    sscanf((char*)strbuf,"%lf, %lf, %lf, %lf", &m_limits(0,0), &m_limits(1,0), &m_limits(0,1), &m_limits(1,1));
    xmlFree(strbuf);


    strbuf = xmlGetProp(generatornode, XMLSTR "sampling");
    if(!strbuf) //not found
        throw XmlFileException(string("In ") +(char*) parentname + " surface generator has no sampling steps",
                               __FILE__, __func__, __LINE__);
    sscanf((char*)strbuf,"%lf, %lf", &m_sampling(0), &m_sampling(1));
    xmlFree(strbuf);

    strbuf = xmlGetProp(generatornode, XMLSTR "sigma");
    if(!strbuf) //not found
        throw XmlFileException(string("In ") +(char*) parentname + " surface generator has no RMS amplitude",
                               __FILE__, __func__, __LINE__);
    sscanf((char*)strbuf,"%lf", &m_nonZsigma);
    xmlFree(strbuf);

    //  process children nodes. If a property is not defined , the corresponding parameter will retain its default value

    xmlNodePtr curnode=xmlFirstElementChild(generatornode);
    while(curnode)
    {
        if(xmlStrcmp(curnode->name, XMLSTR "detrend")==0)
        {
            Index nx,ny;
            strbuf = xmlGetProp(curnode, XMLSTR "dims");
            if(!strbuf) //not found
                throw XmlFileException(string("In ") +(char*) parentname + " invalid detrend dims", __FILE__, __func__, __LINE__);
            sscanf((char*)strbuf,"%Ld, %Ld", &nx, &ny);
            xmlFree(strbuf);

            m_detrendMask.resize(nx,ny);
            strbuf = xmlGetProp(curnode, XMLSTR "values");
            if(!strbuf) //not found
                throw XmlFileException(string("In ") +(char*) parentname + " no detrend  values", __FILE__, __func__, __LINE__);
            char *pbuf=(char*) strbuf;
            for(Index i=0; i< nx*ny; ++i)
            {
                sscanf(pbuf, "%lf", &m_detrendMask(i) );
                pbuf=strchr(pbuf,',')+1;
            }
            xmlFree(strbuf);
        }

        if(xmlStrcmp(curnode->name, XMLSTR "Legendre_max")==0)
        {
            Index nx,ny;
            strbuf = xmlGetProp(curnode, XMLSTR "dims");
            if(!strbuf) //not found
                throw XmlFileException(string("In ") +(char*) parentname + " invalid Legendre_rms dims", __FILE__, __func__, __LINE__);
            sscanf((char*)strbuf,"%Ld, %Ld", &nx, &ny);
            xmlFree(strbuf);

            Legendre_ubound.resize(nx,ny);
            strbuf = xmlGetProp(curnode, XMLSTR "values");
            if(!strbuf) //not found
                throw XmlFileException(string("In ") +(char*) parentname + " no Legendre_rms   values", __FILE__, __func__, __LINE__);
            char *pbuf=(char*) strbuf;
            for(Index i=0; i< nx*ny; ++i)
            {
                sscanf(pbuf, "%lf", &Legendre_ubound(i) );
                pbuf=strchr(pbuf,',')+1;
            }
            xmlFree(strbuf);
        }

        if(xmlStrcmp(curnode->name, XMLSTR "fractal_parameters")==0)
        {  // this node should have 2 children X_fractal and Y_fractal
            xmlNodePtr dirnode=xmlFirstElementChild(curnode);
            while(dirnode)
            {
                const char *axe;
                if(xmlStrcmp(curnode->name, XMLSTR "X_fractal")==0)
                    axe="X";
                else if(xmlStrcmp(curnode->name, XMLSTR "Y_fractal")==0)
                    axe="Y";
                else
                    continue; // unknown parameter skip

                Index nseg;
                strbuf = xmlGetProp(curnode, XMLSTR "n_segments");
                if(!strbuf) //not found
                    throw XmlFileException(string("In ") +(char*) parentname +" "+ axe +" fractal no segments", __FILE__, __func__, __LINE__);
                sscanf((char*)strbuf,"%Ld", &nseg);
                xmlFree(strbuf);

                double exponent[nseg], frequency[nseg-1];
                strbuf = xmlGetProp(curnode, XMLSTR "exponents");
                if(!strbuf) //not found
                    throw XmlFileException(string("In ") +(char*) parentname +" "+ axe + " fractal no exponents", __FILE__, __func__, __LINE__);
                char *pbuf=(char*) strbuf;
                for(Index i=0; i< nseg; ++i)
                {
                    sscanf(pbuf, "%lf", &exponent[i] );
                    pbuf=strchr(pbuf,',')+1;
                }
                xmlFree(strbuf);

                strbuf = xmlGetProp(curnode, XMLSTR "frequencies");
                if(!strbuf) //not found
                    throw XmlFileException(string("In ") +(char*) parentname +" "+ axe + " fractal no frequencies", __FILE__, __func__, __LINE__);
                pbuf=(char*) strbuf;
                for(Index i=0; i< nseg; ++i)
                {
                    sscanf(pbuf, "%lf", &frequency[i] );
                    pbuf=strchr(pbuf,',')+1;
                }
                xmlFree(strbuf);
                // we can set the parameters for this direction
                m_fractalSurf.setXYfractalParams(axe,nseg,exponent,frequency);

                dirnode=xmlNextElementSibling(dirnode);
            }
        } // end fractal params
        curnode=xmlNextElementSibling(curnode);
    }

}
