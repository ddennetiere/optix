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
#include <exception>
#include <string>

using std::string;
using  std::runtime_error;

/** \brief Exception notifying he occurence of an error in ray propagation
 */
class RayException:public runtime_error
{
public:
    RayException(string what ="", string file="", string callingFunction="", int line=-1 ):runtime_error("RayException")
    {
        char str[128];
        sprintf(str, "%s in %s  %s line %d", what.empty()?"RayException":what.c_str(), file.c_str(), callingFunction.c_str(), line);
        what_str=str;
    }
    virtual ~RayException() {}
    virtual string  what(){return what_str;}
    string what_str;
};

/** \brief Exception notifying he occurence of an error in an Eigen library call*/
class EigenException:public runtime_error
{
public:
    EigenException(string what ="", string file="", string callingFunction="", int line=-1 ):runtime_error("EigenException")
    {
        char str[256];
        sprintf(str, "%s in %s  %s line %d", what.empty()?"EigenException":what.c_str(), file.c_str(), callingFunction.c_str(), line);
        what_str=str;
    }
    virtual ~EigenException() {}
    virtual string  what(){return what_str;}
    string what_str;
};

/** \brief Tagged parameter setting or recovering error
 */
class ParameterException:public runtime_error
{
public:
    ParameterException(string what ="", string file="", string callingFunction="", int line=-1 ):runtime_error("ParameterException")
    {
        char str[128];
        sprintf(str, "%s in %s  %s line %d", what.empty()?"ParameterException":what.c_str(), file.c_str(), callingFunction.c_str(), line);
        what_str=str;
    }
    virtual ~ParameterException() {}
    virtual string  what(){return what_str;}
    string what_str;
};


/** \brief Tagged parameter setting or recovering error
 */
class TextFileException:public runtime_error
{
public:
    TextFileException(string what ="", string file="", string callingFunction="", int line=-1 ):runtime_error("TextFileException")
    {
        char str[128];
        sprintf(str, "%s in %s  %s line %d", what.empty()?"TextFileException":what.c_str(), file.c_str(), callingFunction.c_str(), line);
        what_str=str;
    }
    virtual ~TextFileException() {}
    virtual string  what(){return what_str;}
    string what_str;
};



/** \brief Exception raised when an error occurs during element definition or alignment
 */
class ElementException:public runtime_error
{
public:
    ElementException(string what ="", string file="", string callingFunction="", int line=-1 ):runtime_error("ElementException")
    {
        char str[128];
        sprintf(str, "%s in %s  %s line %d", what.empty()?"ElementException":what.c_str(), file.c_str(), callingFunction.c_str(), line);
        what_str=str;
    }
    virtual ~ElementException() {}
    virtual string  what(){return what_str;}
    string what_str;
};

#endif // OPTIXEXCEPTION_H_INCLUDED
