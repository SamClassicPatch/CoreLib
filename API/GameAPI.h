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

#ifndef CECIL_INCL_GAMEAPI_H
#define CECIL_INCL_GAMEAPI_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// API class for the Game library
class CGameAPI {
  public:
    // Network provider type for CGame
    enum ENetworkProvider {
      NP_LOCAL,
      NP_SERVER,
      NP_CLIENT,
    };

    // Difficuties for CSessionProperties
    struct Difficulty {
      INDEX iIndex;
      CTString strName;

      // Default constructor
      Difficulty() : iIndex(0), strName("")
      {
      };

      // Constructor with an index and a translated name
      Difficulty(INDEX iSetIndex, const char *strSetName) :
        iIndex(iSetIndex), strName(TranslateConst(strSetName))
      {
      };
    };
    
  public:
    // Session properties
    CStaticArray<INDEX> sp_aiGameModes; // Game mode indices
    CStaticArray<Difficulty> sp_aGameDifficulties; // Game difficulties

    // Field offsets within CLocalPlayer
    struct LocalPlayerOffsets {
      ULONG ctSize; // sizeof(CLocalPlayer)

      SLONG slActive; // lp_bActive
      SLONG slPlayer; // lp_iPlayer
    } lpOffsets;

    // Amounts of available local players and player profiles for iteration
    INDEX ctLocalPlayers;   // 4
    INDEX ctPlayerProfiles; // 8

    // Pointers to CGame fields
    INDEX    *piConsoleState;  // gm_csConsoleState
    INDEX    *piComputerState; // gm_csComputerState

    CTString *pstrCustomLevel; // gam_strCustomLevel
    CTString *pstrSessionName; // gam_strSessionName
    CTString *pstrJoinAddress; // gam_strJoinAddress

    CTString         *astrAxisNames;      // gm_astrAxisNames[0]
    CHighScoreEntry  *ahseHighScores;     // gm_ahseHighScores[0]
    INDEX            *piLastSetHighScore; // gm_iLastSetHighScore
    CPlayerCharacter *apcPlayers;         // gm_apcPlayers[0]
    CControls        *pctrlControlsExtra; // gm_ctrlControlsExtra
    INDEX            *piSinglePlayer;     // gm_iSinglePlayer

    INDEX    *piMenuSplitCfg;      // gm_MenuSplitScreenCfg
    INDEX    *piStartSplitCfg;     // gm_StartSplitScreenCfg
    INDEX    *piCurrentSplitCfg;   // gm_CurrentSplitScreenCfg

    BOOL     *pbGameOn;            // gm_bGameOn
    BOOL     *pbMenuOn;            // gm_bMenuOn
    BOOL     *pbFirstLoading;      // gm_bFirstLoading

    CTString *pstrNetProvider;     // gm_strNetworkProvider

    INDEX    *aiMenuLocalPlayers;  // gm_aiMenuLocalPlayers[0]
    INDEX    *aiStartLocalPlayers; // gm_aiStartLocalPlayers[0]
    UBYTE    *aLocalPlayers;       // gm_lpLocalPlayers[0]
    
  // Only virtual and defined methods can be used outside the Classics patch
  public:
    // Constructor
    CGameAPI();

    // Hook default fields from CGame
    virtual void HookFields(void);

    // Get game mode index
    INDEX GetGameMode(INDEX i) {
      return sp_aiGameModes[i];
    };

    // Get game difficulty index
    INDEX GetDifficultyIndex(INDEX i) {
      return sp_aGameDifficulties[i].iIndex;
    };

    // Get game difficulty name
    const CTString &GetDifficultyName(INDEX i) {
      return sp_aGameDifficulties[i].strName;
    };

    // Get amount of available local players
    INDEX GetLocalPlayerCount(void) {
      return ctLocalPlayers;
    };

    // Get amount of available player profiles
    INDEX GetProfileCount(void) {
      return ctPlayerProfiles;
    };

  // CGame field wrappers
  public:
    // Get console state
    INDEX GetConState(void) {
      return *piConsoleState;
    };

    // Set console state
    void SetConState(INDEX iState) {
      *piConsoleState = iState;
    };

    // Get computer state
    INDEX GetCompState(void) {
      return *piComputerState;
    };

    // Set computer state
    void SetCompState(INDEX iState) {
      *piComputerState = iState;
    };
    
    // Get custom level filename
    CTString &GetCustomLevel(void) {
      return *pstrCustomLevel;
    };
    
    // Get session name
    CTString &GetSessionName(void) {
      return *pstrSessionName;
    };
    
    // Get address for joining
    CTString &GetJoinAddress(void) {
      return *pstrJoinAddress;
    };

    // Get name of some axis
    const CTString &GetAxisName(INDEX iAxis) {
      return astrAxisNames[iAxis];
    };

    // Get one of the high score entries
    CHighScoreEntry *GetHighScore(INDEX iEntry) {
      return &ahseHighScores[iEntry];
    };

    // Get index of the last set high score
    INDEX GetLastSetHighScore(void) {
      return *piLastSetHighScore;
    };

    // Get player character for some profile
    CPlayerCharacter *GetPlayerCharacter(INDEX iPlayer) {
      return &apcPlayers[iPlayer];
    };

    // Get extra controls
    CControls *GetControls(void) {
      return pctrlControlsExtra;
    };

    // Get actions of extra controls
    const CListHead &GetControlsActions(void) {
      return GetControls()->ctrl_lhButtonActions;
    };

    // Get player index for singleplayer
    INDEX GetPlayerForSP(void) {
      return *piSinglePlayer;
    };

    // Set player index for singleplayer
    void SetPlayerForSP(INDEX iPlayer) {
      *piSinglePlayer = iPlayer;
    };

    // Get menu split screen configuration
    INDEX GetMenuSplitCfg(void) {
      return *piMenuSplitCfg;
    };

    // Set menu split screen configuration
    void SetMenuSplitCfg(INDEX iConfiguration) {
      *piMenuSplitCfg = iConfiguration;
    };

    // Get start split screen configuration
    INDEX GetStartSplitCfg(void) {
      return *piStartSplitCfg;
    };

    // Set start split screen configuration
    void SetStartSplitCfg(INDEX iConfiguration) {
      *piStartSplitCfg = iConfiguration;
    };

    // Get current split screen configuration
    INDEX GetCurrentSplitCfg(void) {
      return *piCurrentSplitCfg;
    };

    // Set current split screen configuration
    void SetCurrentSplitCfg(INDEX iConfiguration) {
      *piCurrentSplitCfg = iConfiguration;
    };

    // Get game state
    BOOL GetGameState(void) {
      return *pbGameOn;
    };

    // Set game state
    void SetGameState(BOOL bState) {
      *pbGameOn = bState;
    };

    // Get menu state
    BOOL GetMenuState(void) {
      return *pbMenuOn;
    };

    // Set menu state
    void SetMenuState(BOOL bState) {
      *pbMenuOn = bState;
    };

    // Get first loading state
    BOOL GetFirstLoading(void) {
      return *pbFirstLoading;
    };

    // Set if loading for the first time
    void SetFirstLoading(BOOL bState) {
      *pbFirstLoading = bState;
    };

    // Set network provider
    void SetNetworkProvider(ENetworkProvider eProvider) {
      static const char *astrProviders[3] = {
        "Local", "TCP/IP Server", "TCP/IP Client",
      };

      *pstrNetProvider = astrProviders[eProvider];
    };

    // Get menu player index
    INDEX GetMenuPlayer(INDEX i) {
      return aiMenuLocalPlayers[i];
    };

    // Set menu player index
    void SetMenuPlayer(INDEX i, INDEX iPlayer) {
      aiMenuLocalPlayers[i] = iPlayer;
    };

    // Get start player index
    INDEX GetStartPlayer(INDEX i) {
      return aiStartLocalPlayers[i];
    };

    // Set start player index
    void SetStartPlayer(INDEX i, INDEX iPlayer) {
      aiStartLocalPlayers[i] = iPlayer;
    };

    // Check if local player is active
    BOOL IsLocalPlayerActive(INDEX iPlayer) {
      UBYTE *pLocalPlayer = aLocalPlayers + (lpOffsets.ctSize * iPlayer);
      BOOL *pbActive = (BOOL *)(pLocalPlayer + lpOffsets.slActive);

      return *pbActive;
    };

    // Get index of a local player
    INDEX GetLocalPlayerIndex(INDEX iPlayer) {
      UBYTE *pLocalPlayer = aLocalPlayers + (lpOffsets.ctSize * iPlayer);
      INDEX *piPlayer = (INDEX *)(pLocalPlayer + lpOffsets.slPlayer);

      return *piPlayer;
    };
};

#endif
