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

/** \brief Exception notifying athe occurence of an error in ray propagation
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

#endif // OPTIXEXCEPTION_H_INCLUDED
