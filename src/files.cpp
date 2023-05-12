/**
 *************************************************************************
*   \file           files.cpp
*
*   \brief             implementation file of classes TextFile and SolemioFile
*
*
*   \author             François Polack  <francois.polack@synchroton-soleil.fr>
*   \date               Creation: 2021-02-10
*   \date               Last update: 2021-02-10
 ***************************************************************************/

 #include "files.h"
 #include "interface.h"
 #include "elementbase.h"
 #include "surface.h"
 #include "collections.h"
 #include "opticalelements.h"
 #ifdef HAS_REFLEX
    #include "ReflectivityAPI.h"
 #endif // HAS_REFLEX
 #include "ApertureAPI.h"
 #include <queue>

 #define   TAILLEPARAMETRES 20
 #define   NOMBRETAGS 16
 #define   MAXFLOATING 50
 #define   NUMEROSUPERFICI 28
 #define   MAXGLOBALPARAMS 8
 #define   NUMCOPARAMETERS 32
#define    FILM  1
#define    PLAN  2
#define    CILYNDRE  3
#define    TORUS  4
#define    SPHERE  5
#define    ELLIPSE  6
#define    RESEAUXOLPLANDEVCONST 7
#define    RESEAUXVLSPLANDEVCONST 8
#define    SORGENTESIMP 9
#define    FENTE 10
#define    SORGENTERANDOMGAUSSIANA 11
#define    TORUSDEFORMED           12
#define    SORGENTERANDOMGAUSSIANADIVLINEARE 13
#define    SURFPOL  14
#define    RESEAUXOLPLANDEVCONSTANGLE 15
#define    RESEAUXOLSPHERDEVCONSTANGLE 16
#define    RESEAUXVLSSPHERDEVCONSTANGLE 17
#define    FILMSPHERE 18
#define    RESEAUXOLVLSSPHERDEVCONSTANGLE 19
#define    RESEAUXOLTORUSDEVCONSTANGLE 20
#define    RESEAUXOLSPHERTRANSM 21
#define    SURFACECOPIE 22
#define    RESEAUXOLVLSSPHERTRANSM 23
#define    CONO 24
#define    SORGENTEONDULEURGAUSSIANA 25
#define    SYSTEMGLOBALPARAMETERS 26
#define    SOURCEAIMANT 27

extern ElementCollection System;

vector<string> paramName={"Omega","Theta","dOmega","dTheta","Spin","dX","dY","dZ","distback","distfor",
                        "algtLambda","OsloBack","clipX1","clipX2","clipY1","clipY2"};
vector<string> SolemioElements={ "invalide",
    "Film", "Plan", "Cylindre", "Tore", "Sphere",
    "Ellipse", "Reseau holo. plan cst dev def delta cos", "Reseau VLS plan cst. dev. def delta cos",
    "Source simple", "Fente", "Source aleatoire gaussienne", "Tore deforme",
    "Source aleatoire gaussienn a divergence linéaire", "Surface Poly",
    "Reseau holo. plan cst dev def angles", "Reseau holo. sphere cst dev def angles",
    "Reseau VLS. sphere cst dev def angles", "film sphere", "Reseau holo VLS sphere cst. dev. def delta cos",
    "Reseau holo. tore cst dev def angles",  "Reseau holo. sphere transmission", "Copie surface",
    "Reseau holo. VLS Sphere transmission", "Cone", "Source Onduleur gaussienne",
    "Systeme de paramètres globaux", "Source aimant"
};

vector<int> numParameters={
0, // 0 invalide
5, // 1 film.cpp
2, // 2 plan.cpp
3, // 3 cilyndre.cpp
4, // 4 torus.cpp
3, // 5 sphere.cpp
5, // 6 ellipse.cpp // le stockage des tagged parameters n'a pas l'air utilise donc 2 au lieu de 5
14, // 7 reseauxolplandevconst.cpp
9, // 8 reseauxVLSplandevconst.cpp
12, // 9 sorgentesimp.cpp
3, // 10 fente.cpp
7, // 11 sorgenterandomgaussiana.cpp
8, // 12 torusdeformed.cpp
7, // 13 sorgenterandomgaussianadivlineare.cpp
28, // 14 surfpol.cpp
14, // 15 reseauxolplandevconstAngle.cpp
12, // 16 reseauxolSpherdevconstAngle.cpp
10, // 17 reseauxVLSSpherdevconstAngle.cpp
2, // 18 filmsphere.cpp
7, // 19 reseauxOLVLSSpherdevconstAngle.cpp
13, // 20 reseauxolTorusdevconstAngle.cpp
13, // 21 reseauxolSphereTransm.cpp
0, // 22 surfaceCopie.cpp
7, // 23 reseauxOLVLSSpherTransm.cpp
3, // 24 cono.cpp
9, // 25 sorgenteonduleurgaussiana.cpp
MAXGLOBALPARAMS, // 26 systemparameters.cpp
9}; // 27 sourceaimant.cpp

//D:\projets\projetsCB\Beamline\generalsourcepol.cpp|54|n_parametres = 8 ;|
//D:\projets\projetsCB\Beamline\minimizzatore.cpp|97|n_parametres = 11 ;|
//D:\projets\projetsCB\Beamline\minimizzatore.cpp|154|n_parametres = 63 ;|
//D:\projets\projetsCB\Beamline\minimizzatore.cpp|214|n_parametres = 15 ;|
//D:\projets\projetsCB\Beamline\polsourceimage.cpp|82|n_parametres = 61 ;|

/** \brief structure and functions to enable reading and decoding a Solemio file
 */
struct SolemioSurface
{
	SolemioSurface(int stype ):type(stype){}

    int type, coatingset, option, parmisvariable[MAXFLOATING], coparmisvariable[NUMCOPARAMETERS];
	double  param[MAXFLOATING], varparamin[MAXFLOATING], varparamax[MAXFLOATING],
            coparam[NUMCOPARAMETERS], coparmin[NUMCOPARAMETERS], coparmax[NUMCOPARAMETERS];
	string  nomemezzo1, nomemezzo2;

	bool ReadFromFile(SolemioFile& Sfile)
	{
        if(Sfile.version < 19)  // la version minimale archivée ssemble être 20 (juin 2000 APE)
        {
            cout<< "file version not implemented\n";
            return false;
        }
        int i;
	    for (i=0; i < numParameters[type]; ++i)
            Sfile>> parmisvariable[i] >> varparamax[i] >>varparamin[i];
        Sfile.skipline(1);
        if(!Sfile.check_comment(" nomemezzo1  "))
            return false;
        Sfile.getScript(nomemezzo1);
        Sfile.skipline(1);
        if(!Sfile.check_comment(" nomemezzo2  "))
            return false;
        Sfile.getScript(nomemezzo2);
        Sfile >> coatingset;
        for(i=0; i < NUMCOPARAMETERS; ++i)
            Sfile >> coparmisvariable[i] >> coparmax[i] >> coparmin[i] >> coparam[i];

        if(Sfile.version > 19)
            Sfile >> option;

        return true;
	}

};

 SolemioFile::SolemioFile(string filename):fstream(filename,  ios::in ) // ouverture en lecture seule
 {
    iconTable.clear();
    elemTable.clear();
    *this >> version;
    skipline(1);
 }

 void SolemioFile::skipline(int n)
 {
    string dummy;
    for(int i=0; i < n; ++i)
        std::getline(*this, dummy);
 }

 void SolemioFile::getPrefixedString(string& str)
 {
    int n;
    *this >> n  ;
    char buf[++n];
 //   *this >> setw(n) >> buf >> setw(0);
    read(buf, n);
    buf[n]=0;
    str=buf+1;
 }

 void SolemioFile::getScript(string& str)
 {
    int n;
    char s;
    str="";
    *this >> n ;
    if(n<0)
        return;
    *this>> s ;
    if(s!='s')
    {
        cout << "invalid script prefix\n";
        return;
    }
    if(n==0)
        return;
    else
    {
        char buf[++n];
        read(buf, n);
        buf[n]=0;
        str=buf;
    }
 }
int  getNextToken(const string &line, int pos, string &token)
{
    size_t index0, index1;
    index0=line.find_first_not_of(" \t",pos);
  //  cout << "GetToken:" <<line.substr(pos) << "  " << index0 <<endl;
    if(index0==string::npos)
        return index0;
    index1=line.find_first_of(" \t",index0);
    if(index1==string::npos)
        token=line.substr(index0, line.size()-index0);
    else
        token=line.substr(index0, index1-index0);
    return index1;
}

bool getTrimmedEnding(const string &line, size_t pos, string &token)
{    size_t  index;
     pos=line.find_first_not_of(" \t",pos);
     index=line.find_last_not_of(" \t");
     if(pos == string::npos || index == string::npos)
        return false;
     token=line.substr(pos,index-pos+1);
     return true;
}

 void parseScriptVariables(const string &script, map<string,string> &dict)
 {
     size_t start=0, pos=0;
     string line, verb, variable, value;
     cout << "\nparsing TCL script  size=" << script.size() << endl;
     while(start < script.size() )
     {
         pos=script.find('\n',start);
         if(pos==string::npos)
         {
             line=script.substr(start);
             start=script.size();
         }
         else
         {
             line=script.substr(start,pos-start);
             start=pos+1;
         }

         while(line[line.size()-1]=='\\')
         {
             pos=script.find('\n',start);
             if(pos==string::npos)
             {
                 line+=script.substr(start);
                 start=script.size();
             }
             else
             {
                 line+=script.substr(start,pos-start);
                 start=pos+1;
             }

         }

//         cout << line.size() << " >>" << line << endl;
//         continue;
         //line.remove_prefix(std::min(line.find_first_not_of(" "), line.size()));
         if(line.empty() || line[0]=='#')
            continue;
         pos=getNextToken(line,0, verb);
         if(pos==string::npos)
            continue;
         if(verb=="set")
         {
             pos=getNextToken(line,pos,variable );
             if(pos==string::npos)
             {
                cout << "no variable found after set\n";
                continue;
             }
             if(! getTrimmedEnding(line, pos, value))
             {
                 cout << "no value found for variable\n";
                 continue;
             }
             dict.insert(pair<string,string>(variable,value));
             cout << variable <<" = "<< value << endl;
         }

     }
     cout <<"END of TCL script\n\n";
 }

 SolemioFile& SolemioFile::operator>>(ArrayXd&  darray)
 {
     for(int i=0; i< darray.size(); ++i)
        *this >>darray(i);
     return *this;
 }


 bool SolemioFile::check_comment(const string comment)
 {
     string rdstr;
     std::getline(*this, rdstr);
     bool assert= (rdstr==comment);
     if(!assert)
        cout << "Invalid comment string.  <<" << rdstr << ">> found instead of <<" << comment << ">>\n";
     return assert;
 }



 bool  SolemioFile::get_element(size_t * pelemID)
 {
     int i, type, inpruntPol, imprunt, auxset, XYZalign=0, clipping=0, activeFilm=0,
            unit[NOMBRETAGS], unitmin[NOMBRETAGS], unitmax[NOMBRETAGS],
            unitF[MAXFLOATING], unitminF[MAXFLOATING], unitmaxF[MAXFLOATING],
            elemParamvar[TAILLEPARAMETRES];

     uint32_t  nextPol, sourcePol, pElemIcon, previousElemIcon, nextElemIcon,
               pElem, previousElem, nextElem ;
     double clipX1=0, clipX2=0, clipY1=0, clipY2=0, sigmaslopeLong, sigmaslopeTrans,
            elemParam[TAILLEPARAMETRES], elemParamin[TAILLEPARAMETRES], elemParamax[TAILLEPARAMETRES];
    SolemioLinkType iconLink, elemLink;

     // les rayons sont stockés dans l'ordre X, X', Y, Y', Z, Z', lambda
     ArrayXd axein(7), axeout(7),planelem(7), rotaxe(7), aux(7), poleNormal(7), yaxe(7);

     string name, tclScript;
     map<string,string> tclDict;
     ElementBase* elem=NULL; // no object created

     skipline(3); // saute le positionnement sur le plan de travail

     if(!check_comment(" unita_unitamin_unitamax  "))
            return false;
     for(i=0; i< NOMBRETAGS; ++i)
        *this >> unit[i] >> unitmin[i] >> unitmax[i];

     skipline(2); // va 2 lignes plus loin
     if(!check_comment(" unita_unitamin_unitamax_FLOATING  "))
            return false;
     for(i=0; i< MAXFLOATING; ++i)
        *this >> unitF[i] >> unitminF[i] >> unitmaxF[i];

     skipline(1); // ligne suivante
     getPrefixedString(name);
     cout << "element: " << name <<endl;

     skipline(1);
     if(!check_comment(" ecco_tipo  "))
        return false;
     *this >> type >> i >> inpruntPol >> nextPol >> pElemIcon ;
     cout << "Type " << type << "  " << SolemioElements[type] << endl;
     cout <<"Poly inprunt " << inpruntPol <<  "  nextPoly " << nextPol << endl;
//     if(1)
//        return true;

     skipline(1);
     if(!check_comment(" attacchi  "))
            return false;
     *this >> previousElemIcon >> nextElemIcon;
     cout << "Element object " <<pElemIcon  << " linked from "  << previousElemIcon << " to " << nextElemIcon << endl;

     iconLink.name=name;
     iconLink.prev=previousElemIcon;
     iconLink.next=nextElemIcon;
     iconTable.insert(pair<int32_t,SolemioLinkType>(pElemIcon, iconLink));


     for(i=0; i<TAILLEPARAMETRES; ++i)
        *this >> elemParam[i] >> elemParamvar[i] >> elemParamax[i] >> elemParamin[i];
//     {
//        cout << i << "  "  <<  param[i] << (paramvar[i]? " Varying ": " Fixed  ") << paramin[i] << " to "<< paramax[i] << endl;
//     }
     for(i=0; i<NOMBRETAGS; ++i)
        cout << paramName[i] << "  tag " << i << "  "  <<  elemParam[i] << (elemParamvar[i]? " Varying ": " Fixed  ") << elemParamin[i] << " to "<< elemParamax[i] << endl;

     *this >> axein >>axeout >> planelem >> rotaxe;

     *this >> pElem >> aux >> poleNormal;

     skipline(1);
     if(!check_comment(" sono_in_element  "))
            return false;
     *this >> auxset >> previousElem >> nextElem;

     elemLink.name=name;
     elemLink.prev=previousElem;
     elemLink.next=nextElem;
     elemTable.insert(pair<int32_t,SolemioLinkType>(pElem, elemLink));


      cout << "Surface " << pElem  << " linked from " << previousElem << " to " <<nextElem << endl;

      cout << "Input axis \n" << axein.transpose() <<endl;
      cout << "Output axis \n" << axeout.transpose() <<endl;
      cout << "Elem plane \n" << planelem.transpose() <<endl;
      cout << "Rot axis \n" << rotaxe.transpose() <<endl;

      cout << "\nPole Normal\n" << poleNormal.transpose() <<endl;
      cout << "Auxiliary axis \n" << aux.transpose() <<endl;


     skipline(1);
     if(!check_comment(" passo__superficie  "))
            return false;
     SolemioSurface SSurf(type);
     switch (type)
     {
     case FILM: //  1
        {
            int yaxeset;
            double  ax2=0,ax3=0, ay2=0, by2=0, by4=0; ;

            *this >>aux >> poleNormal;
            skipline(1);
            if(!check_comment(" fine_superficie  "))
                return false;
            *this >> yaxeset >> yaxe;
            if( !SSurf.ReadFromFile(*this))
                return false;
            if(version >19)
            {
                // introduit une distorsion d'ordre 3 en X et 2 en Y. by2 et by4 ne sont pas utilisés et tjs nuls
                *this >> ax2 >> ax3 >> ay2 >> by2 >> by4;
                if(SSurf.option)
                    cout << "polynomial surface - conversion NOT implemented\n";
            }
            cout << "\nPole Normal\n" << poleNormal.transpose() <<endl;
            cout << "Auxiliary axis \n" << aux.transpose() <<endl;
            cout << "Yaxe " << (yaxeset  ? "set\n" :"not set\n" )<< yaxe.transpose() <<endl;
            if(pelemID)
            {
                *pelemID=CreateElement("Film<Plane>",name.c_str());
                elem=(ElementBase*)*pelemID;
            }
        }
        break;
     case PLAN:     //  2
        {
             *this >>aux >> poleNormal >> sigmaslopeLong >> sigmaslopeTrans;
             if( !SSurf.ReadFromFile(*this))
                return false;
            cout << "\nPole Normal\n" << poleNormal.transpose() <<endl;
            cout << "Auxiliary axis \n" << aux.transpose() <<endl;
            cout << "Slope  sigmas    tang. " << sigmaslopeLong << " sag. " << sigmaslopeTrans << endl;
            if(pelemID)
            {
                *pelemID=CreateElement("Mirror<Plane>",name.c_str());
                elem=(ElementBase*)*pelemID;
            }
        }
        break;
     case   CILYNDRE:    //  3
        {
            //  vector<string> nom={"Radius","sigmapentelong","sigmapentetransv"};
            ArrayXd axe(7);
            *this >> aux >> poleNormal >> axe;
            for(int j=0; j < numParameters[type]; ++j )
                *this >> SSurf.param[j];
            if( !SSurf.ReadFromFile(*this))
                return false;
            cout << "\nPole Normal\n" << poleNormal.transpose() <<endl;
            cout << "Auxiliary axis \n" << aux.transpose() <<endl;
            cout << "Axe "  << axe.transpose() <<endl;
            cout << "Radius " << SSurf.param[0]  <<  "  curvature " << 1./SSurf.param[0] <<endl;
            cout << "Slope  sigmas    tang. " << SSurf.param[1] << " sag. " << SSurf.param[2] << endl;

            if(pelemID)
            {
                *pelemID=CreateElement("Mirror<Cylinder>",name.c_str());
                elem=(ElementBase*)*pelemID;
                Parameter param;
                elem->getParameter("curvature", param); //="Radius inverse  "
                param.value=1./SSurf.param[0];
                param.bounds[1]=1./SSurf.varparamin[0];
                param.bounds[0]=1./SSurf.varparamax[0];
                elem->setParameter("curvature", param);
                elem->getParameter("psi",param);
                param.value-=M_PI_2; // définition d'axe tournée de 90° dans Solemio
                param.bounds[0]-=M_PI_2;
                param.bounds[1]-=M_PI_2;
                elem->setParameter("psi",param);
            }

            cout << "Implementation not tested\n";
        }
        break;
     case   TORUS:    //  4
        {
            //  vector<string> nom={"Big_Radius","Small_Radius","sigmapentelong","sigmapentetransv"};

            ArrayXd axe(7);
            *this >> aux >> poleNormal >> axe;
            for(int j=0; j < numParameters[type]; ++j )
                *this >> SSurf.param[j];
            if( !SSurf.ReadFromFile(*this))
                return false;
            cout << "\nPole Normal\n" << poleNormal.transpose() <<endl;
            cout << "Auxiliary axis \n" << aux.transpose() <<endl;
            cout << "Axe "  << axe.transpose() <<endl;
            cout << "Big_Radius   " << SSurf.param[0] <<  "   major_curvature " << 1./SSurf.param[0] <<endl;
            cout << "Small_Radius " << SSurf.param[1] <<  "   minor_curvature " << 1./SSurf.param[1] <<endl;
            cout << "Slope  sigmas    tang. " << SSurf.param[1] << " sag. " << SSurf.param[2] << endl;

            if(pelemID)
            {
                *pelemID=CreateElement("Mirror<Toroid>",name.c_str());
                elem=(ElementBase*)*pelemID;
                Parameter param;
                elem->getParameter("major_curvature", param); //="Radius inverse  "
                param.value=1./SSurf.param[0];
                param.bounds[1]=1./SSurf.varparamin[0];
                param.bounds[0]=1./SSurf.varparamax[0];
                elem->setParameter("major_curvature", param);
                elem->getParameter("minor_curvature", param); //="Radius inverse  "
                param.value=1./SSurf.param[0];
                param.bounds[1]=1./SSurf.varparamin[0];
                param.bounds[0]=1./SSurf.varparamax[0];
                elem->setParameter("minor_curvature", param);
            }

            cout << "Implementation not tested\n";
        }
        break;
      case   SPHERE:    //  5
        {
            //vector<string> nom={"Inv_Radius","sigmapentelong","sigmapentetransv"};
            ArrayXd centro(7);
            *this >> aux >> poleNormal >> centro;
            for(int j=0; j < numParameters[type]; ++j )
                *this >> SSurf.param[j];
            if( !SSurf.ReadFromFile(*this))
                return false;
            cout << "\nPole Normal\n" << poleNormal.transpose() <<endl;
            cout << "Auxiliary axis \n" << aux.transpose() <<endl;
            cout << "Center "  << centro.transpose() <<endl;
            cout << "Radius inverse  " << SSurf.param[0] << "   radius "<< 1./SSurf.param[0] << endl;
            cout << "Slope  sigmas    tang. " << SSurf.param[1] << " sag. " << SSurf.param[2] << endl;
            if(pelemID)
            {
                *pelemID=CreateElement("Mirror<Sphere>",name.c_str());
                elem=(ElementBase*)*pelemID;
                Parameter param;
                elem->getParameter("curvature", param); //="Radius inverse  "
                param.value=SSurf.param[0];
                param.bounds[0]=SSurf.varparamin[0];
                param.bounds[1]=SSurf.varparamax[0];
                elem->setParameter("curvature", param);
            }
        }
        break;
     case   ELLIPSE:    //  6
        {
            double theta0, sommeDist, dist[2];   // dist[0]= Distance au foyer forward et dist[1]= Distance au foyer backward
            ArrayXd foyer0(7),foyer1(7);
            *this >> aux >> poleNormal >>theta0 >>sommeDist >> foyer0 >>dist[0]>> foyer1 >>dist[1];
            for(int j=3; j < numParameters[type]; ++j )    // seuls les parms de pente sont tockés en séquence
                *this >> SSurf.param[j];
            if( !SSurf.ReadFromFile(*this))
                return false;
            cout << "\nPole Normal\n" << poleNormal.transpose() <<endl;
            cout << "Auxiliary axis \n" << aux.transpose() <<endl;
            cout << "Theta0 "  << theta0 << " [" <<SSurf.varparamin[2] << ", " << SSurf.varparamax[2] << "]\n";
            cout << "Object focus  " << foyer1.transpose() << endl;
            cout << "Object focal distance " << dist[1] << " [" <<SSurf.varparamin[0] << ", " << SSurf.varparamax[0] << "]\n";  // les définition d'Alex sont toujours aussi cohérentes !!!
            cout << "Image focus  " << foyer0.transpose() << endl;
            cout << "Image focal distance "  << dist[0] << " [" <<SSurf.varparamin[1] << ", " << SSurf.varparamax[1] << "]\n";
            cout << "Image focus  " << foyer0.transpose() << endl;

            cout << "Slope  sigmas    tang. " << SSurf.param[3] << " sag. " << SSurf.param[4] << endl;
            cout << "IMPLEMENTATION UNDER TEST\n";
            if(pelemID)
            {
                *pelemID=CreateElement("Mirror<RevolutionQuadric>",name.c_str());
//                *pelemID=CreateElement("Mirror<ConicBaseCylinder>",name.c_str());
                elem=(ElementBase*)*pelemID;
                Parameter param;
                elem->getParameter("invp", param);
                param.value=-1./dist[1];
                param.bounds[0]=-1./SSurf.varparamin[0];
                param.bounds[1]=-1./SSurf.varparamax[0];
                elem->setParameter("invp", param);
                elem->getParameter("invq", param);
                param.value=1./dist[0];
                param.bounds[1]=1./SSurf.varparamin[1];
                param.bounds[0]=1./SSurf.varparamax[1];
                elem->setParameter("invq", param);
                elem->getParameter("theta0", param);
                param.value=M_PI_2-theta0;
                param.bounds[1]=M_PI_2-SSurf.varparamin[2];
                param.bounds[0]=M_PI_2-SSurf.varparamax[2];
                elem->setParameter("theta0", param);

            }
        }
        break;
     case   RESEAUXOLPLANDEVCONST:    // 7
        {
            int j, maxmodifiable;
            vector<string> nom={"Deviation","lambdaLaser", "arm1", "cos1", "arm2", "deltacos2","ordrealign",
                                "ordreout", "sigmapente tang.", "sigmapente sag.", "tpmm", "tpm1", "tpm2", "tpm3"};

            *this >>aux >> poleNormal >> yaxe;
            for(j=0; j < numParameters[type]; ++j)
                *this >> SSurf.param[j];
            *this  >> maxmodifiable;
            ++maxmodifiable;
            if( !SSurf.ReadFromFile(*this))
                return false;
            cout << "\nPole Normal\n" << poleNormal.transpose() <<endl;
            cout << "Auxiliary axis \n" << aux.transpose() <<endl;
            cout << "Yaxe "  << yaxe.transpose() <<endl;
            for (j=0; j < maxmodifiable; ++j)
                cout << nom[j] << " " <<SSurf.param[j] << (SSurf.parmisvariable[j]? " variable [": " fixed [" ) <<
                            SSurf.varparamin[j] <<" "<< SSurf.varparamax[j] << "]\n";
            for(j=maxmodifiable; j< numParameters[type]; ++j)
                cout << nom[j] << " " <<SSurf.param[j] << " not variable\n";

            if(pelemID)
            {
                *pelemID=CreateElement("Grating<Holo,Plane>",name.c_str());
                elem=(ElementBase*)*pelemID;
                Parameter param;
                // non variable parameters from GratingBase
                elem->getParameter("order_align", param); //=ordrealign
                param.value=SSurf.param[6]; // should not be allowed to vary
                elem->setParameter("order_align", param);
                elem->getParameter("order_use", param); //=ordreout
                param.value=SSurf.param[7]; // should not be allowed to vary
                elem->setParameter("order_use", param);
                // non variable parameters from Holo
                elem->getParameter("recordingWavelength", param); //=lambdaLaser
                param.value=SSurf.param[1]; // should not be allowed to vary
                elem->setParameter("recordingWavelength", param);

                elem->getParameter("lineDensity", param);
                param.value=-SSurf.param[5]/SSurf.param[1]; //- Deltacos2/wavelength
                elem->setParameter("lineDensity", param);

                // other parameters from HOLO
                elem->getParameter("inverseDist1", param);
                param.value=-SSurf.param[2];
                double signdist1= copysign(1., param.value);
                param.bounds[0]=-SSurf.varparamax[2];
                param.bounds[1]=-SSurf.varparamin[2];
                elem->setParameter("inverseDist1",param);
                elem->getParameter("inverseDist2", param);
                param.value=-SSurf.param[4];
                param.bounds[0]=-SSurf.varparamax[4];
                param.bounds[1]=-SSurf.varparamin[4];
                elem->setParameter("inverseDist2",param);

                elem->getParameter("azimuthAngle1", param);
                param.value=param.bounds[0]=param.bounds[1]=0;
                elem->setParameter("azimuthAngle1",param);
                elem->getParameter("azimuthAngle2", param);   // parameter passé par adresse et susceptible d'être modifié
                param.value=param.bounds[0]=param.bounds[1]=0;
                elem->setParameter("azimuthAngle2",param);

                elem->getParameter("elevationAngle1", param);
                param.value=signdist1*acos(SSurf.param[3]);  // convention de direction des sources différente de Solemio
                if(signdist1 >0)
                {
                    param.bounds[0]=SSurf.varparamin[3];
                    param.bounds[1]=SSurf.varparamax[3];
                }
                else
                {
                    param.bounds[0]=-SSurf.varparamax[3];
                    param.bounds[1]=-SSurf.varparamin[3];
                }
                if(!elem->setParameter("elevationAngle1",param))
                    cout << "cannot find a second constrution point with this elevation angle " << param.value << endl;

//                {
//                    Grating<Holo,Plane> *grating=dynamic_cast<Grating<Holo,Plane>*>(elem);
//                    cout<< "Direction1  " << grating->m_direction1.transpose()<<endl;
//                    cout<< "Direction2  " << grating->m_direction2.transpose()<<endl;
//                    cout << "line density " << grating->m_lineDensity << endl;
//                }
            }
        }
        break;
     case   RESEAUXVLSPLANDEVCONST:    // 8
        {
            vector<string> nom={"Deviation","tpmm", "tpmm1", "tpmm2", "tpmm3",
                                "ordrealign", "ordreout", "sigmapente tang.", "sigmapente sag." };
            int j;
            *this >>aux >> poleNormal >> yaxe;
            for(j=0; j < numParameters[type]; ++j)
                *this >> SSurf.param[j];
            if( !SSurf.ReadFromFile(*this))
                return false;
            for (j=0; j < numParameters[type]; ++j)
                cout << nom[j] << " " <<SSurf.param[j] << (SSurf.parmisvariable[j]? " variable [": " fixed [" ) <<
                            SSurf.varparamin[j] <<" "<< SSurf.varparamax[j] << "]\n";

            if(pelemID)
            {
                *pelemID=CreateElement("Grating<Poly1D,Plane>",name.c_str());
                elem=(ElementBase*)*pelemID;
                Parameter param;
                // non variable parameters from GratingBase
                elem->getParameter("order_align", param); //=ordrealign
                param.value=SSurf.param[5]; // not allowed to vary
                elem->setParameter("order_align", param);
                elem->getParameter("order_use", param); //=ordreout
                param.value=SSurf.param[6];
                elem->setParameter("order_use", param);
                // non variable parameters from Poly1D
                elem->getParameter("degree", param);
                param.value=3;
                elem->setParameter("degree", param);
                elem->getParameter("lineDensity", param);
                param.value=SSurf.param[1]*1.e3;   // unité line/mm
                elem->setParameter("lineDensity", param);

                char namebuf[32];
                for (int i=3; i >0; --i)
                {
                    sprintf(namebuf, "lineDensityCoeff_%d", i);
                    elem->getParameter(namebuf, param);
                    param.value=SSurf.param[i+1];
                    elem->setParameter(namebuf, param);
                }
            }
        }
        break;
     case   SORGENTESIMP:    // 9
        {
            cout << "NOT IMPLEMENTED\n";
        }
        break;
     case   FENTE:    // 10
        {
            vector <string> parmname={"Slit width","shift","Inv_Radius"};
            *this >>aux >> poleNormal;
            cout << "\nPole Normal\n" << poleNormal.transpose() <<endl;
            cout << "Auxiliary axis \n" << aux.transpose() <<endl;
            if( !SSurf.ReadFromFile(*this))
                return false;
            switch(SSurf.option)
            {
            case 2:
            case 3:
                parmname[0]="increment";
                break;
            case 4:
                parmname[0]="unused";
            }

            for(int j=0; j < numParameters[type]; ++j )
            {
                *this >> SSurf.param[j];
                 cout <<parmname[j] << " " << SSurf.param[j] << endl;
            }

            if(pelemID)
            {
                cout << "Slits are not implemented under Optix : Element is replaced by a plane Film\n";
                *pelemID=CreateElement("Film<Plane>",name.c_str());
                elem=(ElementBase*)*pelemID;
            }

        }
        break;
     case   SORGENTERANDOMGAUSSIANA:    // 11
        {
            vector<string> nom={"nombre de points", "lambda", "sigmaY", "sigmaZ", "sigma Yp",  "sigmaZp", "sensibilitapente"};

            *this >> aux >> poleNormal ;
            for(int j=0; j < numParameters[type]; ++j )
                *this >> SSurf.param[j];
            cout << "\nPole Normal\n" << poleNormal.transpose() <<endl;
            cout << "Auxiliary axis \n" << aux.transpose() <<endl;
            for(int j=0; j < numParameters[type]; ++j )
                cout << nom[j] << " " << SSurf.param[j] <<  ((j%2) ? "\n" : "         " );

            if(pelemID)
            {
                *pelemID=CreateElement("Source<Gaussian>",name.c_str());
                elem=(ElementBase*)*pelemID;
                Parameter param;
                elem->getParameter("nRays", param);
                param.value=SSurf.param[0];
                elem->setParameter("nRays", param);

                elem->getParameter("sigmaX", param);
                param.value=SSurf.param[2];
                param.bounds[0]=SSurf.varparamin[2];
                param.bounds[1]=SSurf.varparamax[2];
                elem->setParameter("sigmaX", param);
                elem->getParameter("sigmaY", param);
                param.value=SSurf.param[3];
                param.bounds[0]=SSurf.varparamin[3];
                param.bounds[1]=SSurf.varparamax[3];
                elem->setParameter("sigmaY", param);
                elem->getParameter("sigmaXdiv", param);
                param.value=SSurf.param[4];
                param.bounds[0]=SSurf.varparamin[4];
                param.bounds[1]=SSurf.varparamax[4];
                elem->setParameter("sigmaXdiv", param);
                elem->getParameter("sigmaYdiv", param);
                param.value=SSurf.param[5];
                param.bounds[0]=SSurf.varparamin[5];
                param.bounds[1]=SSurf.varparamax[5];
                elem->setParameter("sigmaYdiv", param);
            }
        }
        break;
     case   TORUSDEFORMED:    //           12
        {
         //   double majorRadius, minorRadius, Ax3, Ax4, Ax5, Axy2;
        //    vector <string> parmname={"Major radius","minor radius","Ax3", "Axy2", "Ax4", "Ax5", "sigmaslopeLong", "sigmaslopeTrans"};

            ArrayXd torusAxis(7);

            *this >> aux >> poleNormal >> torusAxis ;
            for(int j=0; j < numParameters[type]; ++j )
                *this >> SSurf.param[j];
            if( !SSurf.ReadFromFile(*this))
                return false;
            cout << "\nPole Normal\n" << poleNormal.transpose() <<endl;
            cout << "Auxiliary axis \n" << aux.transpose() <<endl;
            cout << "Toroid axis \n" << torusAxis.transpose() <<endl;
            cout << "Major radius " << SSurf.param[0] << "    minor radius "<< SSurf.param[1] <<endl;
            cout << "Deformation " << SSurf.param[2] << " X^3 + " << SSurf.param[3] << " X Y^2 + " << SSurf.param[4]
                        << " X^4 + " << SSurf.param[5] << " X^5\n";
            cout << "Slope  sigmas    tang. " << SSurf.param[6] << " sag. " << SSurf.param[7] << endl;

            if(pelemID)
            {
                if(SSurf.param[2]!=0 || SSurf.param[3]!=0 || SSurf.param[4]!=0 || SSurf.param[5]!=0 )
                {
                    cout << "Toroid deformation will be replaced  by a polynomial surface \n please input the surface limits: Xmin Xmax Ymine Ymax\n";
                    double Xmin,Xmax, Ymin,Ymax;
                    std::cin >> Xmin >> Xmax >> Ymin >> Ymax;
                    cout << "coefficients will be computed inside rectangle " << Xmin << ", "<< Xmax << ", " << Ymin << ", " << Ymax << endl;
                    *pelemID=CreateElement("Mirror<NaturalPolynomialSurface>",name.c_str());
                    elem=(ElementBase*)*pelemID;
                    Parameter param(2, 2, Distance, ShapeGroup); // create an array parameter
                    if(!elem->getParameter("surfaceLimits",param))
                    {
                        ForwardOptiXLastError(__FILE__, __func__, __LINE__);
                        return false;
                    }
                    cout << "Setting surfaceLimits \n";
                    ArrayXXd mat(2,2);
                    mat << Xmin, Ymin, Xmax, Ymax;
                    *param.paramArray=mat;
                    DumpArgParameter(&param);
                    elem->setParameter("surfaceLimits",param);
                    elem->dumpParameter("surfaceLimits");
//                    int nx=2,ny=1;
//                    if(SSurf.param[3] !=0)
//                        ny=3;
                    int nx=3,ny=3;
                    if(SSurf.param[5] !=0)
                        nx=6;
                    else if(SSurf.param[4] !=0)
                        nx=5;
                    else if(SSurf.param[2] !=0)
                        nx=4;
                    param=Parameter(nx,ny,Distance,ShapeGroup);
                    if(!elem->getParameter("coefficients",param))
                    {
                        ForwardOptiXLastError(__FILE__, __func__, __LINE__);
                        return false;
                    }
                    cout << "Setting coefficients \n";
                    mat=ArrayXXd::Zero(nx,ny);
                    mat(2,0)=1./(2.*SSurf.param[0]);
                    mat(0,2)=1./(2.*SSurf.param[1]);
                    if(SSurf.param[3] !=0)
                        mat(1,2)=SSurf.param[3]*Xmax*Ymax*Ymax;
                    if(SSurf.param[5] !=0)
                        mat(5,0)=SSurf.param[5]*pow(Xmax,5);
                    else  if(SSurf.param[4] !=0)
                        mat(4,0)=SSurf.param[4]*pow(Xmax,4);
                    else  if(SSurf.param[2] !=0)
                        mat(3,0)=SSurf.param[2]*pow(Xmax,3);
                    *param.paramArray=mat;
                    DumpArgParameter(&param);

                    elem->setParameter("coefficients",param);
                    elem->dumpParameter("coefficients");
                    cout << "parameter set\n";
                }
                else
                {
                    *pelemID=CreateElement("Mirror<Toroid>",name.c_str());
                    elem=(ElementBase*)*pelemID;
                    Parameter param;
                    elem->getParameter("major_curvature", param);
                    param.value=1./SSurf.param[0];
                    param.bounds[0]=1./SSurf.varparamax[0];
                    param.bounds[1]=1./SSurf.varparamin[0];
                    elem->setParameter("major_curvature", param);
                    elem->getParameter("minor_curvature", param);
                    param.value=1./SSurf.param[1];
                    param.bounds[0]=1./SSurf.varparamax[1];
                    param.bounds[1]=1./SSurf.varparamin[1];
                    elem->setParameter("minor_curvature", param);
                }
            }
        }
        break;
     case   SORGENTERANDOMGAUSSIANADIVLINEARE:    // 13
        {
            cout << "NOT IMPLEMENTED\n";
        }
        break;
     case   SURFPOL:    //  14
        {
            cout << "NOT IMPLEMENTED\n";
        }
        break;
     case   RESEAUXOLPLANDEVCONSTANGLE:    // 15
        {
            cout << "NOT IMPLEMENTED\n";
        }
        break;
     case   RESEAUXOLSPHERDEVCONSTANGLE:    // 16
        {
            cout << "NOT IMPLEMENTED\n";
        }
        break;
     case   RESEAUXVLSSPHERDEVCONSTANGLE:    // 17
        {
            cout << "NOT IMPLEMENTED\n";
        }
        break;
     case   FILMSPHERE:    // 18
        {
            cout << "NOT IMPLEMENTED\n";
        }
        break;
     case   RESEAUXOLVLSSPHERDEVCONSTANGLE:    // 19
        {
            cout << "NOT IMPLEMENTED\n";
        }
        break;
     case   RESEAUXOLTORUSDEVCONSTANGLE:    // 20
        {
            cout << "NOT IMPLEMENTED\n";
        }
        break;
     case   RESEAUXOLSPHERTRANSM:    // 21
        {
            cout << "NOT IMPLEMENTED\n";
        }
        break;
     case   SURFACECOPIE:    // 22
        {
            cout << "NOT IMPLEMENTED\n";
        }
        break;
     case   RESEAUXOLVLSSPHERTRANSM:    // 23
        {
            cout << "NOT IMPLEMENTED\n";
        }
        break;
     case   CONO:    // 24
        {
            cout << "NOT IMPLEMENTED\n";
        }
        break;
     case   SORGENTEONDULEURGAUSSIANA:    // 25
        {
             vector<string> nom={"nombre de points", "lambda", "sigmaY", "sigmaZ", "WaistY", "WaistZ", "sigma Yp",  "sigmaZp", "sensibilitapente"};
            // Attn la position du waist n'est pas sur le fichier mais définie par le script TCL
            memset(SSurf.param,0, numParameters[type] );
            *this >> aux >> poleNormal ;
            for(int j=0; j < 4; ++j )
                *this >> SSurf.param[j];
            for(int j=6; j < numParameters[type]; ++j )
                *this >> SSurf.param[j];    // bounds are saved in the file
            cout << "\nPole Normal\n" << poleNormal.transpose() <<endl;
            cout << "Auxiliary axis \n" << aux.transpose() <<endl;
            for(int j=0; j < numParameters[type]; ++j )
                cout << nom[j] << " " << SSurf.param[j] <<  ((j%2) ? "\n" : "         " );

            if(pelemID)
            {
              //  cout << "Waist Z shifts are not implemented under OptiX\n";
                *pelemID=CreateElement("Source<Astigmatic,Gaussian>",name.c_str());
                elem=(ElementBase*)*pelemID;
                Parameter param;
                elem->getParameter("nRays", param);
                param.value=SSurf.param[0];
                elem->setParameter("nRays", param);

                elem->getParameter("sigmaX", param);
                param.value=SSurf.param[2];
//                param.bounds[0]=SSurf.varparamin[2];  // 09/07/21 ces valeurs ne semblent fas figurer dans le fichier Solemio
//                param.bounds[1]=SSurf.varparamax[2];
                elem->setParameter("sigmaX", param);
                elem->getParameter("sigmaY", param);
                param.value=SSurf.param[3];
//                param.bounds[0]=SSurf.varparamin[3];
//                param.bounds[1]=SSurf.varparamax[3];
                elem->setParameter("sigmaY", param);
                elem->getParameter("sigmaXdiv", param);
                param.value=SSurf.param[6];
//                param.bounds[0]=SSurf.varparamin[6];
//                param.bounds[1]=SSurf.varparamax[6];
                elem->setParameter("sigmaXdiv", param);
                elem->getParameter("sigmaYdiv", param);
                param.value=SSurf.param[7];
//                param.bounds[0]=SSurf.varparamin[7];
//                param.bounds[1]=SSurf.varparamax[7];
                elem->setParameter("sigmaYdiv", param);
            }

        }
        break;
     case SYSTEMGLOBALPARAMETERS:   //  26
        {
            if( !SSurf.ReadFromFile(*this))
                return false;
            if (version > 22 )
            {
                cout << "Parameter list\n";
                for(int j=0; j<MAXGLOBALPARAMS; ++j)
                {
                    *this >> SSurf.param[j];
                    cout << SSurf.param[j] << "  ";
                }
                cout << endl;
            }
                        if(pelemID)
            {
                *pelemID=CreateElement("Film<Plane>",name.c_str());
                elem=(ElementBase*)*pelemID;
                cout << "Parameter list " <<  name <<  " replaced by a film\n";
            }

        }
        break;
    case   SOURCEAIMANT:    // 27
        {
            vector<string> nom={"nombre de points", "lambda", "sigmaY", "sigmaZ", "Yp in", "Yp out", "sigmaZp", "rayon e-", "sensibilitapente"};

            *this >> aux >> poleNormal ;
            for(int j=0; j < numParameters[type]; ++j )
                *this >> SSurf.param[j];
            cout << "\nPole Normal\n" << poleNormal.transpose() <<endl;
            cout << "Auxiliary axis \n" << aux.transpose() <<endl;
            for(int j=0; j < numParameters[type]; ++j )
                cout << nom[j] << " " << SSurf.param[j] <<  ((j%2) ? "\n" : "         " );

        }
        break;
    default:  // cas d'appel à serializza() de la classe de base Surface
        if( !SSurf.ReadFromFile(*this))
            return false;
     }
     // set Current parameters : Omega=0,Theta=1,dOmega=2,dTheta=3,Spin=4,dX=5,dY=6,dZ=7,distback=8,
     //                          distfor=9, algtLambda=10,OsloBack=11,clipX1=12,clipX2=13,clipY1=14,clipY2=15
     if(!elem)
     {
        // cout << "error return from get_element\n";
         return false;
     }
     Parameter param;
     elem->getParameter("distance", param); //=distBackward
     param.value=elemParam[8];
     param.bounds[0]=elemParamin[8];
     param.bounds[1]=elemParamax[8];
     elem->setParameter("distance", param);
     elem->getParameter("theta", param); //(Pi -Theta)/2. déviation si reflechissant Theta sinon
     if(elem->getTransmissive())
     {
         param.value=elemParam[1];
         param.bounds[0]=elemParamin[1];
         param.bounds[1]=elemParamax[1];
     }
     else
     {
         param.value=(M_PI -elemParam[1])/2.;
         param.bounds[0]=(!elemParamvar[1]&& (elemParamax[1]==0))? 0 : (M_PI-elemParamax[1])/2.;
         param.bounds[1]=(!elemParamvar[1]&& (elemParamax[1]==0))? 0 : (M_PI-elemParamin[1])/2.;
     }
     elem->setParameter("theta", param);
     elem->getParameter("phi", param);  // Omega
     param.value=elemParam[0];
     param.bounds[0]=elemParamin[0];
     param.bounds[1]=elemParamax[0];
     elem->setParameter("phi", param);
     elem->getParameter("psi", param); // Spin
     param.value=elemParam[4];
     param.bounds[0]=elemParamin[4];
     param.bounds[1]=elemParamax[4];
     elem->setParameter("psi", param);
     elem->getParameter("Dtheta", param); //-dTheta)/2. Déviation si réfléchssant
     if(elem->getTransmissive())
     {
         param.value=elemParam[3];
         param.bounds[0]=elemParamin[3];
         param.bounds[1]=elemParamax[3];
     }
     else
     {
         param.value=-elemParam[3]/2.;
         param.bounds[0]=(!elemParamvar[3]&& (elemParamax[3]==0))? 0 : -elemParamax[3]/2.;
         param.bounds[1]=(!elemParamvar[3]&& (elemParamax[3]==0))? 0 : -elemParamin[3]/2.;
     }
     elem->setParameter("Dtheta", param);
     elem->getParameter("Dphi", param);  // dOmega
     param.value=elemParam[2];
     param.bounds[0]=elemParamin[2];
     param.bounds[1]=elemParamax[2];
     elem->setParameter("Dphi", param);
     elem->getParameter("DX", param);  // dX
     param.value=elemParam[5];
     param.bounds[0]=elemParamin[5];
     param.bounds[1]=elemParamax[5];
     elem->setParameter("DX", param);
     elem->getParameter("DY", param);  // dX
     param.value=elemParam[6];
     param.bounds[0]=elemParamin[6];
     param.bounds[1]=elemParamax[6];
     elem->setParameter("DY", param);
     elem->getParameter("DZ", param);  // dZ
     param.value=elemParam[7];
     param.bounds[0]=elemParamin[7];
     param.bounds[1]=elemParamax[7];
     elem->setParameter("DZ", param);
    //dPsi non defini dans Solemio & clipping not yet implemented in OptiX


     *this  >> imprunt >> sourcePol ;
     cout <<"imprunt " << imprunt <<  "  source Poly " << sourcePol << endl;

     skipline();
     if(!check_comment(" COMANDITCL  "))
        return false;
     getScript(tclScript);
   //  cout << " Script Tcl "  << endl << tclScript <<endl;
     cout << "TCL script " << tclScript.length() << "bytes\n";
     if(tclScript.size() !=0)
     {
        parseScriptVariables(tclScript, tclDict );
        if(type==SORGENTEONDULEURGAUSSIANA)
        {
            cout <<"add waist position from script variables\n";
            double lambda= SSurf.param[1] ;
            double centerOnd=0,sdOffset=0;
            bool csdRef=false;
            if( tclDict.find("Cond")!= tclDict.end())
            {
                if(tclDict["Cond"]!="")
                {
                    centerOnd=stod(tclDict["Cond"]);
                    sdOffset=0;
                    csdRef=true;
                }
            }
            cout << "ondulator referred to CSD " << csdRef <<endl;
            cout << "ondulator Cond = " << centerOnd <<endl;
            cout << "ondulator CSD in dict : " << tclDict["Csd"] <<endl;
            if(tclDict["Csd"] == "\"\""){
                    cout << "Csd read as empty string, setting to 0" << endl;
                    tclDict["Csd"] = "0";
            }
            cout << "ondulator CSD = " << stod(tclDict["Csd"]) <<endl;
            cout<<"csd ref " <<csdRef << " csd found "<<(tclDict.find("Csd")!=tclDict.end())<< " =>condition " << ((!csdRef) && tclDict.find("Csd")!=tclDict.end())<<endl;
            if( tclDict.find("Csd")!=tclDict.end())
            {
              sdOffset=stod(tclDict["Csd"]);
              centerOnd -= sdOffset;
            }
            else
            {
                SetOptiXLastError("Unknown undulator script type",__FILE__,__func__, __LINE__);
                return false;
            }
            double longOnd=stod(tclDict["Lond"]);
            double detuningFactor=1.;
            if(tclDict.find("Ka")!= tclDict.end())
                detuningFactor=stod(tclDict["Ka"]);
            double sigmaH=stod(tclDict["Sh"]);
            double sigmaV=stod(tclDict["Sv"]);
            double sigmaPrimH=stod(tclDict["Sph"]);
            double sigmaPrimV=stod(tclDict["Spv"]);
            cout << "dist SD to ond= " << centerOnd << "  Long ond= " << longOnd <<endl;
            cout << "sigmas H,V " << sigmaH << " " << sigmaV <<endl;
            cout << "sigmaPrims H,V " << sigmaPrimH << " " << sigmaPrimV <<endl;
            double diffSizeSquare=lambda*longOnd/(detuningFactor*8* M_PI*M_PI);
            double diffDivSquare=lambda*detuningFactor/(longOnd*2);
            double temp=sigmaPrimH*sigmaPrimH+diffDivSquare;
            double waistH=sdOffset+centerOnd*diffDivSquare/ temp ;
            double sigmaPrimTotH=  sqrt(temp)  ;
            temp=sigmaPrimV*sigmaPrimV+diffDivSquare;
            double waistV=sdOffset+centerOnd*diffDivSquare/temp ;
            double sigmaPrimTotV=sqrt(temp);
            temp=centerOnd*centerOnd;
            double sigmaWidenH2=temp/(1./(sigmaPrimH*sigmaPrimH)+1./diffDivSquare);
            double sigmaWidenV2=temp/(1./(sigmaPrimV*sigmaPrimV)+1./diffDivSquare);
            double sigmaTotH=sqrt(sigmaH*sigmaH+diffSizeSquare+sigmaWidenH2);
            double sigmaTotV=sqrt(sigmaV*sigmaV+diffSizeSquare+sigmaWidenV2);

            cout << "waists H & V  " <<waistH << "  " << waistV <<endl;
            cout << "Sigmas H & V  " <<sigmaTotH << "  " << sigmaTotV <<endl;
            cout << "SigmaPrims  H & V  " <<sigmaPrimTotH << "  " << sigmaPrimTotV <<endl;

                elem->getParameter("waistX", param);
                param.value=waistH;
                elem->setParameter("waistX", param);
                elem->getParameter("waistY", param);
                param.value=waistV;
                elem->setParameter("waistY", param);
                elem->getParameter("sigmaX", param);
                param.value=sigmaTotH;
                elem->setParameter("sigmaX", param);
                elem->getParameter("sigmaY", param);
                param.value=sigmaTotV;
                elem->setParameter("sigmaY", param);
                elem->getParameter("sigmaXdiv", param);
                param.value=sigmaPrimTotH;
                elem->setParameter("sigmaXdiv", param);
                elem->getParameter("sigmaYdiv", param);
                param.value=sigmaPrimTotV;
                elem->setParameter("sigmaYdiv", param);

        }
     }
     if(version > 14)
        *this >>XYZalign;

     if(version >21)  // redondant mais coherent avec les pvaleurs des paramètres courants
         *this >> clipX1 >> clipX2 >>clipY1 >> clipY2 >> clipping >> activeFilm ;

     cout << "X bounds " << clipX1 <<"  " << clipX2 <<endl;
     cout << "Y bounds " << clipY1 <<"  " << clipY2 <<endl;
     if(XYZalign) cout << "alignement XYZ  " ; else  cout << "alignement relatif  " ;
     if(clipping) cout << "clipping actif  " ; else  cout << "clipping inactif  ";
     if(activeFilm) cout << "recording impacts\n"; else cout << "not recording impacts\n";
     if(elem && activeFilm)
            dynamic_cast<Surface*>(elem)->setRecording(RecordOutput);

     skipline(1);

     return true;
 }

 void ReadSolemioFile(string filename)
 {
    int i, numElem=0,numMinim=0, numPoly=0, numInteg=0, numHistory=0;
    string comment , titre, author, contactComment, projectComment, date, sourcefile, savedfile;
    SolemioFile Sfile(filename.c_str());
    if(!Sfile.is_open())
    {
         cout << "can't open the file\n";
         return;
    }
    cout << "\n-------------------------------------------------------------------------------------\n\n";

    cout << "file version " << Sfile.version << endl;

    Sfile.skipline(4);  // skip main window size
    if(! Sfile.check_comment(" numero_elementivirtuali  "))
        return;
    Sfile >> numElem  ;
    cout << "number of elements " << numElem << endl;

    Sfile.skipline(2);
    cout << "\n-------------------------------------------------------------------------------------\n\n";

    for(i=0; i <numElem; ++i)
    {
        cout << "ELEMENT " << i <<endl << endl;
        if (!Sfile.get_element())
            break;

        cout << "\n-------------------------------------------------------------------------------------\n\n";
    }
    cout << "LINKAGE SOLEMIO\n\n";
    SolemioFile::LinkMap::iterator it, itl;
    string sprev, snext;
    for(it= Sfile.iconTable.begin(); it !=Sfile.iconTable.end();++it)
    {
        sprev=snext="NONE";
        if(it->second.prev)
        {
            itl=Sfile.iconTable.find(it->second.prev);
            if(itl==Sfile.iconTable.end())
                sprev="NOT_FOUND";
            else
                sprev=itl->second.name;
        }
        if(it->second.next)
        {
            itl=Sfile.iconTable.find(it->second.next);
            if(itl==Sfile.iconTable.end())
                snext="NOT_FOUND";
            else
                snext=itl->second.name;
        }
        cout << it->second.name << "  linked from " << sprev << " to "<< snext << endl;
    }
    cout << "\nLINKAGE BEAMLINE\n\n";

    for(it= Sfile.elemTable.begin(); it !=Sfile.elemTable.end();++it)
    {
        sprev=snext="NONE";
        if(it->second.prev)
        {
            itl=Sfile.elemTable.find(it->second.prev);
            if(itl==Sfile.elemTable.end())
                sprev="NOT_FOUND";
            else
                sprev=itl->second.name;
        }
        if(it->second.next)
        {
            itl=Sfile.elemTable.find(it->second.next);
            if(itl==Sfile.elemTable.end())
                snext="NOT_FOUND";
            else
                snext=itl->second.name;
        }
        cout << it->second.name << "  linked from " << sprev << " to "<< snext << endl;
    }

    cout << "\n-------------------------------------------------------------------------------------\n\n";

   // Sfile.skipline(1);
    if(! Sfile.check_comment(" numero_minimizzatori  "))
        return;
    Sfile >> numMinim  ;
    cout <<"Number of minimizers " << numMinim << endl;
    Sfile.skipline(2);

    for(i=0; i < numMinim; ++i)
    {
        int j, option, n_parameters=0, unitFminim[MAXFLOATING], unitminFminim[MAXFLOATING], unitmaxFminim[MAXFLOATING];
        uint32_t pMinimobj, pMinimsource, pMinimout;
        string name, tclScript;

        Sfile.skipline(3); // saute le positionnement sur le plan de travail
        if(! Sfile.check_comment(" unita_unitamin_unitamax_FLOATING  "))
            break;

        for(j=0; j< MAXFLOATING; ++j)
            Sfile >> unitFminim[j] >> unitminFminim[j] >> unitmaxFminim[j];

        Sfile.skipline(1); // ligne suivante
        Sfile.getPrefixedString(name);
        cout << "Minimizer " << i  << " : " << name <<endl;

        Sfile.skipline(1);
        if(! Sfile.check_comment(" minimizzatore  "))
            break;
        Sfile >> pMinimobj >> pMinimsource >> pMinimout ;
        cout  << "object "  << pMinimobj << " source "  << pMinimsource << " image " << pMinimout << endl;

        sprev=snext="NONE";

        itl=Sfile.iconTable.find(pMinimsource);
        if(itl==Sfile.iconTable.end())
            sprev="NOT_FOUND";
        else
            sprev=itl->second.name;

        itl=Sfile.iconTable.find(pMinimout);
        if(itl==Sfile.iconTable.end())
            snext="NOT_FOUND";
        else
            snext=itl->second.name;

        cout << "Minimizer  " <<  name << "  linked from " << sprev << " to "<< snext << endl;


        Sfile.skipline(1);
        Sfile >> option;
        switch (option)
        {
        case 0:
            n_parameters = 63 ;
            break;
        case 1:
            n_parameters = 11;
            break;
        case 2:
            n_parameters = 15 ;
        }
        double parameter[n_parameters];
        for(int j =0; j < n_parameters; ++j)
        {Sfile >> parameter[j];
        cout << parameter[j] << " ";
        }
        cout << endl;
        Sfile.skipline(1);
        if(! Sfile.check_comment(" COMANDITCL  "))
            break;
        Sfile.getScript(tclScript);
        //  cout << " Script Tcl "  << endl << tclScript <<endl;
        cout << "TCL script " << tclScript.length() << " bytes\n";

        cout << "-------------- End Minimizer " << i << "------------------------------\n\n";

    }

    if(! Sfile.check_comment(" numero_polinomidifase  "))
        return;
    Sfile >> numPoly  ;
    cout <<"Number of polynomes de phase " << numPoly << endl;
    Sfile.skipline(2);

    if(numPoly >0)
    {
        cout << "Reading Phase Polynomial is not implemented\n";
        return;
    }

    if(! Sfile.check_comment(" numero_integratori  "))
        return;
    Sfile >> numInteg  ;
    cout <<"Number of polynomes de phase " << numInteg << endl;
    Sfile.skipline(2);

    if(numInteg >0)
    {
        cout << "Reading Phase Integratorl is not implemented\n";
        return;
    }

    if(! Sfile.check_comment(" ciaociao"))
        return;
    Sfile.skipline(1);

    Sfile >> numHistory;
    cout << "\nNombre de versions dans l'historique  " << numHistory <<endl;
    for(i=0; i < numHistory; ++i)
    {
         Sfile.getScript(titre);
         Sfile.getScript(author);
         Sfile.getScript(contactComment);
         Sfile.getScript(projectComment);
         Sfile.getScript(date);
         Sfile.getScript(sourcefile);
         Sfile.getScript(savedfile);
         cout << titre << " " << author << " " << contactComment << " " << projectComment << " " << date << "\n" << sourcefile << " " << savedfile <<endl;
    }


    Sfile.close();
 }



 bool SolemioImport(string filename)
 {
    int i, numElem=0 ;    //   ,numMinim=0, numPoly=0, numInteg=0, numHistory=0;
    string errstr, comment , titre, author, contactComment, projectComment, date, sourcefile, savedfile;
    SolemioFile Sfile(filename.c_str());
    if(!Sfile.is_open())
    {
         errstr="can't open the file " + filename;
         SetOptiXLastError(errstr, __FILE__, __func__);
         return false;
    }
    cout << "\n-------------------------------------------------------------------------------------\n\n";

    cout << "file version " << Sfile.version << endl;

    Sfile.skipline(4);  // skip main window size
    if(! Sfile.check_comment(" numero_elementivirtuali  "))
    {
         errstr="Format error  while reading file  " + filename;
         SetOptiXLastError(errstr, __FILE__, __func__, __LINE__);
         return false;
    }
    Sfile >> numElem  ;
    cout << "number of elements " << numElem << endl;

    Sfile.skipline(2);
    cout << "\n-------------------------------------------------------------------------------------\n\n";
    ElementBase * pelement=0;
    for(i=0; i <numElem; ++i)
    {
        cout << "ELEMENT " << i <<endl << endl;
        if (!Sfile.get_element((size_t*) &pelement))
        {
             char buf[256];
             sprintf(buf,"Format error  while getting element #%d in file %s loading stopped:\n", i, filename.c_str());
             errstr=buf;
             GetOptiXLastError(buf,256);
             SetOptiXLastError(errstr+buf, __FILE__, __func__, __LINE__);
             return false;
        }

        if(pelement)
            cout << pelement->getName()  << " Created as " << pelement << endl;
        else
            cout  << "Element NOT CREATED\n";
        cout << "\n-------------------------------------------------------------------------------------\n\n";
    }
    cout << "SOLEMIO LINKAGE\n\n";
    SolemioFile::LinkMap::iterator it, itl;
    map<string,ElementBase*>:: iterator elit, pointedElemIt;
    string sprev, snext;
    SolemioLinkType curlink;
    int32_t prevptr, nextptr;
    for(it= Sfile.iconTable.begin(); it !=Sfile.iconTable.end();++it)
    {
        sprev=snext="NONE";
        curlink=it->second;
        elit= System.find(curlink.name);
        if(elit!=System.end())
        {
            prevptr=curlink.prev;
            for(;;)
            {
                if(prevptr==0)
                    elit->second->setPrevious(NULL);
                else
                {
                    itl=Sfile.iconTable.find(prevptr);
                    if(itl==Sfile.iconTable.end())
                        sprev="NOT_FOUND";
                    else
                    {
                         sprev=itl->second.name;
                         pointedElemIt=System.find(sprev);
                         if(pointedElemIt!=System.end())
                            elit->second->setPrevious(pointedElemIt->second);
                         else
                         {
                             prevptr =itl->second.prev ;
                             continue;
                         }
                    }
                }
                break;
            }

            nextptr=curlink.next;
            for(;;)
            {
                if(nextptr==0)
                    elit->second->setNext(NULL);
                else
                {
                    itl=Sfile.iconTable.find(nextptr);
                    if(itl==Sfile.iconTable.end())
                        snext="NOT_FOUND";
                    else
                    {
                         snext=itl->second.name;
                         pointedElemIt=System.find(snext);
                         if(pointedElemIt!=System.end())
                            elit->second->setNext(pointedElemIt->second);
                         else
                         {
                             nextptr =itl->second.next ;
                             continue;
                         }
                    }
                }
                break;
            }

            cout << elit->second << ": " << it->second.name << "  linked from " << sprev << " to "<< snext << endl;
        }
        else
        {
            cout << "element " << it->second.name  << " not created \n";
        }
    }  // next elem in linktable

    Sfile.close();
    return true;
 }

//  *************************    Configuration file    *************************

inline size_t tokenize(char* line, vector<string> & tokens)
{
    char *pstr=strtok(line," ");
    tokens.clear();
    while(pstr && pstr[0]!='#') // discard all comments
    {
        tokens.emplace_back(pstr);
        pstr=strtok(NULL," ");
    }
    return tokens.size();
}

double ValueOf(string strIn)
{
    size_t openPar=strIn.find('(');
    if (openPar==string::npos)
        return stod(strIn);
    string macro=strIn.substr(0,openPar);
    size_t closePar=strIn.find(')',openPar);
    if(closePar==string::npos)
    {
        cout << "\n  macro closing parenthesis not found\n";
        closePar=string::npos;
    }
    double val=stod(strIn.substr(openPar+1,closePar-openPar-1));
    if(macro=="INV")
        return 1./val;
    else if(macro=="DEGREE")
        return M_PI*val/180.;
    else
    {
        cout << "\n UNKNOWN MACRO  " << macro << endl;
        return NAN;
    }
}
int OpacityOf(string strIn)
{
    if(strIn=="open"|| strIn=="false" || strIn=="0")
        return 0;
    else if(strIn=="opaque"|| strIn=="true" || strIn=="1")
        return 1;
    else
        return -1;
}

 bool LoadConfiguration(string filename)
 {
    char line[256];
    size_t level=0, tokCount, elementID=0;
    std::priority_queue<size_t> indentStack;
    indentStack.push(0);
    vector<string> token;
    string keyword, object, subObject, dbasePath;
    fstream Cfile(filename.c_str());
    if(!Cfile.is_open())
    {
         SetOptiXLastError(string("Can't open the config file ")+filename , __FILE__, __func__);
         return false;
    }

    while (!Cfile.eof())
    {
        Cfile.getline(line,256);
        if(Cfile.gcount()==1)continue;   //  The eol sequence is counted as 1

        size_t indent= strspn(line," ");
        if(line[indent]=='#'||line[indent]==0)
                continue;

        if(indent!=indentStack.top())
        {
            if(indent > indentStack.top())
            {
                ++level;
                indentStack.push(indent);
            }
            else
                do
                {
                    --level;
                    indentStack.pop();
                }while(indentStack.top()>indent);
        }

        if(level <2)
            elementID=0;
#ifdef HAS_REFLEX
        if(level==0)
        {
            if(keyword=="COATINGTABLE")
            {
                if(!CoatingTableCompute(object.c_str()))
                {
                    cout << "     FAILED ti compute Reflectivity.     Reason:  " << LastError  << endl;
                    return false;
                }
                else
                cout << "     REFLECTIVITY SUCCESSFULLY computed\n ";
            }
            keyword=strtok(line," ");
        }

        if(keyword=="DBASEPATH")
        {
            dbasePath= strtok(NULL," ");
            cout << "Path to databases: "<< dbasePath << endl;
        }
        else if(keyword=="DATABASE")
        {
            string dbase= dbasePath+"/"+strtok(NULL," ");
            cout << "Path to databases: "<< dbase <<endl;
            string dbname=OpenDatabase(dbase.c_str());
            if(dbname.empty())
            {
                cout << "   database WAS NOT OPEN reason:\n" << LastError << endl;
                return false;
            }
            cout << "   OPEN as  " << dbname <<endl;
        }
        else if(keyword=="INDEXTABLE")
        {
            switch(level)
            {
            case 0:
                if(tokenize(NULL,token) <1)
                {
                    SetOptiXLastError("No name after INDEXTABLE declaration", __FILE__, __func__);
                    return false;
                }
                else
                    object=token[0];
                cout << "IndexTable " << object ;
                if(!CreateIndexTable(object.c_str()))
                {
                    cout << "     Index Table NOT CREATED reason:\n" << LastError << endl;
                    return false;
                }
                cout << "    Created as  "    <<object <<endl;
                break;
            case 1:
                {
                    tokCount=tokenize(line, token);
                    if(tokCount <2)
                    {
                        SetOptiXLastError(string(" Incomplete material entry in Table ")+object, __FILE__, __func__);
                        return false;
                    }
                    cout << "      Material "<< token[0] << ":"<<token[1] ;
                    if(!AddMaterial(object.c_str(),token[0].c_str(), token[1].c_str()))
                    {
                        cout << "     NOT ADDED to table " <<object << " reason:\n" << LastError << endl;
                        return false;
                    }
                    cout << "    Added to table  "    <<object <<endl;
                }
            }
        }
        else if(keyword=="COATINGTABLE")
        {
            switch(level)
            {
            case 0:
                if(tokenize(NULL,token) <1)
                {
                    SetOptiXLastError("No name after COATINGTABLE declaration", __FILE__, __func__);
                    return false;
                }
                else
                    object=token[0];
                cout << "CoatingTable " << object ;
                if(!CreateCoatingTable(object.c_str()))
                {
                    cout << "     Coating Table NOT CREATED reason:\n" << LastError << endl;
                    return false;
                }
                cout << "    Created as  "    <<object <<endl;
                break;
            case 1:
                tokCount=tokenize(line, token);
                subObject=token[0];
                if(subObject=="ANGLERANGE")
                {
                    if(tokCount <4)
                    {
                        SetOptiXLastError(string("ANGLERANGE of table ")+object+"Should have 3 numeric parameters", __FILE__, __func__);
                        return false;
                    }
                    cout << "     Angle range " << token[1] << ", " << token[2] << ", " <<token[3] ;
                    if(! SetCoatingTableAngles(object.c_str(), stod(token[1]),stod(token[2]), stoll(token[3])))
                    {
                        cout << "     Angle range  NOT SET reason:\n" << LastError << endl;
                        return false;
                    }
                    cout << "    SET\n";
                }
                else if(subObject=="ENERGYRANGE")
                {
                    if(tokCount <3)
                    {
                        SetOptiXLastError(string("ENERGYRANGE of table ")+object+"Should have at least 2 explicit numeric parameters", __FILE__, __func__);
                        return false;
                    }
                    int64_t numE=(tokCount <4? -1:stoll(token[3]));
                    bool logscale;
                    if(tokCount<5 ||token[4]=="false" || token[4]=="0")
                        logscale=false;
                    else if(token[4]=="true" || token[4]=="1")
                        logscale=true;
                    else
                    {
                        SetOptiXLastError(string("invalid value for Energy range log-spacing in table ")+object, __FILE__, __func__);
                        return false;
                    }
                    cout << "     Energy range " << token[1] << ", " << token[2] << ", " <<numE <<", " <<(logscale? "true":"false") ;
                    if(! SetCoatingTableEnergies (object.c_str(), stod(token[1]),stod(token[2]), numE, logscale))
                    {
                        cout << "     Energy range  NOT SET reason:\n" << LastError << endl;
                        return false;
                    }
                    cout << "    SET\n";
                }
                else
                {
                    if(tokCount <4)
                    {
                        SetOptiXLastError(string("Coating entry ")+subObject+" of table "+object+"Should have 3 parameters", __FILE__, __func__);
                        return false;
                    }
                    cout << "     Coating  " << subObject << " substrate " << token[1] <<":" << token[2] << "  roughness " <<token[3] ;
                    if(! CreateCoating(object.c_str(), subObject.c_str(), token[1].c_str(), token[2].c_str()))
                    {
                        cout << "     Cooating  NOT SET reason:\n" << LastError << endl;
                        return false;
                    }
                    if(!SetCoatingRoughness(object.c_str(),subObject.c_str(),stod(token[3])))
                    {
                        cout << "     roughness  NOT SET reason:\n" << LastError << endl;
                        return false;
                    }
                    cout << "    SET\n";
                }
                break;
            case 2:
                {
                    tokCount=tokenize(line, token);
                    if(tokCount < 3)
                    {
                        cout <<"not enough tokens for a layer entry\n";
                        break;
                    }
                    double compactness=tokCount>=4 ? stod(token[3]): 1.;
                    cout << "              " << subObject << "  layer " << token[0] <<":" << token[1] << "  thickness " <<token[2] ;
                    if(tokCount>=4)
                        cout << "  compactness " << token[3] ;
                    if(! AddCoatingLayer(object.c_str(), subObject.c_str(), token[0].c_str(), token[1].c_str(),stod(token[2]), compactness))
                    {
                        cout << "     Layer  NOT ADDED reason:\n" << LastError << endl;
                        return false;
                    }

                    cout << "     Layer  ADDED\n";
                }
            }
        }
        else if(keyword=="BEAMLINE")
#else
        if(keyword=="BEAMLINE")
#endif  //HAS_REFLEX
        {
            switch(level)
            {
            case 0:
                cout << "Beamline elements:\n";
                break;
            case 1:
                tokCount=tokenize(line, token);
                if(tokCount <2)
                {
                    SetOptiXLastError("Beamline elements should be declared by class-name and name", __FILE__, __func__);
                    return false;
                }
                else
                {
                    object=token[1];
                    cout << "     Element  " << object << " of class " << token[0]  ;
                    elementID=CreateElement(token[0].c_str(), token[1].c_str());
                    if(!elementID)
                    {
                        cout << "     was NOT CREATED reason:\n" << LastError << endl;
                        return false;
                    }
                    cout << "   CREATED\n";
                }
                break;
            case 2:
                tokCount=tokenize(line, token);
                subObject=token[0];
                if(subObject=="APERTURE")
                {
                    bool activity=true;   // true is default
                    if(tokCount<2 ||token[1]=="active" || token[1]=="1")
                        activity=true;
                    else if(token[1]=="inactive" || token[1]=="0")
                        activity=false;
                    cout <<  "              APERTURE  ";
                    if(SetApertureActivity(elementID,activity))  // return -1 in case of an error
                    {
                        cout<< " CANNOT SET aperture activity. Reason:   "<< LastError <<endl;
                        return false ;
                    }
                    cout << (activity ? "active" : "inactive")<< endl;
                    break;
                    // Regions definition at level 3
                }
#ifdef HAS_REFLEX
                else if(token[0]=="COATING")
                {
                    cout <<  "              COATING  " ;
                    if(tokCount <3)
                    {
                        SetOptiXLastError("Coating entry should specify a coating table and a coating name ", __FILE__, __func__);
                        return false;
                    }
                    if(!elementID)
                    {
                        SetOptiXLastError("Coating is not referred to a valid optical element ", __FILE__, __func__);
                    }
                    cout << token[1].c_str() << ":" << token[2].c_str() ;
                    if(! SetCoating(elementID, token[1].c_str(), token[2].c_str() ) )
                    {
                        cout << "   NOT Set reason:\n" << LastError << endl;
                        return false;
                    }
                    cout <<   "  SET\n";
                    break;
                }
#endif //HAS_REFLEX

                else
                {
                    if(tokCount <2)
                    {
                        SetOptiXLastError("Parameter "+token[0]+" of  beamline element"+ object+" should have at least a value", __FILE__, __func__);
                        return false;
                    }
                    if(tokCount==3)
                    {
                        SetOptiXLastError("Parameter range "+token[0]+" of  beamline element"+ object+" should be given 2 prameters", __FILE__, __func__);
                        return false;
                    }
                    Parameter param;
                    if(!GetParameter(elementID,token[0].c_str(), &param))
                    {
                        cout << "  INVALID PARAMETER : " << token[0] <<endl;
                        return false ;
                    }
                    cout << "              " << token[0] << " = " << ValueOf(token[1]) ;
                    param.value=ValueOf(token[1]);
                    if(tokCount>3)
                    {
                        cout << "  range ["<< ValueOf(token[1]) << ", "<< ValueOf(token[3]) <<"]";
                        param.bounds[0]=ValueOf(token[1]);
                        param.bounds[1]=ValueOf(token[3]);
                    }
                    if(!SetParameter(elementID, token[0].c_str(), param))
                    {
                        cout << "  Can get but not set parameter : " << token[0] <<endl;
                        return false ;
                    }
                    cout <<  "     SET\n";
                }
                break;
            case 3:     // only Aperture subentries can be on  level 3
                if(subObject!= "APERTURE")
                {
                    cout <<"invalid config file structure: unexpected level\n";
                    return false;
                }
                    tokCount=tokenize(line,token);
                    if(token[0]=="RECTANGLE")
                    {
                        cout << "              RECTANGLE region \n" ;
                        double  angle=0, Xcenter=0, Ycenter=0;
                        int opacity=0;
                        switch(tokCount)
                        {
                        case 7:
                            angle=ValueOf(token[6]);
                        case 6:
                            Ycenter=ValueOf(token[5]);
                        case 5:
                            Xcenter=ValueOf(token[4]);
                        case 4:
                            opacity=OpacityOf(token[3]);
                            if(opacity==-1)
                            {
                                cout  << "     invalid opacity value \n";
                                return false;
                            }
                        case 3:
                            break;
                        default:
                             cout << "rectangle region need at least 2 explicit parameters\n";
                             return false;
                        }
                        if(AddRectangularStop(elementID, ValueOf(token[1]), ValueOf(token[2]), (opacity==1? true:false),Xcenter, Ycenter,angle))
                        {
                            cout << "  NOT ADDED to aperture definition. Reason:   " << LastError << endl;
                        }
                        cout << "  ADDED to aperture definition\n";
                    }
                    else  if(token[0]=="ELLIPTICAL")
                    {
                        cout << "              ELLIPTICAL region \n" ;
                    }
                    else  if(token[0]=="CIRCULAR")
                    {
                        cout << "              CIRCULAR region \n" ;
                    }
                    else   if(token[0]=="POLYGONAL")
                    {
                        cout << "              POLYGONAL region \n" ;
                    }
                    else
                    {
                        cout << "Unknown aperture region shape:  " << token[0] << endl;
                        return false;
                    }

            }
        }
        else if(keyword=="CHAIN")
        {
            tokCount=tokenize(NULL, token);
            auto it =token.begin();
            cout << " Chain : " << *it;
            for(++it; it != token.end(); ++it)
                cout <<" --> " << *it;

            cout <<endl;
        }
        else
            cout  << level <<'\t' << line << endl;
    };

     return true;
 }
