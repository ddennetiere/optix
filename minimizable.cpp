/**   *************************************************************************
*   \file            minimizable.cpp
*
*    \brief          base mechanism  for applying some minimization  to an object of the OptX library
*
*     based on             minimizzabile.cpp  -  description
*                             -------------------
*    begin                : Tue Dec 14 1999
*    author               : (C) 1999 by Alessandro MIRONE
*    email                : mirone@lure.u-psud.fr
*
*   \author             François Polack  <francois.polack@synchroton-soleil.fr>
*   \date               Creation : 2020-09-07
 ***************************************************************************/

#include"minimizable.h"
 minimizable::minimizable()
 {
 	StopFlag=false;
 }

double minimizable::scarto(double *x)
{
	return 1.0;
}

/** Read property of bool StopFlag. */
const bool& minimizable::getStopFlag(){
	return StopFlag;
}

/** Write property of bool StopFlag. */
void minimizable::setStopFlag( const bool& _newVal){
	StopFlag = _newVal;
}
