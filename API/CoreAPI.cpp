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

#include "StdH.h"

#include <Engine/Base/Console_internal.h>
#include "Interfaces/FileFunctions.h"

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

  ulVersion = MakeVersion(1, 5, 1);

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
void CCoreAPI::LoadGameLib(const CTString &strSettingsFile) {
  // Already loaded
  if (_pGame != NULL) return;

  try {
    // Obtain Game plugin
    CPluginModule *pGameLib = LoadGamePlugin();

    // Create Game class
    CGame *(*pGameCreateFunc)(void) = NULL;
    pGameLib->GetSymbol_t(&pGameCreateFunc, "GAME_Create");

    _pGame = pGameCreateFunc();

  } catch (char *strError) {
    FatalError("%s", strError);
  }

  // Initialize Game, if needed
  if (strSettingsFile != "") {
    _pGame->Initialize(strSettingsFile);

    // Hook default fields
    GetGameAPI()->HookFields();
  }
};

// Set metadata for the Game plugin
CPluginModule *CCoreAPI::LoadGamePlugin(void) {
  // Obtain Game library
  CPluginModule *pGameLib = GetPluginAPI()->LoadPlugin_t(GetGameLibPath());
  CPrintF(TRANS("Loading game library '%s'...\n"), pGameLib->GetName());

  // Set metadata
  CPluginAPI::PluginInfo &info = pGameLib->GetInfo();

  info.strName = "Game library";
  info.strAuthor = "Croteam";
  info.ulVersion = MakeVersion(1, 0, _SE_BUILD_MINOR);
  info.strDescription = "Main component that provides game logic.";

  return pGameLib;
};

// Load all user plugins of specific utility types
void CCoreAPI::LoadPlugins(ULONG ulUtilityFlags) {
  // List all library files
  CFileList afnmDir;
  IFiles::ListGameFiles(afnmDir, "Bin\\Plugins\\", "*.dll", IFiles::FLF_RECURSIVE | IFiles::FLF_SEARCHMOD | IFiles::FLF_IGNORELISTS);

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
      CPrintF("^cffff00%s\n", strError);
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

  // Release plugin modules one by one
  CPluginStock *pStock = GetPluginAPI()->pPluginStock;

  FOREACHINDYNAMICCONTAINER(cToRelease, CPluginModule, itRelease) {
    pStock->Release(itRelease);
  }

  CPrintF("--- Done! ---\n");
};

// Reinitialize console in the engine
void CCoreAPI::ReinitConsole(INDEX ctCharsPerLine, INDEX ctLines) {
  CConsole &con = *_pConsole;

  // Synchronize access to console
  CTSingleLock slConsole(&con.con_csConsole, TRUE);

  // Limit characters per line
  ctCharsPerLine = Clamp(ctCharsPerLine, (INDEX)90, (INDEX)256);

  // Save last console log
  CStaticStackArray<CTString> aLastLines;
  INDEX ctLastLines = con.con_ctLines;

  while (--ctLastLines >= 0) {
    CTString strLine = CON_GetLastLine(ctLastLines);

    // Remove the rest of the line
    strLine.TrimRight(con.con_ctCharsPerLine);
    strLine.TrimSpacesRight();

    aLastLines.Push() = strLine;
  }

  // Allocate the buffer
  con.con_ctCharsPerLine = ctCharsPerLine;
  con.con_ctLines = ctLines;
  con.con_ctLinesPrinted = 0;

  // Add line break to the line
  ctCharsPerLine += 1;

  // Add a null terminator at the end of string buffers
  ResizeMemory((void **)&con.con_strBuffer, ctCharsPerLine * ctLines + 1);
  ResizeMemory((void **)&con.con_strLineBuffer, ctCharsPerLine + 1);
  ResizeMemory((void **)&con.con_atmLines, (ctLines + 1) * sizeof(TIME));

  // Clear all lines
  for (INDEX iLine = 0; iLine < ctLines; iLine++) {
    // Fill the line with spaced from the start
    char *pchLine = con.con_strBuffer + iLine * (con.con_ctCharsPerLine + 1);
    memset(pchLine, ' ', con.con_ctCharsPerLine);

    // Add line break at the end
    pchLine[con.con_ctCharsPerLine] = '\n';
    con.con_atmLines[iLine] = _pTimer->GetRealTimeTick();
  }

  // Set null terminator
  con.con_strBuffer[ctCharsPerLine * ctLines] = '\0';

  // Start printing from the last line
  con.con_strLastLine = con.con_strBuffer + ctCharsPerLine * (ctLines - 1);
  con.con_strCurrent = con.con_strLastLine;

  // Restore contents of the last log
  for (INDEX iRestore = 0; iRestore < aLastLines.Count(); iRestore++) {
    CPutString(aLastLines[iRestore] + "\n");
  }
};

// Called after starting world simulation
void CCoreAPI::OnGameStart(void)
{
  // Call game start function for each plugin
  FOREACHPLUGINHANDLER(GetPluginAPI()->cGameEvents, IGameEvents, pEvents) {
    if ((IAbstractEvents *)pEvents == NULL) continue;

    pEvents->OnGameStart();
  }
};

// Called before stopping world simulation
void CCoreAPI::OnGameStop(void)
{
  // Call game stop function for each plugin
  FOREACHPLUGINHANDLER(GetPluginAPI()->cGameEvents, IGameEvents, pEvents) {
    if ((IAbstractEvents *)pEvents == NULL) continue;

    pEvents->OnGameStop();
  }
};

// Called after saving the game
void CCoreAPI::OnGameSave(const CTFileName &fnmSave)
{
  // Call game save function for each plugin
  FOREACHPLUGINHANDLER(GetPluginAPI()->cGameEvents, IGameEvents, pEvents) {
    if ((IAbstractEvents *)pEvents == NULL) continue;

    pEvents->OnGameSave(fnmSave);
  }
};

// Called after loading a saved game
void CCoreAPI::OnGameLoad(const CTFileName &fnmSave)
{
  // Call game load function for each plugin
  FOREACHPLUGINHANDLER(GetPluginAPI()->cGameEvents, IGameEvents, pEvents) {
    if ((IAbstractEvents *)pEvents == NULL) continue;

    pEvents->OnGameLoad(fnmSave);
  }
};

// Called after finishing reading the world file
void CCoreAPI::OnWorldLoad(CWorld *pwo, const CTFileName &fnmWorld)
{
  // Call world load function for each plugin
  FOREACHPLUGINHANDLER(GetPluginAPI()->cWorldEvents, IWorldEvents, pEvents) {
    if ((IAbstractEvents *)pEvents == NULL) continue;

    pEvents->OnWorldLoad(pwo, fnmWorld);
  }
};

// Called every simulation tick
void CCoreAPI::OnTick(void)
{
  // Call step function for each plugin
  FOREACHPLUGINHANDLER(GetPluginAPI()->cProcessors, IProcessingEvents, pEvents) {
    if ((IAbstractEvents *)pEvents == NULL) continue;

    pEvents->OnStep();
  }
};

// Called before redrawing game view
void CCoreAPI::OnPreDraw(CDrawPort *pdp)
{
  // Call pre-draw function for each plugin
  FOREACHPLUGINHANDLER(GetPluginAPI()->cRenderers, IRenderingEvents, pEvents) {
    if ((IAbstractEvents *)pEvents == NULL) continue;

    pEvents->OnPreDraw(pdp);
  }
};

// Called after redrawing game view
void CCoreAPI::OnPostDraw(CDrawPort *pdp)
{
  // Call post-draw function for each plugin
  FOREACHPLUGINHANDLER(GetPluginAPI()->cRenderers, IRenderingEvents, pEvents) {
    if ((IAbstractEvents *)pEvents == NULL) continue;

    pEvents->OnPostDraw(pdp);
  }
};

// Called after rendering the world
void CCoreAPI::OnRenderView(CWorld &wo, CEntity *penViewer, CAnyProjection3D &apr, CDrawPort *pdp)
{
  // Call render view function for each plugin
  FOREACHPLUGINHANDLER(GetPluginAPI()->cRenderers, IRenderingEvents, pEvents) {
    if ((IAbstractEvents *)pEvents == NULL) continue;

    pEvents->OnRenderView(wo, penViewer, apr, pdp);
  }
};

// Called every render frame
void CCoreAPI::OnFrame(CDrawPort *pdp)
{
  // Call frame function for each plugin
  FOREACHPLUGINHANDLER(GetPluginAPI()->cProcessors, IProcessingEvents, pEvents) {
    if ((IAbstractEvents *)pEvents == NULL) continue;

    pEvents->OnFrame(pdp);
  }
};
