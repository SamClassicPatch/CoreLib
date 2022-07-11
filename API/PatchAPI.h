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

#ifndef CECIL_INCL_PATCHAPI_H
#define CECIL_INCL_PATCHAPI_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// Declare certain classes | Which files to include to define classes
class CPatch; // #include <CoreLib/Patcher/patcher.h>

// Pointer to a function patch under a hashed name
struct SFuncPatch {
  CTString strName; // Patch name
  ULONG ulHash; // Name hash

  CPatch *pPatch; // Pointer to the patch
  
  // Default constructor
  SFuncPatch() : strName(""), ulHash(0), pPatch(NULL)
  {
  };

  // Constructor from name and patch
  SFuncPatch(const CTString &strSetName, CPatch *pSetPatch) :
    strName(strSetName), pPatch(pSetPatch)
  {
    // Calculate name hash
    ulHash = strName.GetHash();
  };
};

// API for handling function patches
class CPatchAPI {
  public:
    CStaticStackArray<SFuncPatch> aPatches; // Function patch storage
    
  public:
    // Constructor
    CPatchAPI();

    // Enable specific function patch
    virtual BOOL EnablePatch(INDEX iPatch);

    // Disable specific function patch
    virtual void DisablePatch(INDEX iPatch);

    // Find function patch index by its name
    virtual INDEX FindPatch(const CTString &strName);
};

#endif
