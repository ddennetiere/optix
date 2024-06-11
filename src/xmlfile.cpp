/**
 *************************************************************************
*   \file           xmlfile.cpp
*
*   \brief             implementation file
*
*
*
*   \author             François Polack  <francois.polack@synchroton-soleil.fr>
*   \date               Creation: 2021-05-04
*   \date               Last update: 2021-05-04
 ***************************************************************************/

#include <cstring>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

#include "xmlfile.h"
#include "sources.h"
#include "opticalelements.h"
#include "interface.h"

#define XMLSTR (xmlChar*)
//#define MAXBUF 512

extern bool enableReflectivity, enableApertureLimit, enableSurfaceErrors;


bool SaveElementsAsXml(const char * filename, ElementCollection &system)
{
    // This function is declared friend of ElementBase
//    char cvbuf[MAXBUF];
//    xmlDocPtr doc;
//    xmlNodePtr sysnode, elemnode, parmnode, arraynode;
//    xmlAttrPtr attr;
    xmlDocPtr doc = xmlNewDoc(XMLSTR "1.0");
    xmlNodePtr sysnode = xmlNewDocRawNode(doc, NULL, (const xmlChar*)"system", NULL);
    xmlDocSetRootElement(doc, sysnode);

    xmlNewProp(sysnode,XMLSTR "reflectivity", XMLSTR (enableReflectivity ? "on":"off"));
    xmlNewProp(sysnode,XMLSTR "aperture_limit", XMLSTR (enableApertureLimit ? "on":"off"));
    xmlNewProp(sysnode,XMLSTR "surface_errors",XMLSTR (enableSurfaceErrors ? "on":"off"));

    map<string,ElementBase*>::iterator it;
    for (it=system.begin(); it!=system.end(); ++it)
    {
        *it->second >> sysnode;  // will create the element node including Surface class parameters
                                // and call the Surface::operator>>(element node)
    }
    //xmlIndentTreeOutput = 1;  // to indent
    //  xmlKeepBlanksDefault(0);
    xmlSaveFormatFile(filename, doc, 1); // avec indentation
    return true;
}

/** \brief auxiliary function for DumpXmlSys */
void PrintParameters(xmlDocPtr doc, xmlNodePtr cur)
{
	xmlChar *att, *name;
    cur = cur->xmlChildrenNode;
	while (cur != NULL) {
	    if ((!xmlStrcmp(cur->name, XMLSTR "param")))
        {
            name = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		    printf("parameter: %s",name);
            xmlFree(name);
		    att= xmlGetProp(cur, XMLSTR "val");
		    if(att)
            {
                printf("  value: %s", att );
                xmlFree(att);
            }
            else
            {
                xmlNodePtr arraynode=cur->xmlChildrenNode->next;
                if(arraynode)
                {
                   // int dim0, dim1;
                    att=xmlGetProp(arraynode, XMLSTR "dims");
                    if(att)
                    {
                        printf("    Array %s ", att);
                      //  sscanf(att,"%d, %d", dim0,dim1);
                        xmlFree(att);
                    }
                    att=xmlGetProp(arraynode, XMLSTR "data");
                    if(att)
                    {
                        printf("\n           %s ", att);
                        xmlFree(att);
                    }
                }
                else
                    printf(" INCOMPLETE parameter no value found");
            }
		    att= xmlGetProp(cur, XMLSTR "min");
            if(att)
            {
                printf("  min: %s", att );
                xmlFree(att);
            }
            att= xmlGetProp(cur, XMLSTR "max");
            if(att)
            {
                printf("  max: %s", att );
                xmlFree(att);
            }
            att= xmlGetProp(cur, XMLSTR "mult");
            if(att)
            {
                printf("  multiplier: %s", att );
                xmlFree(att);
            }

		    printf("\n");
 	    }
	cur = cur->next;
	}
    return;
}

bool DumpXmlSys(const char* filename)
{
    xmlDocPtr doc;
    xmlNodePtr sysnode, curnode;
    xmlChar *attrStr, *nameStr, *nextelem;

	doc = xmlParseFile(filename);

	if (doc == NULL ) {
		SetOptiXLastError(string("Parsing ")+filename+ " failed", __FILE__, __func__);
		xmlFreeDoc(doc);
		return false;
	}
    sysnode = xmlDocGetRootElement(doc);

	if (sysnode == NULL) {
		fprintf(stderr,"empty document\n");
		xmlFreeDoc(doc);
		return false ;
	}
	curnode = sysnode->xmlChildrenNode;
	while (curnode != NULL)
    {
		if ((!xmlStrcmp(curnode->name, XMLSTR "element"))) // for xml, name is the tag;  The element name is an attribute
		{
			nameStr= xmlGetProp(curnode, XMLSTR "name");
			attrStr= xmlGetProp(curnode, XMLSTR "class");
			nextelem=xmlGetProp(curnode, XMLSTR "next");
            printf("%s class: %s  next: %s", nameStr, attrStr, nextelem);
            xmlFree(attrStr);
            xmlFree(nameStr);
            xmlFree(nextelem);
            attrStr= xmlGetProp(curnode, XMLSTR "trans");
            if(attrStr)  // if defined, value is 'true'. Value false is not written to XML
            {
                printf("  transmissive\n");
                xmlFree(attrStr);
            }
            attrStr= xmlGetProp(curnode, XMLSTR "rec");
            if(attrStr)
            {
                printf("  record: %d\n", atoi((char*)attrStr));
                xmlFree(attrStr);
            }
            else
                printf("\n");
            PrintParameters(doc,curnode);

		}

	curnode = curnode->next;
	}

    xmlFreeDoc(doc);
	xmlCleanupParser();
    return true;
}

/** \brief auxiliary fonction for loading parameter from XML. Iterates on the XML doc and sets the parameters a new element as they are read.
 * \ingroup GlobalCpp
 * \param doc Thee open XML document
 * \param elem pointer to the newly created element
 * \param[in] cur the current element node
 * \return bool
 * \deprecated This fonction is no longer user: functionnally replaced by operator<<() of Element classes
 */
bool SetXmlParameters(xmlDocPtr doc, ElementBase* elem, xmlNodePtr cur)
{
    cout << "\n USING SetXmlParameters\n\n";
	xmlChar *att, *name;
    cur = cur->xmlChildrenNode;
    Parameter param;
    bool success=true;
	while (cur != NULL)
	{
	    if ((!xmlStrcmp(cur->name, XMLSTR "param")))
        {
            //name = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            name=xmlNodeGetContent(cur);
            // initialise param avec les valeurs défaut du constructeur de elem
           // cout <<"setting parameter " << name << ":";
		    if(elem->getParameter((char*) name, param))
            {
                att= xmlGetProp(cur, XMLSTR "val");
                if(att)
                {
                    param.value=atof((char*)att);
                    xmlFree(att);
                }
                else
                {
                    //xmlNodePtr arraynode=cur->xmlChildrenNode->next;
                    xmlNodePtr arraynode=xmlFirstElementChild(cur);
                    if(arraynode)
                    {
                        int dim0=0, dim1=0;
                        att=xmlGetProp(arraynode, XMLSTR "dims");
                        if(att)
                        {
                            printf("    Array %s ", att);
                            sscanf((char*)att,"%d, %d", &dim0, &dim1);
                            xmlFree(att);
                        }
                        att=xmlGetProp(arraynode, XMLSTR "data");
                        if(att)
                        {
                            printf("\n           %s \n", att);
                            int N=dim0*dim1;
                            if(N)
                            {
                                if(param.paramArray)
                                    delete param.paramArray;
                                param.paramArray=new ArrayParameter(dim0,dim1);
                                double * pdat=param.paramArray->data;
                                char *pbuf= (char*)att;
                                sscanf(pbuf, "%lg", pdat);

                                for (int i=1; i <N; ++i)
                                {
                                    pbuf=strchr(pbuf,',');
                                    sscanf(++pbuf, "%lg", ++pdat);
                                }
                            }
                            xmlFree(att);
                        }
                    }
                    else
                        printf(" INCOMPLETE parameter no value found");
                }


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
                //    cout << "  do...  ";
                if(!elem->setParameter((char*) name, param))
                {
                    cout << " ERROR:  " << LastError <<endl;
                    success=false;
                }
                //else cout <<" OK\n";
                if(param.flags &ArrayData)
                    elem->dumpParameter((char*) name) ;
            }
            else
            {
                cout << "Parameter " << (char*) name << " is not valid for object " << elem->getName() <<
                        " of class " << elem->getOptixClass() << endl;
                success=false; // ce parametre n'existe pas dans cet objet : on signale
            }
            xmlFree(name);


        }
        // traiter ici ApertureStop et SurfaceGenerator
        cur = cur->next;
	}
    return success;
}

bool LoadElementsFromXml(const char * filename, ElementCollection &system)
{
    xmlDocPtr doc;
    xmlNodePtr sysnode, curnode;
    xmlChar *sclass, *sname, *nextelem, *attr; //, *srec, *trans;
    //size_t elemID;
    ElementBase *elem;
    map<ElementBase*, string> chaining;
    bool exitcode=true;

	doc = xmlParseFile(filename);

	if (doc == NULL ) {
		SetOptiXLastError(string("Parsing ")+filename+ " failed", __FILE__, __func__);
		xmlFreeDoc(doc);
		return false;
	}
    else
        cout << filename << " successfully parsed\n";

    sysnode = xmlDocGetRootElement(doc);

	if (sysnode == NULL)
    {
		fprintf(stderr,"empty document\n");
		xmlFreeDoc(doc);
		return false ;
	}

    attr=xmlGetProp(sysnode, XMLSTR "reflectivity");
    if(attr)
    {
        enableReflectivity=xmlStrEqual(attr,XMLSTR "on") ? true : false;
        xmlFree(attr);
    }
    attr=xmlGetProp(sysnode, XMLSTR "aperture_limit");
    if(attr)
    {
        enableApertureLimit=xmlStrEqual(attr,XMLSTR "on") ? true : false;
        xmlFree(attr);
    }
    attr=xmlGetProp(sysnode, XMLSTR "surface_errors");
    if(attr)
    {
        enableSurfaceErrors=xmlStrEqual(attr,XMLSTR "on") ? true : false;
        xmlFree(attr);
    }


	curnode = sysnode->xmlChildrenNode;
	while (curnode != NULL)
    {
		if ((!xmlStrcmp(curnode->name, XMLSTR "element"))) // for xml, name is the tag;  The element name is an attribute
		{

			sname= xmlGetProp(curnode, XMLSTR "name");
			sclass= xmlGetProp(curnode, XMLSTR "class");
			nextelem=xmlGetProp(curnode, XMLSTR "next");


            elem= system.createElement((char*) sclass, (char*) sname );
            if(elem==NULL)
            {
                char errstr[128];
                sprintf(errstr, "Cannot create element %s of type %s", sname, sclass);
                SetOptiXLastError("File reading error",__FILE__,__func__);
                // do the cleanup before leaving
            }
            else
            {
                cout << "element "<<sname<<" of type "<<sclass<<" created. ";
                if( nextelem)
                {
                    cout << " Will be linked to "<< nextelem << endl;
                    string nextname=(char*)nextelem;
                    chaining.insert(pair<ElementBase*,string>(elem,nextname));
                }
                else
                    cout << " Not further linked\n";
            }

            xmlFree(sclass);
            xmlFree(sname);
            xmlFree(nextelem);

            if(elem==NULL)   // now we can leave
                return false;

            // Surface class needs to be called before the base class to define supplementary parameters (namely Error generator)

            Surface* surf=dynamic_cast<Surface*>(elem);
            if(surf)
                *surf << curnode;

            cout << "entering elementbase load\n";
            *elem << curnode;

            cout <<"elementbase " << curnode->name  << "loaded\n";

		}
        curnode = curnode->next;
	}
	for(map<ElementBase*,string>::iterator it=chaining.begin(); it!=chaining.end(); ++it)
        (it->first)->chainNext(system.find(it->second)->second);

    return exitcode;
}
