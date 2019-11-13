#include <windows.h>

#include "clips.h"

BOOL WINAPI DllMain(
  HINSTANCE hinstDLL,
  DWORD fwdreason,
  LPVOID lpvReserved)
  {
   return 1;
  }

void __declspec(dllexport) * __CreateEnvironment()
  {
   return CreateEnvironment();
  }

void __declspec(dllexport) __DestroyEnvironment(
  void *theEnv)
  {
   DestroyEnvironment(theEnv);
  }

void __declspec(dllexport) __CommandLoop(
  void *theEnv)
  {
   CommandLoop(theEnv);
  }

void __declspec(dllexport) __EnvClear(
  void *theEnv)
  {
   EnvClear(theEnv);
  }

void __declspec(dllexport) __EnvReset(
  void *theEnv)
  {
   EnvReset(theEnv);
  }

int __declspec(dllexport) __EnvLoad(
  void *theEnv,
  char *theFile)
  {
   return EnvLoad(theEnv,theFile);
  }

void __declspec(dllexport) __LoadConstructsFromLogicalName(
  void *theEnv,
  char *logicalName)
  {
   LoadConstructsFromLogicalName(theEnv,logicalName);
  }

int __declspec(dllexport) __OpenStringSource(
  void *theEnv,
  const char *logicalName,
  const char *stringData,
  size_t size)
  {
   return OpenStringSource(theEnv,logicalName,stringData,size);
  }

int __declspec(dllexport) __CloseStringSource(
  void *theEnv,
  char *logicalName)
  {
   return CloseStringSource(theEnv,logicalName);
  }

long long __declspec(dllexport) __EnvRun(
  void *theEnv,
  long long runLimit)
  {
   return EnvRun(theEnv,runLimit);
  }
  
int __declspec(dllexport) __EnvBuild(
  void *theEnv,
  char *buildString)
  {
   return EnvBuild(theEnv,buildString);
  }
  
int __declspec(dllexport) __EnvEval(
  void *theEnv,
  char *evalString,
  void *rv)
  {
   return EnvEval(theEnv,evalString,(DATA_OBJECT *) rv);
  }  

void __declspec(dllexport) __EnvIncrementFactCount(
  void *theEnv,
  void *theFact)
  {
   EnvIncrementFactCount(theEnv,theFact);
  }

void __declspec(dllexport) __EnvDecrementFactCount(
  void *theEnv,
  void *theFact)
  {
   EnvDecrementFactCount(theEnv,theFact);
  }

void __declspec(dllexport) __EnvIncrementInstanceCount(
  void *theEnv,
  void *theInstance)
  {
   EnvIncrementInstanceCount(theEnv,theInstance);
  }

void __declspec(dllexport) __EnvDecrementInstanceCount(
  void *theEnv,
  void *theInstance)
  {
   EnvDecrementFactCount(theEnv,theInstance);
  }

long long __declspec(dllexport) __EnvFactIndex(
  void *theEnv,
  void *theFact)
  {
   return EnvFactIndex(theEnv,theFact);
  }

int __declspec(dllexport) __EnvGetFactSlot(
  void *theEnv,
  void *theFact,
  char *slotName,
  void *returnValue)
  {
   return EnvGetFactSlot(theEnv,theFact,slotName,(DATA_OBJECT *) returnValue);  
  }  

void __declspec(dllexport) * __EnvAssertString(
  void *theEnv,
  const char *factString)
  {
   return EnvAssertString(theEnv,factString);
  }

const char __declspec(dllexport) * __EnvGetInstanceName(
  void *theEnv,
  void *theInstance)
  {
   return EnvGetInstanceName(theEnv,theInstance);  
  } 
  
void __declspec(dllexport) __EnvDirectGetSlot(
  void *theEnv,
  void *theInstance,
  char *slotName,
  void *returnValue)
  {
   EnvDirectGetSlot(theEnv,theInstance,slotName,(DATA_OBJECT *) returnValue);  
  }  
  
int __declspec(dllexport) __EnvWatch(
  void *theEnv,
  char *item)
  {
   return EnvWatch(theEnv,item);
  }

int __declspec(dllexport) __EnvUnwatch(
  void *theEnv,
  char *item)
  {
   return EnvUnwatch(theEnv,item);
  }

void __declspec(dllexport) * __GetEnvironmentContext(
  void *theEnv)
  {
   return GetEnvironmentContext(theEnv);
  }

void __declspec(dllexport) * __GetEnvironmentRouterContext(
  void *theEnv)
  {
   return GetEnvironmentRouterContext(theEnv);
  }

int __declspec(dllexport) __EnvAddRouterWithContext(
  void *theEnv,
  const char *routerName,
  int priority,
  int (*queryFunction)(void *,const char *),
  int (*printFunction)(void *,const char *,const char *),
  int (*getcFunction)(void *,const char *),
  int (*ungetcFunction)(void *,int,const char *),
  int (*exitFunction)(void *,int),
  void *context)
  {
   return EnvAddRouterWithContext(theEnv,routerName,priority,queryFunction,printFunction,getcFunction,ungetcFunction,exitFunction,context);
  }
 
 size_t __declspec(dllexport) __EnvInputBufferCount(
  void *theEnv)
  {
   return EnvInputBufferCount(theEnv);
  }
