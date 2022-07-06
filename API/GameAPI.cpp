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

// Constructor
CGameAPI::CGameAPI() {
  // Session properties game modes used by the patch
  sp_aiGameModes.New(2);
  sp_aiGameModes[0] = -1; // Flyover
  sp_aiGameModes[1] =  0; // Cooperative
  
  // Session properties difficulties used by the patch
  sp_aGameDifficulties.New(6);
  sp_aGameDifficulties[0] = Difficulty(-1, "Tourist");
  sp_aGameDifficulties[1] = Difficulty( 0, "Easy");
  sp_aGameDifficulties[2] = Difficulty( 1, "Normal");
  sp_aGameDifficulties[3] = Difficulty( 2, "Hard");
  sp_aGameDifficulties[4] = Difficulty( 3, "Serious");
  sp_aGameDifficulties[5] = Difficulty( 4, "Mental");
};

// Hook default fields from CGame
void CGameAPI::HookFields(void) {
  piConsoleState  = (INDEX *)&_pGame->gm_csConsoleState;
  piComputerState = (INDEX *)&_pGame->gm_csComputerState;
  pstrNetProvider = &_pGame->gm_strNetworkProvider;
  pbFirstLoading  = &_pGame->gm_bFirstLoading;
  pbMenuOn        = &_pGame->gm_bMenuOn;
  pbGameOn        = &_pGame->gm_bGameOn;

  pstrCustomLevel = &_pGame->gam_strCustomLevel;
  pstrSessionName = &_pGame->gam_strSessionName;
  pstrJoinAddress = &_pGame->gam_strJoinAddress;
};
