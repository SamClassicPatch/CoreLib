/* Copyright (c) 2023 Dreamy Cecil
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

// Patch-independent Core data
class CCoreVariables {
  public:
    // Gameplay extensions
    struct GameplayExt {
      BOOL bGameplayExt; // Utilize gameplay extensions (resets other settings to vanilla, if disabled)
      BOOL bFixTimers; // Fix timers for entity logic to delay slowdown bug

      BOOL bUnlimitedAirControl; // Let players move while in air indefinitely
      FLOAT fMoveSpeed; // Movement speed multiplier
      FLOAT fJumpHeight; // Jump height multiplier

      // Default constructor
      GameplayExt(BOOL bVanilla = TRUE) {
        Reset(bVanilla);
      };

      // Reset settings
      void Reset(BOOL bVanilla) {
        // Vanilla-compatible settings
        if (bVanilla) {
          bGameplayExt = FALSE;
          bFixTimers = FALSE;

        // Recommended settings
        } else {
          bGameplayExt = TRUE;
          bFixTimers = TRUE;
        }

        // Default settings
        bUnlimitedAirControl = FALSE;
        fMoveSpeed = 1.0f;
        fJumpHeight = 1.0f;
      };

      // Assignment operator
      GameplayExt &operator=(const GameplayExt &other) {
        // Copy all data
        memcpy(this, &other, sizeof(GameplayExt));
        return *this;
      };
    };

  public:
    // Pointer to CCoreAPI
    void *pAPI;

    // Variable data (changed by Classics Patch)
    GameplayExt gex;

  public:
    // Default constructor
    CCoreVariables() : pAPI(NULL), gex(TRUE)
    {
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
