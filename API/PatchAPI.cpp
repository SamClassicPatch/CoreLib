/* Copyright (c) 2022 Dreamy Cecil
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

// List available function patches
static void ListFuncPatches(void) {
  if (GetPatchAPI()->aPatches.Count() == 0) {
    CPrintF("No function patches available!\n");
    return;
  }

  CPrintF("Available function patches:\n");
  
  for (INDEX iPatch = 0; iPatch < GetPatchAPI()->aPatches.Count(); iPatch++) {
    const SFuncPatch &fpPatch = GetPatchAPI()->aPatches[iPatch];

    // Mark as enabled or not and indent the index
    const char *strPatched = (fpPatch.pPatch->IsPatched() ? " [^c00ff00ON^r]" : "[^cff0000OFF^r]");
    const INDEX ctIdent = ClampDn(2 - INDEX(log10((FLOAT)iPatch)), (INDEX)0);

    CPrintF("%s %*s%d - %s\n", strPatched, ctIdent, "", iPatch, fpPatch.strName);
  }
};

// Enable specific function patch
static void EnableFuncPatch(INDEX iPatch) {
  iPatch = Clamp(iPatch, (INDEX)0, INDEX(GetPatchAPI()->aPatches.Count() - 1));

  const CTString &strPatch = GetPatchAPI()->aPatches[iPatch].strName;
  BOOL bPatched = GetPatchAPI()->EnablePatch(iPatch);

  if (bPatched) {
    CPrintF("Successfully set '%s' function patch!\n", strPatch);
  } else {
    CPrintF("Cannot set '%s' function patch!\n", strPatch);
  }
};

// Disable specific function patch
static void DisableFuncPatch(INDEX iPatch) {
  iPatch = Clamp(iPatch, (INDEX)0, INDEX(GetPatchAPI()->aPatches.Count() - 1));

  const CTString &strPatch = GetPatchAPI()->aPatches[iPatch].strName;
  GetPatchAPI()->DisablePatch(iPatch);

  CPrintF("Successfully removed '%s' function patch!\n", strPatch);
};

// Constructor
CPatchAPI::CPatchAPI() {
  // Commands for manually toggling function patches
  _pShell->DeclareSymbol("void ListPatches(void);",   &ListFuncPatches);
  _pShell->DeclareSymbol("void EnablePatch(INDEX);",  &EnableFuncPatch);
  _pShell->DeclareSymbol("void DisablePatch(INDEX);", &DisableFuncPatch);
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

// Find function patch index by its name
INDEX CPatchAPI::FindPatch(const CTString &strName) {
  const ULONG ulNameHash = strName.GetHash();
  const INDEX ctPatches = aPatches.Count();

  for (INDEX iPatch = 0; iPatch < ctPatches; iPatch++) {
    const SFuncPatch &fpPatch = aPatches[iPatch];

    // Matching name hash
    if (fpPatch.ulHash == ulNameHash) {
      // Found function patch
      return iPatch;
    }
  }

  // Not found
  return -1;
};
