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
 ***************************************************************************/#include <libxml/xmlmemory.h>

#include <cstring>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

#include "xmlfile.h"
#include "sources.h"
#include "opticalelements.h"
#include "interface.h"

#define XMLSTR (xmlChar*)

bool SaveElementsAsXml(const char * filename, ElementCollection &system)
{
    char cvbuf[16];
    xmlDocPtr doc;
    xmlNodePtr sysnode, elemnode, parmnode;
//    xmlAttrPtr attr;
    doc = xmlNewDoc(XMLSTR "1.0");
    sysnode = xmlNewDocRawNode(doc, NULL, (const xmlChar*)"system", NULL);
    xmlDocSetRootElement(doc, sysnode);

    map<string,ElementBase*>::iterator it;
    for (it=system.begin(); it!=system.end(); ++it)
    {
        ElementBase& elem = *it->second   ; // for convenience
        elemnode=xmlNewTextChild(sysnode,NULL, XMLSTR "element", NULL);
        xmlNewProp (elemnode, XMLSTR "name", XMLSTR it->first.c_str());
        xmlNewProp (elemnode, XMLSTR "class", XMLSTR elem.getOptixClass().c_str()); // on n'a pas besoin des pointeurs etournés
        //   pour le chaînage next suffit; Si l'attribut de chaînage next n'est écrit,
        //   il n'y a pas de suivant (m_next=null qui est le défaut de création)
        if(elem.getNext())
            xmlNewProp (elemnode, XMLSTR "next", XMLSTR elem.getNext()->getName().c_str());
        Surface* surf=dynamic_cast<Surface*>(it->second);
        if(surf)  // en prévision des groupes qui ne sont pas des surfaces
        {
            RecordMode rec=surf->getRecording();
            if(rec)
                xmlNewProp (elemnode, XMLSTR "rec", XMLSTR std::to_string(rec).c_str());
        }
        if(elem.getTransmissive() && elem.getOptixClass().compare(0,8,"Grating<" )!=0 ) //gratings are reflective by default
            xmlNewProp (elemnode, XMLSTR "trans", XMLSTR "true");


        map<string,Parameter>::iterator it;
        for(it=elem.m_parameters.begin(); it != elem.m_parameters.end(); ++it)  // protected but function is declared friend of OpticalElement
        {
            parmnode=xmlNewTextChild(elemnode,NULL,XMLSTR "param", XMLSTR it->first.c_str());
            Parameter &param=it->second;
            sprintf(cvbuf,"%.8g",param.value);
            xmlNewProp (parmnode, XMLSTR "val", XMLSTR cvbuf);
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
		    printf("  value: %s", att );
		    xmlFree(att);
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
 */
bool SetXmlParameters(xmlDocPtr doc, ElementBase* elem, xmlNodePtr cur)
{
	xmlChar *att, *name;
    cur = cur->xmlChildrenNode;
    Parameter param;
    bool success=true;
	while (cur != NULL)
	{
	    if ((!xmlStrcmp(cur->name, XMLSTR "param")))
        {
            name = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            // initialise param avec les valeurs défaut du constructeur de elem
		    if(elem->getParameter((char*) name, param))
            {
                att= xmlGetProp(cur, XMLSTR "val");
                param.value=atof((char*)att);
                xmlFree(att);

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

                elem->setParameter((char*) name, param) ;
            }
            else
            {
                cout << "Parameter " << (char*) name << " is not valid for object " << elem->getName() <<
                        " of class " << elem->getOptixClass() << endl;
                success=false; // ce parametre n'existe pas dans cet objet : on signale
            }
            xmlFree(name);


        }
        cur = cur->next;
	}
    return success;
}

bool LoadElementsFromXml(const char * filename, ElementCollection &system)
{
    xmlDocPtr doc;
    xmlNodePtr sysnode, curnode;
    xmlChar *sclass, *sname, *nextelem, *srec, *trans;
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
                cout << "element "<<sname<<" of type "<<sclass<<"created.";
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

            trans= xmlGetProp(curnode, XMLSTR "trans");
            if(trans)
            {
                elem->setTransmissive(true); //Value false is not stored in the XML
                xmlFree(trans);
            }

            srec= xmlGetProp(curnode, XMLSTR "rec");
            if(srec && dynamic_cast<Surface*>(elem))
            {
                dynamic_cast<Surface*>(elem)->setRecording((RecordMode)atoi((char*)srec));
                xmlFree(srec);
            }

            if(! SetXmlParameters(doc, elem , curnode))
            {
               // au moins un paramètre invalide, le signaler
               exitcode=false;
            }
		}
        curnode = curnode->next;
	}
	for(map<ElementBase*,string>::iterator it=chaining.begin(); it!=chaining.end(); ++it)
        (it->first)->chainNext(system.find(it->second)->second);

    return exitcode;
}
