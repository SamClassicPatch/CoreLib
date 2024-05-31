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

#ifndef CECIL_INCL_PLUGINAPI_H
#define CECIL_INCL_PLUGINAPI_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// Chat command function type
#include <CoreLib/Networking/Modules/ChatCommands.h>

// Define plugin symbols
#include "Plugins/PluginSymbols.h"

// Declare certain classes | Which files to include to define classes
class CPluginModule;   // #include <CoreLib/Modules/PluginModule.h>
class CPluginStock;    // #include <CoreLib/Modules/PluginStock.h>
class IAbstractEvents; // Included at the end

// Pointers to plugin interfaces
typedef CDynamicContainer<IAbstractEvents> CPluginInterfaces;

// Various plugin flags
enum EPluginFlags {
  // Isn't loaded nor released by the plugin API
  PLF_MANUAL = 0, // Manually managed libraries (e.g. Game, GameGUI)
  PLF_UNSUITABLE = 0, // For aborting plugin loading by making it unsuitable

  // Utility types
  PLF_ENGINE = (1 << 0), // Internal functionality
  PLF_GAME   = (1 << 1), // In-game functionality
  PLF_SERVER = (1 << 2), // Server functionality
  PLF_EDITOR = (1 << 3), // Addons for Serious Editor
  PLF_TOOLS  = (1 << 4), // Addons for other tools

  // Multiple utility types
  PLF_GAMEPLAY_LOGIC = PLF_GAME | PLF_EDITOR, // Suitable for playing the game

  PLF_ALL = (1 << 5) - 1, // Suitable for everything
};

// Structure for information exchange between plugins and the core
class CPluginInfo {
  public:
    ULONG apiVer; // Version of the API used (always CORE_API_VERSION)
    ULONG ulFlags; // Plugin flags

    // Metadata
    CTString strName;        // Plugin name
    CTString strAuthor;      // Author name
    ULONG    ulVersion;      // Plugin version
    CTString strDescription; // Brief plugin description

    // Read-only data
    CIniConfig props; // Loaded plugin properties

  public:
    // Constructor
    CPluginInfo() : apiVer(0), ulFlags(PLF_UNSUITABLE), ulVersion(0),
      strAuthor("Unknown"), strName("No name"), strDescription("None")
    {
    };

    // Set utility flags using EPluginFlags (for module's Module_GetInfo method)
    inline void SetUtility(ULONG ulSetFlags) {
      apiVer = CORE_API_VERSION;
      ulFlags = ulSetFlags;
    };
};

// API for handling plugin modules
class CORE_API CPluginAPI {
  public:
    CPluginStock *pPluginStock; // Stock of plugin modules

    // Containers of plugin handlers
    CPluginInterfaces cProcessors;
    CPluginInterfaces cRenderers;
    CPluginInterfaces cNetworkEvents;
    CPluginInterfaces cPacketEvents;
    CPluginInterfaces cGameEvents;
    CPluginInterfaces cDemoEvents;
    CPluginInterfaces cWorldEvents;
    CPluginInterfaces cListenerEvents;
    CPluginInterfaces cTimerEvents;

  public:
    // Constructor
    CPluginAPI();
    
    // Obtain pointer to a plugin module of specific utility types
    virtual CPluginModule *ObtainPlugin_t(const CTFileName &fnmModule, ULONG ulUtilityFlags);

    // Load plugin module without safety checks
    virtual CPluginModule *LoadPlugin_t(const CTFileName &fnmModule);

    // Retrieve loaded plugins
    virtual CDynamicContainer<CPluginModule> &GetPlugins(void);

    // Add plugin's function patch into its patch container
    virtual void AddNewPatch(CPatch *pPatch);

    // Register a custom chat command
    virtual void RegisterChatCommand(const char *strName, SChatCommand::CCommandFunc pHandler);

    // Unregister a custom chat command
    virtual void UnregisterChatCommand(const char *strName);

    // Register a symbol from the plugin and set a pointer to the symbol to it
    static inline void RegisterSymbol(CPluginSymbol *pps, const char *strName)
    {
      // Get symbol if it already exists
      CShellSymbol *pss = _pShell->GetSymbol(strName, TRUE);

      // Set existing symbol
      if (pss != NULL) {
        pps->SetSymbol(pss);
        return;
      }

      // External declaration
      CTString strDeclaration;
      CTString strDeclFlags = "";

      // Symbol flags
      const ULONG ulFlags = pps->GetFlags();

      if (ulFlags & SSF_CONSTANT)   strDeclFlags += "const ";
      if (ulFlags & SSF_PERSISTENT) strDeclFlags += "persistent ";
      if (ulFlags & SSF_USER)       strDeclFlags += "user ";

      // Symbol type
      static const char *astrTypes[3] = { "INDEX", "FLOAT", "CTString" };
      const char *strType = astrTypes[pps->GetType()];

      // E.g. "extern persistent user INDEX iSymbol = (INDEX)0;"
      strDeclaration.PrintF("extern %s%s %s = (%s)%s;", strDeclFlags, strType, strName, strType, pps->GetDefaultValue());
      _pShell->Execute(strDeclaration);

      // Assign newly declared symbol
      pps->SetSymbol(_pShell->GetSymbol(strName, TRUE));
    };

    // Register a shell method from the plugin or replace an existing one
    static inline void RegisterMethod(BOOL bUser, const char *strType, const char *strName, const char *strArguments, void *pFunction)
    {
      // Get symbol if it already exists
      CShellSymbol *pss = _pShell->GetSymbol(strName, TRUE);

      // Replace method of the existing symbol
      if (pss != NULL) {
        pss->ss_pvValue = pFunction;
        return;
      }

      // Declaration
      CTString strDeclaration;
      const char *strDeclFlags = (bUser ? "user " : "");

      // E.g. "user INDEX GetValue(void);"
      strDeclaration.PrintF("%s%s %s(%s);", strDeclFlags, strType, strName, strArguments);
      _pShell->DeclareSymbol(strDeclaration, pFunction);
    };
};

// Define symbol registering method
void CPluginSymbol::Register(const char *strSymbolName, const char *strPreFunc = "", const char *strPostFunc = "")
{
  CPluginAPI::RegisterSymbol(this, strSymbolName);

  ASSERT(_pss != NULL);

  // Assign pre-function
  if (strcmp(strPreFunc, "") != 0) {
    CShellSymbol *pssPreFunc = _pShell->GetSymbol(strPreFunc, TRUE);
    ASSERT(pssPreFunc != NULL);

    if (pssPreFunc != NULL) {
      _pss->ss_pPreFunc = (BOOL (*)(void *))pssPreFunc->ss_pvValue;
    }
  }

  // Assign post-function
  if (strcmp(strPostFunc, "") != 0) {
    CShellSymbol *pssPostFunc = _pShell->GetSymbol(strPostFunc, TRUE);
    ASSERT(pssPostFunc != NULL);

    if (pssPostFunc != NULL) {
      _pss->ss_pPostFunc = (void (*)(void *))pssPostFunc->ss_pvValue;
    }
  }
};

// Define plugin event interfaces
#include "Plugins/PluginEvents.h"

// Exportable module symbols (use before defining plugin module methods)
#define MODULE_API extern "C" __declspec(dllexport)

#endif
