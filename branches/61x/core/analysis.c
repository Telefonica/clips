   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*             CLIPS Version 6.10  04/13/98            */
   /*                                                     */
   /*                  ANALYSIS MODULE                    */
   /*******************************************************/

/*************************************************************/
/* Purpose: Analyzes LHS patterns to check for semantic      */
/*   errors and to determine variable comparisons and other  */
/*   tests which must be performed either in the pattern or  */
/*   join networks.                                          */
/*                                                           */
/* Principal Programmer(s):                                  */
/*      Gary D. Riley                                        */
/*                                                           */
/* Contributing Programmer(s):                               */
/*                                                           */
/* Revision History:                                         */
/*                                                           */
/*************************************************************/

#define _ANALYSIS_SOURCE_

#include "setup.h"

#if (! RUN_TIME) && (! BLOAD_ONLY) && DEFRULE_CONSTRUCT

#include <stdio.h>
#define _STDIO_INCLUDED_

#include "constant.h"
#include "symbol.h"
#include "memalloc.h"
#include "exprnpsr.h"
#include "reorder.h"
#include "generate.h"
#include "pattern.h"
#include "router.h"
#include "ruledef.h"
#include "cstrnchk.h"
#include "cstrnutl.h"
#include "cstrnops.h"
#include "rulecstr.h"
#include "modulutl.h"
#include "analysis.h"

#if DEFGLOBAL_CONSTRUCT
#include "globldef.h"
#endif

/***************************************/
/* LOCAL INTERNAL FUNCTION DEFINITIONS */
/***************************************/

   static int                     GetVariables(struct lhsParseNode *);
   static BOOLEAN                 UnboundVariablesInPattern(struct lhsParseNode *,int);
   static int                     PropagateVariableToNodes(struct lhsParseNode *,
                                                           int,
                                                           struct symbolHashNode *,
                                                           struct lhsParseNode *,
                                                           int,int,int);
   static struct lhsParseNode    *CheckExpression(struct lhsParseNode *,
                                                  struct lhsParseNode *,
                                                  int,
                                                  struct symbolHashNode *,
                                                  int);
   static void                    VariableReferenceErrorMessage(struct symbolHashNode *,
                                                                struct lhsParseNode *,
                                                                int,
                                                                struct symbolHashNode *,
                                                                int);
   static int                     ProcessField(struct lhsParseNode *,
                                               struct lhsParseNode *,
                                               struct lhsParseNode *);
   static int                     ProcessVariable(struct lhsParseNode *,
                                               struct lhsParseNode *,
                                               struct lhsParseNode *);
   static void                    VariableMixingErrorMessage(struct symbolHashNode *);
   static int                     PropagateVariableDriver(struct lhsParseNode *,
                                                          struct lhsParseNode *,
                                                          struct lhsParseNode *,
                                                          int,struct symbolHashNode *,
                                                          struct lhsParseNode *,
                                                          int);

/******************************************************************/
/* VariableAnalysis: Propagates variables references to other     */
/*   variables in the LHS and determines if there are any illegal */
/*   variable references (e.g. referring to an unbound variable). */
/*   The propagation of variable references simply means all      */
/*   subsequent references of a variable are made to "point" back */
/*   to the variable being propagated.                            */
/******************************************************************/
globle int VariableAnalysis(
  struct lhsParseNode *patternPtr)
  {
   struct lhsParseNode *rv, *theList, *tempList;
   int errorFlag = FALSE;

   /*======================================================*/
   /* Loop through all of the CEs in the rule to determine */
   /* which variables refer to other variables and whether */
   /* any semantic errors exist when refering to variables */
   /* (such as referring to a variable that was not        */
   /* previously bound).                                   */
   /*======================================================*/

   while (patternPtr != NULL)
     {
      /*=========================================================*/
      /* If a pattern CE is encountered, propagate any variables */
      /* found in the pattern and note any illegal references to */
      /* other variables.                                        */
      /*=========================================================*/

      if (patternPtr->type == PATTERN_CE)
        {
         /*====================================================*/
         /* Determine if the fact address associated with this */
         /* pattern illegally refers to other variables.       */
         /*====================================================*/

         if ((patternPtr->value != NULL) &&
             (patternPtr->referringNode != NULL))
           {
            errorFlag = TRUE;
            if (patternPtr->referringNode->index == -1)
              {
               PrintErrorID("ANALYSIS",1,TRUE);
               PrintRouter(WERROR,"Duplicate pattern-address ?");
               PrintRouter(WERROR,ValueToString(patternPtr->value));
               PrintRouter(WERROR," found in CE #");
               PrintLongInteger(WERROR,(long) patternPtr->whichCE);
               PrintRouter(WERROR,".\n");
              }
            else
              {
               PrintErrorID("ANALYSIS",2,TRUE);
               PrintRouter(WERROR,"Pattern-address ?");
               PrintRouter(WERROR,ValueToString(patternPtr->value));
               PrintRouter(WERROR," used in CE #");
               PrintLongInteger(WERROR,(long) patternPtr->whichCE);
               PrintRouter(WERROR," was previously bound within a pattern CE.\n");
              }
           }

         /*====================================================*/
         /* Propagate the pattern and field location of bound  */
         /* variables found in this pattern to other variables */
         /* in the same semantic scope as the bound variable.  */
         /*====================================================*/

         if (GetVariables(patternPtr)) return(TRUE);
        }

      /*==============================================================*/
      /* If a test CE is encountered, make sure that all references   */
      /* to variables have been previously bound. If they are bound   */
      /* then replace the references to variables with function calls */
      /* to retrieve the variables.                                   */
      /*==============================================================*/

      else if (patternPtr->type == TEST_CE)
        {
         /*=====================================================*/
         /* Verify that all variables were referenced properly. */
         /*=====================================================*/

         rv = CheckExpression(patternPtr->expression,NULL,(int) patternPtr->whichCE,NULL,0);

         /*=========================================================*/
         /* Determine the type and value constraints implied by the */
         /* expression and propagate these constraints to other     */
         /* variables in the LHS. For example, the expression       */
         /* (+ ?x 1) implies that ?x is a number.                   */
         /*=========================================================*/

         theList = GetExpressionVarConstraints(patternPtr->expression);
         for (tempList = theList; tempList != NULL; tempList = tempList->right)
            {
             if (PropagateVariableDriver(patternPtr,patternPtr,NULL,SF_VARIABLE,
                                         (SYMBOL_HN *) tempList->value,tempList,FALSE))
               {
                ReturnLHSParseNodes(theList);
                return(TRUE);
               }
            }
         ReturnLHSParseNodes(theList);

         /*========================================================*/
         /* If the variables in the expression were all referenced */
         /* properly, then create the expression to use in the     */
         /* join network.                                          */
         /*========================================================*/

         if (rv != NULL)
           { errorFlag = TRUE; }
         else
           { patternPtr->networkTest = GetvarReplace(patternPtr->expression); }
        }

      /*=====================================================*/
      /* Move on to the next pattern in the LHS of the rule. */
      /*=====================================================*/

      patternPtr = patternPtr->bottom;
     }

   /*==========================================*/
   /* Return the error status of the analysis. */
   /*==========================================*/

   return(errorFlag);
  }

/****************************************************************/
/* GetVariables: Loops through each field/slot within a pattern */
/*   and propagates the pattern and field location of bound     */
/*   variables found in the pattern to other variables within   */
/*   the same semantic scope as the bound variables.            */
/****************************************************************/
static int GetVariables(
  struct lhsParseNode *thePattern)
  {
   struct lhsParseNode *patternHead = thePattern;
   struct lhsParseNode *multifieldHeader = NULL;

   /*======================================================*/
   /* Loop through all the fields/slots found in a pattern */
   /* looking for binding instances of variables.          */
   /*======================================================*/

   while (thePattern != NULL)
     {
      /*================================================*/
      /* A multifield slot contains a sublist of fields */
      /* that must be traversed and checked.            */
      /*================================================*/

      if (thePattern->multifieldSlot)
        {
         multifieldHeader = thePattern;
         thePattern = thePattern->bottom;
        }

      /*==================================================*/
      /* Propagate the binding occurences of single field */
      /* variables, multifield variables, and fact        */
      /* addresses to other occurences of the variable.   */
      /* If an error is encountered, return TRUE.         */
      /*==================================================*/

      if (thePattern != NULL)
        {
         if ((thePattern->type == SF_VARIABLE) ||
             (thePattern->type == MF_VARIABLE) ||
             ((thePattern->type == PATTERN_CE) && (thePattern->value != NULL)))
           {
            if (ProcessVariable(thePattern,multifieldHeader,patternHead))
              { return(TRUE); }
           }
         else
           {
            if (ProcessField(thePattern,multifieldHeader,patternHead))
              { return(TRUE); }
           }
        }

      /*===============================================*/
      /* Move on to the next field/slot in the pattern */
      /* or to the next field in a multifield slot.    */
      /*===============================================*/

      if (thePattern == NULL)
        { thePattern = multifieldHeader; }
      else if ((thePattern->right == NULL) && (multifieldHeader != NULL))
        {
         thePattern = multifieldHeader;
         multifieldHeader = NULL;
        }

      thePattern = thePattern->right;
     }

   /*===============================*/
   /* Return FALSE to indicate that */
   /* no errors were detected.      */
   /*===============================*/

   return(FALSE);
  }

/******************************************************/
/* ProcessVariable: Processes a single occurence of a */
/*   variable by propagating references to it.        */
/******************************************************/
static int ProcessVariable(
  struct lhsParseNode *thePattern,
  struct lhsParseNode *multifieldHeader,
  struct lhsParseNode *patternHead)
  {
   int theType;
   struct symbolHashNode *theVariable;
   struct constraintRecord *theConstraints;

   /*=============================================================*/
   /* If a pattern address is being propagated, then treat it as  */
   /* a single field pattern variable and create a constraint     */
   /* which indicates that is must be a fact or instance address. */
   /* This code will have to be modified for new data types which */
   /* can match patterns.                                         */
   /*=============================================================*/

   if (thePattern->type == PATTERN_CE)
     {
      theType = SF_VARIABLE;
      theVariable = (struct symbolHashNode *) thePattern->value;
      if (thePattern->derivedConstraints) RemoveConstraint(thePattern->constraints);
      theConstraints = GetConstraintRecord();
      thePattern->constraints = theConstraints;
      thePattern->constraints->anyAllowed = FALSE;
      thePattern->constraints->instanceAddressesAllowed = TRUE;
      thePattern->constraints->factAddressesAllowed = TRUE;
      thePattern->derivedConstraints = TRUE;
     }

   /*===================================================*/
   /* Otherwise a pattern variable is being propagated. */
   /*===================================================*/

   else
     {
      theType = thePattern->type;
      theVariable = (struct symbolHashNode *) thePattern->value;
     }

   /*===================================================*/
   /* Propagate the variable location to any additional */
   /* fields associated with the binding variable.      */
   /*===================================================*/

   if (thePattern->type != PATTERN_CE)
     {
      PropagateVariableToNodes(thePattern->bottom,theType,theVariable,
                               thePattern,patternHead->beginNandDepth,
                               TRUE,FALSE);

      if (ProcessField(thePattern,multifieldHeader,patternHead))
        { return(TRUE); }
     }

   /*=================================================================*/
   /* Propagate the constraints to other fields, slots, and patterns. */
   /*=================================================================*/

   return(PropagateVariableDriver(patternHead,thePattern,multifieldHeader,theType,
                                  theVariable,thePattern,TRUE));
  }

/*******************************************/
/* PropagateVariableDriver: Driver routine */
/*   for propagating variable references.  */
/*******************************************/
static int PropagateVariableDriver(
  struct lhsParseNode *patternHead,
  struct lhsParseNode *theNode,
  struct lhsParseNode *multifieldHeader,
  int theType,
  struct symbolHashNode *variableName,
  struct lhsParseNode *theReference,
  int assignReference)
  {
   /*===================================================*/
   /* Propagate the variable location to any additional */
   /* constraints associated with the binding variable. */
   /*===================================================*/

   if (multifieldHeader != NULL)
     {
      if (PropagateVariableToNodes(multifieldHeader->right,theType,variableName,
                                   theReference,patternHead->beginNandDepth,assignReference,FALSE))
        {
         VariableMixingErrorMessage(variableName);
         return(TRUE);
        }
     }

   /*========================================================*/
   /* Propagate the variable location to fields/slots in the */
   /* same pattern which appear after the binding variable.  */
   /*========================================================*/

   if (PropagateVariableToNodes(theNode->right,theType,variableName,theReference,
                                patternHead->beginNandDepth,assignReference,FALSE))
     {
      VariableMixingErrorMessage(variableName);
      return(TRUE);
     }

   /*======================================================*/
   /* Propagate values to other patterns if the pattern in */
   /* which the variable is found is not a "not" CE or the */
   /* last pattern within a nand CE.                       */
   /*======================================================*/

   if (((patternHead->type == PATTERN_CE) || (patternHead->type == TEST_CE)) &&
       (patternHead->negated == FALSE) &&
       (patternHead->beginNandDepth <= patternHead->endNandDepth))
     {
      int ignoreVariableMixing;

      /*============================================================*/
      /* If the variables are propagated from a test CE, then don't */
      /* check for mixing of single and multifield variables (since */
      /* previously bound multifield variables typically have the $ */
      /* removed when passed as an argument to a function unless    */
      /* sequence expansion is desired).                            */
      /*============================================================*/

      if (patternHead->type == TEST_CE) ignoreVariableMixing = TRUE;
      else ignoreVariableMixing = FALSE;

      /*==========================*/
      /* Propagate the reference. */
      /*==========================*/

      if (PropagateVariableToNodes(patternHead->bottom,theType,variableName,theReference,
                                   patternHead->beginNandDepth,assignReference,
                                   ignoreVariableMixing))
       {
         VariableMixingErrorMessage(variableName);
         return(TRUE);
        }
     }

   /*==============================================*/
   /* Return FALSE to indicate that no errors were */
   /* generated by the variable propagation.       */
   /*==============================================*/

   return(FALSE);
  }

/********************************************************/
/* ProcessField: Processes a field or slot of a pattern */
/*   which does not contain a binding variable.         */
/********************************************************/
static int ProcessField(
  struct lhsParseNode *thePattern,
  struct lhsParseNode *multifieldHeader,
  struct lhsParseNode *patternHead)
  {
   struct lhsParseNode *theList, *tempList;

   /*====================================================*/
   /* Nothing needs to be done for the node representing */
   /* the entire pattern. Return FALSE to indicate that  */
   /* no errors were generated.                          */
   /*====================================================*/

   if (thePattern->type == PATTERN_CE) return(FALSE);

   /*====================================================================*/
   /* Derive a set of constraints based on values found in the slot or   */
   /* field. For example, if a slot can only contain the values 1, 2, or */
   /* 3, the field constraint ~2 would generate a constraint record that */
   /* only allows the value 1 or 3. Once generated, the constraints are  */
   /* propagated to other slots and fields.                              */
   /*====================================================================*/

   theList = DeriveVariableConstraints(thePattern);
   for (tempList = theList; tempList != NULL; tempList = tempList->right)
     {
      if (PropagateVariableDriver(patternHead,thePattern,multifieldHeader,tempList->type,
                                  (SYMBOL_HN *) tempList->value,tempList,FALSE))
        {
         ReturnLHSParseNodes(theList);
         return(TRUE);
        }
     }
   ReturnLHSParseNodes(theList);

   /*===========================================================*/
   /* Check for "variable referenced, but not previously bound" */
   /* errors. Return TRUE if this type of error is detected.    */
   /*===========================================================*/

   if (UnboundVariablesInPattern(thePattern,(int) patternHead->whichCE))
     { return(TRUE); }

   /*==================================================*/
   /* Check for constraint errors for this slot/field. */
   /* If the slot/field has unmatchable constraints    */
   /* then return TRUE to indicate a semantic error.   */
   /*==================================================*/

   if (ProcessConnectedConstraints(thePattern,multifieldHeader,patternHead))
     { return(TRUE); }

   /*==============================================================*/
   /* Convert the slot/field constraint to a series of expressions */
   /* that will be used in the pattern and join networks.          */
   /*==============================================================*/

   FieldConversion(thePattern,patternHead);

   /*=========================================================*/
   /* Return FALSE to indicate that no errors were generated. */
   /*=========================================================*/

   return(FALSE);
  }

/*************************************************************/
/* PropagateVariableToNodes: Propagates variable references  */
/*  to all other variables within the semantic scope of the  */
/*  bound variable. That is, a variable reference cannot be  */
/*  beyond an enclosing not/and CE combination. The          */
/*  restriction of propagating variables beyond an enclosing */
/*  not CE is handled within the GetVariables function.      */
/*************************************************************/
static int PropagateVariableToNodes(
  struct lhsParseNode *theNode,
  int theType,
  struct symbolHashNode *variableName,
  struct lhsParseNode *theReference,
  int startDepth,
  int assignReference,
  int ignoreVariableTypes)
  {
   struct constraintRecord *tempConstraints;

   /*===========================================*/
   /* Traverse the nodes using the bottom link. */
   /*===========================================*/

   while (theNode != NULL)
     {
      /*==================================================*/
      /* If the field/slot contains a predicate or return */
      /* value constraint, then propagate the variable to */
      /* the expression associated with that constraint.  */
      /*==================================================*/

      if (theNode->expression != NULL)
        {
         PropagateVariableToNodes(theNode->expression,theType,variableName,
                                  theReference,startDepth,assignReference,TRUE);
        }

      /*======================================================*/
      /* If the field/slot is a single or multifield variable */
      /* with the same name as the propagated variable,       */
      /* then propagate the variable location to this node.   */
      /*======================================================*/

      else if (((theNode->type == SF_VARIABLE) || (theNode->type == MF_VARIABLE)) &&
               (theNode->value == (void *) variableName))
        {
         /*======================================================*/
         /* Check for mixing of single and multifield variables. */
         /*======================================================*/

         if (ignoreVariableTypes == FALSE)
           {
            if (((theType == SF_VARIABLE) && (theNode->type == MF_VARIABLE)) ||
                ((theType == MF_VARIABLE) && (theNode->type == SF_VARIABLE)))
              { return(TRUE); }
           }

         /*======================================================*/
         /* Intersect the propagated variable's constraints with */
         /* the current constraints for this field/slot.         */
         /*======================================================*/

         if ((theReference->constraints != NULL) && (! theNode->negated))
           {
            tempConstraints = theNode->constraints;
            theNode->constraints = IntersectConstraints(theReference->constraints,
                                                        tempConstraints);
            if (theNode->derivedConstraints)
              { RemoveConstraint(tempConstraints); }

            theNode->derivedConstraints = TRUE;
           }


         /*=====================================================*/
         /* Don't propagate the variable if it originates from  */
         /* a different type of pattern object and the variable */
         /* reference has already been resolved.                */
         /*=====================================================*/

         if (assignReference)
           {
            if (theNode->referringNode == NULL)
              { theNode->referringNode = theReference; }
            else if (theReference->pattern == theNode->pattern)
              { theNode->referringNode = theReference; }
            else if (theReference->patternType == theNode->patternType)
              { theNode->referringNode = theReference; }
           }
        }

      /*========================================================*/
      /* If the field/slot is the node representing the entire  */
      /* pattern, then propagate the variable location to the   */
      /* fact address associated with the pattern (if it is the */
      /* same variable name).                                   */
      /*========================================================*/

      else if ((theNode->type == PATTERN_CE) &&
               (theNode->value == (void *) variableName) &&
               (assignReference == TRUE))
        {
         if (theType == MF_VARIABLE) return(TRUE);

         theNode->referringNode = theReference;
        }

      /*=====================================================*/
      /* Propagate the variable to other fields contained    */
      /* within the same & field constraint or same pattern. */
      /*=====================================================*/

      if (theNode->right != NULL)
        {
         if (PropagateVariableToNodes(theNode->right,theType,variableName,
                                      theReference,startDepth,assignReference,ignoreVariableTypes))
           { return(TRUE); }
        }

      /*============================================================*/
      /* Propagate the variable to other patterns within the same   */
      /* semantic scope (if dealing with the node for an entire     */
      /* pattern) or to the next | field constraint within a field. */
      /*============================================================*/

      if ((theNode->type == PATTERN_CE) || (theNode->type == TEST_CE))
        {
         if (theNode->endNandDepth < startDepth) theNode = NULL;
         else theNode = theNode->bottom;
        }
      else
        { theNode = theNode->bottom; }
     }

   /*========================================================*/
   /* Return FALSE to indicate that no errors were detected. */
   /*========================================================*/

   return(FALSE);
  }

/*************************************************************/
/* UnboundVariablesInPattern: Verifies that variables within */
/*   a slot/field have been referenced properly (i.e. that   */
/*   variables have been previously bound if they are not a  */
/*   binding occurrence).                                    */
/*************************************************************/
static BOOLEAN UnboundVariablesInPattern(
  struct lhsParseNode *theSlot,
  int pattern)
  {
   struct lhsParseNode *andField;
   struct lhsParseNode *rv;
   int result;
   struct lhsParseNode *orField;
   struct symbolHashNode *slotName;
   CONSTRAINT_RECORD *theConstraints;
   int theField;

   /*===================================================*/
   /* If a multifield slot is being checked, then check */
   /* each of the fields grouped with the multifield.   */
   /*===================================================*/

   if (theSlot->multifieldSlot)
     {
      theSlot = theSlot->bottom;
      while (theSlot != NULL)
        {
         if (UnboundVariablesInPattern(theSlot,pattern))
           { return(TRUE); }
         theSlot = theSlot->right;
        }

      return(FALSE);
     }

   /*=======================*/
   /* Check a single field. */
   /*=======================*/

   slotName = theSlot->slot;
   theField = theSlot->index;
   theConstraints = theSlot->constraints;

   /*===========================================*/
   /* Loop through each of the '|' constraints. */
   /*===========================================*/

   for (orField = theSlot->bottom;
        orField != NULL;
        orField = orField->bottom)
     {
      /*===========================================*/
      /* Loop through each of the fields connected */
      /* by the '&' within the '|' constraint.     */
      /*===========================================*/

      for (andField = orField;
           andField != NULL;
           andField = andField->right)
        {
         /*=======================================================*/
         /* If this is not a binding occurence of a variable and  */
         /* there is no previous binding occurence of a variable, */
         /* then generate an error message for a variable that is */
         /* referred to but not bound.                            */
         /*=======================================================*/

         if (((andField->type == SF_VARIABLE) || (andField->type == MF_VARIABLE)) &&
             (andField->referringNode == NULL))
           {
            VariableReferenceErrorMessage((SYMBOL_HN *) andField->value,NULL,pattern,
                                          slotName,theField);
            return(TRUE);
           }

         /*==============================================*/
         /* Check predicate and return value constraints */
         /* to insure that all variables used within the */
         /* constraint have been previously bound.       */
         /*==============================================*/

         else if ((andField->type == PREDICATE_CONSTRAINT) ||
                  (andField->type == RETURN_VALUE_CONSTRAINT))
           {
            rv = CheckExpression(andField->expression,NULL,pattern,slotName,theField);
            if (rv != NULL) return(TRUE);
           }

         /*========================================================*/
         /* If static constraint checking is being performed, then */
         /* determine if constant values have violated the set of  */
         /* derived constraints for the slot/field (based on the   */
         /* deftemplate definition and propagated constraints).    */
         /*========================================================*/

         else if (((andField->type == INTEGER) || (andField->type == FLOAT) ||
                   (andField->type == SYMBOL) || (andField->type == STRING) ||
                   (andField->type == INSTANCE_NAME)) &&
                  GetStaticConstraintChecking())
           {
            result = ConstraintCheckValue(andField->type,andField->value,theConstraints);
            if (result != NO_VIOLATION)
              {
               ConstraintViolationErrorMessage("A literal restriction value",
                                               NULL,FALSE,pattern,
                                               slotName,theField,result,
                                               theConstraints,TRUE);
               return(TRUE);
              }
           }
        }
     }

   /*===============================*/
   /* Return FALSE to indicate that */
   /* no errors were detected.      */
   /*===============================*/

   return(FALSE);
  }

/******************************************************************/
/* CheckExpression: Verifies that variables within an expression  */
/*   have been referenced properly. All variables within an       */
/*   expression must have been previously bound.                  */
/******************************************************************/
static struct lhsParseNode *CheckExpression(
  struct lhsParseNode *exprPtr,
  struct lhsParseNode *lastOne,
  int whichCE,
  struct symbolHashNode *slotName,
  int theField)
  {
   struct lhsParseNode *rv;
   int i = 1;

   while (exprPtr != NULL)
     {
      /*===============================================================*/
      /* Check that single field variables contained in the expression */
      /* were previously defined in the LHS. Also check to see if the  */
      /* variable has unmatchable constraints.                         */
      /*===============================================================*/

      if (exprPtr->type == SF_VARIABLE)
        {
         if (exprPtr->referringNode == NULL)
           {
            VariableReferenceErrorMessage((SYMBOL_HN *) exprPtr->value,lastOne,
                                          whichCE,slotName,theField);
            return(exprPtr);
           }
         else if ((UnmatchableConstraint(exprPtr->constraints)) &&
                  GetStaticConstraintChecking())
           {
            ConstraintReferenceErrorMessage((SYMBOL_HN *) exprPtr->value,lastOne,i,
                                            whichCE,slotName,theField);
            return(exprPtr);
           }
        }

      /*==================================================*/
      /* Check that multifield variables contained in the */
      /* expression were previously defined in the LHS.   */
      /*==================================================*/

      else if ((exprPtr->type == MF_VARIABLE) && (exprPtr->referringNode == NULL))
        {
         VariableReferenceErrorMessage((SYMBOL_HN *) exprPtr->value,lastOne,
                                       whichCE,slotName,theField);
         return(exprPtr);
        }

      /*=====================================================*/
      /* Check that global variables are referenced properly */
      /* (i.e. if you reference a global variable, it must   */
      /* already be defined by a defglobal construct).       */
      /*=====================================================*/

#if DEFGLOBAL_CONSTRUCT
      else if (exprPtr->type == GBL_VARIABLE)
        {
         int count;

         if (FindImportedConstruct("defglobal",NULL,ValueToString(exprPtr->value),
                                   &count,TRUE,NULL) == NULL)
           {
            VariableReferenceErrorMessage((SYMBOL_HN *) exprPtr->value,lastOne,
                                          whichCE,slotName,theField);
            return(exprPtr);
           }
        }
#endif

      /*============================================*/
      /* Recursively check other function calls to  */
      /* insure variables are referenced correctly. */
      /*============================================*/

      else if (((exprPtr->type == FCALL)
#if DEFGENERIC_CONSTRUCT
             || (exprPtr->type == GCALL)
#endif
#if DEFFUNCTION_CONSTRUCT
             || (exprPtr->type == PCALL)
#endif
         ) && (exprPtr->bottom != NULL))
        {
         if ((rv = CheckExpression(exprPtr->bottom,exprPtr,whichCE,slotName,theField)) != NULL)
           { return(rv); }
        }

      /*=============================================*/
      /* Move on to the next part of the expression. */
      /*=============================================*/

      i++;
      exprPtr = exprPtr->right;
     }

   /*================================================*/
   /* Return NULL to indicate no error was detected. */
   /*================================================*/

   return(NULL);
  }

/********************************************************/
/* VariableReferenceErrorMessage: Generic error message */
/*   for referencing a variable before it is defined.   */
/********************************************************/
static void VariableReferenceErrorMessage(
  struct symbolHashNode *theVariable,
  struct lhsParseNode *theExpression,
  int whichCE,
  struct symbolHashNode *slotName,
  int theField)
  {
   struct expr *temprv;

   /*=============================*/
   /* Print the error message ID. */
   /*=============================*/

   PrintErrorID("ANALYSIS",4,TRUE);

   /*=================================*/
   /* Print the name of the variable. */
   /*=================================*/

   PrintRouter(WERROR,"Variable ?");
   PrintRouter(WERROR,ValueToString(theVariable));
   PrintRouter(WERROR," ");

   /*=================================================*/
   /* If the variable was found inside an expression, */
   /* then print the expression.                      */
   /*=================================================*/

   if (theExpression != NULL)
     {
      temprv = LHSParseNodesToExpression(theExpression);
      ReturnExpression(temprv->nextArg);
      temprv->nextArg = NULL;
      PrintRouter(WERROR,"found in the expression ");
      PrintExpression(WERROR,temprv);
      PrintRouter(WERROR,"\n");
      ReturnExpression(temprv);
     }

   /*====================================================*/
   /* Print the CE in which the variable was referenced. */
   /*====================================================*/

   PrintRouter(WERROR,"was referenced in CE #");
   PrintLongInteger(WERROR,(long int) whichCE);

   /*=====================================*/
   /* Identify the slot or field in which */
   /* the variable was found.             */
   /*=====================================*/

   if (slotName == NULL)
     {
      if (theField > 0)
        {
         PrintRouter(WERROR," field #");
         PrintLongInteger(WERROR,(long int) theField);
        }
     }
   else
     {
      PrintRouter(WERROR," slot ");
      PrintRouter(WERROR,ValueToString(slotName));
     }

   PrintRouter(WERROR," before being defined.\n");
  }

/************************************************************/
/* VariableMixingErrorMessage: Prints the error message for */
/*   the illegal mixing of single and multifield variables  */
/*   on the LHS of a rule.                                  */
/************************************************************/
static void VariableMixingErrorMessage(
  struct symbolHashNode *theVariable)
  {
   PrintErrorID("ANALYSIS",3,TRUE);
   PrintRouter(WERROR,"Variable ?");
   PrintRouter(WERROR,ValueToString(theVariable));
   PrintRouter(WERROR," is used as both a single and multifield variable in the LHS\n");
  }

#endif /* (! RUN_TIME) && (! BLOAD_ONLY) && DEFRULE_CONSTRUCT */


