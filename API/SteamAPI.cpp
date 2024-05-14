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

#include "StdH.h"

// [Cecil] NOTE: Delay loaded, meaning that the library is actually hooked upon calling any function for the first time
#pragma comment(lib, "../Extras/Steamworks/redistributable_bin/steam_api.lib")

// Constructor
CSteamAPI::CSteamAPI() : bInitialized(FALSE), eApiState(k_ESteamAPIInitResult_NoSteamClient)
{
};

// Initialize Steam API
void CSteamAPI::Init(void) {
  // Skip for dedicated servers
  if (GetAPI()->IsServerApp()) {
    if (!CCoreAPI::Props().bSteamForServers) return;

  // Skip for tools
  } else if (GetAPI()->IsEditorApp() || GetAPI()->IsModelerApp()) {
    if (!CCoreAPI::Props().bSteamForTools) return;

  // Skip for unknown non-game applications
  } else if (!GetAPI()->IsGameApp()) {
    return;
  }

  // Steam disabled or already initialized
  if (!CCoreAPI::Props().bSteamEnable || bInitialized) return;
  bInitialized = TRUE;

  // Check if the module is even available
  HINSTANCE hApiLib = LoadLibraryA("steam_api.dll");

  if (hApiLib == NULL) {
    CPrintF("Failed to load 'steam_api.dll': %s\n", GetWindowsError(GetLastError()));
    return;
  }

  CPutString("Successfully loaded 'steam_api.dll'!\n");

  // Create file with Steam application ID
  static BOOL bAppIdFile = TRUE;

  if (bAppIdFile) {
    bAppIdFile = FALSE;

    FILE *file = fopen("steam_appid.txt", "w");

    if (file != NULL) {
      #define SAM_APP_ID CHOOSE_FOR_GAME("41050", "41060", "41060")
      fprintf(file, SAM_APP_ID);
      fclose(file);

      CPrintF(TRANS("Successfully created 'steam_appid.txt' with %s app ID!\n"), SAM_APP_ID);

    } else {
      CPrintF(TRANS("Cannot create 'steam_appid.txt': %s\n"), GetWindowsError(GetLastError()));
    }
  }

  // Try to initialize Steam API
  CPutString("Initializing Steam API... ");

  SteamErrMsg strError;
  eApiState = SteamAPI_InitEx(&strError);

  if (eApiState != k_ESteamAPIInitResult_OK) {
    CPrintF("Failed:\n  %s\n", strError);
    return;
  }

  CPutString("OK!\n");
};

// Shutdown Steam API
void CSteamAPI::End(void) {
  if (!IsUsable()) return;

  // Shut down Steam API
  CPutString("Shutting down Steam API... ");
  SteamAPI_Shutdown();
  CPutString("OK!\n");

  bInitialized = FALSE;
  eApiState = k_ESteamAPIInitResult_NoSteamClient;
};

// Check if Steam has been initialized and can be used
BOOL CSteamAPI::IsUsable(void) {
  // Enabled and initialized correctly
  return (CCoreAPI::Props().bSteamEnable && bInitialized && eApiState == k_ESteamAPIInitResult_OK);
};
