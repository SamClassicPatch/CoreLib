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

bool CExtEntityTeleport::Write(CNetworkMessage &nm) {
  WriteEntity(nm);
  INetCompress::Placement(nm, plSet);

  nm.WriteBits(&bRelative, 1);
  return true;
};

void CExtEntityTeleport::Read(CNetworkMessage &nm) {
  ReadEntity(nm);
  INetDecompress::Placement(nm, plSet);

  bRelative = FALSE;
  nm.ReadBits(&bRelative, 1);
};

void CExtEntityTeleport::Process(void) {
  CEntity *pen = GetEntity();

  if (!EntityExists(pen)) return;

  CPlacement3D pl = plSet;

  // Relative to current position and orientation
  if (bRelative) {
    pl.RelativeToAbsoluteSmooth(pen->GetPlacement());
  }

  pen->Teleport(pl, FALSE);

  ClassicsPackets_ServerReport(this, TRANS("Teleported %u entity to [%.2f, %.2f, %.2f;  %.2f, %.2f, %.2f]\n"), pen->en_ulID,
    pl.pl_PositionVector(1),   pl.pl_PositionVector(2),   pl.pl_PositionVector(3),
    pl.pl_OrientationAngle(1), pl.pl_OrientationAngle(2), pl.pl_OrientationAngle(3));
};

#endif // _PATCHCONFIG_EXT_PACKETS
