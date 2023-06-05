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

#ifndef CECIL_INCL_COREAPI_H
#define CECIL_INCL_COREAPI_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// Include Serious Engine
#include <Engine/Engine.h>

// Classics Patch configuration and game-specific definitions
#include <CoreLib/Config.h>
#include <CoreLib/GameSpecific.h>

#include <CoreLib/Base/ConfigReader.h>

// Useful types
typedef CStaticStackArray<CTString> CStringStack; // Expandable array of strings
typedef CDynamicStackArray<CTFileName> CFileList; // Listed files/paths

// Next argument in the symbol function call
#define NEXT_ARG(Type) (*((Type *&)pArgs)++)

// Translate a string that has already been translated in vanilla localizations
#define LOCALIZE(ConstString) ((char *)TranslateConst(ConstString, 0))

// Define empty API if not utilizing Core directly
#ifndef CORE_API
  #define CORE_API
  #define CORE_POINTER_API

#else
  #define CORE_POINTER_API "C" CORE_API
#endif

// Current API version
#define CORE_API_VERSION 4

// Current Classics Patch version
#define CORE_PATCH_VERSION CCoreAPI::MakeVersion(1, 5, 1)

// Classics Patch configuration file
#define CORE_CONFIG_FILE CTString("Data\\ClassicsPatch\\Config.ini")

// Declare API submodules
class CPatchAPI;
class CGameAPI;
class CPluginAPI;

// Core API class
class CORE_API CCoreAPI {
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
    static IConfig::CProperties aProps; // Patch configuration

    ULONG ulVersion; // Release version

    // API submodules
    CPatchAPI &apiPatches;
    CGameAPI &apiGame;
    CPluginAPI &apiPlugins;

  public:
    // Constructor
    CCoreAPI();

    // Get running application type after initializing the core
    // For modules that aren't utilizing Core library directly (e.g. plugins)
    virtual EAppType GetAppType(void) {
      return eAppType;
    };

    // Get running application type before initializing the core
    static inline EAppType GetApplication(void) {
      return eAppType;
    };

    // Setup the core before initializing it
    static void Setup(EAppType eSetType);

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
      strVersion.PrintF((ubPatch > 0) ? "%u.%u.%u" : "%u.%u", ubRelease, ubUpdate, ubPatch);

      return strVersion;
    };

    // Get path to the Game library
    static inline CTString GetGameLibPath(void) {
      // Construct Game library name for different games
      CTString strGameLib = _fnmApplicationExe.FileDir() + "Game";

      // Append mod extension for TSE
      #if SE1_GAME != SS_TFE
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

    // Get absolute path to the game directory
    static const CTFileName &GetAppPath(void);

    // Set value to config property
    static void SetPropValue(const CTString &strProperty, const CTString &strValue);

    // Get value from config property
    static CTString GetPropValue(const CTString &strProperty);

  public:

    // Retrieve version of the patch
    virtual CTString GetVersion(void) {
      return MakeVersionString(ulVersion);
    };

    // Disable GameSpy usage
    void DisableGameSpy(void);

    // Create a series of directories within the game folder
    virtual void CreateDir(const CTString &strPath);

    // Load Game library as a plugin
    void LoadGameLib(const CTString &strSettingsFile);

    // Set metadata for the Game plugin
    class CPluginModule *LoadGamePlugin(void);

    // Load all user plugins of specific utility types
    void LoadPlugins(ULONG ulUtilityFlags);

    // Release all user plugins of specific utility types
    void ReleasePlugins(ULONG ulUtilityFlags);

    // Reinitialize console in the engine
    void ReinitConsole(INDEX ctCharsPerLine, INDEX ctLines);

  // Global hooks
  public:

    // Called after starting world simulation
    virtual void OnGameStart(void);

    // Called before stopping world simulation
    virtual void OnGameStop(void);

    // Called after saving the game
    virtual void OnGameSave(const CTFileName &fnmSave);

    // Called after loading a saved game
    virtual void OnGameLoad(const CTFileName &fnmSave);

    // Called after finishing reading the world file
    virtual void OnWorldLoad(CWorld *pwo, const CTFileName &fnmWorld);

    // Called every simulation tick
    virtual void OnTick(void);

    // Called before redrawing game view
    virtual void OnPreDraw(CDrawPort *pdp);

    // Called after redrawing game view
    virtual void OnPostDraw(CDrawPort *pdp);

    // Called after rendering the world
    virtual void OnRenderView(CWorld &wo, CEntity *penViewer, CAnyProjection3D &apr, CDrawPort *pdp);

    // Called every render frame
    virtual void OnFrame(CDrawPort *pdp);
};

// This variable can be used to access API of the Classics patch.
// It needs to be defined separately for outside projects. Visit for more info:
// https://github.com/SamClassicPatch/SuperProject/wiki/User-plugins-&-modding#api-utilization
extern CORE_POINTER_API CCoreAPI *_pCoreAPI;

// These methods should only be used outside the Classics patch project
#ifndef CORE_EXPORTS
  // Hook API pointer through the shell symbol
  inline BOOL HookSymbolAPI(void) {
    // Already hooked
    if (_pCoreAPI != NULL) return TRUE;

    CShellSymbol *pssAPI = _pShell->GetSymbol("CoreAPI", TRUE);

    if (pssAPI != NULL) {
      _pCoreAPI = (CCoreAPI *)pssAPI->ss_pvValue;
      return TRUE;
    }

    return FALSE;
  };

  // Hook API pointer through the library's module handle
  inline BOOL HookLibraryAPI(void) {
    // Already hooked
    if (_pCoreAPI != NULL) return TRUE;

    // Get instance of the Core library
    const CTFileName fnmLib = _fnmApplicationExe.FileDir() + "ClassicsCore.dll";
    HMODULE hLib = GetModuleHandleA(_fnmApplicationPath + fnmLib);

    if (hLib != NULL) {
      // Get API pointer from the library module
      void *pPointerToAPI = GetProcAddress(hLib, "_pCoreAPI");

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

// Define API submodules
#include "PatchAPI.h"
#include "GameAPI.h"
#include "PluginAPI.h"

#endif
