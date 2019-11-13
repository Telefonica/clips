   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*             CLIPS Version 6.10  04/13/98            */
   /*                                                     */
   /*             CONSTRAINT CHECKING MODULE              */
   /*******************************************************/

/*************************************************************/
/* Purpose: Provides functions for constraint checking of    */
/*   data types.                                             */
/*                                                           */
/* Principal Programmer(s):                                  */
/*      Gary D. Riley                                        */
/*                                                           */
/* Contributing Programmer(s):                               */
/*      Brian Donnell                                        */
/*                                                           */
/* Revision History:                                         */
/*                                                           */
/*************************************************************/

#define _CSTRNCHK_SOURCE_

#include <stdio.h>
#define _STDIO_INCLUDED_
#include <stdlib.h>

#include "setup.h"

#include "router.h"
#include "multifld.h"
#include "extnfunc.h"
#include "cstrnutl.h"

#include "cstrnchk.h"

/***************************************/
/* LOCAL INTERNAL FUNCTION DEFINITIONS */
/***************************************/

   static BOOLEAN                 CheckRangeAgainstCardinalityConstraint(int,int,CONSTRAINT_RECORD *);
   static int                     CheckFunctionReturnType(int,CONSTRAINT_RECORD *);
   static BOOLEAN                 CheckTypeConstraint(int,CONSTRAINT_RECORD *);
   static BOOLEAN                 CheckRangeConstraint(int,void *,CONSTRAINT_RECORD *);
   static void                    PrintRange(char *,CONSTRAINT_RECORD *);

/******************************************************/
/* CheckFunctionReturnType: Checks a functions return */
/*   type against a set of permissable return values. */
/*   Returns TRUE if the return type is included      */
/*   among the permissible values, otherwise FALSE.   */
/******************************************************/
static int CheckFunctionReturnType(
  int functionReturnType,
  CONSTRAINT_RECORD *constraints)
  {
   if (constraints == NULL) return(TRUE);

   if (constraints->anyAllowed) return(TRUE);

   switch(functionReturnType)
     {
      case 'c':
      case 'w':
      case 'b':
        if (constraints->symbolsAllowed) return(TRUE);
        else return(FALSE);

      case 's':
        if (constraints->stringsAllowed) return(TRUE);
        else return(FALSE);

      case 'j':
        if ((constraints->symbolsAllowed) ||
            (constraints->stringsAllowed) ||
            (constraints->instanceNamesAllowed)) return(TRUE);
        else return(FALSE);

      case 'k':
        if ((constraints->symbolsAllowed) || (constraints->stringsAllowed)) return(TRUE);
        else return(FALSE);

      case 'd':
      case 'f':
        if (constraints->floatsAllowed) return(TRUE);
        else return(FALSE);

      case 'i':
      case 'l':
        if (constraints->integersAllowed) return(TRUE);
        else return(FALSE);

      case 'n':
        if ((constraints->integersAllowed) || (constraints->floatsAllowed)) return(TRUE);
        else return(FALSE);

      case 'm':
        if (constraints->multifieldsAllowed) return(TRUE);
        else return(FALSE);

      case 'a':
        if (constraints->externalAddressesAllowed) return(TRUE);
        else return(FALSE);

      case 'x':
        if (constraints->instanceAddressesAllowed) return(TRUE);
        else return(FALSE);

      case 'o':
        if (constraints->instanceNamesAllowed) return(TRUE);
        else return(FALSE);

      case 'u':
        return(TRUE);

      case 'v':
        if (constraints->voidAllowed) return(TRUE);
     }

   return(TRUE);
  }

/****************************************************/
/* CheckTypeConstraint: Determines if a primitive   */
/*   data type satisfies the type constraint fields */
/*   of aconstraint record.                         */
/****************************************************/
static BOOLEAN CheckTypeConstraint(
  int type,
  CONSTRAINT_RECORD *constraints)
  {
   if (type == RVOID) return(FALSE);

   if (constraints == NULL) return(TRUE);

   if (constraints->anyAllowed == TRUE) return(TRUE);

   if ((type == SYMBOL) && (constraints->symbolsAllowed != TRUE))
     { return(FALSE); }

   if ((type == STRING) && (constraints->stringsAllowed != TRUE))
     { return(FALSE); }

   if ((type == FLOAT) && (constraints->floatsAllowed != TRUE))
     { return(FALSE); }

   if ((type == INTEGER) && (constraints->integersAllowed != TRUE))
     { return(FALSE); }

#if OBJECT_SYSTEM
   if ((type == INSTANCE_NAME) && (constraints->instanceNamesAllowed != TRUE))
     { return(FALSE); }

   if ((type == INSTANCE_ADDRESS) && (constraints->instanceAddressesAllowed != TRUE))
     { return(FALSE); }
#endif

   if ((type == EXTERNAL_ADDRESS) && (constraints->externalAddressesAllowed != TRUE))
     { return(FALSE); }

   if ((type == RVOID) && (constraints->voidAllowed != TRUE))
     { return(FALSE); }

   if ((type == FACT_ADDRESS) && (constraints->factAddressesAllowed != TRUE))
     { return(FALSE); }

   return(TRUE);
  }

/********************************************************/
/* CheckCardinalityConstraint: Determines if an integer */
/*   falls within the range of allowed cardinalities    */
/*   for a constraint record.                           */
/********************************************************/
globle BOOLEAN CheckCardinalityConstraint(
  long number,
  CONSTRAINT_RECORD *constraints)
  {
   /*=========================================*/
   /* If the constraint record is NULL, there */
   /* are no cardinality restrictions.        */
   /*=========================================*/

   if (constraints == NULL) return(TRUE);

   /*==================================*/
   /* Determine if the integer is less */
   /* than the minimum cardinality.    */
   /*==================================*/

   if (constraints->minFields != NULL)
     {
      if (constraints->minFields->value != NegativeInfinity)
        {
         if (number < ValueToLong(constraints->minFields->value))
           { return(FALSE); }
        }
     }

   /*=====================================*/
   /* Determine if the integer is greater */
   /* than the maximum cardinality.       */
   /*=====================================*/

   if (constraints->maxFields != NULL)
     {
      if (constraints->maxFields->value != PositiveInfinity)
        {
         if (number > ValueToLong(constraints->maxFields->value))
           { return(FALSE); }
        }
     }

   /*=========================================================*/
   /* The integer falls within the allowed cardinality range. */
   /*=========================================================*/

   return(TRUE);
  }

/*****************************************************************/
/* CheckRangeAgainstCardinalityConstraint: Determines if a range */
/*   of numbers could possibly fall within the range of allowed  */
/*   cardinalities for a constraint record. Returns TRUE if at   */
/*   least one of the numbers in the range is within the allowed */
/*   cardinality, otherwise FALSE is returned.                   */
/*****************************************************************/
static BOOLEAN CheckRangeAgainstCardinalityConstraint(
  int min,
  int max,
  CONSTRAINT_RECORD *constraints)
  {
   /*=========================================*/
   /* If the constraint record is NULL, there */
   /* are no cardinality restrictions.        */
   /*=========================================*/

   if (constraints == NULL) return(TRUE);

   /*===============================================================*/
   /* If the minimum value of the range is greater than the maximum */
   /* value of the cardinality, then there are no numbers in the    */
   /* range which could fall within the cardinality range, and so   */
   /* FALSE is returned.                                            */
   /*===============================================================*/

   if (constraints->maxFields != NULL)
     {
      if (constraints->maxFields->value != PositiveInfinity)
        {
         if (min > ValueToLong(constraints->maxFields->value))
           { return(FALSE); }
        }
     }

   /*===============================================================*/
   /* If the maximum value of the range is less than the minimum    */
   /* value of the cardinality, then there are no numbers in the    */
   /* range which could fall within the cardinality range, and so   */
   /* FALSE is returned. A maximum range value of -1 indicates that */
   /* the maximum possible value of the range is positive infinity. */
   /*===============================================================*/

   if ((constraints->minFields != NULL) && (max != -1))
     {
      if (constraints->minFields->value != NegativeInfinity)
        {
         if (max < ValueToLong(constraints->minFields->value))
           { return(FALSE); }
        }
     }

   /*=============================================*/
   /* At least one number in the specified range  */
   /* falls within the allowed cardinality range. */
   /*=============================================*/

   return(TRUE);
  }

/**********************************************************************/
/* CheckAllowedValuesConstraint: Determines if a primitive data type  */
/*   satisfies the allowed-... constraint fields of a constraint      */
/*   record. Returns TRUE if the constraints are satisfied, otherwise */
/*   FALSE is returned.                                               */
/**********************************************************************/
globle BOOLEAN CheckAllowedValuesConstraint(
  int type,
  void *vPtr,
  CONSTRAINT_RECORD *constraints)
  {
   struct expr *tmpPtr;

   /*=========================================*/
   /* If the constraint record is NULL, there */
   /* are no allowed-... restrictions.        */
   /*=========================================*/

   if (constraints == NULL) return(TRUE);

   /*=====================================================*/
   /* Determine if there are any allowed-... restrictions */
   /* for the type of the value being checked.            */
   /*=====================================================*/

   switch (type)
     {
      case SYMBOL:
        if ((constraints->symbolRestriction == FALSE) &&
            (constraints->anyRestriction == FALSE))
          { return(TRUE); }
        break;

#if OBJECT_SYSTEM
      case INSTANCE_NAME:
        if ((constraints->instanceNameRestriction == FALSE) &&
            (constraints->anyRestriction == FALSE))
          { return(TRUE); }
        break;
#endif

      case STRING:
        if ((constraints->stringRestriction == FALSE) &&
            (constraints->anyRestriction == FALSE))
          { return(TRUE); }
        break;

      case INTEGER:
        if ((constraints->integerRestriction == FALSE) &&
            (constraints->anyRestriction == FALSE))
          { return(TRUE); }
        break;

      case FLOAT:
        if ((constraints->floatRestriction == FALSE) &&
            (constraints->anyRestriction == FALSE))
          { return(TRUE); }
        break;

      default:
        return(TRUE);
     }

   /*=========================================================*/
   /* Search through the restriction list to see if the value */
   /* matches one of the allowed values in the list.          */
   /*=========================================================*/

   for (tmpPtr = constraints->restrictionList;
        tmpPtr != NULL;
        tmpPtr = tmpPtr->nextArg)
     {
      if ((tmpPtr->type == type) && (tmpPtr->value == vPtr)) return(TRUE);
     }

   /*====================================================*/
   /* If the value wasn't found in the list, then return */
   /* FALSE because the constraint has been violated.    */
   /*====================================================*/

   return(FALSE);
  }

/*************************************************************/
/* CheckRangeConstraint: Determines if a primitive data type */
/*   satisfies the range constraint of a constraint record.  */
/*************************************************************/
static BOOLEAN CheckRangeConstraint(
  int type,
  void *vPtr,
  CONSTRAINT_RECORD *constraints)
  {
   struct expr *minList, *maxList;

   /*===================================*/
   /* If the constraint record is NULL, */
   /* there are no range restrictions.  */
   /*===================================*/

   if (constraints == NULL) return(TRUE);

   /*============================================*/
   /* If the value being checked isn't a number, */
   /* then the range restrictions don't apply.   */
   /*============================================*/

   if ((type != INTEGER) && (type != FLOAT)) return(TRUE);

   /*=====================================================*/
   /* Check each of the range restrictions to see if the  */
   /* number falls within at least one of the allowed     */
   /* ranges. If it falls within one of the ranges, then  */
   /* return TRUE since the constraint is satisifed.      */
   /*=====================================================*/

   minList = constraints->minValue;
   maxList = constraints->maxValue;

   while (minList != NULL)
     {
      if (CompareNumbers(type,vPtr,minList->type,minList->value) == LESS_THAN)
        {
         minList = minList->nextArg;
         maxList = maxList->nextArg;
        }
      else if (CompareNumbers(type,vPtr,maxList->type,maxList->value) == GREATER_THAN)
        {
         minList = minList->nextArg;
         maxList = maxList->nextArg;
        }
      else
        { return(TRUE); }
     }

   /*===========================================*/
   /* Return FALSE since the number didn't fall */
   /* within one of the allowed numeric ranges. */
   /*===========================================*/

   return(FALSE);
  }

/************************************************/
/* ConstraintViolationErrorMessage: Generalized */
/*   error message for constraint violations.   */
/************************************************/
globle void ConstraintViolationErrorMessage(
  char *theWhat,
  char *thePlace,
  int command,
  int thePattern,
  struct symbolHashNode *theSlot,
  int theField,
  int violationType,
  CONSTRAINT_RECORD *theConstraint,
  int printPrelude)
  {
   /*======================================================*/
   /* Don't print anything other than the tail explanation */
   /* of the error unless asked to do so.                  */
   /*======================================================*/

   if (printPrelude)
     {
      /*===================================*/
      /* Print the name of the thing which */
      /* caused the constraint violation.  */
      /*===================================*/

      if (violationType == FUNCTION_RETURN_TYPE_VIOLATION)
        {
         PrintErrorID("CSTRNCHK",1,TRUE);
         PrintRouter(WERROR,"The function return value ");
        }
      else if (theWhat != NULL)
        {
         PrintErrorID("CSTRNCHK",1,TRUE);
         PrintRouter(WERROR,theWhat);
         PrintRouter(WERROR," ");
        }

      /*=======================================*/
      /* Print the location of the thing which */
      /* caused the constraint violation.      */
      /*=======================================*/

      if (thePlace != NULL)
        {
         PrintRouter(WERROR,"found in ");
         if (command) PrintRouter(WERROR,"the ");
         PrintRouter(WERROR,thePlace);
         if (command) PrintRouter(WERROR," command");
        }

      /*================================================*/
      /* If the violation occured in the LHS of a rule, */
      /* indicate which pattern was at fault.           */
      /*================================================*/

      if (thePattern > 0)
        {
         PrintRouter(WERROR,"found in CE #");
         PrintLongInteger(WERROR,(long int) thePattern);
        }
     }

   /*===============================================================*/
   /* Indicate the type of constraint violation (type, range, etc). */
   /*===============================================================*/

   if ((violationType == TYPE_VIOLATION) ||
       (violationType == FUNCTION_RETURN_TYPE_VIOLATION))
     { PrintRouter(WERROR,"\ndoes not match the allowed types"); }
   else if (violationType == RANGE_VIOLATION)
     {
      PrintRouter(WERROR,"\ndoes not fall in the allowed range ");
      PrintRange(WERROR,theConstraint);
     }
   else if (violationType == ALLOWED_VALUES_VIOLATION)
     { PrintRouter(WERROR,"\ndoes not match the allowed values"); }
   else if (violationType == CARDINALITY_VIOLATION)
     { PrintRouter(WERROR,"\ndoes not satisfy the cardinality restrictions"); }

   /*==============================================*/
   /* Print either the slot name or field position */
   /* where the constraint violation occured.      */
   /*==============================================*/

   if (theSlot != NULL)
     {
      PrintRouter(WERROR," for slot ");
      PrintRouter(WERROR,ValueToString(theSlot));
     }
   else if (theField > 0)
     {
      PrintRouter(WERROR," for field #");
      PrintLongInteger(WERROR,(long) theField);
     }

   PrintRouter(WERROR,".\n");
  }

/********************************************************************/
/* PrintRange: Prints the range restriction of a constraint record. */
/*   For example, 8 to +00 (eight to positive infinity).            */
/********************************************************************/
static void PrintRange(
  char *logicalName,
  CONSTRAINT_RECORD *theConstraint)
  {
   if (theConstraint->minValue->value == NegativeInfinity)
     { PrintRouter(logicalName,ValueToString(NegativeInfinity)); }
   else PrintExpression(logicalName,theConstraint->minValue);
   PrintRouter(logicalName," to ");
   if (theConstraint->maxValue->value == PositiveInfinity)
     { PrintRouter(logicalName,ValueToString(PositiveInfinity)); }
   else PrintExpression(logicalName,theConstraint->maxValue);
  }

/*************************************************************/
/* ConstraintCheckDataObject: Given a value stored in a data */
/*   object structure and a constraint record, determines if */
/*   the data object satisfies the constraint record.        */
/*************************************************************/
globle int ConstraintCheckDataObject(
  DATA_OBJECT *theData,
  CONSTRAINT_RECORD *theConstraints)
  {
   long i; /* 6.04 Bug Fix */
   int rv;
   struct field *theMultifield;

   if (theConstraints == NULL) return(NO_VIOLATION);

   if (theData->type == MULTIFIELD)
     {
      if (CheckCardinalityConstraint((theData->end - theData->begin) + 1,
                                     theConstraints) == FALSE)
        { return(CARDINALITY_VIOLATION); }

      theMultifield = ((struct multifield *) theData->value)->theFields;
      for (i = theData->begin; i <= theData->end; i++)
        {
         if ((rv = ConstraintCheckValue(theMultifield[i].type,
                                        theMultifield[i].value,
                                        theConstraints)) != NO_VIOLATION)
           { return(rv); }
        }

      return(NO_VIOLATION);
     }

   if (CheckCardinalityConstraint(1L,theConstraints) == FALSE)
    { return(CARDINALITY_VIOLATION); }

   return(ConstraintCheckValue(theData->type,theData->value,theConstraints));
  }

/****************************************************************/
/* ConstraintCheckValue: Given a value and a constraint record, */
/*   determines if the value satisfies the constraint record.   */
/****************************************************************/
globle int ConstraintCheckValue(
  int theType,
  void *theValue,
  CONSTRAINT_RECORD *theConstraints)
  {
   if (CheckTypeConstraint(theType,theConstraints) == FALSE)
     { return(TYPE_VIOLATION); }

   else if (CheckAllowedValuesConstraint(theType,theValue,theConstraints) == FALSE)
     { return(ALLOWED_VALUES_VIOLATION); }

   else if (CheckRangeConstraint(theType,theValue,theConstraints) == FALSE)
     { return(RANGE_VIOLATION); }

   else if (theType == FCALL)
     {
      if (CheckFunctionReturnType((int) ValueFunctionType(theValue),theConstraints) == FALSE)
        { return(FUNCTION_RETURN_TYPE_VIOLATION); }
     }

   return(NO_VIOLATION);
  }

/********************************************************************/
/* ConstraintCheckExpressionChain: Checks an expression and nextArg */
/* links for constraint conflicts (argList is not followed).        */
/********************************************************************/
globle int ConstraintCheckExpressionChain(
  struct expr *theExpression,
  CONSTRAINT_RECORD *theConstraints)
  {
   struct expr *exp;
   int min = 0, max = 0, vCode;

   /*===========================================================*/
   /* Determine the minimum and maximum number of value which   */
   /* can be derived from the expression chain (max of -1 means */
   /* positive infinity).                                       */
   /*===========================================================*/

   for (exp = theExpression ; exp != NULL ; exp = exp->nextArg)
     {
      if (ConstantType(exp->type)) min++;
      else if (exp->type == FCALL)
        {
         if ((ExpressionFunctionType(exp) != 'm') &&
             (ExpressionFunctionType(exp) != 'u')) min++;
         else max = -1;
        }
      else max = -1;
     }

   /*====================================*/
   /* Check for a cardinality violation. */
   /*====================================*/

   if (max == 0) max = min;
   if (CheckRangeAgainstCardinalityConstraint(min,max,theConstraints) == FALSE)
     { return(CARDINALITY_VIOLATION); }

   /*========================================*/
   /* Check for other constraint violations. */
   /*========================================*/

   for (exp = theExpression ; exp != NULL ; exp = exp->nextArg)
     {
      vCode = ConstraintCheckValue(exp->type,exp->value,theConstraints);
      if (vCode != NO_VIOLATION)
        return(vCode);
     }

   return(NO_VIOLATION);
  }

#if (! RUN_TIME) && (! BLOAD_ONLY)

/***************************************************/
/* ConstraintCheckExpression: Checks an expression */
/*   for constraint conflicts. Returns TRUE if     */
/*   conflicts are found, otherwise FALSE.         */
/***************************************************/
globle int ConstraintCheckExpression(
  struct expr *theExpression,
  CONSTRAINT_RECORD *theConstraints)
  {
   int rv = NO_VIOLATION;

   if (theConstraints == NULL) return(rv);

   while (theExpression != NULL)
     {
      rv = ConstraintCheckValue(theExpression->type,
                                theExpression->value,
                                theConstraints);
      if (rv != NO_VIOLATION) return(rv);
      rv = ConstraintCheckExpression(theExpression->argList,theConstraints);
      if (rv != NO_VIOLATION) return(rv);
      theExpression = theExpression->nextArg;
     }

   return(rv);
  }

#endif /* (! RUN_TIME) && (! BLOAD_ONLY) */

#if (! RUN_TIME)

/*****************************************************/
/* UnmatchableConstraint: Determines if a constraint */
/*  record can still be satisfied by some value.     */
/*****************************************************/
globle BOOLEAN UnmatchableConstraint(
  CONSTRAINT_RECORD *theConstraint)
  {
   if (theConstraint == NULL) return(FALSE);

   if ((! theConstraint->anyAllowed) &&
       (! theConstraint->symbolsAllowed) &&
       (! theConstraint->stringsAllowed) &&
       (! theConstraint->floatsAllowed) &&
       (! theConstraint->integersAllowed) &&
       (! theConstraint->instanceNamesAllowed) &&
       (! theConstraint->instanceAddressesAllowed) &&
       (! theConstraint->multifieldsAllowed) &&
       (! theConstraint->externalAddressesAllowed) &&
       (! theConstraint->voidAllowed) &&
       (! theConstraint->factAddressesAllowed))
     { return(TRUE); }

   return(FALSE);
  }

#endif /* (! RUN_TIME) */



