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

#include "StdH.h"

#include "PluginModule.h"

// Current plugin in the process of initialization
CPluginModule *_pInitializingPlugin = NULL;

// Constructor
CPluginModule::CPluginModule() {
  ResetFields();
};

// Destructor
CPluginModule::~CPluginModule() {
  Clear();
};

// Module initialization
void CPluginModule::Initialize(void) {
  if (IsInitialized()) return;

  // Set to the current plugin
  CPluginModule *pLastPlugin = _pInitializingPlugin;
  _pInitializingPlugin = this;

  // Start the plugin
  if (pm_pStartupFunc != NULL) {
    pm_pStartupFunc(pm_props);
  }

  // Restore last plugin
  _pInitializingPlugin = pLastPlugin;

  pm_bInitialized = TRUE;
};

// Module deactivation
void CPluginModule::Deactivate(void) {
  if (!IsInitialized()) return;

  // Destroy all patches
  for (INDEX i = 0; i < pm_aPatches.Count(); i++) {
    DestroyPatch(pm_aPatches[i]);
  }

  pm_aPatches.PopAll();

  // Stop the plugin
  if (pm_pShutdownFunc != NULL) {
    pm_pShutdownFunc(pm_props);
  }

  pm_bInitialized = FALSE;
};

// Reset class fields
void CPluginModule::ResetFields(void) {
  pm_hLibrary = NULL;
  pm_bInitialized = FALSE;

  pm_pGetInfoFunc = NULL;
  pm_pStartupFunc = NULL;
  pm_pShutdownFunc = NULL;

  pm_props.Clear();
};

// Add new function patch on startup
void CPluginModule::AddPatch(HFuncPatch hPatch)
{
  // Check if it already exists
  for (INDEX i = 0; i < pm_aPatches.Count(); i++) {
    if (pm_aPatches[i] == hPatch) return;
  }

  pm_aPatches.Add(hPatch);
};

// Get specific symbol from the module
void *CPluginModule::GetSymbol_t(const char *strSymbolName) {
  // No module
  if (GetHandle() == NULL) {
    ThrowF_t(TRANS("Plugin module has not been loaded yet!"));
  }

  void *pSymbol = GetProcAddress(GetHandle(), strSymbolName);

  // No symbol
  if (pSymbol == NULL) {
    ThrowF_t(TRANS("Cannot find '%s' symbol in '%s'!"), strSymbolName, GetName());
  }

  return pSymbol;
};

// Module cleanup
void CPluginModule::Clear(void) {
  // Release DLL
  if (pm_hLibrary != NULL) {
    Deactivate();
    FreeLibrary(pm_hLibrary);
  }

  // Reset class
  ResetFields();
  CSerial::Clear();
};

// Load plugin module (override non-virtual CSerial::Load_t)
void CPluginModule::Load_t(const CTFileName &fnmDLL)
{
  ASSERT(!IsUsed());

  // Mark that just changed
  MarkChanged();

  // Remember filename
  ser_FileName = fnmDLL;

  // Load plugin's configuration file
  const CTString strConfig = IDir::AppModBin() + "Plugins\\" + fnmDLL.FileName() + ".ini";

  try {
    pm_props.Load_t(strConfig, TRUE);

  } catch (char *strError) {
    (void)strError;
  }

  // Load library from file
  CTFileName fnmExpanded;
  ExpandFilePath(EFP_READ | EFP_NOZIPS, fnmDLL, fnmExpanded);

  // Load dll
  pm_hLibrary = ILib::LoadLib(fnmExpanded);

  // Main plugin methods
  pm_pGetInfoFunc  = (CInfoFunc)  GetProcAddress(GetHandle(), CLASSICSPATCH_STRINGIFY(PLUGINMODULEMETHOD_GETINFO));
  pm_pStartupFunc  = (CModuleFunc)GetProcAddress(GetHandle(), CLASSICSPATCH_STRINGIFY(PLUGINMODULEMETHOD_STARTUP));
  pm_pShutdownFunc = (CModuleFunc)GetProcAddress(GetHandle(), CLASSICSPATCH_STRINGIFY(PLUGINMODULEMETHOD_SHUTDOWN));

  // Try to get information about the plugin immediately
  if (pm_pGetInfoFunc != NULL) {
    pm_pGetInfoFunc(&pm_info);
  }
};
