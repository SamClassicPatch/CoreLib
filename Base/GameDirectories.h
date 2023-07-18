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

#ifndef CECIL_INCL_GAMEDIRECTORIES_H
#define CECIL_INCL_GAMEDIRECTORIES_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// Different level formats
enum ELevelFormat {
  E_LF_TFE, // Serious Sam: The First Encounter
  E_LF_TSE, // Serious Sam: The Second Encounter
  E_LF_SSR, // Serious Sam Revolution
  E_LF_150, // Games on 1.50

  // Current format
#if SE1_GAME == SS_REV
  E_LF_CURRENT = E_LF_SSR,
#elif SE1_GAME == SS_TFE
  E_LF_CURRENT = E_LF_TFE,
#else
  E_LF_CURRENT = E_LF_TSE,
#endif
};

// Other game directories
#define GAME_DIRECTORIES_CT 2
CORE_API extern CTString _astrGameDirs[GAME_DIRECTORIES_CT];

#define GAME_DIR_TFE (_astrGameDirs[0])
#define GAME_DIR_SSR (_astrGameDirs[1])

// Check if a filename is under a specified game directory
CORE_API BOOL IsFileFromDir(const CTString &strGameDir, const CTFileName &fnm);

#endif
