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

#ifndef CECIL_INCL_COREAPI_H
#define CECIL_INCL_COREAPI_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// Current API version
#define CORE_API_VERSION 1

// API submodules
#include "PatchAPI.h"
#include "GameAPI.h"
#include "PluginAPI.h"

// Core API class
class CCoreAPI {
  public:
    // Types of available applications on Serious Engine
    enum EAppType {
      APP_UNKNOWN = 0, // Unspecified application
      APP_GAME,        // Game Executable
      APP_SERVER,      // Dedicated Server
      APP_EDITOR,      // Serious Editor
      APP_MODELER,     // Serious Modeler or Serious SKA Studio
    };

  public:
    static EAppType eAppType; // Running application type
    ULONG ulVersion; // Release version

    // API submodules
    CPatchAPI &apiPatches;
    CGameAPI &apiGame;
    CPluginAPI &apiPlugins;

  public:
    // Constructor
    CCoreAPI();

    // Get running application type after initializing the core
    virtual EAppType GetApplication(void) {
      return eAppType;
    };

    // Set running application type before initializing the core
    static inline void SetApplication(EAppType eSetType) {
      eAppType = eSetType;
    };

    // Check if running a game
    virtual BOOL IsGameApp(void) {
      return (GetApplication() == APP_GAME);
    };

    // Check if running a server
    virtual BOOL IsServerApp(void) {
      return (GetApplication() == APP_SERVER);
    };

    // Check if running an editor
    virtual BOOL IsEditorApp(void) {
      return (GetApplication() == APP_EDITOR);
    };

    // Check if running a modeler
    virtual BOOL IsModelerApp(void) {
      return (GetApplication() == APP_MODELER);
    };

    // Construct version number
    static inline ULONG MakeVersion(UBYTE ubRelease, UBYTE ubUpdate, UBYTE ubPatch) {
      return (ubRelease << 16) | (ubUpdate << 8) | (ubPatch << 0);
    };

    // Retrieve version number as a string
    static inline CTString MakeVersionString(ULONG ulVersionNumber) {
      const UBYTE ubRelease = (ulVersionNumber >> 16) & 0xFF;
      const UBYTE ubUpdate  = (ulVersionNumber >>  8) & 0xFF;
      const UBYTE ubPatch   = (ulVersionNumber >>  0) & 0xFF;

      CTString strVersion;
      strVersion.PrintF("%u.%u.%u", ubRelease, ubUpdate, ubPatch);

      return strVersion;
    };

    // Get path to the Game library
    static inline CTString GetGameLibPath(void) {
      // Construct Game library name for different games
      CTString strGameLib = _fnmApplicationExe.FileDir() + "Game";

      // Append mod extension for TSE
      #ifndef SE1_TFE
        strGameLib += _strModExt;
      #endif

      // Debug library
      #ifdef _DEBUG
        strGameLib += "D";
      #endif

      // Library extension
      strGameLib += ".dll";

      return strGameLib;
    };

  public:

    // Retrieve version of the patch
    virtual CTString GetVersion(void) {
      return MakeVersionString(ulVersion);
    };

    // Disable GameSpy usage
    void DisableGameSpy(void);

    // Load Game library as a plugin
    void LoadGameLib(void);

    // Load all user plugins of specific utility types
    void LoadPlugins(ULONG ulUtilityFlags);

    // Release all user plugins of specific utility types
    void ReleasePlugins(ULONG ulUtilityFlags);

    // Called every simulation tick
    virtual void OnTick(void);

    // Called before redrawing game view
    virtual void OnPreDraw(CDrawPort *pdp);

    // Called after redrawing game view
    virtual void OnPostDraw(CDrawPort *pdp);

    // Called every render frame
    virtual void OnFrame(CDrawPort *pdp);
};

// This variable can be used to access API of the EXE patch.
// It needs to be defined separately for outside projects. Visit for more info:
// https://github.com/SamClassicPatch/GameExecutable/wiki/Mod-support#api-utilization
extern "C" __declspec(dllexport) CCoreAPI *_pCoreAPI;

// These methods should only be used outside the Classics patch project
#ifndef CORE_EXPORTS
  // Hook API pointer through the shell symbol
  inline BOOL HookSymbolAPI(void) {
    CShellSymbol *pssAPI = _pShell->GetSymbol("CoreAPI", TRUE);

    if (pssAPI != NULL) {
      _pCoreAPI = (CCoreAPI *)pssAPI->ss_pvValue;
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
      void *pPointerToAPI = GetProcAddress(pEXE, "_pCoreAPI");

      if (pPointerToAPI != NULL) {
        _pCoreAPI = *(CCoreAPI **)pPointerToAPI;
      }
    }

    return (_pCoreAPI != NULL);
  };
#endif

// Get core API
inline CCoreAPI *GetAPI(void) {
  return _pCoreAPI;
};

// Get patch API module
inline CPatchAPI *GetPatchAPI(void) {
  return &GetAPI()->apiPatches;
};

// Get Game API module
inline CGameAPI *GetGameAPI(void) {
  return &GetAPI()->apiGame;
};

// Get plugin API module
inline CPluginAPI *GetPluginAPI(void) {
  return &GetAPI()->apiPlugins;
};

#endif
