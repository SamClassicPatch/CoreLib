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

// Display information about the patch
static void PatchInfo(void) {
  static CTString strInfo =
    "\nSerious Sam Classic Patch"
    "\ngithub.com/SamClassicPatch"
    "\n"
    "\n- Engine version: " _SE_VER_STRING
    "\n- Patch version: "
    + _pPatchAPI->strVersion
    + "\n\n(c) Dreamy Cecil, 2022\n";

  CPutString(strInfo);
};

// Initialize Core module
void CECIL_InitCore(void) {
  // Initialize patch API
  _pPatchAPI = new CPatchAPI();

  // Information about the patch
  _pShell->DeclareSymbol("user void PatchInfo(void);", &PatchInfo);

  // Function patches
  CPrintF("^c00ffffCore:\nIntercepting Engine functions:\n");

  extern void CECIL_ApplyMasterServerPatch(void);
  CECIL_ApplyMasterServerPatch();

  CPrintF("^c00ffffDone!\n");

  // Common game variables
  _pShell->DeclareSymbol("           user CTString sam_strFirstLevel;", &sam_strFirstLevel);
  _pShell->DeclareSymbol("persistent user CTString sam_strIntroLevel;", &sam_strIntroLevel);
  _pShell->DeclareSymbol("persistent user CTString sam_strGameName;",   &sam_strGameName);
};

// Disable GameSpy usage
void CECIL_DisableGameSpy(void) {
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
