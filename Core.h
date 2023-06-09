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

// Don't warn about identifier truncation
#pragma warning(disable: 4786)

// Main engine components
#include <Engine/Engine.h>
#include <Engine/CurrentVersion.h>

// Classics Patch configuration
#include "Config.h"

// Components for compatibility
#include "Compatibility/Game.h"
#include "Compatibility/SymbolPtr.h"

// CSessionProperties byte container (replacement for CUniversalSessionProperties)
typedef UBYTE CSesPropsContainer[NET_MAXSESSIONPROPERTIES];

// Import library for use
#ifndef CORE_EXPORTS
  #pragma comment(lib, "ClassicsCore.lib")
  #pragma comment(lib, "Ws2_32.lib")

  #define CORE_API __declspec(dllimport)
#else
  #define CORE_API __declspec(dllexport)
#endif

// Import zlib
#ifndef CORE_NO_ZLIB
  #pragma comment(lib, "zlib.lib")
#endif

// Pointer to the Game module
CORE_API extern class CGame *_pGame;

// Common game variables
CORE_API extern CTString sam_strFirstLevel;
CORE_API extern CTString sam_strIntroLevel;
CORE_API extern CTString sam_strGameName;
CORE_API extern CTString sam_strVersion;

// Common components
#include "API/CoreAPI.h"
#include "Modules/PluginStock.h"

#include "Interfaces/ConfigFunctions.h"
#include "Interfaces/RenderFunctions.h"
#include "Interfaces/WorldFunctions.h"

// Initialize Core module (always after 'SE_InitEngine'!)
CORE_API void CECIL_InitCore(void);

// Clean up Core module (always before 'SE_EndEngine'!)
CORE_API void CECIL_EndCore(void);
