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

#include <Engine/Base/Console_internal.h>
#include <Engine/GameShell.h>
#include "Interfaces/FileFunctions.h"
#include "Query/QueryManager.h"

#include <STLIncludesBegin.h>
#include <string>
#include <fstream>
#include <sstream>
#include <direct.h>
#include <STLIncludesEnd.h>

// Define external core API
CCoreAPI *_pCoreAPI = NULL;

// Define release version of the core
const ULONG CCoreAPI::ulCoreVersion = CORE_PATCH_VERSION;

// Define constant data
static CCoreAPI::EAppType _eAppType = CCoreAPI::APP_UNKNOWN; // Running application type
static BOOL _bCustomMod = FALSE; // Using custom mod from the patch
static CTString _strVanillaExt = ""; // Library extension of the vanilla game

// Define variable data
CCoreVariables CCoreAPI::varData;

// Define patch config and its property holder
static CIniConfig _iniConfig;
static CCoreAPI::SConfigProps _cfgProps;

// Current seasonal event
static CCoreAPI::ESpecialEvent _eSpecialEvent = CCoreAPI::SPEV_NONE;

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

// Load user configs for customization
void CCoreVariables::LoadConfigs(void) {
  // Load modifiable data
  try {
    CIniConfig iniModData;
    iniModData.Load_t(CORE_VARIABLE_DATA_CONFIG_FILE, TRUE);

    bAdjustFOV = iniModData.GetBoolValue("", "AdjustFOV", TRUE);
    bAdjustAR  = iniModData.GetBoolValue("", "AdjustAR", TRUE);
    bProperTextScaling = iniModData.GetBoolValue("", "ProperTextScaling", TRUE);
    fMenuTextScale = iniModData.GetDoubleValue("", "MenuTextScale", 1.0);

  } catch (char *strError) {
    (void)strError;
  }

  // Load difficulties
  CFileList aDiffs;
  BOOL bLoadFromGame = TRUE;

  // Load from the mod
  if (_fnmMod != "") {
    IFiles::ListGameFiles(aDiffs, "Data\\ClassicsPatch\\Difficulties\\", "*.ini", IFiles::FLF_ONLYMOD);

    // Don't load from the game if there are any mod difficulties
    if (aDiffs.Count() != 0) {
      bLoadFromGame = FALSE;
    }
  }

  // Load from the game
  if (bLoadFromGame) {
    IFiles::ListGameFiles(aDiffs, "Data\\ClassicsPatch\\Difficulties\\", "*.ini", 0);
  }

  // Create new difficulties
  const INDEX ctDiffs = ClampUp(aDiffs.Count(), INDEX(MAX_GAME_DIFFICULTIES));

  // No user difficulties
  if (ctDiffs == 0) return;

  ClearGameDiffs();

  for (INDEX i = 0; i < ctDiffs; i++) {
    const CTFileName &fnm = aDiffs[i];
    Difficulty &diff = aGameDiffs[i];

    // Load difficulty properties
    try {
      CIniConfig iniDiff;
      iniDiff.Load_t(fnm, TRUE);

      diff.iLevel = iniDiff.GetIntValue("", "Level", i);
      diff.strCommand = iniDiff.GetValue("", "UnlockCommand", "");
      diff.bFlash = iniDiff.GetBoolValue("", "Flash", FALSE);

    } catch (char *strError) {
      CPrintF(TRANS("Cannot load user difficulty config '%s':\n%s"), fnm.str_String, strError);
    }

    // Get difficulty name and description
    CTString strName = "???";
    CTString strDesc = "";

    try {
      // Try loading text from the description file nearby
      strName.Load_t(fnm.NoExt() + ".des");

      // Separate text into name and description
      ULONG ulLineBreak = IData::FindChar(strName, '\n');

      if (ulLineBreak != -1) {
        strName.Split(ulLineBreak + 1, strName, strDesc);
      }

    } catch (char *strError) {
      // Just set text to the filename
      (void)strError;
      strName = fnm.FileName();
    }

    diff.strName = strName;
    diff.strTip = strDesc;
  }
};

// Constructor that sets default property states
CCoreAPI::SConfigProps::SConfigProps() :
  bCustomMod(TRUE), bDebugPatcher(FALSE), bDPIAware(TRUE), bExtendedFileSystem(TRUE),
  bFullAppIntegration(FALSE), strTFEDir(""), strSSRDir(""), strSSRWorkshop(""),
  // Steam API
  bSteamEnable(TRUE), bSteamForServers(FALSE), bSteamForTools(FALSE)
{
};

// Load properties from the config
void CCoreAPI::SConfigProps::Load(void) {
  // Try loading only once
  static BOOL bLoaded = FALSE;

  if (bLoaded) return;
  bLoaded = TRUE;

  // Try to load from a file
  try {
    _iniConfig.Load_t(CORE_CONFIG_FILE, FALSE);

  // Abort
  } catch (char *strError) {
    (void)strError;

    _iniConfig.Clear();
    return;
  }

  // Get values
  bCustomMod          = _iniConfig.GetBoolValue("", "CustomMod", TRUE);
  bDebugPatcher       = _iniConfig.GetBoolValue("", "DebugPatcher", FALSE);
  bDPIAware           = _iniConfig.GetBoolValue("", "DPIAware", TRUE);
  bExtendedFileSystem = _iniConfig.GetBoolValue("", "ExtendedFileSystem", TRUE);
  bFullAppIntegration = _iniConfig.GetBoolValue("", "FullAppIntegration", FALSE);
  strTFEDir           = _iniConfig.GetValue("", "TFEDir", CONFIG_DEFAULT_DIR_TFE);
  strSSRDir           = _iniConfig.GetValue("", "SSRDir", CONFIG_DEFAULT_DIR_SSR);
  strSSRWorkshop      = _iniConfig.GetValue("", "SSRWorkshop", CONFIG_DEFAULT_DIR_WORKSHOP);

  bSteamEnable        = _iniConfig.GetBoolValue("Steam", "Enable", TRUE);
  bSteamForServers    = _iniConfig.GetBoolValue("Steam", "ForServers", FALSE);
  bSteamForTools      = _iniConfig.GetBoolValue("Steam", "ForTools", FALSE);
};

// Save properties into the config
void CCoreAPI::SConfigProps::Save(void) {
  // Macro for quickly converting any non-string value into string
  #define TO_STR(_Expr) static_cast<std::ostringstream &>((std::ostringstream() << std::dec << _Expr)).str().c_str()

  // Set new values
  _iniConfig.SetValue("", "CustomMod", TO_STR(bCustomMod));
  _iniConfig.SetValue("", "DebugPatcher", TO_STR(bDebugPatcher));
  _iniConfig.SetValue("", "DPIAware", TO_STR(bDPIAware));
  _iniConfig.SetValue("", "ExtendedFileSystem", TO_STR(bExtendedFileSystem));
  _iniConfig.SetValue("", "FullAppIntegration", TO_STR(bFullAppIntegration));
  _iniConfig.SetValue("", "TFEDir", strTFEDir);
  _iniConfig.SetValue("", "SSRDir", strSSRDir);
  _iniConfig.SetValue("", "SSRWorkshop", strSSRWorkshop);

  _iniConfig.SetValue("Steam", "Enable",     TO_STR(bSteamEnable));
  _iniConfig.SetValue("Steam", "ForServers", TO_STR(bSteamForServers));
  _iniConfig.SetValue("Steam", "ForTools",   TO_STR(bSteamForTools));

  #undef TO_STR

  // Save into a file
  GetAPI()->CreateDir(CORE_CONFIG_FILE);

  try {
    _iniConfig.Save_t(CORE_CONFIG_FILE);

  } catch (char *strError) {
    CPrintF(TRANS("Cannot save patch configuration file: %s\n"), strError);
  }
};

// Determine current seasonal event
static CCoreAPI::ESpecialEvent DetermineSpecialEvent(void) {
  // Get current date
  time_t iTime;
  time(&iTime);
  tm *tmLocal = localtime(&iTime);

  const int iDay = tmLocal->tm_mday;

  switch (tmLocal->tm_mon) {
    // January
    case 0:
      if (iDay <= 15) return CCoreAPI::SPEV_CHRISTMAS;
      break;

    // February
    case 1:
      if (iDay >= 10 && iDay <= 18) return CCoreAPI::SPEV_VALENTINE;
      break;

    // March
    case 2:
      if (iDay >= 19 && iDay <= 23) return CCoreAPI::SPEV_BD_PARTY;
      break;

    // June
    case 5:
      if (iDay >= 20 && iDay <= 24) return CCoreAPI::SPEV_BD_PARTY;
      break;

    // October
    case 9:
      return CCoreAPI::SPEV_HALLOWEEN;
      break;

    // December
    case 11:
      if (iDay >= 15) return CCoreAPI::SPEV_CHRISTMAS;
      break;
  }

  return CCoreAPI::SPEV_NONE;
};

// Constructor
CCoreAPI::CCoreAPI() :
  apiPatches(*new CPatchAPI), apiGame(*new CGameAPI), apiPlugins(*new CPluginAPI), apiSteam(*new CSteamAPI)
{
  // Make sure that this is the only API class
  ASSERT(_pCoreAPI == NULL);

  _pCoreAPI = this;
  varData.pAPI = this;

  // Prepare variable data
  varData.LoadConfigs();

  // Disable custom mod if it was never set
  SetCustomMod(FALSE);

  // Determine current seasonal event
  _eSpecialEvent = DetermineSpecialEvent();

  // Add core API and variable data to symbols
  CShellSymbol *pssNew = _pShell->sh_assSymbols.New(2);

  pssNew[0].ss_strName = "CoreAPI";
  pssNew[0].ss_pvValue = this;

  pssNew[1].ss_strName = CORE_VARIABLE_DATA_SYMBOL;
  pssNew[1].ss_pvValue = &varData;

  // Default values for shell symbols
  for (INDEX iSymbol = 0; iSymbol < 2; iSymbol++) {
    pssNew[iSymbol].ss_istType = 0; // Should be '_shell_istUndeclared'
    pssNew[iSymbol].ss_ulFlags = SSF_CONSTANT; // Unchangable
    pssNew[iSymbol].ss_pPreFunc = NULL; // Unused
    pssNew[iSymbol].ss_pPostFunc = NULL; // Unused
  }

  // Update shadows in a current world
  extern void UpdateShadows(void);
  _pShell->DeclareSymbol("user void UpdateShadows(void);", &UpdateShadows);

  extern INDEX gam_bAutoUpdateShadows;
  _pShell->DeclareSymbol("persistent user INDEX gam_bAutoUpdateShadows;", &gam_bAutoUpdateShadows);
};

// Get running application type after initializing the core
// For modules that aren't utilizing Core library directly (e.g. plugins)
CCoreAPI::EAppType CCoreAPI::GetAppType(void) {
  return _eAppType;
};

// Get running application type before initializing the core
CCoreAPI::EAppType CCoreAPI::GetApplication(void) {
  return _eAppType;
};

// Check if playing with a custom mod
BOOL CCoreAPI::IsCustomModActive(void) {
  return _bCustomMod;
};

// Set custom mod state only once
void CCoreAPI::SetCustomMod(BOOL bState) {
  // Don't let the state be changed
  static BOOL bCustomModSet = FALSE;

  if (bCustomModSet) return;
  bCustomModSet = TRUE;

  _bCustomMod = bState;
};

// Get vanilla library extension after initializing the core
// For modules that aren't utilizing Core library directly (e.g. plugins)
const CTString &CCoreAPI::GetModExt(void) {
  return _strVanillaExt;
};

// Get vanilla library extension before initializing the core
const CTString &CCoreAPI::GetVanillaExt(void) {
  return _strVanillaExt;
};

// Setup the core before initializing it
void CCoreAPI::Setup(EAppType eSetType) {
  // Set application type
  _eAppType = eSetType;

  // Specify extra DLL directory
  SetVanillaBinDirectory();

  // Load configuration properties
  _cfgProps.Load();

  // Load vanilla extension
  std::ifstream strm((CCoreAPI::AppPath() + "ModExt.txt").str_String);

  if (!strm.fail()) {
    std::string strReadExt;

    if (std::getline(strm, strReadExt)) {
      _strVanillaExt = strReadExt.c_str();
    }
  }

  // Enable debug output for patcher actions
  if (Props().bDebugPatcher) {
    CPatch::SetDebug(true);
  }
};

// Get full path relative to the game to some library (mod Bin -> patch Bin -> vanilla Bin)
CTString CCoreAPI::FullLibPath(const CTString &strLibName, const CTString &strLibExt) {
  // Check for existence of libraries in order:
  // 1. Mods/<mod>/Bin/<lib> (e.g. Mods/ClassicsPatchMod/Bin/Debug/Game_CustomD.dll)
  // 2. <patch bin>/<lib>    (e.g. Bin_ClassicsPatch/Debug/Game_CustomD.dll)
  // 3. Bin/<lib>            (e.g. Bin/Debug/Game_CustomD.dll)
  const CTFileName &fnmRootDir = AppPath();
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

// Get relative path to the mod's Bin directory (folder name)
CTFileName CCoreAPI::AppModBin(void) {
  // Use game's or mod's Bin directory
  return (_fnmMod == "") ? AppBin() : CTString("Bin\\");
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
CCoreAPI::SConfigProps &CCoreAPI::Props(void) {
  return _cfgProps;
};

// Get current seasonal event
CCoreAPI::ESpecialEvent CCoreAPI::GetCurrentEvent(void) {
  return _eSpecialEvent;
};

// Toggle vanilla query manager
void CCoreAPI::DisableGameSpy(void) {
#if CLASSICSPATCH_NEW_QUERY

  // Symbol for accessing GameSpy master server
  static CSymbolPtr pbHeartbeatGameSpy;

  // Update only when vanilla query is toggled
  static INDEX iLastVanillaQuery = -1;
  if (iLastVanillaQuery == ms_bVanillaQuery) return;

  // Find the symbol
  if (!pbHeartbeatGameSpy.Exists()) {
    pbHeartbeatGameSpy.Find("ser_bHeartbeatGameSpy");

    // Better luck next time
    if (!pbHeartbeatGameSpy.Exists()) return;
  }

  iLastVanillaQuery = ms_bVanillaQuery;

  // Remember pointer to the original value
  static INDEX *piValue = &pbHeartbeatGameSpy.GetIndex();

  if (!ms_bVanillaQuery) {
    // Update last value
    static INDEX iDummyValue;
    iDummyValue = *piValue;

    // Forcefully disable it
    *piValue = FALSE;

    // Make it inaccessible
    pbHeartbeatGameSpy._pss->ss_pvValue = &iDummyValue;

  } else {
    // Make it accessible again
    pbHeartbeatGameSpy._pss->ss_pvValue = piValue;
  }

#endif // CLASSICSPATCH_NEW_QUERY
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
    // Hook default fields
    GetGameAPI()->HookFields();

    // Can be used to hook new fields
    _pGame->Initialize(strSettingsFile);

    // Set mod URL to the latest patch release
    if (_strModURL == "" || _strModURL.FindSubstr("croteam.com") != -1) {
      _strModURL = CLASSICSPATCH_URL_LATESTRELEASE;
    }

  #if CLASSICSPATCH_NEW_QUERY
    // Update internal master server
    extern void UpdateInternalGameSpyMS(INDEX);
    UpdateInternalGameSpyMS(0);
  #endif
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

  #define LIST_PLUGINS_FLAGS (IFiles::FLF_RECURSIVE | IFiles::FLF_IGNORELISTS | IFiles::FLF_IGNOREGRO)
  #define LIST_PLUGINS_FLAGS_MOD (LIST_PLUGINS_FLAGS | IFiles::FLF_ONLYMOD | IFiles::FLF_REUSELIST)

  IFiles::ListGameFiles(afnmGameDir, CCoreAPI::AppBin()    + "Plugins\\", "*.dll", LIST_PLUGINS_FLAGS);
  IFiles::ListGameFiles(afnmGameDir, CCoreAPI::AppModBin() + "Plugins\\", "*.dll", LIST_PLUGINS_FLAGS_MOD);

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

  // Break the log before restoring previous lines
  CPrintF("\n--- CCoreAPI::ReinitConsole(%d, %d) ---\n\n", ctCharsPerLine - 1, ctLines);

  // Restore contents of the last log
  BOOL bSkipEmptyLines = TRUE;

  for (INDEX iRestore = 0; iRestore < aLastLines.Count(); iRestore++) {
    // Skip empty lines in the beginning
    if (bSkipEmptyLines) {
      CTString strLine = aLastLines[iRestore];
      strLine.TrimSpacesLeft();

      if (strLine == "") continue;
    }

    // At least one non-empty line printed
    CPutString(aLastLines[iRestore] + "\n");
    bSkipEmptyLines = FALSE;
  }
};
