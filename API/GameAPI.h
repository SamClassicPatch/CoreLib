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
    
  // Only virtual and defined methods can be used outside the Classics patch
  public:
    // Constructor
    CGameAPI();

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

  // CGame field wrappers
  public:
    // Get console state
    INDEX GetConState(void) {
      return _pGame->gm_csConsoleState;
    };

    // Set console state
    void SetConState(INDEX iState) {
      (INDEX &)_pGame->gm_csConsoleState = iState;
    };

    // Get computer state
    INDEX GetCompState(void) {
      return _pGame->gm_csComputerState;
    };

    // Set computer state
    void SetCompState(INDEX iState) {
      (INDEX &)_pGame->gm_csComputerState = iState;
    };

    // Set network provider
    void SetNetworkProvider(ENetworkProvider eProvider) {
      static const char *astrProviders[3] = {
        "Local", "TCP/IP Server", "TCP/IP Client",
      };

      _pGame->gm_strNetworkProvider = astrProviders[eProvider];
    };

    // Get first loading state
    BOOL GetFirstLoading(void) {
      return _pGame->gm_bFirstLoading;
    };

    // Set if loading for the first time
    void SetFirstLoading(BOOL bState) {
      _pGame->gm_bFirstLoading = bState;
    };

    // Get menu state
    BOOL GetMenuState(void) {
      return _pGame->gm_bMenuOn;
    };

    // Set menu state
    void SetMenuState(BOOL bState) {
      _pGame->gm_bMenuOn = bState;
    };

    // Get game state
    BOOL GetGameState(void) {
      return _pGame->gm_bGameOn;
    };

    // Set game state
    void SetGameState(BOOL bState) {
      _pGame->gm_bGameOn = bState;
    };

  // CGame session property wrappers
  public:
    // Get custom level filename
    CTString &GetCustomLevel(void) {
      return _pGame->gam_strCustomLevel;
    };
    
    // Get session name
    CTString &GetSessionName(void) {
      return _pGame->gam_strSessionName;
    };
    
    // Get address for joining
    CTString &GetJoinAddress(void) {
      return _pGame->gam_strJoinAddress;
    };
};

#endif
