/* Copyright (c) 2022-2024 Dreamy Cecil
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

// Include global hooks and variable data
#include <CoreLib/API/CoreHooks.h>
#include <CoreLib/API/CoreVariables.h>

// Declare API submodules
class CPatchAPI;
class CGameAPI;
class CPluginAPI;
class CSteamAPI;

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

    // Seasonal events triggered at specific dates
    enum ESpecialEvent {
      SPEV_NONE = 0,
      SPEV_VALENTINE, // Feb 10 - Feb 18
      SPEV_BD_PARTY,  // Mar 19 - Mar 23 (Sam); Jun 20 - Jun 24 (Classics Patch)
      SPEV_HALLOWEEN, // Oct 01 - Oct 31
      SPEV_CHRISTMAS, // Dec 15 - Jan 15
    };

    // Easily accessible properties from the global config
    struct SConfigProps {
      #define CONFIG_DEFAULT_DIR_TFE "..\\Serious Sam Classic The First Encounter\\"
      #define CONFIG_DEFAULT_DIR_SSR "..\\Serious Sam Revolution\\"
      #define CONFIG_DEFAULT_DIR_WORKSHOP "..\\..\\workshop\\content\\227780\\"

      // Game mounting
      BOOL bMountTFE;
      CTString strTFEDir;
      BOOL bMountSSR;
      CTString strSSRDir;
      BOOL bMountSSRWorkshop;
      CTString strSSRWorkshop;

      // Common options
      BOOL bCustomMod;
      BOOL bDebugPatcher;
      BOOL bDPIAware;
      BOOL bExtendedFileSystem;
      BOOL bFullAppIntegration;

      // Steam API
      BOOL bSteamEnable;     // Initialize and use Steamworks API
      BOOL bSteamForServers; // Initialize for dedicated servers
      BOOL bSteamForTools;   // Initialize for tool applications

      SConfigProps(); // Constructor that sets default property states
      void Load(void); // Load properties from the config
      void Save(void); // Save properties into the config
    };

  public:
    static const ULONG ulCoreVersion; // Release version
    static CCoreVariables varData; // Variable data

    // API submodules
    CPatchAPI &apiPatches;
    CGameAPI &apiGame;
    CPluginAPI &apiPlugins;
    CSteamAPI &apiSteam;

  public:
    // Constructor
    CCoreAPI();

    // Get running application type after initializing the core
    // For modules that aren't utilizing Core library directly (e.g. plugins)
    virtual EAppType GetAppType(void);

    // Get running application type before initializing the core
    static EAppType GetApplication(void);

    // Check if playing with a custom mod
    static BOOL IsCustomModActive(void);

    // Set custom mod state only once
    static void SetCustomMod(BOOL bState);

    // Get vanilla library extension after initializing the core
    // For modules that aren't utilizing Core library directly (e.g. plugins)
    virtual const CTString &GetModExt(void);

    // Get vanilla library extension before initializing the core
    static const CTString &GetVanillaExt(void);

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

    // Get global properties
    static SConfigProps &Props(void);

    // Get current seasonal event
    virtual ESpecialEvent GetCurrentEvent(void);

  public:

    // Retrieve version of the patch before initializing the core
    static inline CTString GetCoreVersion(void) {
      CTString strVer = MakeVersionString(ulCoreVersion);

      // Append a dev build tag
      #if CORE_DEV_BUILD
        strVer += "-dev";
      #endif

      return strVer;
    };

    // Retrieve version of the patch after initializing the core
    virtual CTString GetVersion(void) {
      return GetCoreVersion();
    };

    // Toggle vanilla query manager
    void DisableGameSpy(void);

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
    const CTFileName fnmLib = IDir::AppBin() + "ClassicsCore.dll";
    HMODULE hLib = GetModuleHandleA((IDir::AppPath() + fnmLib).str_String);

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

// Get core variable data
inline CCoreVariables &CoreVarData(void) {
  return CCoreAPI::varData;
};

#if CLASSICSPATCH_GAMEPLAY_EXT

// Get gameplay extensions
inline CCoreVariables::GameplayExt &CoreGEX(void) {
  return CoreVarData().gex;
};

#endif // CLASSICSPATCH_GAMEPLAY_EXT

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

// Get Steam API module
inline CSteamAPI *GetSteamAPI(void) {
  return &GetAPI()->apiSteam;
};

// Define API submodules
#include "PatchAPI.h"
#include "GameAPI.h"
#include "PluginAPI.h"
#include "SteamAPI.h"

#endif
