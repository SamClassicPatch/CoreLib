/* Copyright (c) 2024 Dreamy Cecil
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

#include "Networking/ExtPackets.h"

#if CLASSICSPATCH_EXT_PACKETS

#if CLASSICSPATCH_GAMEPLAY_EXT

// Structure that describes one field inside CCoreVariables::GameplayExt
struct GexOffset {
  ULONG ulType; // 0 - INDEX, 1 - FLOAT, 2 - CTString
  const char *strName;
  size_t ulOffset;
};

// Define a list of all GEX variables
#define GEX_OFFSET(_Type, _Variable) { _Type, #_Variable, offsetof(CCoreVariables::GameplayExt, _Variable) }

static const GexOffset _aVarOffsets[] = {
  { -1, "", -1 }, // Invalid
  GEX_OFFSET(0, bGameplayExt),
  GEX_OFFSET(0, bFixTimers),
  GEX_OFFSET(0, bUnlimitedAirControl),
  GEX_OFFSET(1, fMoveSpeed),
  GEX_OFFSET(1, fJumpHeight),
  GEX_OFFSET(1, fGravityAcc),
};

static const UWORD _ctVarOffsets = ARRAYCOUNT(_aVarOffsets);

#endif // CLASSICSPATCH_GAMEPLAY_EXT

// Find variable index by its name
UWORD CExtGameplayExt::FindVar(const CTString &strVar) {
#if CLASSICSPATCH_GAMEPLAY_EXT

  for (UWORD i = 1; i < _ctVarOffsets; i++) {
    if (strVar == _aVarOffsets[i].strName) return i;
  }

#endif // CLASSICSPATCH_GAMEPLAY_EXT

  ExtServerReport(TRANS("Couldn't find index for '%s' gameplay extension!\n"), strVar.str_String);
  return 0;
};

// Set string value
void CExtGameplayExt::SetValue(const CTString &strVar, const CTString &str) {
  iVar = FindVar(strVar);
  bString = TRUE;
  strValue = str;
};

// Set number value
void CExtGameplayExt::SetValue(const CTString &strVar, DOUBLE f) {
  iVar = FindVar(strVar);
  bString = FALSE;
  fValue = f;
};

void CExtGameplayExt::Write(CNetworkMessage &nm) {
#if CLASSICSPATCH_GAMEPLAY_EXT

  // It's not like there will ever be more than 16383 GEX variables,
  // plus if 'fValue' is 0, it'll all be neatly packed in just 2 bytes!
  nm.WriteBits(&iVar, 14);
  nm.WriteBits(&bString, 1);

  if (bString) {
    nm << strValue;
  } else {
    INetCompress::Double(nm, fValue);
  }

#endif // CLASSICSPATCH_GAMEPLAY_EXT
};

void CExtGameplayExt::Read(CNetworkMessage &nm) {
#if CLASSICSPATCH_GAMEPLAY_EXT

  iVar = 0;
  bString = FALSE;

  nm.ReadBits(&iVar, 14);
  nm.ReadBits(&bString, 1);

  if (bString) {
    nm >> strValue;
  } else {
    INetDecompress::Double(nm, fValue);
  }

#endif // CLASSICSPATCH_GAMEPLAY_EXT
};

void CExtGameplayExt::Process(void) {
#if CLASSICSPATCH_GAMEPLAY_EXT

  // Invalid offset
  if (iVar <= 0 || iVar >= _ctVarOffsets) return;

  const GexOffset &off = _aVarOffsets[iVar];

  // Got a number but expected a string
  if (!bString && off.ulType == 2) {
    ExtServerReport(TRANS("Expected a string value for '%s' gameplay extension!\n"), off.strName);
    return;

  // Got a string but expected a number
  } else if (bString && off.ulType != 2) {
    ExtServerReport(TRANS("Expected a number value for '%s' gameplay extension!\n"), off.strName);
    return;
  }

  // Set new value depending on type
  UBYTE *pField = ((UBYTE *)&CoreGEX()) + off.ulOffset;

  switch (off.ulType) {
    case 0: *((INDEX    *)pField) = fValue; break;
    case 1: *((FLOAT    *)pField) = fValue; break;
    case 2: *((CTString *)pField) = strValue; break;
  }

#else
  ASSERTALWAYS("CLASSICSPATCH_GAMEPLAY_EXT is turned off in this build!");
  ExtServerReport("CLASSICSPATCH_GAMEPLAY_EXT is turned off in this build!");
#endif // CLASSICSPATCH_GAMEPLAY_EXT
};

#endif // CLASSICSPATCH_EXT_PACKETS
