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

#ifndef CECIL_INCL_GAMEAPI_H
#define CECIL_INCL_GAMEAPI_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// Declare certain classes | Which files to include to define classes
class CHighScoreEntry; // #include <Game/Game.h>
class CControls;       // #include <Game/Game.h>

// API class for the Game library
class CORE_API CGameAPI {
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
        iIndex(iSetIndex), strName(TRANSV(strSetName))
      {
      };
    };
    
  public:
    // Session properties
    CStaticArray<INDEX> sp_aiGameModes; // Game mode indices
    CStaticArray<Difficulty> sp_aGameDifficulties; // Game difficulties

    BOOL bGameHooked; // CGame fields have been hooked

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
    
  public:
    // Constructor
    CGameAPI();

    // Hook default fields from CGame
    virtual void HookFields(void);

    // Check if CGame fields have been hooked
    virtual BOOL IsHooked(void) {
      return bGameHooked;
    };

    // Set hooked state (for custom field hooking)
    virtual void SetHooked(BOOL bState) {
      bGameHooked = bState;
    };

    // Get game mode index
    virtual INDEX GetGameMode(INDEX i) const {
      return sp_aiGameModes[i];
    };

    // Get game difficulty index
    virtual INDEX GetDifficultyIndex(INDEX i) const {
      return sp_aGameDifficulties[i].iIndex;
    };

    // Get game difficulty name
    virtual const CTString &GetDifficultyName(INDEX i) const {
      return sp_aGameDifficulties[i].strName;
    };

    // Get amount of available local players
    virtual INDEX GetLocalPlayerCount(void) const {
      return ctLocalPlayers;
    };

    // Get amount of available player profiles
    virtual INDEX GetProfileCount(void) const {
      return ctPlayerProfiles;
    };

    // Start new game
    BOOL NewGame(const CTString &strSession, const CTFileName &fnmWorld, class CSessionProperties &sp);

    // Stop current game
    void StopGame(void);

  // Shell symbol wrappers
  public:

    // Get name of a specific gamemode
    virtual CTString GetGameTypeNameSS(INDEX iGameMode);

    // Get name of the current gamemode
    virtual CTString GetCurrentGameTypeNameSS(void);

    // Get spawn flags of a specific gamemode
    virtual ULONG GetSpawnFlagsForGameTypeSS(INDEX iGameMode);

    // Check if some menu is enabled
    virtual BOOL IsMenuEnabledSS(const CTString &strMenu);

  // CGame field wrappers
  public:

    // Get console state
    virtual INDEX GetConState(void) const {
      return *piConsoleState;
    };

    // Set console state
    virtual void SetConState(INDEX iState) {
      *piConsoleState = iState;
    };

    // Get computer state
    virtual INDEX GetCompState(void) const {
      return *piComputerState;
    };

    // Set computer state
    virtual void SetCompState(INDEX iState) {
      *piComputerState = iState;
    };
    
    // Get custom level filename
    virtual const CTString &GetCustomLevel(void) const {
      return *pstrCustomLevel;
    };
    
    // Set custom level filename
    virtual void SetCustomLevel(const CTString &strLevel) {
      *pstrCustomLevel = strLevel;
    };
    
    // Get session name
    virtual const CTString &GetSessionName(void) const {
      return *pstrSessionName;
    };
    
    // Set session name
    virtual void SetSessionName(const CTString &strName) {
      *pstrSessionName = strName;
    };
    
    // Get address for joining
    virtual const CTString &GetJoinAddress(void) const {
      return *pstrJoinAddress;
    };
    
    // Set address for joining
    virtual void SetJoinAddress(const CTString &strAddress) {
      *pstrJoinAddress = strAddress;
    };

    // Get name of some axis
    virtual const CTString &GetAxisName(INDEX iAxis) const {
      return astrAxisNames[iAxis];
    };

    // Set name of some axis
    virtual void SetAxisName(INDEX iAxis, const CTString &strAxis) {
      astrAxisNames[iAxis] = strAxis;
    };

    // Get one of the high score entries
    virtual CHighScoreEntry *GetHighScore(INDEX iEntry) const;

    // Get index of the last set high score
    virtual INDEX GetLastSetHighScore(void) const {
      return *piLastSetHighScore;
    };

    // Set index of the last set high score
    virtual void SetLastSetHighScore(INDEX iHighScore) {
      *piLastSetHighScore = iHighScore;
    };

    // Get player character for some profile
    virtual CPlayerCharacter *GetPlayerCharacter(INDEX iPlayer) const {
      return &apcPlayers[iPlayer];
    };

    // Get extra controls
    virtual CControls *GetControls(void) const {
      return pctrlControlsExtra;
    };

    // Get actions of extra controls
    virtual const CListHead &GetControlsActions(void) const;

    // Get player index for singleplayer
    virtual INDEX GetPlayerForSP(void) const {
      return *piSinglePlayer;
    };

    // Set player index for singleplayer
    virtual void SetPlayerForSP(INDEX iPlayer) {
      *piSinglePlayer = iPlayer;
    };

    // Get menu split screen configuration
    virtual INDEX GetMenuSplitCfg(void) const {
      return *piMenuSplitCfg;
    };

    // Set menu split screen configuration
    virtual void SetMenuSplitCfg(INDEX iConfiguration) {
      *piMenuSplitCfg = iConfiguration;
    };

    // Get start split screen configuration
    virtual INDEX GetStartSplitCfg(void) const {
      return *piStartSplitCfg;
    };

    // Set start split screen configuration
    virtual void SetStartSplitCfg(INDEX iConfiguration) {
      *piStartSplitCfg = iConfiguration;
    };

    // Get current split screen configuration
    virtual INDEX GetCurrentSplitCfg(void) const {
      return *piCurrentSplitCfg;
    };

    // Set current split screen configuration
    virtual void SetCurrentSplitCfg(INDEX iConfiguration) {
      *piCurrentSplitCfg = iConfiguration;
    };

    // Get game state
    virtual BOOL IsGameOn(void) const {
      return *pbGameOn;
    };

    // Set game state
    virtual void SetGameState(BOOL bState) {
      *pbGameOn = bState;
    };

    // Get menu state
    virtual BOOL IsMenuOn(void) const {
      return *pbMenuOn;
    };

    // Set menu state
    virtual void SetMenuState(BOOL bState) {
      *pbMenuOn = bState;
    };

    // Get first loading state
    virtual BOOL GetFirstLoading(void) const {
      return *pbFirstLoading;
    };

    // Set if loading for the first time
    virtual void SetFirstLoading(BOOL bState) {
      *pbFirstLoading = bState;
    };

    // Set network provider
    virtual void SetNetworkProvider(ENetworkProvider eProvider) {
      static const char *astrProviders[3] = {
        "Local", "TCP/IP Server", "TCP/IP Client",
      };

      *pstrNetProvider = astrProviders[eProvider];
    };

    // Get menu player index
    virtual INDEX GetMenuPlayer(INDEX i) const {
      return aiMenuLocalPlayers[i];
    };

    // Set menu player index
    virtual void SetMenuPlayer(INDEX i, INDEX iPlayer) {
      aiMenuLocalPlayers[i] = iPlayer;
    };

    // Get start player index
    virtual INDEX GetStartPlayer(INDEX i) const {
      return aiStartLocalPlayers[i];
    };

    // Set start player index
    virtual void SetStartPlayer(INDEX i, INDEX iPlayer) {
      aiStartLocalPlayers[i] = iPlayer;
    };

    // Reset all start players
    virtual void ResetStartPlayers(void) {
      for (INDEX iPlayer = 0; iPlayer < GetLocalPlayerCount(); iPlayer++) {
        SetStartPlayer(iPlayer, -1);
      }
    };

    // Copy menu player indices into start player indices
    virtual void SetStartPlayersFromMenuPlayers(void) {
      for (INDEX iPlayer = 0; iPlayer < GetLocalPlayerCount(); iPlayer++) {
        SetStartPlayer(iPlayer, GetMenuPlayer(iPlayer));
      }
    };

    // Check if local player is active
    virtual BOOL IsLocalPlayerActive(INDEX iPlayer) const {
      UBYTE *pLocalPlayer = aLocalPlayers + (lpOffsets.ctSize * iPlayer);
      BOOL *pbActive = (BOOL *)(pLocalPlayer + lpOffsets.slActive);

      return *pbActive;
    };

    // Get index of a local player
    virtual INDEX GetLocalPlayerIndex(INDEX iPlayer) const {
      UBYTE *pLocalPlayer = aLocalPlayers + (lpOffsets.ctSize * iPlayer);
      INDEX *piPlayer = (INDEX *)(pLocalPlayer + lpOffsets.slPlayer);

      return *piPlayer;
    };
};

#endif
