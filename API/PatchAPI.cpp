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

// Define table of entity property references
#include <EngineEx/PropertyTables.h>

static CPropertyRefTable _mapPropRefs;

// Define static references that get added into the reference table immediately
#define ENTITYPROPERTYREF_DECL static
#define ENTITYPROPERTYREF_ENTRY(Class, Refs, RefsCount) \
  struct Class##_PropRefsEntryInit { \
    int iDummy; \
    Class##_PropRefsEntryInit() { \
      _mapPropRefs.FillPropertyReferences(#Class, Refs, RefsCount); \
    }; \
  } Class##_proprefsentry;

#include <EntitiesV/_DefinePropertyRefLists.inl>

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
    const INDEX ctIndentLog10 = (iPatch == 0) ? 0 : log10((FLOAT)iPatch);
    const INDEX ctIndent = ClampDn(2 - ctIndentLog10, (INDEX)0);

    CPrintF("%s %*s%d - %s\n", strPatched, ctIndent, "", iPatch, fpPatch.strName);
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
  hEngine = HMODULE_ENGINE;

  #if SE1_VER >= SE1_107
    fnmEntities = CCoreAPI::FullLibPath("Entities" + _strModExt);
  #else
    fnmEntities = "Bin\\" + CCoreAPI::GetLibFile("Entities" + CCoreAPI::GetVanillaExt());
  #endif
  hEntities = LoadLibraryA((CCoreAPI::AppPath() + fnmEntities).str_String);

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

// Retrieve address from the engine by a symbol name (before Core initialization)
void *CPatchAPI::GetEngineSymbolPortable(const char *strSymbol) {
  #ifdef NDEBUG
    return GetProcAddress(GetModuleHandleA("Engine.dll"), strSymbol);
  #else
    return GetProcAddress(GetModuleHandleA("EngineD.dll"), strSymbol);
  #endif
};

// Retrieve address from the engine by a symbol name
void *CPatchAPI::GetEngineSymbol(const char *strSymbol) {
  return GetProcAddress(hEngine, strSymbol);
};

// Retrieve address from the entities by a symbol name
void *CPatchAPI::GetEntitiesSymbol(const char *strSymbol) {
  return GetProcAddress(hEntities, strSymbol);
};

// Find entity property data by a variable name of a specific class
const CEntityProperty *CPatchAPI::FindProperty(const CTString &strClass, const CTString &strVariable) {
  // Find property in the table
  const CEntityProperty *pep = _mapPropRefs.FindProperty(strClass, strVariable);
  ASSERTMSG(pep != NULL, "Cannot find entity property data in the reference table!");

  return pep;
};
