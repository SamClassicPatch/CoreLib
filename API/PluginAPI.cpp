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

// List loaded plugin modules
static void ListPlugins(void) {
  CPluginStock *pStock = GetPluginAPI()->pPluginStock;

  if (pStock->GetTotalCount() == 0) {
    CPrintF("No plugins have been loaded!\n");
    return;
  }

  CPrintF("Loaded plugins:\n");
  
  for (INDEX iPlugin = 0; iPlugin < pStock->GetTotalCount(); iPlugin++) {
    CPluginModule *pPlugin = pStock->st_ctObjects.Pointer(iPlugin);

    // Indent the index
    const INDEX ctIdent = ClampDn(2 - INDEX(log10((FLOAT)iPlugin)), (INDEX)0);

    CPrintF("%*s%d - %s\n", ctIdent, "", iPlugin, pPlugin->GetName().str_String);
  }
};

// Constructor
CPluginAPI::CPluginAPI() {
  // Create stock of plugin modules
  pPluginStock = new CPluginStock;

  // List loaded plugin modules
  _pShell->DeclareSymbol("user void ListPlugins(void);", &ListPlugins);
};

// Obtain pointer to a plugin module of specific utility types
CPluginModule *CPluginAPI::ObtainPlugin_t(const CTFileName &fnmModule, ULONG ulUtilityFlags)
{
  // Obtain info from the plugin
  CPluginModule *pPlugin = pPluginStock->Obtain_t(fnmModule);
  const CPluginAPI::PluginInfo &info = pPlugin->GetInfo();

  const CTString strFileName = fnmModule.FileName() + fnmModule.FileExt();

  // Check used API version and utility types
  BOOL bVersion = (info.apiVer == CORE_API_VERSION);
  BOOL bUtility = (info.ulFlags & ulUtilityFlags) != 0;

  // Warn about load cancelling
  if (!bVersion) {
    CPrintF("'%s' load cancelled: Wrong API version (%u)\n", strFileName, info.apiVer);
  } else if (!bUtility) {
    CPrintF("'%s' load cancelled: Mismatching utility flags (0x%X)\n", strFileName, info.ulFlags);
  }
  
  // Cannot load some plugin
  if (!bVersion || !bUtility) {
    // Release it
    pPluginStock->ForceRelease(pPlugin);

    // No plugin has been loaded
    return NULL;
  }

  // Initialize the plugin and return it
  pPlugin->Initialize();

  return pPlugin;
};

// Load plugin module without safety checks
CPluginModule *CPluginAPI::LoadPlugin_t(const CTFileName &fnmModule)
{
  CPluginModule *pPlugin = pPluginStock->Obtain_t(fnmModule);
  pPlugin->Initialize();

  return pPlugin;
};

// Get loaded plugins
CDynamicContainer<CPluginModule> &CPluginAPI::GetPlugins(void) {
  return pPluginStock->st_ctObjects;
};

// Register new symbol from the plugin and return a pointer to its value
void *CPluginAPI::RegisterSymbol(const char *strSymbolName, CTString strDeclaration)
{
  // Get symbol if it already exists
  CShellSymbol *pss = _pShell->GetSymbol(strSymbolName, TRUE);
  
  // Return value of the existing symbol
  if (pss != NULL) {
    return pss->ss_pvValue;
  }

  // Insert symbol name to the declaration
  strDeclaration.PrintF(strDeclaration, strSymbolName);

  // Allocate new symbol and declare it
  void *pvNewValue = &aSymbols.Push();
  _pShell->DeclareSymbol(strDeclaration, pvNewValue);

  return pvNewValue;
};
