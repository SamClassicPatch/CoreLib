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

// [Cecil] NOTE: This file contains global switches for Classics Patch features that can be toggled
// in order to disable building of specific code. Useful for porting code to another engine (e.g. SSR or 1.50).

#ifndef CECIL_INCL_CLASSICSPATCH_CONFIG_H
#define CECIL_INCL_CLASSICSPATCH_CONFIG_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// [Cecil] TEMP: Feature should be disabled for Revolution
#define NOT_REV (SE1_GAME != SS_REV)

// CoreLib

  // Definitions
  #define CLASSICSPATCH_CLASS_DEFINITIONS (1) // Define classes that aren't exported from the engine
  #define CLASSICSPATCH_FUNC_DEFINITIONS  (1) // Define methods that aren't exported from the engine

  // Networking
  #define CLASSICSPATCH_EXT_PACKETS  (1 && NOT_REV) // Support for custom extension packets
  #define CLASSICSPATCH_GUID_MASKING (1 && NOT_REV) // System for masking GUIDs of player characters separately for each server client

  // Query
  #define CLASSICSPATCH_NEW_QUERY (1 && NOT_REV) // Utilize new query manager and switch master servers

// EnginePatches

  // Global
  #define CLASSICSPATCH_ENGINEPATCHES (1) // Patch any engine functions at all or not
  #define CLASSICSPATCH_CONVERT_MAPS  (1) // Implement functionality for converting loaded worlds

  // Patch modules
  #define CLASSICSPATCH_EXTEND_ENTITIES   (1) // Extend entities functionality by patching their methods
  #define CLASSICSPATCH_EXTEND_FILESYSTEM (1) // Extend file system functionality by patching its methods
  #define CLASSICSPATCH_EXTEND_NETWORK    (1) // Extend networking functionality by patching its methods
  #define CLASSICSPATCH_FIX_RENDERING     (1) // Fix FOV and other rendering issues by patching methods
  #define CLASSICSPATCH_FIX_SKA           (1 && NOT_REV) // Fix SKA issues by patching methods
  #define CLASSICSPATCH_FIX_STRINGS       (1) // Fix CTString methods by patching them
  #define CLASSICSPATCH_EXTEND_TEXTURES   (1) // Extend texture functionality by patching its methods
  #define CLASSICSPATCH_FIX_LOGICTIMERS   (1 && NOT_REV) // Fix imprecise timers for entity logic
  #define CLASSICSPATCH_FIX_STREAMPAGING  (1 && NOT_REV) // Fix streams by patching their paging methods (no need for 1.10)

#undef NOT_REV

#endif
