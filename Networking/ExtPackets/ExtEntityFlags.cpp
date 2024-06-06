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

#include "Networking/ExtPackets.h"

#if _PATCHCONFIG_EXT_PACKETS

void CExtEntityFlags::Write(CNetworkMessage &nm) {
  WriteEntity(nm);
  INetCompress::Integer(nm, ulFlags);
  
  nm.WriteBits(&ubType, 2);
  nm.WriteBits(&bRemove, 1);
};

void CExtEntityFlags::Read(CNetworkMessage &nm) {
  ReadEntity(nm);
  INetDecompress::Integer(nm, ulFlags);

  ubType = 0;
  nm.ReadBits(&ubType, 2);

  bRemove = FALSE;
  nm.ReadBits(&bRemove, 1);
};

void CExtEntityFlags::Process(void) {
  CEntity *pen = GetEntity();

  if (!EntityExists(pen)) return;

  ULONG *pulFlags = &pen->en_ulFlags;
  CTString strReport = TRANS("Changed flags of %u entity: 0x%08X -> 0x%08X\n");

  if (ubType == 1) {
    pulFlags = &pen->en_ulPhysicsFlags;
    strReport = TRANS("Changed physical flags of %u entity: 0x%08X -> 0x%08X\n");

  } else if (ubType == 2) {
    pulFlags = &pen->en_ulCollisionFlags;
    strReport = TRANS("Changed collision flags of %u entity: 0x%08X -> 0x%08X\n");
  }

  const ULONG ulOld = *pulFlags;

  if (bRemove) {
    *pulFlags &= ~ulFlags;
  } else {
    *pulFlags |= ulFlags;
  }

  ExtServerReport(strReport, pen->en_ulID, ulOld, *pulFlags);
};

#endif // _PATCHCONFIG_EXT_PACKETS
