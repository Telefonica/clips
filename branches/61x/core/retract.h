   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*             CLIPS Version 6.05  04/09/97            */
   /*                                                     */
   /*                RETRACT HEADER FILE                  */
   /*******************************************************/

/*************************************************************/
/* Purpose:  Handles join network activity associated with   */
/*   with the removal of a data entity such as a fact or     */
/*   instance.                                               */
/*                                                           */
/* Principal Programmer(s):                                  */
/*      Gary D. Riley                                        */
/*                                                           */
/* Contributing Programmer(s):                               */
/*                                                           */
/* Revision History:                                         */
/*                                                           */
/*************************************************************/

#ifndef _H_retract
#define _H_retract

#ifndef _H_match
#include "match.h"
#endif
#ifndef _H_network
#include "network.h"
#endif

#ifdef LOCALE
#undef LOCALE
#endif

#ifdef _RETRACT_SOURCE_
#define LOCALE
#else
#define LOCALE extern
#endif

LOCALE void                           NetworkRetract(struct patternMatch *);
LOCALE void                           PosEntryRetract(struct joinNode *,struct alphaMatch *,struct partialMatch *,int,int);
LOCALE void                           ReturnPartialMatch(struct partialMatch *);
LOCALE void                           FlushGarbagePartialMatches(void);
LOCALE void                           NegEntryRetract(struct joinNode *,struct partialMatch *,int);

#ifndef _RETRACT_SOURCE_
   extern struct partialMatch        *GarbagePartialMatches;
   extern struct alphaMatch          *GarbageAlphaMatches;
#endif

#endif



