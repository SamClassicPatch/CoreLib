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

#ifndef CECIL_INCL_FUNCPATCHING_H
#define CECIL_INCL_FUNCPATCHING_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include "patcher.h"
#include <CoreLib/Objects/StructPtr.h>

// Don't terminate the game in debug
#ifndef NDEBUG
  #define PATCH_ERROR_OUTPUT InfoMessage
#else
  #define PATCH_ERROR_OUTPUT FatalError
#endif

// Create a new function patch (using initialized API)
template<class FuncType1, class FuncType2> inline
CPatch *NewPatch(FuncType1 &funcOld, FuncType2 funcNew, const char *strName, BOOL bAddToRegistry = TRUE) {
  CPrintF("  %s\n", strName);

  // Create new patch and hook the functions
  CPatch *pPatch = GetPatchAPI()->CreatePatch(FALSE);

  long &iNewCallAddress = *reinterpret_cast<long *>(&funcOld);
  long iJumpAddress = *reinterpret_cast<long *>(&funcNew);
  pPatch->HookFunction(iNewCallAddress, iJumpAddress, &iNewCallAddress, true);

  // Successfully patched
  if (pPatch->IsValid()) {
    // Add to the patch registry
    if (bAddToRegistry) {
      GetPatchAPI()->aPatches.Push() = SFuncPatch(strName, pPatch);
    }

  // Couldn't patch
  } else {
    PATCH_ERROR_OUTPUT("Cannot set function patch for %s!\nAddress: 0x%p", strName, funcOld);
  }

  return pPatch;
};

// Create a new function patch (before initializing API)
template<class FuncType1, class FuncType2> inline
CPatch *NewRawPatch(FuncType1 &funcOld, FuncType2 funcNew, const char *strName) {
  CPrintF("  %s\n", strName);

  // Create new patch and hook the functions
  CPatch *pPatch = new CPatch(FALSE);

  long &iNewCallAddress = *reinterpret_cast<long *>(&funcOld);
  long iJumpAddress = *reinterpret_cast<long *>(&funcNew);
  pPatch->HookFunction(iNewCallAddress, iJumpAddress, &iNewCallAddress, true);

  // Couldn't patch
  if (!pPatch->IsValid()) {
    PATCH_ERROR_OUTPUT("Cannot set function patch for %s!\nAddress: 0x%p", strName, funcOld);
  }

  return pPatch;
};

// Create a new function patch for the plugin
template<class FuncType1, class FuncType2> inline
CPatch *NewPluginPatch(FuncType1 &funcOld, FuncType2 funcNew, const char *strName, BOOL bAddToRegistry = TRUE) {
  // Add it to the plugin
  CPatch *pPatch = NewPatch(funcOld, funcNew, strName, bAddToRegistry);
  GetPluginAPI()->AddNewPatch(pPatch);

  return pPatch;
};

// Check if the current function is being called from a specific address
__forceinline BOOL CallingFrom(const ULONG ulFrom, const INDEX ctDepth) {
  // Create thread context
  CONTEXT context;
  ZeroMemory(&context, sizeof(CONTEXT));
  context.ContextFlags = CONTEXT_CONTROL;

  // Retrieve call stack
  __asm {
  Label:
    mov [context.Ebp], ebp
    mov [context.Esp], esp
    mov eax, [Label]
    mov [context.Eip], eax
  }

  DWORD ulCallAddress = context.Eip;

  PDWORD pFrame = (PDWORD)context.Ebp;
  PDWORD pPrevFrame = NULL;

  // Iterate through the last N calls from here
  for (INDEX iDepth = 0; iDepth < ctDepth; iDepth++)
  {
    // Calling from the right address
    if (ulCallAddress == ulFrom) {
      return TRUE;
    }

    // Get next call address
    ulCallAddress = pFrame[1];

    // Advance the frame
    pPrevFrame = pFrame;
    pFrame = (PDWORD)pFrame[0];

    if ((DWORD)pFrame & 3) break;
    if (pFrame <= pPrevFrame) break;

    if (IsBadWritePtr(pFrame, sizeof(PVOID) * 2)) break;
  }

  return FALSE;
};

#endif
