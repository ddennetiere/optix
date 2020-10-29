////////////////////////////////////////////////////////////////////////////////
/**
*      \file           sphere.h
*
*      \brief         TODO  fill in file purpose
*
*      \author         François Polack <francois.polack@synchroton-soleil.fr>
*      \date        2020-10-28  Creation
*      \date         Last update
*

*/
///////////////////////////////////////////////////////////////////////////////////
//
//             REVISIONS
//
////////////////////////////////////////////////////////////////////////////////////
#ifndef SPHERE_H
#define SPHERE_H

#include "quadric.h"


class Sphere : public Quadric
{
    public:
        /** Default constructor */
        Sphere();
        /** Default destructor */
        virtual ~Sphere(){}
        virtual inline string getRuntimeClass(){return "Sphere";}
        inline ParamRef setParameter(string name, Parameter& param)
        {
            ParamRef pRef = Surface::setParameter(name, param);
            if(name=="curvature")
                createSurface();
            return pRef;
        }
    protected:
        void createSurface();
    private:
};

#endif // SPHERE_H
