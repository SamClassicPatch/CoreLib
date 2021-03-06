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

// Define running application type
CCoreAPI::EAppType CCoreAPI::eAppType = CCoreAPI::APP_UNKNOWN;

// Constructor
CCoreAPI::CCoreAPI() :
  apiPatches(*new CPatchAPI), apiGame(*new CGameAPI), apiPlugins(*new CPluginAPI)
{
  // Make sure that this is the only API class
  ASSERT(_pCoreAPI == NULL);
  _pCoreAPI = this;

  // Add core API to symbols
  CShellSymbol &ssNew = *_pShell->sh_assSymbols.New(1);

  ssNew.ss_strName = "CoreAPI"; // Access by this symbol name
  ssNew.ss_istType = 0; // Should be '_shell_istUndeclared'
  ssNew.ss_pvValue = this; // Pointer to self
  ssNew.ss_ulFlags = SSF_CONSTANT; // Unchangable
  ssNew.ss_pPreFunc = NULL; // Unused
  ssNew.ss_pPostFunc = NULL; // Unused

  ulVersion = MakeVersion(1, 2, 0);

  pdpRenderSpace = NULL;

  // Output patcher actions
  if (FileExists(_fnmApplicationExe.FileDir() + "PatcherOutput")) {
    CPatch::SetDebug(true);
  }
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
    // Obtain Game library
    CPluginModule *pGameLib = GetPluginAPI()->LoadPlugin_t(GetGameLibPath());
    CPrintF(TRANS("Loading game library '%s'...\n"), pGameLib->GetName());

    // Set metadata
    CPluginAPI::PluginInfo &info = pGameLib->_info;

    info.strName = "Game library";
    info.strAuthor = "Croteam";
    info.ulVersion = MakeVersion(1, 0, _SE_BUILD_MINOR);
    info.strDescription = "Main component that provides game logic.";

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

      CPrintF("%s\n", strError);
    }
  }

  CPrintF("--- Done! ---\n");
};

// Release all user plugins of specific utility types
void CCoreAPI::ReleasePlugins(ULONG ulUtilityFlags) {
  CPrintF("--- Releasing user plugins (flags: 0x%X) ---\n", ulUtilityFlags);

  CDynamicContainer<CPluginModule> &cPlugins = GetPluginAPI()->GetPlugins();
  CDynamicContainer<CPluginModule> cToRelease;

  // Gather plugins that need to be released
  FOREACHINDYNAMICCONTAINER(cPlugins, CPluginModule, itPlugin) {
    // Matching utility flags
    if (itPlugin->GetInfo().ulFlags & ulUtilityFlags) {
      cToRelease.Add(itPlugin);
    }
  }

  // Delete plugin modules one by one
  while (cToRelease.Count() > 0) {
    CPluginModule *pModule = cToRelease.Pointer(0);

    cToRelease.Remove(pModule);
    delete pModule;
  }

  CPrintF("--- Done! ---\n");
};

// Called after starting world simulation
void CCoreAPI::OnGameStart(void)
{
  // Call game start function for each plugin
  FOREACHPLUGINHANDLER(GetPluginAPI()->cNetworkEvents, INetworkEvents, pEvents) {
    pEvents->OnGameStart();
  }
};

// Called before stopping world simulation
void CCoreAPI::OnGameStop(void)
{
  // Call game stop function for each plugin
  FOREACHPLUGINHANDLER(GetPluginAPI()->cNetworkEvents, INetworkEvents, pEvents) {
    pEvents->OnGameStop();
  }
};

// Called every simulation tick
void CCoreAPI::OnTick(void)
{
  // Call step function for each plugin
  FOREACHPLUGINHANDLER(GetPluginAPI()->cProcessors, IProcessingEvents, pEvents) {
    pEvents->OnStep();
  }
};

// Called before redrawing game view
void CCoreAPI::OnPreDraw(CDrawPort *pdp)
{
  // Call pre-draw function for each plugin
  FOREACHPLUGINHANDLER(GetPluginAPI()->cRenderers, IRenderingEvents, pEvents) {
    pEvents->OnPreDraw(pdp);
  }
};

// Called after redrawing game view
void CCoreAPI::OnPostDraw(CDrawPort *pdp)
{
  // Call post-draw function for each plugin
  FOREACHPLUGINHANDLER(GetPluginAPI()->cRenderers, IRenderingEvents, pEvents) {
    pEvents->OnPostDraw(pdp);
  }
};

// Called every render frame
void CCoreAPI::OnFrame(CDrawPort *pdp)
{
  // Call frame function for each plugin
  FOREACHPLUGINHANDLER(GetPluginAPI()->cProcessors, IProcessingEvents, pEvents) {
    pEvents->OnFrame(pdp);
  }
};
