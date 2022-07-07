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

// Define external patch API
CPatchAPI *_pPatchAPI = NULL;

// List available function patches
static void ListFuncPatches(void) {
  if (_pPatchAPI->aPatches.Count() == 0) {
    CPrintF("No function patches available!\n");
    return;
  }

  CPrintF("Available function patches:\n");
  
  for (INDEX iPatch = 0; iPatch < _pPatchAPI->aPatches.Count(); iPatch++) {
    const SFuncPatch &fpPatch = _pPatchAPI->aPatches[iPatch];

    // Mark as enabled or not and indent the index
    const char *strPatched = (fpPatch.pPatch->patched() ? " [^c00ff00ON^r]" : "[^cff0000OFF^r]");
    const INDEX ctIdent = ClampDn(2 - INDEX(log10((FLOAT)iPatch)), (INDEX)0);

    CPrintF("%s %*s%d - %s\n", strPatched, ctIdent, "", iPatch, fpPatch.strName);
  }
};

// Enable specific function patch
static void EnableFuncPatch(INDEX iPatch) {
  iPatch = Clamp(iPatch, (INDEX)0, INDEX(_pPatchAPI->aPatches.Count() - 1));

  const CTString &strPatch = _pPatchAPI->aPatches[iPatch].strName;
  BOOL bPatched = _pPatchAPI->EnablePatch(iPatch);

  if (bPatched) {
    CPrintF("Successfully set '%s' function patch!\n", strPatch);
  } else {
    CPrintF("Cannot set '%s' function patch!\n", strPatch);
  }
};

// Disable specific function patch
static void DisableFuncPatch(INDEX iPatch) {
  iPatch = Clamp(iPatch, (INDEX)0, INDEX(_pPatchAPI->aPatches.Count() - 1));

  const CTString &strPatch = _pPatchAPI->aPatches[iPatch].strName;
  _pPatchAPI->DisablePatch(iPatch);

  CPrintF("Successfully removed '%s' function patch!\n", strPatch);
};

// List loaded plugin modules
static void ListPlugins(void) {
  if (_pPatchAPI->pPluginStock->GetTotalCount() == 0) {
    CPrintF("No plugins have been loaded!\n");
    return;
  }

  CPrintF("Loaded plugins:\n");
  
  for (INDEX iPlugin = 0; iPlugin < _pPatchAPI->pPluginStock->GetTotalCount(); iPlugin++) {
    CPluginModule *pPlugin = _pPatchAPI->pPluginStock->st_ctObjects.Pointer(iPlugin);

    // Indent the index
    const INDEX ctIdent = ClampDn(2 - INDEX(log10((FLOAT)iPlugin)), (INDEX)0);

    CPrintF("%*s%d - %s\n", ctIdent, "", iPlugin, pPlugin->GetName().str_String);
  }
};

// Constructor
CPatchAPI::CPatchAPI() {
  // Add patch API to symbols
  CShellSymbol &ssNew = *_pShell->sh_assSymbols.New(1);

  ssNew.ss_strName = "PatchAPI"; // Access by this symbol name
  ssNew.ss_istType = 0; // Should be '_shell_istUndeclared'
  ssNew.ss_pvValue = this; // Pointer to self
  ssNew.ss_ulFlags = SSF_CONSTANT; // Unchangable
  ssNew.ss_pPreFunc = NULL; // Unused
  ssNew.ss_pPostFunc = NULL; // Unused

  strVersion = "1.1.2";

  // Create stock of plugin modules
  pPluginStock = new CPluginStock;

  // Output patcher actions
  if (FileExists(_fnmApplicationExe.FileDir() + "PatcherOutput")) {
    Patch_DebugOutput() = true;
  }

  // Commands for manually toggling function patches
  _pShell->DeclareSymbol("void ListPatches(void);",   &ListFuncPatches);
  _pShell->DeclareSymbol("void EnablePatch(INDEX);",  &EnableFuncPatch);
  _pShell->DeclareSymbol("void DisablePatch(INDEX);", &DisableFuncPatch);

  // List loaded plugin modules
  _pShell->DeclareSymbol("user void ListPlugins(void);", &ListPlugins);
};

// Enable specific function patch
BOOL CPatchAPI::EnablePatch(INDEX iPatch) {
  SFuncPatch &fpPatch = _pPatchAPI->aPatches[iPatch];
  fpPatch.pPatch->set_patch();

  return fpPatch.pPatch->ok();
};

// Disable specific function patch
void CPatchAPI::DisablePatch(INDEX iPatch) {
  SFuncPatch &fpPatch = _pPatchAPI->aPatches[iPatch];
  fpPatch.pPatch->remove_patch();
};

// Obtain pointer to a plugin module
CPluginModule *CPatchAPI::ObtainPlugin_t(const CTFileName &fnmModule)
{
  return pPluginStock->Obtain_t(fnmModule);
};
