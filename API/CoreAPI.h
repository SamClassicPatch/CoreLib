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

// Include Serious Engine and other common components
#include <CoreLib/Base/CommonCore.h>
#include <CoreLib/API/ApiConfig.h>

// Include global hooks
#include <CoreLib/API/CoreHooks.h>

// Declare API submodules
class CPatchAPI;
class CGameAPI;
class CPluginAPI;

// Core API class (inherit hooks for backwards compatibility)
class CORE_API CCoreAPI : public ICoreHooks {
  public:
    // Types of available applications on Serious Engine
    enum EAppType {
      APP_UNKNOWN = 0, // Unspecified application
      APP_GAME,        // Game Executable
      APP_SERVER,      // Dedicated Server
      APP_EDITOR,      // Serious Editor
      APP_MODELER,     // Serious Modeler or Serious SKA Studio
    };

    // Easily accessible properties from the global config
    struct SConfigProps {
      BOOL bCustomMod;
      BOOL bDebugPatcher;
      BOOL bDPIAware;
      BOOL bExtendedFileSystem;
      CTString strTFEDir;

      SConfigProps(); // Constructor that sets default property states
      void Load(void); // Load properties from the config
      void Save(void); // Save properties into the config
    };

  public:
    static const ULONG ulCoreVersion; // Release version
    static EAppType eAppType; // Running application type
    static BOOL bCustomMod; // Using custom mod from the patch
    static CTString strVanillaExt; // Library extension of the vanilla game

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

    // Check if playing with a custom mod
    static inline BOOL IsCustomModActive(void) {
      return bCustomMod;
    };

    // Get vanilla library extension after initializing the core
    // For modules that aren't utilizing Core library directly (e.g. plugins)
    virtual CTString GetModExt(void) {
      return strVanillaExt;
    };

    // Get vanilla library extension before initializing the core
    static inline CTString GetVanillaExt(void) {
      return strVanillaExt;
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

    // Get filename of some library (e.g. "Game" + _strModExt for standard Game library)
    static inline CTString GetLibFile(const CTString &strLibName, const CTString &strLibExt = ".dll") {
      // Construct library filename
      #ifdef NDEBUG
        return strLibName + strLibExt; // Release library
      #else
        return "Debug\\" + strLibName + "D" + strLibExt; // Debug library
      #endif
    };

    // Get full path relative to the game to some library (mod Bin -> patch Bin -> vanilla Bin)
    static CTString FullLibPath(const CTString &strLibName, const CTString &strLibExt = ".dll");

    // Get relative path to the game executable
    static const CTFileName &AppExe(void);

    // Get relative path to the Bin directory (folder name)
    static inline CTFileName AppBin(void) {
      return AppExe().FileDir();
    };

    // Get relative path to the mod's Bin directory (folder name)
    static inline CTFileName AppModBin(void) {
      // Use game's or mod's Bin directory
      return (_fnmMod == "") ? AppBin() : CTString("Bin\\");
    };

    // Get absolute path to the game directory
    static const CTFileName &AppPath(void);

    // Get global properties
    static SConfigProps &Props(void);

  public:

    // Retrieve version of the patch
    virtual CTString GetVersion(void) {
      CTString strVer = MakeVersionString(ulCoreVersion);

      // Append a dev build tag
      #if CORE_DEV_BUILD
        strVer += "-dev";
      #endif

      return strVer;
    };

    // Disable GameSpy usage
    void DisableGameSpy(void);

    // Create a series of directories within the game folder
    virtual void CreateDir(const CTString &strPath);

    // Load dynamic link library and throw exception upon any error
    static HINSTANCE LoadLib(const char *strFileName);

    // Load Game library as a plugin
    void LoadGameLib(const CTString &strSettingsFile);

    // Load GameGUI library and return pointer to GameGUI_interface
    void *LoadGameGuiLib(const CTString &strSettingsFile);

    // Set metadata for the Game plugin
    class CPluginModule *LoadGamePlugin(void);

    // Set metadata for the GameGUI plugin
    class CPluginModule *LoadGameGuiPlugin(void);

    // Load all user plugins of specific utility types
    void LoadPlugins(ULONG ulUtilityFlags);

    // Release all user plugins of specific utility types
    void ReleasePlugins(ULONG ulUtilityFlags);

    // Reinitialize console in the engine
    void ReinitConsole(INDEX ctCharsPerLine, INDEX ctLines);
};

// This variable can be used to access API of the Classics patch.
// It needs to be defined separately for outside projects.
// See documentation on plugins & modding for more info.
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
    const CTFileName fnmLib = CCoreAPI::AppBin() + "ClassicsCore.dll";
    HMODULE hLib = GetModuleHandleA((CCoreAPI::AppPath() + fnmLib).str_String);

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
