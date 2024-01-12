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

// Constructor
CGameAPI::CGameAPI() {
  // Session properties game modes used by the patch
  sp_aiGameModes.New(2);
  sp_aiGameModes[0] = -1; // Flyover - for intro screen
  sp_aiGameModes[1] =  0; // Cooperative - for singleplayer

  // Fields are not hooked yet
  bGameHooked = FALSE;
};

// Hook default fields from CGame
void CGameAPI::HookFields(void) {
  lpOffsets.ctSize = sizeof(CLocalPlayer);
  lpOffsets.slActive = offsetof(CLocalPlayer, lp_bActive);
  lpOffsets.slPlayer = offsetof(CLocalPlayer, lp_iPlayer);

  ctLocalPlayers = 4;
  ctPlayerProfiles = 8;

  // CGame fields
  piConsoleState  = (INDEX *)&_pGame->gm_csConsoleState;
  piComputerState = (INDEX *)&_pGame->gm_csComputerState;

  pstrCustomLevel = &_pGame->gam_strCustomLevel;
  pstrSessionName = &_pGame->gam_strSessionName;
  pstrJoinAddress = &_pGame->gam_strJoinAddress;

  astrAxisNames      = &_pGame->gm_astrAxisNames[0];

  // [Cecil] Rev: No high score table
#if SE1_GAME != SS_REV
  ahseHighScores     = &_pGame->gm_ahseHighScores[0];
  piLastSetHighScore = &_pGame->gm_iLastSetHighScore;
#endif

  apcPlayers         = &_pGame->gm_apcPlayers[0];
  pctrlControlsExtra = &_pGame->gm_ctrlControlsExtra;
  piSinglePlayer     = &_pGame->gm_iSinglePlayer;

  piMenuSplitCfg    = (INDEX *)&_pGame->gm_MenuSplitScreenCfg;
  piStartSplitCfg   = (INDEX *)&_pGame->gm_StartSplitScreenCfg;
  piCurrentSplitCfg = (INDEX *)&_pGame->gm_CurrentSplitScreenCfg;

  pbGameOn       = &_pGame->gm_bGameOn;
  pbMenuOn       = &_pGame->gm_bMenuOn;
  pbFirstLoading = &_pGame->gm_bFirstLoading;

  pstrNetProvider = &_pGame->gm_strNetworkProvider;

  aiMenuLocalPlayers  = &_pGame->gm_aiMenuLocalPlayers[0];
  aiStartLocalPlayers = &_pGame->gm_aiStartLocalPlayers[0];
  aLocalPlayers       = (UBYTE *)&_pGame->gm_lpLocalPlayers[0];

  // Mark as hooked
  SetHooked(TRUE);
};

// Start new game
BOOL CGameAPI::NewGame(const CTString &strSession, const CTFileName &fnmWorld, CSessionProperties &sp) {
  // Stop last game for Core
  if (IsGameOn()) {
    GetAPI()->OnGameStop();
  }

  BOOL bResult = _pGame->NewGame(strSession, fnmWorld, sp);

  // [Cecil] Rev: Prepare world after starting the game
  #if SE1_GAME == SS_REV
    if (bResult) bResult = _pGame->WorldStart();
  #endif

  if (bResult) {
    // Start game for Core
    GetAPI()->OnGameStart();
  }

  return bResult;
};

// Get name of a specific gamemode
CTString CGameAPI::GetGameTypeNameSS(INDEX iGameMode)
{
  static CSymbolPtr symptr(CHOOSE_FOR_ENGINE("GetGameTypeName", "GetGameTypeName", "GetGameTypeNameSS"));

  // No symbol
  if (!symptr.Exists()) {
    ASSERT(FALSE);
    return "";
  }

  typedef CTString (*CGetNameFunc)(INDEX);
  CGetNameFunc pFunc = (CGetNameFunc)symptr.GetValue();

  // Retrieve the name
  return pFunc(iGameMode);
};

// Get name of the current gamemode
CTString CGameAPI::GetCurrentGameTypeNameSS(void)
{
  static CSymbolPtr symptr(CHOOSE_FOR_ENGINE("GetCurrentGameTypeName", "GetCurrentGameTypeName", "GetCurrentGameTypeNameSS"));

  // No symbol
  if (!symptr.Exists()) {
    ASSERT(FALSE);
    return "";
  }

  typedef CTString (*CGetCurrentNameFunc)(void);
  CGetCurrentNameFunc pFunc = (CGetCurrentNameFunc)symptr.GetValue();

  // Retrieve the name
  return pFunc();
};

// Get spawn flags of a specific gamemode
ULONG CGameAPI::GetSpawnFlagsForGameTypeSS(INDEX iGameMode)
{
  // Default to singleplayer
  if (iGameMode == -1) {
    return SPF_SINGLEPLAYER;
  }

  static CSymbolPtr symptr(CHOOSE_FOR_ENGINE("GetSpawnFlagsForGameType", "GetSpawnFlagsForGameType", "GetSpawnFlagsForGameTypeSS"));

  // No symbol
  if (!symptr.Exists()) {
    ASSERT(FALSE);
    return NONE;
  }

  typedef ULONG (*CGetSpawnFlagsFunc)(INDEX);
  CGetSpawnFlagsFunc pFunc = (CGetSpawnFlagsFunc)symptr.GetValue();

  // Retrieve the flags
  return pFunc(iGameMode);
};

// Check if some menu is enabled
BOOL CGameAPI::IsMenuEnabledSS(const CTString &strMenu)
{
  static CSymbolPtr symptr(CHOOSE_FOR_ENGINE("IsMenuEnabled", "IsMenuEnabled", "IsMenuEnabledSS"));

  // No symbol
  if (!symptr.Exists()) {
    ASSERT(FALSE);

    // All menus are enabled
    return TRUE;
  }

  typedef INDEX (*CIsMenuEnabledFunc)(const CTString &);
  CIsMenuEnabledFunc pFunc = (CIsMenuEnabledFunc)symptr.GetValue();

  // Retrieve menu state
  return pFunc(strMenu);
};

// Get one of the high score entries
CHighScoreEntry *CGameAPI::GetHighScore(INDEX iEntry) const {
  #if SE1_GAME != SS_REV
    return &ahseHighScores[iEntry];

  #else
    // [Cecil] Rev: Dummy table entry
    static const struct DummyHighScore {
      CTString strPlayer;
      INDEX iDiff;
      TIME tmTime;
      INDEX ctKills;
      INDEX ctScore;
    } hse = { "<invalid>", 0xBBD1EB, 0.0, 0, 0 };

    return (CHighScoreEntry *)&hse;
  #endif
};

// Get actions of extra controls
const CListHead &CGameAPI::GetControlsActions(void) const {
  return GetControls()->ctrl_lhButtonActions;
};
