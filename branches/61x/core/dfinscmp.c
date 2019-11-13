   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*              CLIPS Version 6.05  04/09/97           */
   /*                                                     */
   /*                                                     */
   /*******************************************************/

/*************************************************************/
/* Purpose: Definstances Construct Compiler Code             */
/*                                                           */
/* Principal Programmer(s):                                  */
/*      Brian L. Donnell                                     */
/*                                                           */
/* Contributing Programmer(s):                               */
/*                                                           */
/* Revision History:                                         */
/*                                                           */
/*************************************************************/

/* =========================================
   *****************************************
               EXTERNAL DEFINITIONS
   =========================================
   ***************************************** */
#include "setup.h"

#if DEFINSTANCES_CONSTRUCT && CONSTRUCT_COMPILER && (! RUN_TIME)

#include "conscomp.h"
#include "defins.h"

#define _DFINSCMP_SOURCE_
#include "dfinscmp.h"

/* =========================================
   *****************************************
                   CONSTANTS
   =========================================
   ***************************************** */

/* =========================================
   *****************************************
               MACROS AND TYPES
   =========================================
   ***************************************** */

/* =========================================
   *****************************************
      INTERNALLY VISIBLE FUNCTION HEADERS
   =========================================
   ***************************************** */

static void ReadyDefinstancesForCode(void);
static int DefinstancesToCode(char *,int,FILE *,int,int);
static void CloseDefinstancesFiles(FILE *,FILE *,int);
static void DefinstancesModuleToCode(FILE *,struct defmodule *,int,int);
static void SingleDefinstancesToCode(FILE *,DEFINSTANCES *,int,int,int);

/* =========================================
   *****************************************
      INTERNALLY VISIBLE GLOBAL VARIABLES
   =========================================
   ***************************************** */
static struct CodeGeneratorItem *DefinstancesCodeItem;

/* =========================================
   *****************************************
          EXTERNALLY VISIBLE FUNCTIONS
   =========================================
   ***************************************** */

/***************************************************
  NAME         : SetupDefinstancesCompiler
  DESCRIPTION  : Initializes the construct compiler
                   item for definstances
  INPUTS       : None
  RETURNS      : Nothing useful
  SIDE EFFECTS : Code generator item initialized
  NOTES        : None
 ***************************************************/
globle void SetupDefinstancesCompiler()
  {
   DefinstancesCodeItem = AddCodeGeneratorItem("definstances",0,ReadyDefinstancesForCode,
                                               NULL,DefinstancesToCode,2);
  }


/****************************************************
  NAME         : DefinstancesCModuleReference
  DESCRIPTION  : Prints out a reference to a
                 definstances module
  INPUTS       : 1) The output file
                 2) The id of the module item
                 3) The id of the image
                 4) The maximum number of elements
                    allowed in an array
  RETURNS      : Nothing useful
  SIDE EFFECTS : Definstances module reference printed
  NOTES        : None
 ****************************************************/
globle void DefinstancesCModuleReference(
  FILE *theFile,
  int count,
  int imageID,
  int maxIndices)
  {
   fprintf(theFile,"MIHS &%s%d_%d[%d]",
                      ModulePrefix(DefinstancesCodeItem),
                      imageID,
                      (count / maxIndices) + 1,
                      (count % maxIndices));
  }

/* =========================================
   *****************************************
          INTERNALLY VISIBLE FUNCTIONS
   =========================================
   ***************************************** */

/***************************************************
  NAME         : ReadyDefinstancesForCode
  DESCRIPTION  : Sets index of deffunctions
                   for use in compiled expressions
  INPUTS       : None
  RETURNS      : Nothing useful
  SIDE EFFECTS : BsaveIndices set
  NOTES        : None
 ***************************************************/
static void ReadyDefinstancesForCode()
  {
   MarkConstructBsaveIDs(DefinstancesModuleIndex);
  }

/*******************************************************
  NAME         : DefinstancesToCode
  DESCRIPTION  : Writes out static array code for
                   definstances
  INPUTS       : 1) The base name of the construct set
                 2) The base id for this construct
                 3) The file pointer for the header file
                 4) The base id for the construct set
                 5) The max number of indices allowed
                    in an array
  RETURNS      : -1 if no definstances, 0 on errors,
                  1 if definstances written
  SIDE EFFECTS : Code written to files
  NOTES        : None
 *******************************************************/
static int DefinstancesToCode(
  char *fileName,
  int fileID,
  FILE *headerFP,
  int imageID,
  int maxIndices)
  {
   int fileCount = 1;
   struct defmodule *theModule;
   DEFINSTANCES *theDefinstances;
   int moduleCount = 0, moduleArrayCount = 0, moduleArrayVersion = 1;
   int definstancesArrayCount = 0, definstancesArrayVersion = 1;
   FILE *moduleFile = NULL, *definstancesFile = NULL;

   /* ================================================
      Include the appropriate definstances header file
      ================================================ */
   fprintf(headerFP,"#include \"defins.h\"\n");

   /* =============================================================
      Loop through all the modules and all the definstances writing
      their C code representation to the file as they are traversed
      ============================================================= */
   theModule = (struct defmodule *) GetNextDefmodule(NULL);

   while (theModule != NULL)
     {
      SetCurrentModule((void *) theModule);

      moduleFile = OpenFileIfNeeded(moduleFile,fileName,fileID,imageID,&fileCount,
                                    moduleArrayVersion,headerFP,
                                    "DEFINSTANCES_MODULE",ModulePrefix(DefinstancesCodeItem),
                                    FALSE,NULL);

      if (moduleFile == NULL)
        {
         CloseDefinstancesFiles(moduleFile,definstancesFile,maxIndices);
         return(0);
        }

      DefinstancesModuleToCode(moduleFile,theModule,imageID,maxIndices);
      moduleFile = CloseFileIfNeeded(moduleFile,&moduleArrayCount,&moduleArrayVersion,
                                     maxIndices,NULL,NULL);

      theDefinstances = (DEFINSTANCES *) GetNextDefinstances(NULL);

      while (theDefinstances != NULL)
        {
         definstancesFile = OpenFileIfNeeded(definstancesFile,fileName,fileID,imageID,&fileCount,
                                             definstancesArrayVersion,headerFP,
                                             "DEFINSTANCES",ConstructPrefix(DefinstancesCodeItem),
                                             FALSE,NULL);
         if (definstancesFile == NULL)
           {
            CloseDefinstancesFiles(moduleFile,definstancesFile,maxIndices);
            return(0);
           }

         SingleDefinstancesToCode(definstancesFile,theDefinstances,imageID,
                                  maxIndices,moduleCount);
         definstancesArrayCount++;
         definstancesFile = CloseFileIfNeeded(definstancesFile,&definstancesArrayCount,
                                              &definstancesArrayVersion,maxIndices,NULL,NULL);

         theDefinstances = (DEFINSTANCES *) GetNextDefinstances(theDefinstances);
        }

      theModule = (struct defmodule *) GetNextDefmodule(theModule);
      moduleCount++;
      moduleArrayCount++;
     }

   CloseDefinstancesFiles(moduleFile,definstancesFile,maxIndices);

   return(1);
  }

/***************************************************
  NAME         : CloseDefinstancesFiles
  DESCRIPTION  : Closes construct compiler files
                  for definstances structures
  INPUTS       : 1) The definstances module file
                 2) The definstances structure file
                 3) The maximum number of indices
                    allowed in an array
  RETURNS      : Nothing useful
  SIDE EFFECTS : Files closed
  NOTES        : None
 ***************************************************/
static void CloseDefinstancesFiles(
  FILE *moduleFile,
  FILE *definstancesFile,
  int maxIndices)
  {
   int count = maxIndices;
   int arrayVersion = 0;

   if (definstancesFile != NULL)
     {
      count = maxIndices;
      CloseFileIfNeeded(definstancesFile,&count,&arrayVersion,
                                         maxIndices,NULL,NULL);
     }

   if (moduleFile != NULL)
     {
      count = maxIndices;
      CloseFileIfNeeded(moduleFile,&count,&arrayVersion,maxIndices,NULL,NULL);
     }
  }

/***************************************************
  NAME         : DefinstancesModuleToCode
  DESCRIPTION  : Writes out the C values for a
                 definstances module item
  INPUTS       : 1) The output file
                 2) The module for the definstances
                 3) The compile image id
                 4) The maximum number of elements
                    in an array
  RETURNS      : Nothing useful
  SIDE EFFECTS : Definstances module item written
  NOTES        : None
 ***************************************************/
static void DefinstancesModuleToCode(
  FILE *theFile,
  struct defmodule *theModule,
  int imageID,
  int maxIndices)
  {
   fprintf(theFile,"{");
   ConstructModuleToCode(theFile,theModule,imageID,maxIndices,
                         DefinstancesModuleIndex,ConstructPrefix(DefinstancesCodeItem));
   fprintf(theFile,"}");
  }

/***************************************************
  NAME         : SingleDefinstancesToCode
  DESCRIPTION  : Writes out a single definstances'
                 data to the file
  INPUTS       : 1) The output file
                 2) The definstances
                 3) The compile image id
                 4) The maximum number of
                    elements in an array
                 5) The module index
  RETURNS      : Nothing useful
  SIDE EFFECTS : Definstances data written
  NOTES        : None
 ***************************************************/
static void SingleDefinstancesToCode(
  FILE *theFile,
  DEFINSTANCES *theDefinstances,
  int imageID,
  int maxIndices,
  int moduleCount)
  {
   /* ===================
      Definstances Header
      =================== */

   fprintf(theFile,"{");
   ConstructHeaderToCode(theFile,&theDefinstances->header,imageID,maxIndices,moduleCount,
                         ModulePrefix(DefinstancesCodeItem),
                         ConstructPrefix(DefinstancesCodeItem));

   /* ==========================
      Definstances specific data
      ========================== */
   fprintf(theFile,",0,");
   ExpressionToCode(theFile,theDefinstances->mkinstance);
   fprintf(theFile,"}");
  }

#endif

/***************************************************
  NAME         :
  DESCRIPTION  :
  INPUTS       :
  RETURNS      :
  SIDE EFFECTS :
  NOTES        :
 ***************************************************/
