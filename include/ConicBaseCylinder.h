#ifndef HEADER_2EAB188D8C944ED0
#define HEADER_2EAB188D8C944ED0

/**
*************************************************************************
*   \file       ConicBaseCylinder.h

*
*   \brief     definition file
*
*
*
*
*   \author             François Polack  <francois.polack@synchroton-soleil.fr>
*   \date               Creation : 2021-06-20

*   \date               Last update: 2021-06-20

*
*
 ***************************************************************************/

#ifndef CONICBASECYLINDER_H
#define CONICBASECYLINDER_H

#include "quadric.h"


class ConicBaseCylinder : virtual public Surface, public Quadric
{
    public:
        ConicBaseCylinder();
        virtual ~ConicBaseCylinder(){}
        virtual inline string getOptixClass(){return "ConicBaseCylinder";}
        inline bool setParameter(string name, Parameter& param)
        {
            if(! Surface::setParameter(name, param))
                    return false;
            if(name=="invp" || name=="invq" || name=="theta0")
                createSurface();
            return true;
        }


    protected:
        void createSurface(); /**< \brief Inilialize the local equation. Called when a parameter is changed  */

    private:
};

#endif // CONICBASECYLINDER_H


#endif // header guard

