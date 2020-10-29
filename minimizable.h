/**  *************************************************************************
*   \file            minimizable.h
*
*      base mechanism  for applying some minimization  to an object of the OptX library
*
*    based on              minimizzabile.h  -  description
*                            -------------------
*    begin                : Tue Dec 14 1999
*    copyright            : (C) 1999 by Alessandro MIRONE
*    email                : mirone@lure.u-psud.fr
*
*   \author             François Polack  <francois.polack@synchroton-soleil.fr>
*   \date               Creation : 2020-09-07
 ***************************************************************************/

#ifndef _MINIMIZABLE_H_
#define _MINIMIZABLE_H_


/** \brief base class for optimization
 * \todo this class is inherited from Solemio, its usefulness must be evaluated
 */
class minimizable{
public:
	minimizable();
	virtual double scarto(double *);/**<  \brief refererence to the quantity to be minimized  */
  /** Write property of bool StopFlag. */  // TODO not requested since StopFlag is public
  virtual void setStopFlag( const bool& _newVal=true);
  /** Read property of bool StopFlag. */     // TODO not requested since StopFlag is public
  virtual const bool& getStopFlag();
public: // Public attributes
  /** \brief This variable is set to immediately stop the minimization proedure */
  bool StopFlag;
};
#endif



