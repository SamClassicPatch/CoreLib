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

// Main engine components
#include <Engine/Engine.h>
#include <Engine/CurrentVersion.h>

// Components for compatibility
#include "Compatibility/Game.h"
#include "Compatibility/SymbolPtr.h"

// CSessionProperties byte container (replacement for CUniversalSessionProperties)
typedef UBYTE CSesPropsContainer[NET_MAXSESSIONPROPERTIES];

// Import library for use
#ifndef CORE_EXPORTS
  #pragma comment(lib, "Core.lib")
#endif

// Import zlib
#ifndef CORE_NO_ZLIB
  #pragma comment(lib, "zlib.lib")
#endif

// Pointer to the Game module
extern CGame *_pGame;

// Common game variables
extern CTString sam_strFirstLevel;
extern CTString sam_strIntroLevel;
extern CTString sam_strGameName;
extern CTString sam_strVersion;

// Game-specific definitions
#include "GameSpecific.h"

// Common components
#include "API/CoreAPI.h"
#include "Patcher/FuncPatching.h"
#include "Query/QueryMgr.h"
#include "Modules/PluginStock.h"

#include "Interfaces/RenderFunctions.h"
#include "Interfaces/WorldFunctions.h"

// Initialize Core module (always after Serious Engine!)
void CECIL_InitCore(void);

// Clean up Core module (always before Serious Engine!)
void CECIL_EndCore(void);
