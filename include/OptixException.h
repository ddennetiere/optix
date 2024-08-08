#ifndef OPTIXEXCEPTION_H_INCLUDED
#define OPTIXEXCEPTION_H_INCLUDED

////////////////////////////////////////////////////////////////////////////////
/**
*      \file           OptixException.h
*
*      \brief         Exceptions raised by OptiX functions
*
*      \author         François Polack <francois.polack@synchroton-soleil.fr>
*      \date        2020-10-14  Creation
*      \date         Last update
*

*/
///////////////////////////////////////////////////////////////////////////////////
//
//             REVISIONS
//
////////////////////////////////////////////////////////////////////////////////////
#include <stdexcept>
#include <string>

using std::string;
using  std::runtime_error, std::logic_error;

/** \brief Generic exception of the library
 */
class OptixException: public runtime_error
{
public:
    OptixException(string except_class="OptixException", string what =""):runtime_error("OptixException"), exception_class(except_class)
    {
        what_str=(what.empty()?string("OptiXException"):what); // default message
    }
    virtual ~OptixException() {}
    virtual string  what(){return what_str;}
    string what_str;
    string exception_class;
};

/** \brief Exception notifying he occurrence of an error in ray propagation
 */
class RayException:public OptixException
{
public:
    RayException(string what ="", string file="", string callingFunction="", int line=-1 ):OptixException("RayException")
    {
        char str[256];
        sprintf(str, " %s  %s line %d", file.c_str(), callingFunction.c_str(), line);
        what_str=(what.empty()?string("RayException in"):what)+ str;
    }
    virtual ~RayException() {}
//    virtual string  what(){return what_str;}
//    string what_str;
};

/** \brief Exception notifying he occurrence of an error in an Eigen library call*/
class EigenException:public OptixException
{
public:
    EigenException(string cause ="", string file="", string callingFunction="", int line=-1 ):OptixException("EigenException")
    {
        char str[256];
        sprintf(str, "  %s  %s line %d",  file.c_str(), callingFunction.c_str(), line);
        what_str=(cause.empty()?string("EigenException in"):cause) + str;
    }
    virtual ~EigenException() {}
//    virtual string  what(){return what_str;}
//    string what_str;
};

/** \brief Exception notifying he occurrence of an error in computing an intecept*/
class InterceptException:public OptixException
{
public:
    InterceptException(string cause ="", string file="", string callingFunction="", int line=-1 ):OptixException("InterceptException")
    {
        char str[256];
        sprintf(str, "   %s function  %s line %d", file.c_str(), callingFunction.c_str(), line);
        what_str=(cause.empty()?string("InterceptException in"):cause) + str;
    }
    virtual ~InterceptException() {}
//    virtual string  what(){return what_str;}
//    string what_str;
};

/** \brief Tagged parameter setting or recovering error
 */
class ParameterException:public OptixException
{
public:
    ParameterException(string what ="", string file="", string callingFunction="", int line=-1 ):OptixException("ParameterException")
    {
        char str[256];
        sprintf(str, "  %s  %s line %d",  file.c_str(), callingFunction.c_str(), line);
        what_str=(what.empty()?string("ParameterException in"):what) + str;
    }
    virtual ~ParameterException() {}
//    virtual string  what(){return what_str;}
//    string what_str;
};


/** \brief Non fatal error emitted whenever a parameter is not in the expected range
 */
class ParameterWarning:public logic_error
{
public:
    ParameterWarning(string what ="", string file="", string callingFunction="", int line=-1 ):logic_error("ParameterWarning")
    {
        char str[256];
        sprintf(str, "  %s  %s line %d",  file.c_str(), callingFunction.c_str(), line);
        what_str=(what.empty()?string("ParameterWarning in"):what) + str;
    }
    virtual ~ParameterWarning() {}
    virtual string  what(){return what_str;}
    string what_str;
};



/** \brief Tagged parameter setting or recovering error
 */
class TextFileException:public OptixException
{
public:
    TextFileException(string what ="", string file="", string callingFunction="", int line=-1 ):OptixException("TextFileException")
    {
        char str[256];
        sprintf(str, " %s  %s line %d",  file.c_str(), callingFunction.c_str(), line);
        what_str=(what.empty()?string("TextFileException in"):what) + str;
    }
    virtual ~TextFileException() {}
//    virtual string  what(){return what_str;}
//    string what_str;
};

/** \brief exception raised by xml reader if the object being read-in cannot be constructed
 */
class XmlFileException:public OptixException
{
public:
    XmlFileException(string what ="", string file="", string callingFunction="", int line=-1 ):OptixException("XmlFileException")
    {
        char str[256];
        sprintf(str, " %s  %s line %d",  file.c_str(), callingFunction.c_str(), line);
        what_str=(what.empty()?string("XmlFileException in"):what) + str;
    }
    virtual ~XmlFileException() {}
//    virtual string  what(){return what_str;}
//    string what_str;
};



/** \brief Exception raised when an error occurs during element definition or alignment
 */
class ElementException:public OptixException
{
public:
    ElementException(string what ="", string file="", string callingFunction="", int line=-1 ):OptixException("ElementException")
    {
        char str[128];
        sprintf(str, " in %s  %s line %d",  file.c_str(), callingFunction.c_str(), line);
        what_str=(what.empty()?string("ElementException"):what) + str;
    }
    virtual ~ElementException() {}
//    virtual string  what(){return what_str;}
//    string what_str;
};

#endif // OPTIXEXCEPTION_H_INCLUDED
