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
    CPutString(TRANS("No plugins have been loaded!\n"));
    return;
  }

  CPutString(TRANS("^cffffffLoaded plugins:\n"));
  
  for (INDEX iPlugin = 0; iPlugin < pStock->GetTotalCount(); iPlugin++) {
    CPluginModule *pPlugin = pStock->st_ctObjects.Pointer(iPlugin);

    // Light blue - ON; Red - OFF
    CPutString(pPlugin->IsInitialized() ? "^c00ffff" : "^cff0000");

    // Indent the index
    const INDEX ctIdent = ClampDn(2 - INDEX(log10((FLOAT)iPlugin)), (INDEX)0);

    CPrintF("%*s%d - %s\n", ctIdent, "", iPlugin, pPlugin->GetName().str_String);

    // Print metadata
    const CPluginAPI::PluginInfo &info = pPlugin->GetInfo();

    CPrintF(TRANS("  Name: %s\n"), info.strName);
    CPrintF(TRANS("  Author: %s\n"), info.strAuthor);
    CPrintF(TRANS("  Version: %s\n"), CCoreAPI::MakeVersionString(info.ulVersion));
    CPrintF(TRANS("  Description: %s\n"), info.strDescription);
    CPutString("---\n");
  }
};

// Enable specific plugin
static void EnablePlugin(INDEX iPlugin) {
  CPluginStock *pStock = GetPluginAPI()->pPluginStock;

  if (iPlugin < 0 || iPlugin >= pStock->GetTotalCount()) {
    CPutString(TRANS("Invalid plugin index!\n"));
    return;
  }

  CPluginModule *pPlugin = pStock->st_ctObjects.Pointer(iPlugin);
  const char *strPlugin = pPlugin->GetName().str_String;

  if (pPlugin->IsInitialized()) {
    CPrintF(TRANS("'%s' plugin is already enabled!\n"), strPlugin);
    return;
  }

  pPlugin->Initialize();

  if (pPlugin->IsInitialized()) {
    CPrintF(TRANS("Successfully enabled '%s' plugin!\n"), strPlugin);
  } else {
    CPrintF(TRANS("Cannot enable '%s' plugin!\n"), strPlugin);
  }
};

// Disable specific plugin
static void DisablePlugin(INDEX iPlugin) {
  CPluginStock *pStock = GetPluginAPI()->pPluginStock;

  if (iPlugin < 0 || iPlugin >= pStock->GetTotalCount()) {
    CPutString(TRANS("Invalid plugin index!\n"));
    return;
  }

  CPluginModule *pPlugin = pStock->st_ctObjects.Pointer(iPlugin);
  const char *strPlugin = pPlugin->GetName().str_String;

  if (!pPlugin->IsInitialized()) {
    CPrintF(TRANS("'%s' plugin is already disabled!\n"), strPlugin);
    return;
  }

  pPlugin->Deactivate();

  CPrintF(TRANS("Successfully disabled '%s' plugin!\n"), strPlugin);
};

// Constructor
CPluginAPI::CPluginAPI() {
  // Create stock of plugin modules
  pPluginStock = new CPluginStock;

  // Commands for manually toggling plugins
  _pShell->DeclareSymbol("user void ListPlugins(void);",    &ListPlugins);
  _pShell->DeclareSymbol("user void EnablePlugin(INDEX);",  &EnablePlugin);
  _pShell->DeclareSymbol("user void DisablePlugin(INDEX);", &DisablePlugin);
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

  // Warn about loading cancelling
  if (!bVersion) {
    CPrintF(TRANS("'%s' loading cancelled: Wrong API version (%u)\n"), strFileName, info.apiVer);
  } else if (!bUtility) {
    CPrintF(TRANS("'%s' loading cancelled: Mismatching utility flags (0x%X)\n"), strFileName, info.ulFlags);
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

// Add plugin's function patch into its patch container
void CPluginAPI::AddNewPatch(CPatch *pPatch) {
  extern CPluginModule *_pInitializingPlugin;
  ASSERT(_pInitializingPlugin != NULL);

  if (!_pInitializingPlugin->_cPatches.IsMember(pPatch)) {
    _pInitializingPlugin->_cPatches.Add(pPatch);
  }
};
