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

#if _PATCHCONFIG_EXT_PACKETS

// [Cecil] NOTE: Assume that NET_MAXSESSIONPROPERTIES is 2048
#define SESPROPS_BITFIT 11

// Set new data at the current end and expand session properties size
BOOL CExtSessionProps::AddData(const void *pData, size_t ctBytes) {
  // Not enough space
  if (slSize + ctBytes > sizeof(sp)) return FALSE;

  // Copy data to the current offset and add its size
  memcpy(sp + slSize, pData, ctBytes);
  slSize += ctBytes;
  return TRUE;
};

void CExtSessionProps::Write(CNetworkMessage &nm) {
  // From first to last byte
  slOffset = Clamp(slOffset, (SLONG)0, SLONG(sizeof(sp) - 1));
  nm.WriteBits(&slOffset, SESPROPS_BITFIT);

  // Fit as much data as there's space after offset
  slSize = Clamp(slSize, (SLONG)0, SLONG(sizeof(sp) - slOffset));
  nm.WriteBits(&slSize, SESPROPS_BITFIT);

  for (INDEX i = 0; i < slSize; i++) {
    nm << sp[i];
  }
};

void CExtSessionProps::Read(CNetworkMessage &nm) {
  nm.ReadBits(&slOffset, SESPROPS_BITFIT);
  nm.ReadBits(&slSize, SESPROPS_BITFIT);

  for (INDEX i = 0; i < slSize; i++) {
    nm >> sp[i];
  }
};

void CExtSessionProps::Process(void) {
  // No data or past the limits
  if (slSize == 0 || slOffset < 0 || slOffset >= sizeof(sp)) return;

  // Fit as much data as there's space after offset
  slSize = Clamp(slSize, (SLONG)0, SLONG(sizeof(sp) - slOffset));

  // Copy received data from the beginning into current session properties with some offset
  if (slSize != 0) {
    UBYTE *pubProps = (UBYTE *)_pNetwork->GetSessionProperties();
    memcpy(pubProps + slOffset, sp, slSize);
  }
};

#endif // _PATCHCONFIG_EXT_PACKETS
