/* Copyright (c) 2023 Dreamy Cecil
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

void CExtEntityPosition::Write(CNetworkMessage &nm) {
  WriteEntity(nm);
  nm.WriteBits(&bRotation, 1);

  if (bRotation) {
    INetCompress::Angle(nm, vSet(1));
    INetCompress::Angle(nm, vSet(2));
    INetCompress::Angle(nm, vSet(3));

  } else {
    INetCompress::Float(nm, vSet(1));
    INetCompress::Float(nm, vSet(2));
    INetCompress::Float(nm, vSet(3));
  }

  nm.WriteBits(&bRelative, 1);
};

void CExtEntityPosition::Read(CNetworkMessage &nm) {
  ReadEntity(nm);

  bRotation = FALSE;
  nm.ReadBits(&bRotation, 1);

  if (bRotation) {
    INetDecompress::Angle(nm, vSet(1));
    INetDecompress::Angle(nm, vSet(2));
    INetDecompress::Angle(nm, vSet(3));

  } else {
    INetDecompress::Float(nm, vSet(1));
    INetDecompress::Float(nm, vSet(2));
    INetDecompress::Float(nm, vSet(3));
  }

  bRelative = FALSE;
  nm.ReadBits(&bRelative, 1);
};

void CExtEntityPosition::Process(void) {
  CEntity *pen = GetEntity();

  if (pen == NULL) return;

  CPlacement3D pl = pen->GetPlacement();

  // Relative to absolute axes
  if (bRelative) {
    if (bRotation) {
      pl.pl_OrientationAngle += vSet;
    } else {
      pl.pl_PositionVector += vSet;
    }

  // Absolute position or rotation
  } else {
    if (bRotation) {
      pl.pl_OrientationAngle = vSet;
    } else {
      pl.pl_PositionVector = vSet;
    }
  }

  pen->Teleport(pl, FALSE);

  ExtServerReport(TRANS("Teleported %u entity to [%.2f, %.2f, %.2f;  %.2f, %.2f, %.2f]\n"), pen->en_ulID,
    pl.pl_PositionVector(1),   pl.pl_PositionVector(2),   pl.pl_PositionVector(3),
    pl.pl_OrientationAngle(1), pl.pl_OrientationAngle(2), pl.pl_OrientationAngle(3));
};

#endif // CLASSICSPATCH_EXT_PACKETS
