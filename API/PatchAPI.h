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

#ifndef CECIL_INCL_API_H
#define CECIL_INCL_API_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// API submodules
#include "GameAPI.h"

// Declare certain classes | Which files to include to define classes
class CPatch;               // #include <CoreLib/Patcher/patcher.h>
class CPluginModule;        // #include <CoreLib/Modules/PluginModule.h>
class CPluginStock;         // #include <CoreLib/Modules/PluginStock.h>

// Pointer to a function patch under a hashed name
struct SFuncPatch {
  CTString strName; // Patch name
  ULONG ulHash; // Name hash

  CPatch *pPatch; // Pointer to the patch
  
  // Default constructor
  SFuncPatch() : strName(""), ulHash(0), pPatch(NULL)
  {
  };

  // Constructor from name and patch
  SFuncPatch(const CTString &strSetName, CPatch *pSetPatch) :
    strName(strSetName), pPatch(pSetPatch)
  {
    // Calculate name hash
    ulHash = strName.GetHash();
  };
};

// Patch API class
class CPatchAPI {
  public:
    CTString strVersion; // Patch version
    CStaticStackArray<SFuncPatch> aPatches; // Function patch storage
    CPluginStock *pPluginStock; // Stock of plugin modules

    // API submodules
    CGameAPI apiGame;

  // Only virtual and defined methods can be used outside the Classics patch
  public:
    // Constructor
    CPatchAPI();

    // Enable specific function patch
    virtual BOOL EnablePatch(INDEX iPatch);

    // Disable specific function patch
    virtual void DisablePatch(INDEX iPatch);

    // Obtain pointer to a plugin module
    virtual CPluginModule *ObtainPlugin_t(const CTFileName &fnmModule);
};

// This variable can be used to access API of the EXE patch.
// It needs to be defined separately for outside projects. Visit for more info:
// https://github.com/SamClassicPatch/GameExecutable/wiki/Mod-support#api-utilization
extern "C" __declspec(dllexport) CPatchAPI *_pPatchAPI;

// These methods should only be used outside the Classics patch project
#ifndef CORE_EXPORTS
  // Hook API pointer through the shell symbol
  inline BOOL HookSymbolAPI(void) {
    CShellSymbol *pssAPI = _pShell->GetSymbol("PatchAPI", TRUE);

    if (pssAPI != NULL) {
      _pPatchAPI = (CPatchAPI *)pssAPI->ss_pvValue;
      return TRUE;
    }

    return FALSE;
  };

  // Hook API pointer through the executable's module handle
  inline BOOL HookExecutableAPI(void) {
    // Get instance of the running executable
    const CTFileName fnmEXE = _fnmApplicationPath + _fnmApplicationExe;
    HMODULE pEXE = GetModuleHandleA(fnmEXE);

    if (pEXE != NULL) {
      // Get API pointer from the executable module
      void *pPointerToAPI = GetProcAddress(pEXE, "_pPatchAPI");

      if (pPointerToAPI != NULL) {
        _pPatchAPI = *(CPatchAPI **)pPointerToAPI;
      }
    }

    return (_pPatchAPI != NULL);
  };
#endif

// Get Game API module
inline CGameAPI *GetGameAPI(void) {
  return &_pPatchAPI->apiGame;
};

#endif
