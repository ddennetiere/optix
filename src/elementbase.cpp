/**
 *************************************************************************
*   \file            elementbase.cpp
*
*   \brief Element base class implementation file
*
*
*
*   \author             François Polack  <francois.polack@synchroton-soleil.fr>
*   \date               Creation: 2021-02-02
*   \date               Last update: 2021-02-02
 ***************************************************************************/


#include "elementbase.h"
#include "sourcebase.h"
#define XMLSTR (xmlChar*)
#define MAXBUF 512

map<string, string> ElementBase::m_helpstrings;
int ElementBase::m_nameIndex=0;

FloatType ElementBase::m_FlipSurfCoefs[]={0, 0, 1, 0,  1, 0, 0, 0,  0, 1, 0, 0,   0, 0, 0, 1 };

//
//char LastError[ERROR_MAXSIZE];
string LastError;
char noerror[]="No error";
bool OptiXError=false;

//char* LastError=LastErrorBuffer;



ElementBase::ElementBase(bool transparent, string name, ElementBase* previous):m_name(name), m_previous(previous), m_next(NULL),
                    m_parent(NULL), m_transmissive(transparent), m_isaligned(false)
{
    Parameter param;
    param.type=Angle;
    defineParameter("theta", param);
    defineParameter("phi", param);
    defineParameter("psi", param);
    defineParameter("Dtheta", param);
    defineParameter("Dphi", param);
    defineParameter("Dpsi", param);
    param.type=Distance;
    defineParameter("distance", param);
    defineParameter("DX", param);
    defineParameter("DY", param);
    defineParameter("DZ", param);

    if (previous)
        m_previous->m_next= this;
    setHelpstrings();
    if(name.empty())
    {
        char strName[16];
        sprintf(strName,"Surface%d",m_nameIndex++);
        m_name=strName;
    }
    m_exitFrame.setIdentity();
    m_surfaceDirect.setIdentity();
    m_surfaceInverse.setIdentity();
    m_frameDirect.setIdentity();
    m_frameInverse.setIdentity();
    m_translationFromPrevious.setZero();
}

int ElementBase::setFrameTransforms(double wavelength)
{
    // retrouve ou définit l'orientation absolue du trièdre d'entrée
    RotationType inputFrameRot; // rotation part
    VectorType inputFrameTranslation;  // translation part
    if (m_previous==NULL)
    {
        inputFrameRot= RotationType::Identity() ;
        inputFrameTranslation.setZero();
    }
    else
    {
         inputFrameRot= m_previous->m_exitFrame.linear(); // extract linear= base rotation
         inputFrameTranslation=m_previous->m_exitFrame.translation();
    }

    // NB pour calculer la position de l'optique dans le repère absolu local on utilise les desaxements Rx(phi+Dphi)*Ry(-theta-Dtheta), Rz(psi+Dpsi)
    // mais pas pour calculer le le reference frame sortant Rx(phi)*Ry(-2*theta)

    // positionne la surface par rapport à la précédente
    Parameter param;
    RayBaseType inRay=(m_previous==NULL)?RayBaseType::OZ() : RayBaseType(VectorType::Zero(), inputFrameRot.col(2) ) ;  // alignment exit Ray is normalized and its position is at previous optics
    getParameter("distance", param);
    (inRay+=param.value).rebase();  // inray a maintenant son origine à la position absolue de la surface
    m_translationFromPrevious=inRay.position();



    getParameter("phi",param);
    FloatType angle(param.value);
    m_exitFrame=inputFrameRot*AngleAxis<FloatType>(angle, VectorType::UnitZ());
    getParameter("Dphi",param);
    angle+=param.value;

    m_surfaceDirect= IsometryType(inputFrameRot)*AngleAxis<FloatType>(angle, VectorType::UnitZ()); // rot/nouveau Z

    getParameter("theta",param);
    angle=param.value;
    // si la surface est transmissive l'axe d'alignement de sortie reste celui d'entrée mais la rotation phi change
                // le trièdre.   La normale pointe vers l'aval et theta donne le désalignement de la surface / l'axe d'entrée autour de OX
    if(!m_transmissive)
        m_exitFrame*=AngleAxis<FloatType>(-2.*angle,VectorType::UnitX()) ; // axe X nouveau

    getParameter("Dtheta",param);
    angle+=param.value;
    /** \todo Should we keep the same sign convention on theta angle for transmissive and reflective elements ? */
    m_surfaceDirect*=AngleAxis<FloatType>(-angle, VectorType::UnitX()) ;  // convention déviation vers le haut si phi=0, vers l'extérieur anneau si phi=Pi/2 (M_PI_2)

   // m_frameDirect=rayTransform;
    m_exitFrame.translation()=inputFrameTranslation+m_translationFromPrevious;
    m_frameDirect=m_exitFrame.linear();
    m_frameInverse=m_frameDirect.inverse();


    getParameter("psi",param);
    angle=param.value;
    getParameter("Dpsi",param);
    angle+=param.value;
    if(!m_transmissive)// si reflection
    {
        m_surfaceDirect*= Matrix<FloatType,4,4>(m_FlipSurfCoefs); // la surface est basculée normale vers Y
    }

    m_surfaceDirect*=AngleAxis<FloatType>(angle, VectorType::UnitZ()) ; // rotation psi

    VectorType surfShift;
    getParameter("DX",param);
    surfShift(0)=param.value;
    getParameter("DY",param);
    surfShift(1)=param.value;
    getParameter("DZ",param);
    surfShift(2)=param.value;
    m_surfaceDirect.pretranslate(surfShift);
    m_surfaceInverse=m_surfaceDirect.inverse();

#ifdef ALIGNMENT_DUMP
    cout << m_name <<" surface_direct:\n" << m_surfaceDirect.matrix() << endl;
    cout << "           exit frame:\n" << m_exitFrame.matrix() <<endl <<endl;
#endif // ALIGNMENT_DUMP

    return 0;
}


void ElementBase::setHelpstrings()
{
  setHelpstring("distance", "distance to previous"); // sources normally have distance=0
  setHelpstring("theta", "Chief ray half-deviation"); // transmissive surfaces shoud set the transmissive flag
  setHelpstring("phi", "Surface reference frame rotation around the incident chief ray");
  setHelpstring("psi", "Surface reference frame rotation around its normal"); // rotation order is phi, theta, psi
  setHelpstring("Dtheta", "incidence theta correction");
  setHelpstring("Dphi", "frame rotation phi correction");
  setHelpstring("Dpsi", "in-plane rotation psi correction");
  setHelpstring("DX", "X offset in surface reference frame");
  setHelpstring("DY", "Y offset in surface reference frame");
  setHelpstring("DZ", "Z offset in surface reference frame");
}

int ElementBase::alignFromHere(double wavelength)
{
    int ret=align(wavelength);
    if(ret)
    {
        m_isaligned=false;
        return ret;
    }
    else
        m_isaligned=true;
    if(m_next!=NULL)
        m_next->alignFromHere(wavelength);
    return 0;
}

bool ElementBase::isAligned()/**< Eventuellement retourner le pointeur du 1er élément non aligné et NULL si OK */
{
    if(! m_isaligned )
        return false;
    if(m_next!=NULL)
        return m_next->isAligned();
    else
        return true;
}

 bool ElementBase::isSource()
{
    if(dynamic_cast<SourceBase*>(this))
        return true;
    else
        return false;
}

ElementBase* ElementBase::getSource()
{
    SourceBase * pSource=NULL, *ps;
    ElementBase* pSurf=m_previous;
    while(pSurf)
    {
        ps=dynamic_cast<SourceBase*>(pSurf);
        if(ps)
            pSource=ps;
        pSurf=pSurf->getPrevious();
    }
    return pSource;
}

void ElementBase::dumpData()
{
    cout  <<  "m_exitFrame" << endl << m_exitFrame.matrix() << endl;
    cout  <<  "m_surfaceDirect" << endl << m_surfaceDirect.matrix() << endl;
    cout  <<  "m_surfaceInverse" << endl << m_surfaceInverse.matrix() << endl;
    cout  <<  "surface transform product" << endl <<m_surfaceDirect* m_surfaceInverse.matrix() << endl;
    cout  <<  "m_translationFromPrevious" << endl << m_translationFromPrevious.transpose() << endl;
    cout  <<  "m_frameDirect" << endl << m_frameDirect.matrix() << endl;
    cout  <<  "m_frameInverse" << endl << m_frameInverse.matrix() << endl<<endl;
}

void memoryDump(void* address, uint64_t size)
{
    uint64_t index=0;
    uint8_t * pos=(uint8_t*)address;
    cout << "memory dump from address " << std::hex << address << endl;
    while(index < size)
    {
        // cout << std::setw(2) <<  std::hex <<  *pos ;
        printf("%02X ", *pos);
        if(index%16==15)
            cout << endl;
        else if(index%8==7)
            cout << ' ';
        ++ pos;
        ++ index;
    }
    cout  << endl;
}

TextFile& operator<<(TextFile& file,  ElementBase& elem)
{
    string namestr;
    file << elem.getOptixClass(); // <<'\0';
    file << elem.m_name;

    if(elem.m_previous)
        file << elem.m_previous->getName();
    else
        file << string();

    if(elem.m_next)
        file << elem.m_next->getName();
    else
        file << string();
//    file << '\0';

    map<string,Parameter>::iterator it;
    for(it=elem.m_parameters.begin(); it != elem.m_parameters.end(); ++it)
    {
        file << it->first  ;
        file << it->second;
    }
    file << '\0' << '\n';
    return file;
}

TextFile& operator>>(TextFile& file,  ElementBase* elem)
{
    string str;
    Parameter param;
    if (elem) delete elem;
    elem=NULL;
    file >>str; // gets runtime class
    return file;
}

void ElementBase::operator>>(xmlNodePtr sysnode)
{
     cout << "entering ElementBase::operator>>()\n";
    char cvbuf[MAXBUF];
    xmlNodePtr parmnode,arraynode;
    xmlNodePtr elemnode=xmlNewTextChild(sysnode,NULL, XMLSTR "element", NULL);
    xmlNewProp (elemnode, XMLSTR "name", XMLSTR m_name.c_str());
    xmlNewProp (elemnode, XMLSTR "class", XMLSTR getOptixClass().c_str()); // on n'a pas besoin des pointeurs etournés
    //   pour le chaînage next suffit; Si l'attribut de chaînage next n'est écrit,
    //   il n'y a pas de suivant (m_next=null qui est le défaut de création)
    if(m_next)
        xmlNewProp (elemnode, XMLSTR "next", XMLSTR m_next->getName().c_str());
    Surface* surf=dynamic_cast<Surface*>(this);
    if(surf)  // en prévision des groupes qui ne sont pas des surfaces
    {
        *surf >> elemnode;
    }
    if(m_transmissive && getOptixClass().compare(0,8,"Grating<" )!=0 ) //gratings are reflective by default
        xmlNewProp (elemnode, XMLSTR "trans", XMLSTR "true");

    map<string,Parameter>::iterator it;
    for(it=m_parameters.begin(); it != m_parameters.end(); ++it)
    {
//        parmnode=xmlNewTextChild(elemnode,NULL,XMLSTR "param", XMLSTR it->first.c_str());
        Parameter &param=it->second;
        if(param.flags & ArrayData)
        {  //modified 04/06/2024 to record data as node text
            parmnode=xmlNewTextChild(elemnode,NULL,XMLSTR "param", NULL);
            cvbuf[0]=0;
            char *pbuf=cvbuf;
            int parmsize=param.paramArray->dims[0]*param.paramArray->dims[1];
            if (parmsize)
            {
                pbuf+=sprintf(pbuf,"%.8g", param.paramArray->data[0]);
                for(int i=1; i<  parmsize; ++i)
                    pbuf+=sprintf(pbuf,", %.8g", param.paramArray->data[i]);
            }
            arraynode=xmlNewTextChild(parmnode, NULL, XMLSTR "array", XMLSTR cvbuf);
//            xmlNewProp (arraynode, XMLSTR "data", XMLSTR cvbuf);
            sprintf(cvbuf,"%Ld, %Ld", param.paramArray->dims[0], param.paramArray->dims[1]);
            xmlNewProp (arraynode, XMLSTR "dims", XMLSTR cvbuf);
        }
        else
        {
            sprintf(cvbuf,"%.8g",param.value);
            parmnode=xmlNewTextChild(elemnode,NULL,XMLSTR "param", XMLSTR cvbuf);
         }
        xmlNewProp (parmnode, XMLSTR "name", XMLSTR it->first.c_str());
        if(param.bounds[0]!=0)
        {
            sprintf(cvbuf,"%.8g",param.bounds[0]);
            xmlNewProp (parmnode, XMLSTR "min", XMLSTR cvbuf);
        }
        if(param.bounds[1]!=0)
        {
            sprintf(cvbuf,"%.8g",param.bounds[1]);
            xmlNewProp (parmnode, XMLSTR "max", XMLSTR cvbuf);
        }
        if(param.multiplier!=1.)
        {
            sprintf(cvbuf,"%.8g",param.multiplier);
            xmlNewProp (parmnode, XMLSTR "mult", XMLSTR cvbuf);
        }
        // Les paramètres type, group, et flags sont non modifiables
    }
}

void ElementBase::operator<<(xmlNodePtr elemnode)
{
    xmlChar* trans= xmlGetProp(elemnode, XMLSTR "trans");
    if(trans)
    {
        setTransmissive(true); //Value false is not stored in the XML
        xmlFree(trans);
    }

	xmlChar *att=NULL, *name=NULL;
    xmlNodePtr cur = xmlFirstElementChild(elemnode); //  elemnode->xmlChildrenNode;
    Parameter param;
//    bool success=true;
	while (cur != NULL)  //parameter nodes
	{  // std::cout << "node name " << (char*) cur->name << std::endl;
	    if ((!xmlStrcmp(cur->name, XMLSTR "param")))
        {
            xmlNodePtr arraynode=NULL;
            xmlChar *val=NULL, *data=NULL;
            //check version
           // bool V1=xmlHasProp(cur, XMLSTR "name");
            if(xmlHasProp(cur, XMLSTR "name"))   // new format
            {
                name=xmlGetProp(cur, XMLSTR "name");
//                std::cout << "Parameter " << (char*) name << " version2\n";
                if(!getParameter((char*) name, param))
                {
                    std::cout << "parameter name " << name << "is not valid for class " << getOptixClass() << endl;
                    break;
                }
                if(xmlChildElementCount(cur)==0)
                    val=xmlNodeGetContent(cur);
                else
                {
                    arraynode=xmlFirstElementChild(cur);
                    if(!arraynode)
                    {
                        std::cout << "parameter name " << name << " no data found" << endl;
                        break;
                    }
                    data=xmlNodeGetContent(arraynode);  // if size=0 data =""
                }
            }
            else //old version
            {
                name=xmlNodeGetContent(cur);
                std::cout << " param " << (char*) name << "  old version\n";
                if(!getParameter((char*) name, param))
                {
                    std::cout << "parameter name " << name << "is not valid for class " << getOptixClass() << endl;
                    break;
                }
                val= xmlGetProp(cur, XMLSTR "val");
                if(!val)
                {
                    arraynode=xmlFirstElementChild(cur);
                    if(!arraynode)
                    {
                        std::cout << "parameter name " << name << " no data found" << endl;
                        break;
                    }
                    data=xmlGetProp(arraynode, XMLSTR "data");
                }

            }
            //std::cout << " array node:" << (uint64_t) arraynode << "  Val:" << (uint64_t) val << "  data:" <<(uint64_t) data <<std::endl;
            if(arraynode)
            {
                int dim0=0, dim1=0;
                att=xmlGetProp(arraynode, XMLSTR "dims");
                if(att)
                {
                   // printf("    Array %s ", att);
                    sscanf((char*)att,"%d, %d", &dim0, &dim1);
                    xmlFree(att);
                }
                else
                {
                    std::cout << "dims not found";
                    break;
                }
                if(data)
                {
                    cout << "array data=" << data << endl;
                   // printf("\n           %s \n", data);
                    int N=dim0*dim1;
                    if(param.paramArray)
                        delete param.paramArray;
                    param.paramArray=new ArrayParameter(dim0,dim1);
                    if(N)
                    {
                        double * pdat=param.paramArray->data;
                        char *pbuf= (char*)data;
                        sscanf(pbuf, "%lg", pdat);

                        for (int i=1; i <N; ++i)
                        {
                            pbuf=strchr(pbuf,',');
                            sscanf(++pbuf, "%lg", ++pdat);
                        }
                    }
                    xmlFree(data);
                }
            }
            else if(val)  // single value case we also  min max attributes
            {
               // std::cout << "val=" << val<< std::endl;
                param.value=atof((char*)val);
                xmlFree(val);

                att= xmlGetProp(cur, XMLSTR "min");
                if(att)
                {
                    param.bounds[0]=atof((char*)att);
                    xmlFree(att);
                }
                att= xmlGetProp(cur, XMLSTR "max");
                if(att)
                {
                    param.bounds[0]=atof((char*)att);
                    xmlFree(att);
                }
                att= xmlGetProp(cur, XMLSTR "mult");
                if(att)
                {
                    param.multiplier=atof((char*)att);
                    xmlFree(att);
                }
            }
            else
                printf(" INCOMPLETE parameter no value found\n");


        // Now we can update parameter
            if(!setParameter((char*) name, param))
            {
                cout << " ERROR: " << LastError <<endl;
//                    success=false;
            }

            // no need to dump  usually
//            if(param.flags &ArrayData)
//                dumpParameter((char*) name) ;
            xmlFree(name);
        }
        else
          cout << cur->name << "  skipped\n ";


       // cout << "old cur = " << cur << endl;

        //cur = cur->next;
        // calling NextElementSibling avoid entering intor the text node
        cur=xmlNextElementSibling(cur);
        //cout << "new cur=" << cur << endl;
	} // parameter nodes

}

