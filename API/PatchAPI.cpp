/* Copyright (c) 2022-2023 Dreamy Cecil
This program is free software; you can redistribute it and/or modify
it under the terms of version 2 of the GNU General Public License as published by
the Free Software Foundation


This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */

#include "StdH.h"

// Engine library handle
HINSTANCE CPatchAPI::hEngine = NULL;

// Entities library handle
CTFileName CPatchAPI::fnmEntities;
HINSTANCE CPatchAPI::hEntities = NULL;

// List available function patches
static void ListFuncPatches(void) {
  const INDEX ctPatches = GetPatchAPI()->aPatches.Count();

  if (ctPatches == 0) {
    CPutString(TRANS("No function patches available!\n"));
    return;
  }

  CPutString(TRANS("Available function patches:\n"));

  for (INDEX iPatch = 0; iPatch < ctPatches; iPatch++) {
    const SFuncPatch &fpPatch = GetPatchAPI()->aPatches[iPatch];

    // Mark as enabled or not and indent the index
    const char *strPatched = (fpPatch.pPatch->IsPatched() ? " [^c00ff00ON^r]" : "[^cff0000OFF^r]");
    const INDEX ctIdent = ClampDn(2 - INDEX(log10((FLOAT)iPatch)), (INDEX)0);

    CPrintF("%s %*s%d - %s\n", strPatched, ctIdent, "", iPatch, fpPatch.strName);
  }
};

// Enable specific function patch
static void EnableFuncPatch(INDEX iPatch) {
  if (iPatch < 0 || iPatch >= GetPatchAPI()->aPatches.Count()) {
    CPutString(TRANS("Invalid patch index!\n"));
    return;
  }

  const CTString &strPatch = GetPatchAPI()->aPatches[iPatch].strName;
  BOOL bPatched = GetPatchAPI()->EnablePatch(iPatch);

  if (bPatched) {
    CPrintF(TRANS("Successfully set '%s' function patch!\n"), strPatch);
  } else {
    CPrintF(TRANS("Cannot set '%s' function patch!\n"), strPatch);
  }
};

// Disable specific function patch
static void DisableFuncPatch(INDEX iPatch) {
  if (iPatch < 0 || iPatch >= GetPatchAPI()->aPatches.Count()) {
    CPutString(TRANS("Invalid patch index!\n"));
    return;
  }

  const CTString &strPatch = GetPatchAPI()->aPatches[iPatch].strName;
  GetPatchAPI()->DisablePatch(iPatch);

  CPrintF(TRANS("Successfully removed '%s' function patch!\n"), strPatch);
};

// Get function patch index by its name
static INDEX GetFuncPatch(const CTString &strName) {
  return GetPatchAPI()->GetPatchIndex(strName, NULL);
};

// Constructor
CPatchAPI::CPatchAPI() {
  // Hook up the libraries
  #ifdef NDEBUG
    hEngine = GetModuleHandleA("Engine.dll");
  #else
    hEngine = GetModuleHandleA("EngineD.dll");
  #endif

  CTString strEntitiesLib = CCoreAPI::AppBin() + "Entities" + _strModExt
  #ifndef NDEBUG
    + "D"
  #endif
    + ".dll";

  ExpandFilePath(EFP_READ, strEntitiesLib, fnmEntities);
  hEntities = GetModuleHandleA(fnmEntities.str_String);

  // Commands for manually toggling function patches
  _pShell->DeclareSymbol("void ListPatches(void);",   &ListFuncPatches);
  _pShell->DeclareSymbol("void EnablePatch(INDEX);",  &EnableFuncPatch);
  _pShell->DeclareSymbol("void DisablePatch(INDEX);", &DisableFuncPatch);
  _pShell->DeclareSymbol("INDEX GetFuncPatch(CTString);", &GetFuncPatch);
};

// Externally create a new patch class
CPatch *CPatchAPI::CreatePatch(BOOL bSetForever) {
  return new CPatch(bSetForever != FALSE);
};

// Enable specific function patch
BOOL CPatchAPI::EnablePatch(INDEX iPatch) {
  SFuncPatch &fpPatch = aPatches[iPatch];
  fpPatch.pPatch->SetPatch();

  return fpPatch.pPatch->IsValid();
};

// Disable specific function patch
void CPatchAPI::DisablePatch(INDEX iPatch) {
  SFuncPatch &fpPatch = aPatches[iPatch];
  fpPatch.pPatch->RemovePatch();
};

// Get function patch index by its name or patch pointer
INDEX CPatchAPI::GetPatchIndex(const CTString &strName, CPatch *pPatch) {
  const ULONG ulNameHash = strName.GetHash();
  const INDEX ctPatches = aPatches.Count();

  for (INDEX iPatch = 0; iPatch < ctPatches; iPatch++) {
    SFuncPatch &fp = aPatches[iPatch];

    // Matching pointer or name hash
    if (fp.pPatch == pPatch || fp.ulHash == ulNameHash) {
      // Found function patch
      return iPatch;
    }
  }

  // Not found
  return -1;
};

// Find function patch by its name or its patch pointer
SFuncPatch *CPatchAPI::FindFuncPatch(const CTString &strName, CPatch *pPatch) {
  const ULONG ulNameHash = strName.GetHash();
  const INDEX ctPatches = aPatches.Count();

  for (INDEX iPatch = 0; iPatch < ctPatches; iPatch++) {
    SFuncPatch &fp = aPatches[iPatch];

    // Matching pointer or name hash
    if (fp.pPatch == pPatch || fp.ulHash == ulNameHash) {
      // Found function patch
      return &fp;
    }
  }

  // Not found
  return NULL;
};

// Retrieve address from the engine by a symbol name
void *CPatchAPI::GetEngineSymbol(const char *strSymbolName) {
  return GetProcAddress(hEngine, strSymbolName);
};

// Retrieve address from the entities by a symbol name
void *CPatchAPI::GetEntitiesSymbol(const char *strSymbolName) {
  return GetProcAddress(hEntities, strSymbolName);
};
