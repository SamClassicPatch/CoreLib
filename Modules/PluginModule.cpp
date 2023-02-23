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

#include "StdH.h"

#include "PluginModule.h"

// Current plugin in the process of initialization
CPluginModule *_pInitializingPlugin = NULL;

// Constructor
CPluginModule::CPluginModule() {
  ResetFields();
};

// Destructor
CPluginModule::~CPluginModule() {
  Clear();
};

// Module initialization
void CPluginModule::Initialize(void) {
  if (IsInitialized()) return;

  // Set to the current plugin
  CPluginModule *pLastPlugin = _pInitializingPlugin;
  _pInitializingPlugin = this;

  // Start the plugin
  OnStartup();

  // Restore last plugin
  _pInitializingPlugin = pLastPlugin;

  pm_bInitialized = TRUE;
};

// Module deactivation
void CPluginModule::Deactivate(void) {
  if (!IsInitialized()) return;

  // Destroy all patches
  for (INDEX i = 0; i < pm_cPatches.Count(); i++) {
    CPatch *pPatch = pm_cPatches.Pointer(i);

    // Remove from the storage
    SFuncPatch *pfp = GetPatchAPI()->FindFuncPatch("", pPatch);
    ASSERTMSG(pfp == NULL, "Trying to remove a function patch that's not in the patch storage!");

    if (pfp != NULL) {
      GetPatchAPI()->aPatches.Delete(pfp);
    }

    // Remove the patch
    delete pPatch;
  }

  pm_cPatches.Clear();

  // Stop the plugin
  OnShutdown();

  pm_bInitialized = FALSE;
};

// Reset class fields
void CPluginModule::ResetFields(void) {
  pm_hLibrary = NULL;
  pm_bInitialized = FALSE;

  pm_pOnStartupFunc = NULL;
  pm_pOnShutdownFunc = NULL;
  pm_pGetInfoFunc = NULL;
};

// Add new function patch
void CPluginModule::AddPatch(CPatch *pPatch) {
  // Add if it's not there yet
  if (!pm_cPatches.IsMember(pPatch)) {
    pm_cPatches.Add(pPatch);
  }
};

// Call startup method
void CPluginModule::OnStartup(void) {
  if (pm_pOnStartupFunc != NULL) {
    pm_pOnStartupFunc();
  }
};

// Call shutdown method
void CPluginModule::OnShutdown(void) {
  if (pm_pOnShutdownFunc != NULL) {
    pm_pOnShutdownFunc();
  }
};

// Module cleanup
void CPluginModule::Clear(void) {
  // Release DLL
  if (pm_hLibrary != NULL) {
    Deactivate();
    FreeLibrary(pm_hLibrary);
  }

  // Reset class
  ResetFields();
  CSerial::Clear();
};

// Load dynamic link library
static HINSTANCE LoadLibrary_t(const char *strFileName) {
  // Load plugin library
  HINSTANCE hiDLL = ::LoadLibraryA(strFileName);

  // Loaded properly
  if (hiDLL != NULL) {
    return hiDLL;
  }

  // Get the error code
  DWORD dwMessageId = GetLastError();

  // Format the Windows error message
  LPVOID lpMsgBuf;
  DWORD dwSuccess = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
    NULL, dwMessageId, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
    (LPSTR)&lpMsgBuf, 0, NULL);

  CTString strWinError;

  // If formatting succeeds
  if (dwSuccess != 0) {
    // Copy the result
    strWinError = (char *)lpMsgBuf;

    // Free the Windows message buffer
    LocalFree(lpMsgBuf);

  // Otherwise report failure
  } else {
    strWinError.PrintF(TRANS(
      "Cannot format error message!\nOriginal error code: %d\nFormatting error code: %d\n"),
      dwMessageId, GetLastError());
  }

  // Report error
  ThrowF_t(TRANS("Cannot load module '%s':\n%s"), strFileName, strWinError);
  return NULL;
};

// Load plugin module (override non-virtual CSerial::Load_t)
void CPluginModule::Load_t(const CTFileName &fnmDLL)
{
  ASSERT(!IsUsed());

  // Mark that just changed
  MarkChanged();

  // Remember filename
  ser_FileName = fnmDLL;

  // Load library from file
  CTFileName fnmExpanded;
  ExpandFilePath(EFP_READ | EFP_NOZIPS, fnmDLL, fnmExpanded);

  // Load dll
  pm_hLibrary = LoadLibrary_t(fnmExpanded);

  // Main plugin methods
  pm_pOnStartupFunc  = (CVoidFunc)GetProcAddress(GetHandle(), "Module_Startup");
  pm_pOnShutdownFunc = (CVoidFunc)GetProcAddress(GetHandle(), "Module_Shutdown");
  pm_pGetInfoFunc    = (CInfoFunc)GetProcAddress(GetHandle(), "Module_GetInfo");

  // Get information about the plugin, if possible
  if (pm_pGetInfoFunc != NULL) {
    pm_pGetInfoFunc(&pm_info);
  }
};
