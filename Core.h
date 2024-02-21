/* Copyright (c) 2022-2024 Dreamy Cecil
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

// Include Serious Engine and other common components
#include <CoreLib/Base/CommonCore.h>

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

// Compatibility with vanilla Game interface
#ifndef CORE_NO_GAME_HEADER
  #include <CoreLib/Compatibility/Game.h>
#endif

CORE_API extern class CGame *_pGame;

// Revolution support
#if SE1_GAME == SS_REV
  // Pre-1.10 variables removed from Revolution engine
  CORE_API extern CTFileName _fnmCDPath;
  CORE_API extern CTFileName _fnmMod;
  CORE_API extern CTString _strModName;
  CORE_API extern CTString _strModURL;
  CORE_API extern CTString _strModExt;
#endif

// Common game variables
CORE_API extern CTString sam_strFirstLevel;
CORE_API extern CTString sam_strIntroLevel;
CORE_API extern CTString sam_strGameName;
CORE_API extern CTString sam_strVersion;

// Temporary password for connecting to some server
CORE_API extern CTString cli_strConnectPassword;

// Current values of input axes
CORE_API extern FLOAT inp_afAxisValues[MAX_OVERALL_AXES];

// Own macros to replace SERVER_CLIENTS, NET_MAXGAMECOMPUTERS, NET_MAXGAMEPLAYERS and NET_MAXLOCALPLAYERS
#define CORE_MAX_SERVER_CLIENTS 16
#define CORE_MAX_GAME_COMPUTERS CORE_MAX_SERVER_CLIENTS // 16 in vanilla
#define CORE_MAX_GAME_PLAYERS   CORE_MAX_SERVER_CLIENTS // 16 in vanilla
#define CORE_MAX_LOCAL_PLAYERS  CORE_MAX_SERVER_CLIENTS // 4 in vanilla

// Common components
#include <CoreLib/API/CoreAPI.h>
#include <CoreLib/Base/ObserverCamera.h>
#include <CoreLib/Base/GameDirectories.h>
#include <CoreLib/Modules/PluginStock.h>

#include <CoreLib/Interfaces/ConfigFunctions.h>
#include <CoreLib/Interfaces/RenderFunctions.h>
#include <CoreLib/Interfaces/WorldFunctions.h>

// Initialize Core module (always after 'SE_InitEngine'!)
CORE_API void ClassicsPatch_InitCore(void);

// Clean up Core module (always before 'SE_EndEngine'!)
CORE_API void ClassicsPatch_EndCore(void);
