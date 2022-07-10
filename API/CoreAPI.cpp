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

// Define external core API
CCoreAPI *_pCoreAPI = NULL;

// List available function patches
static void ListFuncPatches(void) {
  if (GetPatchAPI()->aPatches.Count() == 0) {
    CPrintF("No function patches available!\n");
    return;
  }

  CPrintF("Available function patches:\n");
  
  for (INDEX iPatch = 0; iPatch < GetPatchAPI()->aPatches.Count(); iPatch++) {
    const SFuncPatch &fpPatch = GetPatchAPI()->aPatches[iPatch];

    // Mark as enabled or not and indent the index
    const char *strPatched = (fpPatch.pPatch->patched() ? " [^c00ff00ON^r]" : "[^cff0000OFF^r]");
    const INDEX ctIdent = ClampDn(2 - INDEX(log10((FLOAT)iPatch)), (INDEX)0);

    CPrintF("%s %*s%d - %s\n", strPatched, ctIdent, "", iPatch, fpPatch.strName);
  }
};

// Enable specific function patch
static void EnableFuncPatch(INDEX iPatch) {
  iPatch = Clamp(iPatch, (INDEX)0, INDEX(GetPatchAPI()->aPatches.Count() - 1));

  const CTString &strPatch = GetPatchAPI()->aPatches[iPatch].strName;
  BOOL bPatched = GetPatchAPI()->EnablePatch(iPatch);

  if (bPatched) {
    CPrintF("Successfully set '%s' function patch!\n", strPatch);
  } else {
    CPrintF("Cannot set '%s' function patch!\n", strPatch);
  }
};

// Disable specific function patch
static void DisableFuncPatch(INDEX iPatch) {
  iPatch = Clamp(iPatch, (INDEX)0, INDEX(GetPatchAPI()->aPatches.Count() - 1));

  const CTString &strPatch = GetPatchAPI()->aPatches[iPatch].strName;
  GetPatchAPI()->DisablePatch(iPatch);

  CPrintF("Successfully removed '%s' function patch!\n", strPatch);
};

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
CCoreAPI::CCoreAPI() :
  apiPatches(*new CPatchAPI), apiGame(*new CGameAPI), apiPlugins(*new CPluginAPI)
{
  // Add core API to symbols
  CShellSymbol &ssNew = *_pShell->sh_assSymbols.New(1);

  ssNew.ss_strName = "CoreAPI"; // Access by this symbol name
  ssNew.ss_istType = 0; // Should be '_shell_istUndeclared'
  ssNew.ss_pvValue = this; // Pointer to self
  ssNew.ss_ulFlags = SSF_CONSTANT; // Unchangable
  ssNew.ss_pPreFunc = NULL; // Unused
  ssNew.ss_pPostFunc = NULL; // Unused

  ulVersion = MakeVersion(1, 2, 0);

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

// Disable GameSpy usage
void CCoreAPI::DisableGameSpy(void) {
  static BOOL bDisabled = FALSE;

  if (bDisabled) return;

  // Get symbol for accessing GameSpy master server
  CShellSymbol *pssGameSpy = _pShell->GetSymbol("ser_bHeartbeatGameSpy", TRUE);

  if (pssGameSpy != NULL) {
    // Store last value
    INDEX *piValue = (INDEX *)pssGameSpy->ss_pvValue;
    static INDEX iDummyValue = *piValue;

    // Forcefully disable it
    *piValue = FALSE;

    // Make it inaccessible
    pssGameSpy->ss_pvValue = &iDummyValue;

    bDisabled = TRUE;
  }
};

// Load Game library as a plugin
void CCoreAPI::LoadGameLib(void) {
  // Already loaded
  if (_pGame != NULL) return;

  try {
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

    // Obtain Game library
    CPluginModule *pGameLib = GetPluginAPI()->LoadPlugin_t(strGameLib);
    CPrintF(TRANS("Loading game library '%s'...\n"), pGameLib->GetName());

    // Create Game class
    CGame *(*pGameCreateFunc)(void) = NULL;
    pGameLib->GetSymbol_t(&pGameCreateFunc, "GAME_Create");

    _pGame = pGameCreateFunc();

  } catch (char *strError) {
    FatalError("%s", strError);
  }

  // Initialize Game
  _pGame->Initialize(CTString("Data\\SeriousSam.gms"));

  // Hook default fields
  GetGameAPI()->HookFields();
};

// Load all user plugins of specific utility types
void CCoreAPI::LoadPlugins(ULONG ulUtilityFlags) {
  // List all library files
  CDynamicStackArray<CTFileName> afnmDir;
  MakeDirList(afnmDir, CTString("Bin\\Plugins\\"), "*.dll", DLI_RECURSIVE);

  CPrintF("--- Loading user plugins (flags: 0x%X) ---\n", ulUtilityFlags);

  // Load every plugin
  for (INDEX i = 0; i < afnmDir.Count(); i++)
  {
    CPrintF("  %d - %s\n", i + 1, afnmDir[i].str_String);

    try {
      // Try to load the plugin
      GetPluginAPI()->ObtainPlugin_t(afnmDir[i], ulUtilityFlags);

    } catch (char *strError) {
      // Plugin initialization failed
      MessageBoxA(NULL, strError, TRANS("Warning"), MB_OK|MB_ICONEXCLAMATION|MB_SETFOREGROUND|MB_TASKMODAL);
    }
  }

  CPrintF("--- Done! ---\n");
};

// Called every simulation tick
void CCoreAPI::OnTick(void)
{
  CDynamicContainer<CPluginModule> &cPlugins = GetPluginAPI()->GetPlugins();

  // Call step function for each plugin
  FOREACHINDYNAMICCONTAINER(cPlugins, CPluginModule, itPlugin) {
    itPlugin->OnStep();
  }
};

// Called every render frame
void CCoreAPI::OnFrame(CDrawPort *pdp)
{
  CDynamicContainer<CPluginModule> &cPlugins = GetPluginAPI()->GetPlugins();

  // Call draw function for each plugin
  FOREACHINDYNAMICCONTAINER(cPlugins, CPluginModule, itPlugin) {
    itPlugin->OnDraw(pdp);
  }
};
