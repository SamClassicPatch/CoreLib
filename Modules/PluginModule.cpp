/* Copyright (c) 2021-2022 by ZCaliptium.

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

//! Constructor.
CPluginModule::CPluginModule()
{
  _hiLibrary = NULL;
  pOnStartupFunc = NULL;
  pOnShutdownFunc = NULL;
}

//! Destructor
CPluginModule::~CPluginModule()
{
  Clear();
}

// Count used memory
SLONG CPluginModule::GetUsedMemory(void)
{
  return sizeof(CPluginModule);
}

// Write to stream
void CPluginModule::Write_t(CTStream *ostrFile)
{
}

// Read from stream
void CPluginModule::Read_t(CTStream *istrFile)
{
  // [Cecil] Obsolete method
  ASSERTALWAYS("Don't load plugin modules using traditional stocks! Use CPluginStock instead!");
}

// Load a Dynamic Link Library.
static HINSTANCE LoadLibrary_t(const char *strFileName)
{
  HINSTANCE hiDLL = ::LoadLibraryA(strFileName);

  // If the DLL can not be loaded
  if (hiDLL == NULL) {
    // Get the error code
    DWORD dwMessageId = GetLastError();
    
    // Format the windows error message
    LPVOID lpMsgBuf;
    DWORD dwSuccess = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        dwMessageId,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // default language
        (LPTSTR) &lpMsgBuf,
        0,
        NULL
    );
    
    CTString strWinError;
    // If formatting succeeds
    if (dwSuccess != 0) {
      // Copy the result
      strWinError = ((char *)lpMsgBuf);
      
      // Free the windows message buffer
      LocalFree(lpMsgBuf );
    } else {
      // Set our message about the failure
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
}

// [Cecil] Load plugin module manually
void CPluginModule::LoadPlugin_t(const CTFileName &fnmDLL)
{
  // [Cecil] Load library from file
  CTFileName fnmExpanded;
  ExpandFilePath(EFP_READ | EFP_NOZIPS, fnmDLL, fnmExpanded);

  // Load dll
  _hiLibrary = LoadLibrary_t(fnmExpanded);

  // [Cecil] Get startup and shutdown methods
  pOnStartupFunc  = (CVoidFunc)GetProcAddress(GetHandle(), "Module_Startup");
  pOnShutdownFunc = (CVoidFunc)GetProcAddress(GetHandle(), "Module_Shutdown");

  // [Cecil] Get other methods
  pOnStepFunc = (CVoidFunc)GetProcAddress(GetHandle(), "Module_Step");
  pOnDrawFunc = (CDrawFunc)GetProcAddress(GetHandle(), "Module_Draw");

  // [Cecil] Call startup method if it exists
  if (pOnStartupFunc != NULL) {
    pOnStartupFunc();
  }
}

// Clear modyle 
void CPluginModule::Clear(void)
{
  // Release dll
  if (_hiLibrary != NULL) {
    if (pOnShutdownFunc) {
      pOnShutdownFunc();
    }

    FreeLibrary(_hiLibrary);
  }
}
