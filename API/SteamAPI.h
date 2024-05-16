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

#ifndef CECIL_INCL_STEAMAPI_H
#define CECIL_INCL_STEAMAPI_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#if CLASSICSPATCH_STEAM_API

// Compatibility with C++11
#define nullptr NULL
#define snprintf _snprintf
typedef int int32_t;
typedef __int64 int64_t;
typedef size_t intptr_t;
#include <Extras/Steamworks/public/steam/steam_api.h>

#else

// For compatibility
enum ESteamAPIInitResult {
  k_ESteamAPIInitResult_NoSteamClient = 2,
};

#endif // CLASSICSPATCH_STEAM_API

// API for interacting with the Steam client
class CORE_API CSteamAPI {
  private:
    // Current Steam API state
    BOOL bInitialized;
    ESteamAPIInitResult eApiState;

    BOOL bSteamOverlay;
    BOOL bScreenshotRequested; // Requested to take a new screenshot

  public:
    CTString strJoinCommandMidGame;
    CImageInfo iiScreenshot; // Custom screenshot to write (can be modified externally)

  public:
    // Constructor
    CSteamAPI();

    // Initialize Steam API
    void Init(void);

    // Shutdown Steam API
    void End(void);

    // Check if Steam has been initialized and can be used
    BOOL IsUsable(void);

    // Interact with Steam once in a while
    void Update(void);

  private:
    // Reset the API
    void Reset(void);

  public:

    // Check if Steam overlay is currently active
    inline BOOL IsOverlayOn(void) {
      return bSteamOverlay;
    };

    // Set server address to be used in "Join game" option
    void SetJoinAddress(const CTString &strAddress);

    // Activate Steam Overlay web browser directly to the specified URL
    BOOL OpenWebPage(const char *strURL);

    // Set drawport that will be used for making Steam screenshots from within the game
    // This makes Steam send screenshot requests instead of capturing the entire game window automatically
    void SetScreenshotHook(CDrawPort *pdpScreenshotSurface);

    // Make a custom Steam screenshot by manually writing a bitmap from image info
    void WriteScreenshot(CImageInfo &ii);

  public:
  #if CLASSICSPATCH_STEAM_API

    // Update Steam callbacks (should be called each frame/timer tick)
    void UpdateCallbacks(void);

    STEAM_CALLBACK_MANUAL(CSteamAPI, OnGameOverlayActivated, GameOverlayActivated_t, cbOnGameOverlayActivated);
    STEAM_CALLBACK_MANUAL(CSteamAPI, OnGameJoinRequested, GameRichPresenceJoinRequested_t, cbOnGameJoinRequested);
    STEAM_CALLBACK_MANUAL(CSteamAPI, OnScreenshotRequested, ScreenshotRequested_t, cbOnScreenshotRequested);
    STEAM_CALLBACK_MANUAL(CSteamAPI, OnScreenshotReady, ScreenshotReady_t, cbOnScreenshotReady);

  #endif // CLASSICSPATCH_STEAM_API
};

#endif
