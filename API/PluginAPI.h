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

#ifndef CECIL_INCL_PLUGINAPI_H
#define CECIL_INCL_PLUGINAPI_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// Define plugin symbols
#include "PluginSymbols.h"

// Declare certain classes | Which files to include to define classes
class CPluginModule; // #include <CoreLib/Modules/PluginModule.h>
class CPluginStock;  // #include <CoreLib/Modules/PluginStock.h>

// API for handling plugin modules
class CPluginAPI {
  public:
    // Various plugin flags
    enum EPluginFlags {
      // Utility types
      PF_ENGINE = (1 << 0), // Internal functionality
      PF_GAME   = (1 << 1), // In-game functionality
      PF_SERVER = (1 << 2), // Server functionality
      PF_EDITOR = (1 << 3), // Addons for Serious Editor
      PF_TOOLS  = (1 << 4), // Addons for other tools

      // All utility types
      PF_UTILITY_ALL = (PF_ENGINE | PF_GAME | PF_SERVER | PF_EDITOR | PF_TOOLS),
    };

    // Structure for information exchange between plugins and the core
    struct PluginInfo {
      ULONG apiVer; // Version of the API used (always CORE_API_VERSION)
      ULONG ulFlags; // Plugin flags

      // Metadata
      CTString strName;        // Plugin name
      CTString strAuthor;      // Author name
      ULONG    ulVersion;      // Plugin version
      CTString strDescription; // Brief plugin description

      // Constructor
      PluginInfo() : apiVer(0), ulFlags(0), ulVersion(0),
        strAuthor("Unknown"), strName("No name"), strDescription("None")
      {
      };
    };

  public:
    CPluginStock *pPluginStock; // Stock of plugin modules

  public:
    // Constructor
    CPluginAPI();
    
    // Obtain pointer to a plugin module of specific utility types
    virtual CPluginModule *ObtainPlugin_t(const CTFileName &fnmModule, ULONG ulUtilityFlags);

    // Load plugin module without safety checks
    virtual CPluginModule *LoadPlugin_t(const CTFileName &fnmModule);

    // Retrieve loaded plugins
    virtual CDynamicContainer<CPluginModule> &GetPlugins(void);

    // Register a symbol from the plugin and write a pointer to its value to the pointer variable
    template<class Type>
    void RegisterSymbol(ULONG ulFlags, const char *strType, const char *strName, const char *strValue,
                        const char *strPreFunc, const char *strPostFunc, Type &pVariable)
    {
      // Get symbol if it already exists
      CShellSymbol *pss = _pShell->GetSymbol(strName, TRUE);

      // Return value of the existing symbol
      if (pss != NULL) {
        pVariable = (Type)pss->ss_pvValue;

        // Assign pre-function
        if (strPreFunc != "") {
          CShellSymbol *pssPreFunc = _pShell->GetSymbol(strPreFunc, TRUE);
          pss->ss_pPreFunc = (BOOL (*)(void *))pssPreFunc->ss_pvValue;
        }

        // Assign post-function
        if (strPostFunc != "") {
          CShellSymbol *pssPostFunc = _pShell->GetSymbol(strPostFunc, TRUE);
          pss->ss_pPostFunc = (void (*)(void *))pssPostFunc->ss_pvValue;
        }
        return;
      }

      // External declaration
      CTString strDeclaration;
      CTString strDeclFlags = "";

      // Symbol flags
      if (ulFlags & SSF_CONSTANT)   strDeclFlags += "const ";
      if (ulFlags & SSF_PERSISTENT) strDeclFlags += "persistent ";
      if (ulFlags & SSF_USER)       strDeclFlags += "user ";

      // E.g. "extern persistent user INDEX iSymbol = (INDEX)0;"
      strDeclaration.PrintF("extern %s%s %s = (%s)%s;", strDeclFlags, strType, strName, strType, strValue);
      _pShell->Execute(strDeclaration);

      // Get newly declared symbol
      pss = _pShell->GetSymbol(strName, TRUE);
      ASSERT(pss != NULL);

      // Assign its value to the variable
      pVariable = (Type)pss->ss_pvValue;

      // Assign pre-function
      if (strPreFunc != "") {
        CShellSymbol *pssPreFunc = _pShell->GetSymbol(strPreFunc, TRUE);
        pss->ss_pPreFunc = (BOOL (*)(void *))pssPreFunc->ss_pvValue;
      }

      // Assign post-function
      if (strPostFunc != "") {
        CShellSymbol *pssPostFunc = _pShell->GetSymbol(strPostFunc, TRUE);
        pss->ss_pPostFunc = (void (*)(void *))pssPostFunc->ss_pvValue;
      }
    };

    // Register a symbol from the plugin and set a pointer to the symbol to it
    static inline void RegisterSymbol(CPluginSymbol *pps, const char *strName, const char *strPreFunc, const char *strPostFunc)
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
    template<class Type>
    void RegisterMethod(BOOL bUser, const char *strType, const char *strName, const char *strArguments, Type pFunction)
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
void CPluginSymbol::Register(const char *strSymbolName, const char *strPreFunc, const char *strPostFunc)
{
  CPluginAPI::RegisterSymbol(this, strSymbolName, strPreFunc, strPostFunc);

  ASSERT(_pss != NULL);

  // Assign pre-function
  if (strPreFunc != "") {
    CShellSymbol *pssPreFunc = _pShell->GetSymbol(strPreFunc, TRUE);
    _pss->ss_pPreFunc = (BOOL (*)(void *))pssPreFunc->ss_pvValue;
  }

  // Assign post-function
  if (strPostFunc != "") {
    CShellSymbol *pssPostFunc = _pShell->GetSymbol(strPostFunc, TRUE);
    _pss->ss_pPostFunc = (void (*)(void *))pssPostFunc->ss_pvValue;
  }
};

#endif
