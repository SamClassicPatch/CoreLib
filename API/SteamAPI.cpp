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

// Set if Steam API is unavailable for whatever reason
static BOOL _bUnavailable = FALSE;

// Constructor
CSteamAPI::CSteamAPI() : hApiLib(NULL)
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

  // Steam disabled, already hooked or unavailable
  if (!CCoreAPI::Props().bSteamEnable || hApiLib != NULL || _bUnavailable) return;

  // Hook library
  hApiLib = LoadLibraryA("steam_api.dll");

  if (hApiLib == NULL) {
    CPrintF(TRANS("Failed to load 'steam_api.dll': %s\n"), GetWindowsError(GetLastError()));

    _bUnavailable = TRUE;
    return;
  }

  CPutString(TRANS("Successfully loaded 'steam_api.dll'!\n"));

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

  // Hook initialization method
  bool (*pSteamInitFunc)(void) = (bool (*)(void))GetProcAddress(hApiLib, "SteamAPI_InitSafe");
  CPutString(TRANS("Initializing Steam API...\n"));

  // If method isn't available or cannot initialize
  if (pSteamInitFunc == NULL || !pSteamInitFunc()) {
    CPutString(TRANS("  failed!\n"));

    // Unhook the library
    End();
    _bUnavailable = TRUE;
    return;
  }

  CPutString(TRANS("  done!\n"));
};

// Shutdown Steam API
void CSteamAPI::End(void) {
  // Steam disabled or isn't hooked
  if (!CCoreAPI::Props().bSteamEnable || hApiLib == NULL) return;

  // Hook shutdown method
  void (*pSteamShutdownFunc)(void) = (void (*)(void))GetProcAddress(hApiLib, "SteamAPI_Shutdown");
  CPutString(TRANS("Shutting down Steam API...\n"));

  if (pSteamShutdownFunc != NULL) {
    pSteamShutdownFunc();
    CPutString(TRANS("  done!\n"));

  } else {
    CPutString(TRANS("  failed!\n"));
  }

  FreeLibrary(hApiLib);
  hApiLib = NULL;
};
