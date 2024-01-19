/* Copyright (c) 2024 Dreamy Cecil
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

#ifndef CECIL_INCL_VOTINGSYSTEM_H
#define CECIL_INCL_VOTINGSYSTEM_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// Interface for voting via chat
namespace IVotingSystem {

// Initialize voting system
void Initialize(void);

// Update current vote (needs to be synced with the game loop)
CORE_API void UpdateVote(void);

// Load map pool from a file
CORE_API void LoadMapPool(const CTFileName &fnmMapPool);

// Add world file to the map pool
CORE_API BOOL AddMapToPool(const CTFileName &fnmWorldFile);

// Print current map pool
CORE_API void PrintMapPool(CTString &str);

// Chat commands
struct Chat {
  // Initiate voting to change a map
  static BOOL VoteMap(CTString &strResult, INDEX iClient, const CTString &strArguments);

  // Vote yes in the current vote
  static BOOL VoteYes(CTString &strResult, INDEX iClient, const CTString &strArguments);

  // Vote no in the current vote
  static BOOL VoteNo(CTString &strResult, INDEX iClient, const CTString &strArguments);
};

}; // namespace

#endif
