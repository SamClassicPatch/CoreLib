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

#include "Base/CoreTimerHandler.h"

#include "Networking/AntiFlood.h"
#include "Networking/SplitScreenClients.h"

// Pointer to the Game module
CGame *_pGame = NULL;

// Common game variables
#ifdef SE1_TFE
  CTString sam_strFirstLevel = "Levels\\01_Hatshepsut.wld";
  CTString sam_strIntroLevel = "Levels\\Intro.wld";
  CTString sam_strGameName = "serioussam";
#else
  CTString sam_strFirstLevel = "Levels\\LevelsMP\\1_0_InTheLastEpisode.wld";
  CTString sam_strIntroLevel = "Levels\\LevelsMP\\Intro.wld";
  CTString sam_strGameName = "serioussamse";
#endif

CTString sam_strVersion = _SE_VER_STRING; // Use version string

// Display information about the Classics patch
static void PatchInfo(void) {
  static CTString strInfo =
    "\nSerious Sam Classics Patch"
    "\ngithub.com/SamClassicPatch"
    "\n"
    "\n- Engine version: " _SE_VER_STRING
    "\n- Patch version: "
    + GetAPI()->GetVersion()
    + "\n\n(c) Dreamy Cecil, 2022\n";

  CPutString(strInfo);
};

// Initialize Core module (always after Serious Engine!)
void CECIL_InitCore(void) {
  // Create core API
  new CCoreAPI();

  // Information about the patch
  _pShell->DeclareSymbol("user void PatchInfo(void);", &PatchInfo);

  // Function patches
  CPutString("--- Core: Intercepting Engine functions ---\n");

  extern void CECIL_ApplyMasterServerPatch(void);
  extern void CECIL_ApplyRenderPatch(void);
  extern void CECIL_ApplyUndecoratedPatch(void);
  CECIL_ApplyMasterServerPatch();
  CECIL_ApplyRenderPatch();
  CECIL_ApplyUndecoratedPatch();

  CPutString("--- Done! ---\n");

  // Common symbols
  if (GetAPI()->IsGameApp() || GetAPI()->IsServerApp())
  {
    // Game variables
    _pShell->DeclareSymbol("           user CTString sam_strFirstLevel;", &sam_strFirstLevel);
    _pShell->DeclareSymbol("persistent user CTString sam_strIntroLevel;", &sam_strIntroLevel);
    _pShell->DeclareSymbol("persistent user CTString sam_strGameName;",   &sam_strGameName);
    _pShell->DeclareSymbol("           user CTString sam_strVersion;",    &sam_strVersion);

    // Server commands
    _pShell->DeclareSymbol("persistent user INDEX ser_bEnableAntiFlood;",      &ser_bEnableAntiFlood);
    _pShell->DeclareSymbol("persistent user INDEX ser_iPacketFloodThreshold;", &ser_iPacketFloodThreshold);
    _pShell->DeclareSymbol("persistent user INDEX ser_iMaxMessagesPerSecond;", &ser_iMaxMessagesPerSecond);
    _pShell->DeclareSymbol("persistent user INDEX ser_iMaxPlayersPerClient;",  &ser_iMaxPlayersPerClient);
  }

  // Create timer handler for constant functionatily
  _pTimerHandler = new CCoreTimerHandler;
  _pTimer->AddHandler(_pTimerHandler);

  // Load Core plugins
  GetAPI()->LoadPlugins(CPluginAPI::PF_ENGINE);
};

// Clean up Core module (always before Serious Engine!)
void CECIL_EndCore(void) {
  // Release all loaded plugins
  GetAPI()->ReleasePlugins(CPluginAPI::PF_UTILITY_ALL);

  _pTimer->RemHandler(_pTimerHandler);
  delete _pTimerHandler;
};
