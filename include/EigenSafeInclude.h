#ifndef EIGENSAFEINCLUDE_H_INCLUDED
#define EIGENSAFEINCLUDE_H_INCLUDED

/**
*************************************************************************
*   \file       EigenSafeInclude.h

*
*   \brief     definition file
*
*
*
*
*   \author             François Polack  <francois.polack@synchroton-soleil.fr>
*   \date               Creation : 2023-03-02

*   \date               Last update: 2023-03-02

*
*
 ***************************************************************************/


#include "OptixException.h"

#undef eigen_assert
#define eigen_assert(x) \
    if (!(x)) { throw (EigenException("Eigen_assert ",__FILE__, __func__, __LINE__)); }
//  if (!(x)) { throw (std::runtime_error("Eigen assertion error")); }

// in case it is required when Eigen includes cmath
#define _USE_MATH_DEFINES
#include <Eigen/Eigen>  // Eigen/Core includes cmath


#endif // EIGENSAFEINCLUDE_H_INCLUDED
