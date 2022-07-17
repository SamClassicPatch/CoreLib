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
#include "Game/Game.h"

// CSessionProperties byte container (replacement for CUniversalSessionProperties)
typedef UBYTE CSesPropsContainer[NET_MAXSESSIONPROPERTIES];

// Import library for use
#ifndef CORE_EXPORTS
  #pragma comment(lib, "Core.lib")
#endif

// Pointer to the Game module
extern CGame *_pGame;

// Choose value based on configuration
#ifdef SE1_TFE
  #define CHOOSE_FOR_GAME(_TFE105, _TSE105, _TSE107) _TFE105
#elif SE1_VER == 105
  #define CHOOSE_FOR_GAME(_TFE105, _TSE105, _TSE107) _TSE105
#else
  #define CHOOSE_FOR_GAME(_TFE105, _TSE105, _TSE107) _TSE107
#endif

// Common game variables
extern CTString sam_strFirstLevel;
extern CTString sam_strIntroLevel;
extern CTString sam_strGameName;
extern CTString sam_strVersion;

// Common components
#include "API/CoreAPI.h"
#include "Patcher/FuncPatching.h"
#include "Query/QueryMgr.h"
#include "Modules/PluginStock.h"

#include "Interfaces/RenderFunctions.h"
#include "Interfaces/WorldFunctions.h"

// Initialize Core module
void CECIL_InitCore(void);

// Clean up Core module
void CECIL_EndCore(void);
