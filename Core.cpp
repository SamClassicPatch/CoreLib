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

#include "Base/CoreTimerHandler.h"
#include "Interfaces/FileFunctions.h"
#include "Networking/NetworkFunctions.h"

#include "Networking/Modules/ClientLogging.h"

// Pointer to the Game module
CGame *_pGame = NULL;

// Revolution support
#if SE1_GAME == SS_REV
  // Pre-1.10 variables removed from Revolution engine
  CTFileName _fnmCDPath = CTString("");
  CTFileName _fnmMod = CTString("");
  CTString _strModName = "";
  CTString _strModURL = "";
  CTString _strModExt = "";
#endif

// Common game variables
#if SE1_GAME == SS_TFE
  CTString sam_strFirstLevel = "Levels\\01_Hatshepsut.wld";
  CTString sam_strIntroLevel = "Levels\\Intro.wld";
  CTString sam_strGameName = "serioussam";
#else
  CTString sam_strFirstLevel = "Levels\\LevelsMP\\1_0_InTheLastEpisode.wld";
  CTString sam_strIntroLevel = "Levels\\LevelsMP\\Intro.wld";
  CTString sam_strGameName = "serioussamse";
#endif

CTString sam_strVersion = _SE_VER_STRING; // Use version string

// Temporary password for connecting to some server
CTString cli_strConnectPassword = "";

// Current values of input axes
FLOAT inp_afAxisValues[MAX_OVERALL_AXES];

// Display information about the Classics patch
static void PatchInfo(void) {
  static CTString strInfo =
    "\nSerious Sam Classics Patch"
    "\n" CLASSICSPATCH_URL_SHORT
    "\n"
    "\n- Engine version: " + CTString(_SE_VER_STRING)
  + "\n- Patch version: " + GetAPI()->GetVersion()
  + "\n\n(c) Dreamy Cecil, 2022-2023\n";

  CPutString(strInfo);
};

// Helper method for loading scripts via string variables
static void IncludeScript(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  const CTString &strScript = *NEXT_ARG(CTString *);

  // Include command doesn't support variables, so the string needs to be inserted
  CTString strLoad(0, "include \"%s\";", strScript.str_String);
  _pShell->Execute(strLoad);
};

// Resave config properties into the file after setting them
static void ResaveConfigProperties(void) {
  CCoreAPI::Props().Save();

  CPrintF(TRANS("Config properties have been resaved into '%s'!\n"), CORE_CONFIG_FILE);
  CPutString(TRANS("Restart the game for the new settings to take effect!\n"));
};

// Initialize Core module (always after 'SE_InitEngine'!)
void ClassicsPatch_InitCore(void) {
  // Create core API
  new CCoreAPI();

  // Allow more characters in console by default
  GetAPI()->ReinitConsole(160, 512);

  // Load custom include/exclude lists for mods
  if (_fnmMod != "") {
    IFiles::LoadStringList(IFiles::aBaseWriteInc, CTString("BaseWriteInclude.lst"));
    IFiles::LoadStringList(IFiles::aBaseWriteExc, CTString("BaseWriteExclude.lst"));
    IFiles::LoadStringList(IFiles::aBaseBrowseInc, CTString("BaseBrowseInclude.lst"));
    IFiles::LoadStringList(IFiles::aBaseBrowseExc, CTString("BaseBrowseExclude.lst"));
  }

  // Information about the patch
  _pShell->DeclareSymbol("user void PatchInfo(void);", &PatchInfo);

  // Common symbols
  if (GetAPI()->IsGameApp() || GetAPI()->IsServerApp())
  {
    // Game variables
    _pShell->DeclareSymbol("           user CTString sam_strFirstLevel;", &sam_strFirstLevel);
    _pShell->DeclareSymbol("persistent user CTString sam_strIntroLevel;", &sam_strIntroLevel);
    _pShell->DeclareSymbol("persistent user CTString sam_strGameName;",   &sam_strGameName);
    _pShell->DeclareSymbol("           user CTString sam_strVersion;",    &sam_strVersion);

    _pShell->DeclareSymbol("persistent user CTString cli_strConnectPassword;", &cli_strConnectPassword);
  }

  _pShell->DeclareSymbol("user void IncludeScript(CTString);", &IncludeScript);

  // Current values of input axes
  static const CTString strAxisValues(0, "user const FLOAT inp_afAxisValues[%d];", MAX_OVERALL_AXES);
  _pShell->DeclareSymbol(strAxisValues, &inp_afAxisValues);

  // Input axes constants
  static const INDEX iAxisNone = AXIS_NONE;
  static const INDEX iAxisMouseX = MOUSE_X_AXIS;
  static const INDEX iAxisMouseY = MOUSE_Y_AXIS;
  static const INDEX iAxisMouseZ = 3;
  static const INDEX iAxisMouse2X = 4;
  static const INDEX iAxisMouse2Y = 5;
  static const INDEX iMaxJoysticks = MAX_JOYSTICKS;
  static const INDEX iAxesPerJoystick = MAX_AXES_PER_JOYSTICK;
  static const INDEX iFirstJoystickAxis = FIRST_JOYAXIS;
  static const INDEX iMaxInputAxes = MAX_OVERALL_AXES;

  _pShell->DeclareSymbol("const INDEX AXIS_NONE;",    (void *)&iAxisNone);
  _pShell->DeclareSymbol("const INDEX AXIS_M1_X;",    (void *)&iAxisMouseX);
  _pShell->DeclareSymbol("const INDEX AXIS_M1_Y;",    (void *)&iAxisMouseY);
  _pShell->DeclareSymbol("const INDEX AXIS_M1_Z;",    (void *)&iAxisMouseZ);
  _pShell->DeclareSymbol("const INDEX AXIS_M2_X;",    (void *)&iAxisMouse2X);
  _pShell->DeclareSymbol("const INDEX AXIS_M2_Y;",    (void *)&iAxisMouse2Y);
  _pShell->DeclareSymbol("const INDEX AXIS_JOY_CT;",  (void *)&iMaxJoysticks);
  _pShell->DeclareSymbol("const INDEX AXIS_PER_JOY;", (void *)&iAxesPerJoystick);
  _pShell->DeclareSymbol("const INDEX AXIS_JOY_1;",   (void *)&iFirstJoystickAxis);
  _pShell->DeclareSymbol("const INDEX AXIS_CT;",      (void *)&iMaxInputAxes);

  // Config property symbols
  {
    _pShell->DeclareSymbol("void ResaveConfigProperties(void);", &ResaveConfigProperties);

    #define DEFINE_PROP_SYMBOL(_Type, _Property) \
      _pShell->DeclareSymbol(#_Type " cfg_" #_Property ";", &CCoreAPI::Props()._Property);

    DEFINE_PROP_SYMBOL(INDEX, bCustomMod);
    DEFINE_PROP_SYMBOL(INDEX, bDebugPatcher);
    DEFINE_PROP_SYMBOL(INDEX, bDPIAware);
    DEFINE_PROP_SYMBOL(INDEX, bExtendedFileSystem);
    DEFINE_PROP_SYMBOL(INDEX, bFullAppIntegration);
    DEFINE_PROP_SYMBOL(INDEX, bSteam);
    DEFINE_PROP_SYMBOL(CTString, strTFEDir);
    DEFINE_PROP_SYMBOL(CTString, strSSRDir);
    DEFINE_PROP_SYMBOL(CTString, strSSRWorkshop);

    #undef DEFINE_PROP_SYMBOL
  }

  // Initialize networking
  INetwork::Initialize();

  // Create timer handler for constant functionatily
  _pTimerHandler = new CCoreTimerHandler;
  _pTimer->AddHandler(_pTimerHandler);

  // Load client log
  IClientLogging::LoadLog();

  // Initialize Steam API
  GetSteamAPI()->Init();

  // Load Core plugins
  GetAPI()->LoadPlugins(PLF_ENGINE);
};

// Clean up Core module (always before 'SE_EndEngine'!)
void ClassicsPatch_EndCore(void) {
  // Save configuration properties
  CCoreAPI::Props().Save();

  // Release all loaded plugins
  GetAPI()->ReleasePlugins(PLF_ALL);

  // Shutdown Steam API
  GetSteamAPI()->End();

  _pTimer->RemHandler(_pTimerHandler);
  delete _pTimerHandler;
};
