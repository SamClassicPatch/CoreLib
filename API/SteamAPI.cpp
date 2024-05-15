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

#if CLASSICSPATCH_STEAM_API

// [Cecil] NOTE: Delay loaded, meaning that the library is actually hooked upon calling any function for the first time
#pragma comment(lib, "../Extras/Steamworks/redistributable_bin/steam_api.lib")

#endif // CLASSICSPATCH_STEAM_API

static INDEX steam_bAllowJoin = TRUE; // Allow friends to join the same game
static INDEX steam_bUseWebBrowser = TRUE; // Allow using web browser in Steam overlay

// Display debug information about interactions with Steam
// 0 - Disabled
// 1 - Only callbacks/single actions
// 2 - All interactions in real time
static INDEX steam_iDebugOutput = 0;

// Debug output macros
#define STEAM_DEBUG1 if (steam_iDebugOutput >= 1) CPrintF
#define STEAM_DEBUG2 if (steam_iDebugOutput >= 2) CPrintF

// Constructor
CSteamAPI::CSteamAPI() {
  Reset();

  // Commands for dynamically toggling Steam features
  _pShell->DeclareSymbol("persistent user INDEX steam_bAllowJoin;",     &steam_bAllowJoin);
  _pShell->DeclareSymbol("persistent user INDEX steam_bUseWebBrowser;", &steam_bUseWebBrowser);

  _pShell->DeclareSymbol("user INDEX steam_iDebugOutput;", &steam_iDebugOutput);
};

// Reset the API
void CSteamAPI::Reset(void) {
  bInitialized = FALSE;
  eApiState = k_ESteamAPIInitResult_NoSteamClient;

  bSteamOverlay = FALSE;
  strJoinCommandMidGame = "";
};

// Initialize Steam API
void CSteamAPI::Init(void) {
#if CLASSICSPATCH_STEAM_API

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

  // Register callbacks
  cbOnGameOverlayActivated.Register(this, OnGameOverlayActivated);
  cbOnGameJoinRequested.Register(this, OnGameJoinRequested);

#else
  CPutString("Steam API is disabled in this build!\n");
#endif // CLASSICSPATCH_STEAM_API
};

// Shutdown Steam API
void CSteamAPI::End(void) {
  if (!IsUsable()) return;

#if CLASSICSPATCH_STEAM_API

  // Unregister callbacks
  cbOnGameOverlayActivated.Unregister();
  cbOnGameJoinRequested.Unregister();

  // Shut down Steam API
  CPutString("Shutting down Steam API... ");
  SteamAPI_Shutdown();
  CPutString("OK!\n");

#endif // CLASSICSPATCH_STEAM_API

  Reset();
};

// Check if Steam has been initialized and can be used
BOOL CSteamAPI::IsUsable(void) {
#if CLASSICSPATCH_STEAM_API
  // Enabled and initialized correctly
  return (CCoreAPI::Props().bSteamEnable && bInitialized && eApiState == k_ESteamAPIInitResult_OK);

#else
  return FALSE;
#endif
};

// Interact with Steam once in a while
void CSteamAPI::Update(void) {
#if CLASSICSPATCH_STEAM_API

  // Update every 1/20 of a second
  static CTimerValue _tvLastCheck(-1.0f);
  CTimerValue tvNow = _pTimer->GetHighPrecisionTimer();

  if ((tvNow - _tvLastCheck).GetSeconds() < 0.05f) return;
  _tvLastCheck = tvNow;

  STEAM_DEBUG2("CSteamAPI::Update() - %fs\n", tvNow.GetSeconds());

  // Update Steam callbacks
  GetSteamAPI()->UpdateCallbacks();

  // While connected to a server
  if (!_pNetwork->IsServer() && GetGameAPI()->IsHooked() && GetGameAPI()->IsGameOn()) {
    // Set address of the joined server
    CSymbolPtr piNetPort("net_iPort");
    CTString strAddress(0, "%s:%d", GetGameAPI()->GetJoinAddress(), piNetPort.GetIndex());
    GetSteamAPI()->SetJoinAddress(strAddress);

  } else {
    // Reset join address
    GetSteamAPI()->SetJoinAddress("");
  }

#endif // CLASSICSPATCH_STEAM_API
};

// Set server address to be used in "Join game" option
void CSteamAPI::SetJoinAddress(const CTString &strAddress) {
  if (!IsUsable()) return;

#if CLASSICSPATCH_STEAM_API

  STEAM_DEBUG2("CSteamAPI::SetJoinAddress(\"%s\") - ", strAddress);

  // Reset if needed
  if (!steam_bAllowJoin || strAddress == "") {
    SteamFriends()->SetRichPresence("connect", NULL);
    STEAM_DEBUG2("reset\n");
    return;
  }

  CTString strArgs(0, "+connect %s", strAddress);

  // Add mod argument
  if (_fnmMod != "") {
    // Get mod directory name by removing last slash and "Mods\\"
    CTString strCurrentMod = _fnmMod;
    strCurrentMod.str_String[strCurrentMod.Length() - 1] = '\0';
    strCurrentMod.RemovePrefix("Mods\\");

    strArgs += " +game " + strCurrentMod;
  }

  SteamFriends()->SetRichPresence("connect", strArgs);
  STEAM_DEBUG2("set to '%s'\n", strArgs);

#endif // CLASSICSPATCH_STEAM_API
};

// Activate Steam Overlay web browser directly to the specified URL
BOOL CSteamAPI::OpenWebPage(const char *strURL) {
  if (!steam_bUseWebBrowser || !IsUsable()) return FALSE;

#if CLASSICSPATCH_STEAM_API
  SteamFriends()->ActivateGameOverlayToWebPage(strURL);
  STEAM_DEBUG1("CSteamAPI::OpenWebPage(\"%s\") - opened webpage\n", strURL);
#endif

  return TRUE;
};

#if CLASSICSPATCH_STEAM_API

// Update Steam callbacks (should be called each frame/timer tick)
void CSteamAPI::UpdateCallbacks(void) {
  if (!IsUsable()) return;

  STEAM_DEBUG2("CSteamAPI::UpdateCallbacks() - running callbacks\n");
  SteamAPI_RunCallbacks();
};

void CSteamAPI::OnGameOverlayActivated(GameOverlayActivated_t *pCallback) {
  bSteamOverlay = pCallback->m_bActive;
  STEAM_DEBUG1("CSteamAPI::OnGameOverlayActivated() - %s\n", bSteamOverlay ? "activated" : "deactivated");
};

void CSteamAPI::OnGameJoinRequested(GameRichPresenceJoinRequested_t *pCallback) {
  strJoinCommandMidGame = pCallback->m_rgchConnect;
  STEAM_DEBUG1("CSteamAPI::OnGameJoinRequested() - received '%s'\n", strJoinCommandMidGame);
};

#endif // CLASSICSPATCH_STEAM_API
