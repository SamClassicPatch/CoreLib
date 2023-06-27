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
#include <Engine/GameShell.h>
#include "Interfaces/FileFunctions.h"

#include "Networking/Modules/ClientLogging.h"

#include <STLIncludesBegin.h>
#include <string>
#include <fstream>
#include <sstream>
#include <direct.h>
#include <STLIncludesEnd.h>

// Define external core API
CCoreAPI *_pCoreAPI = NULL;

// Define static fields
CCoreAPI::EAppType CCoreAPI::eAppType = CCoreAPI::APP_UNKNOWN;
BOOL CCoreAPI::bCustomMod = FALSE;
CTString CCoreAPI::strVanillaExt = "";

// Define patch config
static CIniConfig _iniConfig;

// Queue shadow updating for the next connection to a server
static BOOL _bQueueShadowUpdate = TRUE;

// Fix broken shadows and lights by updating them
static void UpdateShadows(void)
{
  FOREACHINDYNAMICCONTAINER(IWorld::GetWorld()->wo_cenEntities, CEntity, iten) {
    if (!IsDerivedFromClass(iten, "Light")) continue;

    // Update shadow layers for each light
    CLightSource *pls = iten->GetLightSource();

    if (pls != NULL) {
      pls->FindShadowLayers(FALSE);
    }
  }

  // Update shadows from the sun and such
  IWorld::GetWorld()->CalculateDirectionalShadows();
};

// Specify vanilla Bin directory as an extra DLL directory
static void SetVanillaBinDirectory(void) {
  // Load DLL directory method
  HINSTANCE hKernel = GetModuleHandleA("Kernel32.dll");

  // This should never happen, but still
  if (hKernel == NULL) return;

  typedef BOOL (*CSetDirFunc)(LPCSTR);
  CSetDirFunc pSetDirFunc = (CSetDirFunc)GetProcAddress(hKernel, "SetDllDirectoryA");

  // Set extra DLL directory
  if (pSetDirFunc != NULL) {
    pSetDirFunc((CCoreAPI::AppPath() + "Bin\\").str_String);
  }
};

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

  ulVersion = CORE_PATCH_VERSION;

  // Update shadows in a current world
  _pShell->DeclareSymbol("user void UpdateShadows(void);", &UpdateShadows);
};

// Setup the core before initializing it
void CCoreAPI::Setup(EAppType eSetType) {
  // Set application type
  eAppType = eSetType;

  // Specify extra DLL directory
  SetVanillaBinDirectory();

  // Load configuration file once
  static BOOL bLoadConfig = TRUE;

  if (bLoadConfig) {
    bLoadConfig = FALSE;

    try {
      Props().Load_t(CORE_CONFIG_FILE, FALSE);

    } catch (char *strError) {
      (void)strError;
    }
  }

  // Load vanilla extension
  std::ifstream strm((CCoreAPI::AppPath() + "ModExt.txt").str_String);

  if (!strm.fail()) {
    std::string strReadExt;

    if (std::getline(strm, strReadExt)) {
      strVanillaExt = strReadExt.c_str();
    }
  }

  // Enable debug output for patcher actions
  if (Props().GetBoolValue("", "DebugPatcher", false)) {
    CPatch::SetDebug(true);
  }
};

// Get full path relative to the game to some library (mod Bin -> patch Bin -> vanilla Bin)
CTString CCoreAPI::FullLibPath(const CTString &strLibName, const CTString &strLibExt) {
  // Check for existence of libraries in order:
  // 1. Mods/<mod>/Bin/<lib> (e.g. Mods/ClassicsPatchMod/Bin/Debug/Game_CustomD.dll)
  // 2. <patch bin>/<lib>    (e.g. Bin_ClassicsPatch/Debug/Game_CustomD.dll)
  // 3. Bin/<lib>            (e.g. Bin/Debug/Game_CustomD.dll)
  static const CTFileName &fnmRootDir = AppPath();
  const CTString strLibFile = GetLibFile(strLibName, strLibExt);

  // Check if library file exists on disk and return it
  CTString strCheckFile;
  #define CHECK_AND_RETURN_PATH { if (IFiles::IsReadable((fnmRootDir + strCheckFile).str_String)) return strCheckFile; }

  // Check in the mod's Bin folder
  if (_fnmMod != "") {
    strCheckFile = _fnmMod + "Bin\\" + strLibFile;
    CHECK_AND_RETURN_PATH;
  }

  // Check in the Bin folder of Classics Patch (from where it's currently running)
  strCheckFile = AppBin() + strLibFile;
  CHECK_AND_RETURN_PATH;

  // Check in the vanilla Bin folder as the last resort
  strCheckFile = "Bin\\" + strLibFile;
  CHECK_AND_RETURN_PATH;

  // No library found
  ASSERT(FALSE);
  return "";
};

// Get cut-off position before the Bin directory
static inline size_t BinDirPos(std::string strExePath) {
  // Cut off module filename to end up with Bin (e.g. "C:\SeriousSam\Bin" and "\SeriousSam_Custom.exe")
  strExePath.erase(strExePath.rfind("\\"));

  // Skip Debug directory
  #ifdef _DEBUG
    // If found Debug directory at the very end, cut it off
    const size_t iDebug = strExePath.rfind("\\Debug");

    if (iDebug == strExePath.length() - 6) {
      strExePath.erase(iDebug);
    }
  #endif

  // Go up to the root directory (e.g. "C:\SeriousSam\" and "Bin\SeriousSam_Custom.exe")
  return strExePath.rfind("\\") + 1;
};

// Get relative path to the game executable
const CTFileName &CCoreAPI::AppExe(void) {
  // Use existing path
  if (_fnmApplicationExe != "") return _fnmApplicationExe;

  static CTFileName fnmLocalPath;

  // Get executable path locally
  if (fnmLocalPath == "") {
    char strPathBuffer[1024];
    GetModuleFileNameA(NULL, strPathBuffer, sizeof(strPathBuffer));

    std::string strExePath = strPathBuffer;
    size_t iBinDir = BinDirPos(strExePath);

    // Copy relative path to the executable with the Bin directory
    fnmLocalPath = CTString(strExePath.substr(iBinDir).c_str());
  }

  return fnmLocalPath;
};

// Get absolute path to the game directory
const CTFileName &CCoreAPI::AppPath(void) {
  // Use existing path
  if (_fnmApplicationPath != "") return _fnmApplicationPath;

  static CTFileName fnmLocalPath;

  // Get application path locally
  if (fnmLocalPath == "") {
    char strPathBuffer[1024];
    GetModuleFileNameA(NULL, strPathBuffer, sizeof(strPathBuffer));

    std::string strExePath = strPathBuffer;
    size_t iBinDir = BinDirPos(strExePath);

    // Copy absolute path to the game directory
    fnmLocalPath = CTString(strExePath.erase(iBinDir).c_str());
  }

  return fnmLocalPath;
};

// Get config with global properties
CIniConfig &CCoreAPI::Props(void) {
  return _iniConfig;
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

// Create a series of directories within the game folder
void CCoreAPI::CreateDir(const CTString &strPath) {
  std::string strDirs = strPath;
  const char *strAppPath = AppPath().str_String;

  size_t iDir = 0;

  // Get next directory from the last position
  while ((iDir = strDirs.find_first_of('\\', iDir)) != std::string::npos) {
    // Include the slash
    iDir++;

    // Create current subdirectory
    _mkdir((strAppPath + strDirs.substr(0, iDir)).c_str());
  }
};

// Load dynamic link library and throw exception upon any error
HINSTANCE CCoreAPI::LoadLib(const char *strFileName) {
  // Load plugin library
  HINSTANCE hiDLL = ::LoadLibraryA(strFileName);

  // Loaded properly
  if (hiDLL != NULL) {
    return hiDLL;
  }

  // Get the error code
  DWORD dwMessageId = GetLastError();

  // Format the Windows error message
  LPVOID lpMsgBuf;
  DWORD dwSuccess = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
    NULL, dwMessageId, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
    (LPSTR)&lpMsgBuf, 0, NULL);

  CTString strWinError;

  // If formatting succeeds
  if (dwSuccess != 0) {
    // Copy the result
    strWinError = (char *)lpMsgBuf;

    // Free the Windows message buffer
    LocalFree(lpMsgBuf);

  // Otherwise report failure
  } else {
    strWinError.PrintF(TRANS(
      "Cannot format error message!\nOriginal error code: %d\nFormatting error code: %d\n"),
      dwMessageId, GetLastError());
  }

  // Report error
  ThrowF_t(TRANS("Cannot load module '%s':\n%s"), strFileName, strWinError);
  return NULL;
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

// Load GameGUI library and return pointer to GameGUI_interface
void *CCoreAPI::LoadGameGuiLib(const CTString &strSettingsFile) {
  static GameGUI_interface *pGameGUI = NULL;

  // Already loaded
  if (pGameGUI != NULL) return pGameGUI;

  try {
    // Obtain GameGUI plugin
    CPluginModule *pGameLib = LoadGameGuiPlugin();

    // Create GameGUI class
    GameGUI_interface *(*pGameGuiCreateFunc)(void) = NULL;
    pGameLib->GetSymbol_t(&pGameGuiCreateFunc, "GAMEGUI_Create");

    pGameGUI = pGameGuiCreateFunc();

  } catch (char *strError) {
    FatalError("%s", strError);
  }

  pGameGUI->Initialize(strSettingsFile);

  return (void *)pGameGUI;
};

// Set metadata for the Game plugin
CPluginModule *CCoreAPI::LoadGamePlugin(void) {
  // Obtain Game library
  CPluginModule *pLib = GetPluginAPI()->LoadPlugin_t(FullLibPath("Game" + _strModExt));
  CPrintF(TRANS("Loading Game library '%s'...\n"), pLib->GetName());

  // Set metadata for vanilla library
  CPluginInfo &info = pLib->GetInfo();

  if (info.ulVersion == 0) {
    info.strAuthor = "Croteam";
    info.strName = "Game library";
    info.strDescription = "Main component that provides game logic.";
    info.ulVersion = MakeVersion(1, 0, _SE_BUILD_MINOR);
  }

  return pLib;
};

// Set metadata for the GameGUI plugin
CPluginModule *CCoreAPI::LoadGameGuiPlugin(void) {
  // Obtain Game library
  CPluginModule *pLib = GetPluginAPI()->LoadPlugin_t(FullLibPath("GameGUI" + _strModExt));
  CPrintF(TRANS("Loading Game GUI library '%s'...\n"), pLib->GetName());

  // Set metadata for vanilla library
  CPluginInfo &info = pLib->GetInfo();

  if (info.ulVersion == 0) {
    info.strAuthor = "Croteam";
    info.strName = "Game GUI library";
    info.strDescription = "Serious Editor component that provides custom game interfaces.";
    info.ulVersion = MakeVersion(1, 0, _SE_BUILD_MINOR);
  }

  return pLib;
};

// Load all user plugins of specific utility types
void CCoreAPI::LoadPlugins(ULONG ulUtilityFlags) {
  // List all library files
  CFileList afnmGameDir;
  CFileList afnmModDir;

  #define LIST_PLUGINS_FLAGS IFiles::FLF_RECURSIVE | IFiles::FLF_IGNORELISTS | IFiles::FLF_IGNOREGRO

  IFiles::ListGameFiles(afnmGameDir, CCoreAPI::AppBin()    + "Plugins\\", "*.dll", LIST_PLUGINS_FLAGS);
  IFiles::ListGameFiles(afnmModDir,  CCoreAPI::AppModBin() + "Plugins\\", "*.dll", LIST_PLUGINS_FLAGS | IFiles::FLF_ONLYMOD);

  // Move mod plugins in the same list
  for (INDEX iMod = 0; iMod < afnmModDir.Count(); iMod++) {
    afnmGameDir.Push() = afnmModDir[iMod];
  }

  CPrintF("--- Loading user plugins (flags: 0x%X) ---\n", ulUtilityFlags);

  // Load every plugin
  for (INDEX i = 0; i < afnmGameDir.Count(); i++)
  {
    CPrintF("  %d - %s\n", i + 1, afnmGameDir[i].str_String);

    try {
      // Try to load the plugin
      GetPluginAPI()->ObtainPlugin_t(afnmGameDir[i], ulUtilityFlags);

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
  CStringStack aLastLines;
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

  // Queue shadow updating again
  _bQueueShadowUpdate = TRUE;

  // Reset all clients
  CActiveClient::ResetAll();

  // Save client log by the end of the game
  IClientLogging::SaveLog();
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

  // Update shadow maps
  UpdateShadows();
};

// Called after starting demo playback
void CCoreAPI::OnDemoPlay(const CTFileName &fnmDemo)
{
  // Call demo play function for each plugin
  FOREACHPLUGINHANDLER(GetPluginAPI()->cDemoEvents, IDemoEvents, pEvents) {
    if ((IAbstractEvents *)pEvents == NULL) continue;

    pEvents->OnDemoPlay(fnmDemo);
  }

  // Update shadow maps
  UpdateShadows();
};

// Called after starting demo recording
void CCoreAPI::OnDemoStart(const CTFileName &fnmDemo)
{
  // Call demo start function for each plugin
  FOREACHPLUGINHANDLER(GetPluginAPI()->cDemoEvents, IDemoEvents, pEvents) {
    if ((IAbstractEvents *)pEvents == NULL) continue;

    pEvents->OnDemoStart(fnmDemo);
  }
};

// Called after stopping demo recording
void CCoreAPI::OnDemoStop(void)
{
  // Call demo stop function for each plugin
  FOREACHPLUGINHANDLER(GetPluginAPI()->cDemoEvents, IDemoEvents, pEvents) {
    if ((IAbstractEvents *)pEvents == NULL) continue;

    pEvents->OnDemoStop();
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

// Called every time a new player is added
void CCoreAPI::OnAddPlayer(CPlayerTarget &plt, BOOL bLocal)
{
  // Call player addition function for each plugin
  FOREACHPLUGINHANDLER(GetPluginAPI()->cNetworkEvents, INetworkEvents, pEvents) {
    if ((IAbstractEvents *)pEvents == NULL) continue;

    pEvents->OnAddPlayer(plt, bLocal);
  }

  // Update shadow maps for connecting players
  if (bLocal && !_pNetwork->IsServer())
  {
    // Only if queued
    if (_bQueueShadowUpdate) {
      _bQueueShadowUpdate = FALSE;
      UpdateShadows();
    }
  }
};

// Called every time a player is removed
void CCoreAPI::OnRemovePlayer(CPlayerTarget &plt, BOOL bLocal)
{
  // Call player removal function for each plugin
  FOREACHPLUGINHANDLER(GetPluginAPI()->cNetworkEvents, INetworkEvents, pEvents) {
    if ((IAbstractEvents *)pEvents == NULL) continue;

    pEvents->OnRemovePlayer(plt, bLocal);
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
