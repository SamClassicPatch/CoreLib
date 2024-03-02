/* Copyright (c) 2023-2024 Dreamy Cecil
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

#ifndef CECIL_INCL_COREVARIABLES_H
#define CECIL_INCL_COREVARIABLES_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// Shell symbol name
#define CORE_VARIABLE_DATA_SYMBOL CTString("CoreVarData")

// Config file
#define CORE_VARIABLE_DATA_CONFIG_FILE CTString("Data\\ClassicsPatch\\ModData.ini")

// Maximum amount of difficulties the game can have
#define MAX_GAME_DIFFICULTIES 16

// Patch-independent Core data
class CCoreVariables {
  public:
    // Gameplay extensions
    struct GameplayExt {
      BOOL bEnable; // Utilize gameplay extensions (resets other settings to vanilla, if disabled)
      BOOL bFixTimers; // Fix timers for entity logic to delay slowdown bug

      BOOL bUnlimitedAirControl; // Let players move while in air indefinitely
      FLOAT fMoveSpeed; // Movement speed multiplier
      FLOAT fJumpHeight; // Jump height multiplier
      FLOAT fGravityAcc; // Gravity acceleration multiplier

      // Default constructor
      GameplayExt(BOOL bVanilla = TRUE) {
        Reset(bVanilla);
      };

      // Reset settings
      void Reset(BOOL bVanilla) {
        // Vanilla-compatible settings
        if (bVanilla) {
          bEnable = FALSE;
          bFixTimers = FALSE;

        // Recommended settings
        } else {
          bEnable = TRUE;
          bFixTimers = TRUE;
        }

        // Default settings
        bUnlimitedAirControl = FALSE;
        fMoveSpeed = 1.0f;
        fJumpHeight = 1.0f;
        fGravityAcc = 1.0f;
      };

      // Assignment operator
      GameplayExt &operator=(const GameplayExt &other) {
        // Copy all data
        memcpy(this, &other, sizeof(GameplayExt));
        return *this;
      };
    };

    // Mod difficuties
    struct Difficulty {
      INDEX iLevel; // Difficulty level (for gam_iStartDifficulty)
      CTString strName; // Display name

      // Menu button settings
      CTString strTip; // Difficulty description
      CTString strCommand; // Allow difficulty selection if the value is TRUE
      BOOL bFlash; // Make text blink

      // Default constructor
      Difficulty() : iLevel(0), strName(""), strTip(""), strCommand(""), bFlash(FALSE)
      {
      };

      // Constructor with an index and a translated name
      Difficulty(INDEX iSetLevel, const char *strSetName, const char *strSetTip, const char *strSetCommand = "") :
        iLevel(iSetLevel), strName(strSetName), strTip(strSetTip), strCommand(strSetCommand), bFlash(FALSE)
      {
      };

      // Check the command to see if the difficulty is active
      BOOL IsActive(void) const {
        // Always active if no command
        if (strCommand == "") return TRUE;

        CShellSymbol *pss = _pShell->GetSymbol(strCommand, TRUE);

        // Always active if no symbol
        if (pss == NULL) return TRUE;

        return *(INDEX *)pss->ss_pvValue;
      };
    };

  public:
    // Pointer to CCoreAPI
    void *pAPI;

    // Variable data (changed by Classics Patch)
    GameplayExt &gex;

    // Modifiable data (changed by user)
    Difficulty aGameDiffs[MAX_GAME_DIFFICULTIES]; // Game difficulties

    BOOL bAdjustFOV; // Let Classics Patch adjust FOV depending on aspect ratio
    BOOL bAdjustAR; // Let Classics Patch set wide adjustment based on aspect ratio
    BOOL bProperTextScaling; // Adjust scaling of menu text based on screen height instead of width
    FLOAT fMenuTextScale; // Multiply scale of menu text

  public:
    // Default constructor
    CCoreVariables() : pAPI(NULL), gex(*new GameplayExt(TRUE)),
      bAdjustFOV(TRUE), bAdjustAR(TRUE), bProperTextScaling(TRUE), fMenuTextScale(1.0f)
    {
      ResetGameDiffs();
    };

    // Load user configs for customization
    void LoadConfigs(void);

  public:
    // Clear difficulty list
    void ClearGameDiffs(INDEX iFrom = 0) {
      for (INDEX i = iFrom; i < MAX_GAME_DIFFICULTIES; i++) {
        aGameDiffs[i] = Difficulty();
      }
    };

    // Set default game difficulties
    void ResetGameDiffs(void) {
      aGameDiffs[0] = Difficulty(-1, "Tourist", "for non-FPS players");
      aGameDiffs[1] = Difficulty( 0, "Easy",    "for unexperienced FPS players");
      aGameDiffs[2] = Difficulty( 1, "Normal",  "for experienced FPS players");
      aGameDiffs[3] = Difficulty( 2, "Hard",    "for experienced Serious Sam players");
      aGameDiffs[4] = Difficulty( 3, "Serious", "are you serious?");

      aGameDiffs[5] = Difficulty( 4, "Mental",  "you are not serious!", "sam_bMentalActivated");
      aGameDiffs[5].bFlash = TRUE;

      // Clear the rest
      ClearGameDiffs(6);
    };

    // Get game difficulty
    const Difficulty &GetDiff(INDEX i) const {
      return aGameDiffs[i];
    };

    // Get amount of difficulties
    INDEX CountDiffs(void) const {
      INDEX i = 0;

      for (; i < MAX_GAME_DIFFICULTIES; i++) {
        // No more difficulties
        if (aGameDiffs[i].strName == "") break;
      }

      return i;
    };

    // Find difficulty index by its level
    INDEX FindDiffByLevel(INDEX iLevel) const {
      for (INDEX i = 0; i < MAX_GAME_DIFFICULTIES; i++) {
        // Matching level
        if (aGameDiffs[i].iLevel == iLevel) return i;
      }

      return -1;
    };
};

// Get variable data through the shell symbol
// To increase performance, call this method just once and save the value in some variable
inline CCoreVariables *GetVarData(void) {
  CShellSymbol *pssAPI = _pShell->GetSymbol(CORE_VARIABLE_DATA_SYMBOL, TRUE);

  if (pssAPI != NULL) {
    return (CCoreVariables *)pssAPI->ss_pvValue;
  }

  return NULL;
};

#endif
