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

void CExtEntityMove::Write(CNetworkMessage &nm) {
  WriteEntity(nm);
  INetCompress::Float(nm, vSpeed(1));
  INetCompress::Float(nm, vSpeed(2));
  INetCompress::Float(nm, vSpeed(3));
};

void CExtEntityMove::Read(CNetworkMessage &nm) {
  ReadEntity(nm);
  INetDecompress::Float(nm, vSpeed(1));
  INetDecompress::Float(nm, vSpeed(2));
  INetDecompress::Float(nm, vSpeed(3));
};

#define REPORT_NOT_MOVABLE TRANS("not a movable entity")

void CExtEntityMove::Process(void) {
  CEntity *pen = GetEntity();

  if (!EntityExists(pen)) return;

  if (IsDerivedFromClass(pen, "MovableEntity")) {
    ((CMovableEntity *)pen)->SetDesiredTranslation(vSpeed);
    ExtServerReport(TRANS("Changed movement speed of %u entity to [%.2f, %.2f, %.2f]\n"), pen->en_ulID, vSpeed(1), vSpeed(2), vSpeed(3));

  } else {
    ExtServerReport(TRANS("Cannot change movement speed for %u entity: %s\n"), pen->en_ulID, REPORT_NOT_MOVABLE);
  }
};

void CExtEntityRotate::Process(void) {
  CEntity *pen = GetEntity();

  if (!EntityExists(pen)) return;

  if (IsDerivedFromClass(pen, "MovableEntity")) {
    ((CMovableEntity *)pen)->SetDesiredRotation(vSpeed);
    ExtServerReport(TRANS("Changed rotation speed of %u entity to [%.2f, %.2f, %.2f]\n"), pen->en_ulID, vSpeed(1), vSpeed(2), vSpeed(3));

  } else {
    ExtServerReport(TRANS("Cannot change rotation speed for %u entity: %s\n"), pen->en_ulID, REPORT_NOT_MOVABLE);
  }
};

void CExtEntityImpulse::Process(void) {
  CEntity *pen = GetEntity();

  if (!EntityExists(pen)) return;

  if (IsDerivedFromClass(pen, "MovableEntity")) {
    ((CMovableEntity *)pen)->GiveImpulseTranslationAbsolute(vSpeed);
    ExtServerReport(TRANS("Gave impulse to %u entity: [%.2f, %.2f, %.2f]\n"), vSpeed(1), vSpeed(2), vSpeed(3), pen->en_ulID);

  } else {
    ExtServerReport(TRANS("Cannot give impulse to %u entity: %s\n"), pen->en_ulID, REPORT_NOT_MOVABLE);
  }
};

#endif // CLASSICSPATCH_EXT_PACKETS
