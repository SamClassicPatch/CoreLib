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

#ifndef CECIL_INCL_PATCHAPI_H
#define CECIL_INCL_PATCHAPI_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include <CoreLib/Patcher/FuncPatching.h>

// Pointer to a function patch under a hashed name
struct SFuncPatch {
  CTString strName; // Patch name
  ULONG ulHash; // Name hash
  CPatch *pPatch; // Pointer to the patch

  // Default constructor
  SFuncPatch() {
    Clear();
  };

  // Constructor from name and patch
  SFuncPatch(const CTString &strSetName, CPatch *pSetPatch) :
    strName(strSetName), pPatch(pSetPatch)
  {
    // Calculate name hash
    ulHash = strName.GetHash();
  };

  // Clear the function patch
  inline void Clear(void) {
    strName = "";
    ulHash = 0;
    pPatch = NULL;
  };
};

// API for handling function patches
class CORE_API CPatchAPI {
  public:
    static HINSTANCE hEngine; // Engine library handle

    static CTFileName fnmEntities; // Entities library path
    static HINSTANCE hEntities; // Entities library handle

    CDynamicStackArray<SFuncPatch> aPatches; // Function patch storage

  public:
    // Constructor
    CPatchAPI();

    // Externally create a new patch class
    virtual CPatch *CreatePatch(BOOL bSetForever);

    // Enable specific function patch
    virtual BOOL EnablePatch(INDEX iPatch);

    // Disable specific function patch
    virtual void DisablePatch(INDEX iPatch);

    // Get function patch index by its name or patch pointer
    virtual INDEX GetPatchIndex(const CTString &strName, CPatch *pPatch);

    // Find function patch by its name or patch pointer
    virtual SFuncPatch *FindFuncPatch(const CTString &strName, CPatch *pPatch);

  public:

    // Retrieve address from the engine by a symbol name
    virtual void *GetEngineSymbol(const char *strSymbol);

    // Retrieve address from the entities by a symbol name
    virtual void *GetEntitiesSymbol(const char *strSymbol);

    // Get path to Entities library
    virtual CTFileName GetEntitiesPath(void) {
      return fnmEntities;
    };

    // Check if Entities library is from a mod
    virtual BOOL IsEntitiesModded(void) {
      return GetEntitiesPath().HasPrefix(CCoreAPI::AppPath() + _fnmMod);
    };
};

#endif
