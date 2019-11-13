   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*               CLIPS Version 6.05  04/09/97          */
   /*                                                     */
   /*                                                     */
   /*******************************************************/

/*************************************************************/
/* Purpose:                                                  */
/*                                                           */
/* Principal Programmer(s):                                  */
/*      Brian L. Donnell                                     */
/*                                                           */
/* Contributing Programmer(s):                               */
/*                                                           */
/* Revision History:                                         */
/*                                                           */
/*************************************************************/

#ifndef _H_dffnxexe
#define _H_dffnxexe

#if DEFFUNCTION_CONSTRUCT

#include "dffnxfun.h"
#ifndef _H_expressn
#include "expressn.h"
#endif
#ifndef _H_evaluatn
#include "evaluatn.h"
#endif

#ifdef LOCALE
#undef LOCALE
#endif

#ifdef _DFFNXEXE_SOURCE_
#define LOCALE
#else
#define LOCALE extern
#endif

LOCALE void CallDeffunction(DEFFUNCTION *,EXPRESSION *,DATA_OBJECT *);

#ifndef _DFFNXEXE_SOURCE_
extern DEFFUNCTION *ExecutingDeffunction;
#endif

#endif

#endif




