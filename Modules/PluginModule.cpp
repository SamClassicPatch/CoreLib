/* Copyright (c) 2021-2022 by ZCaliptium.
   Copyright (c) 2022 Dreamy Cecil

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

  // Start the plugin
  OnStartup();

  _bInitialized = TRUE;
};

// Module deactivation
void CPluginModule::Deactivate(void) {
  if (!IsInitialized()) return;

  // Destroy all patches
  for (INDEX i = 0; i < _cPatches.Count(); i++) {
    delete _cPatches.Pointer(i);
  }

  _cPatches.Clear();

  // Stop the plugin
  OnShutdown();

  _bInitialized = FALSE;
};

// Module cleanup
void CPluginModule::Clear(void) {
  // Release DLL
  if (_hiLibrary != NULL) {
    Deactivate();
    FreeLibrary(_hiLibrary);
  }

  // Reset class
  ResetFields();
  CSerial::Clear();
};

// Reset class fields
void CPluginModule::ResetFields(void) {
  _hiLibrary = NULL;
  _bInitialized = FALSE;

  pOnStartupFunc = NULL;
  pOnShutdownFunc = NULL;
  pGetInfoFunc = NULL;
};

// Write to stream
void CPluginModule::Write_t(CTStream *ostrFile) {
  ASSERTALWAYS("Plugin modules cannot be written!");
};

// Read from stream
void CPluginModule::Read_t(CTStream *istrFile) {
  ASSERTALWAYS("Plugin modules cannot be read! Use CPluginStock for loading plugins!");
};

// Load dynamic link library
static HINSTANCE LoadLibrary_t(const char *strFileName)
{
  HINSTANCE hiDLL = ::LoadLibraryA(strFileName);

  // If the library cannot be loaded
  if (hiDLL == NULL) {
    // Get the error code
    DWORD dwMessageId = GetLastError();
    
    // Format the Windows error message
    LPVOID lpMsgBuf;
    DWORD dwSuccess = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, dwMessageId, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (LPTSTR)&lpMsgBuf, 0, NULL
    );
    
    CTString strWinError;

    // If formatting succeeds
    if (dwSuccess != 0) {
      // Copy the result
      strWinError = ((char *)lpMsgBuf);
      
      // Free the Windows message buffer
      LocalFree(lpMsgBuf);

    } else {
      // Report failure
      CTString strError;

      strError.PrintF(
        TRANS("Cannot format error message!\n"
        "Original error code: %d,\n"
        "Formatting error code: %d.\n"),
        dwMessageId, GetLastError());

      strWinError = strError;
    }

    // Report error
    ThrowF_t(TRANS("Cannot load module '%s':\n%s"), strFileName, strWinError);
  }

  return hiDLL;
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
  _hiLibrary = LoadLibrary_t(fnmExpanded);

  // Main plugin methods
  pOnStartupFunc  = (CVoidFunc)GetProcAddress(GetHandle(), "Module_Startup");
  pOnShutdownFunc = (CVoidFunc)GetProcAddress(GetHandle(), "Module_Shutdown");
  pGetInfoFunc    = (CInfoFunc)GetProcAddress(GetHandle(), "Module_GetInfo");

  // Get information about the plugin, if possible
  if (pGetInfoFunc != NULL) {
    pGetInfoFunc(&_info);
  }
};

// Return amount of used memory in bytes
SLONG CPluginModule::GetUsedMemory(void) {
  return sizeof(CPluginModule);
};

// Call startup method
void CPluginModule::OnStartup(void) {
  if (pOnStartupFunc != NULL) {
    pOnStartupFunc();
  }
};

// Call shutdown method
void CPluginModule::OnShutdown(void) {
  if (pOnShutdownFunc != NULL) {
    pOnShutdownFunc();
  }
};
