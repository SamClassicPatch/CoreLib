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

void CExtEntityDamage::Write(CNetworkMessage &nm) {
  WriteEntity(nm);
  INetCompress::Integer(nm, eDamageType);

  // Write damage amount up to 2 decimal places
  INetCompress::Integer(nm, fDamage * 100.0f);
};

void CExtEntityDamage::Read(CNetworkMessage &nm) {
  ReadEntity(nm);
  INetDecompress::Integer(nm, eDamageType);

  // Read damage amount
  ULONG ulDamagePoints;
  INetDecompress::Integer(nm, ulDamagePoints);
  fDamage = FLOAT(ulDamagePoints) * 0.01f;
};

void CExtEntityDirectDamage::Write(CNetworkMessage &nm) {
  CExtEntityDamage::Write(nm);

  INetCompress::Integer(nm, ulTarget);
  INetCompress::Float3D(nm, vHitPoint);
  INetCompress::Float3D(nm, vDirection);
};

void CExtEntityDirectDamage::Read(CNetworkMessage &nm) {
  CExtEntityDamage::Read(nm);

  INetDecompress::Integer(nm, ulTarget);
  INetDecompress::Float3D(nm, vHitPoint);
  INetDecompress::Float3D(nm, vDirection);
};

void CExtEntityDirectDamage::Process(void) {
  CEntity *pen = GetEntity();

  if (!EntityExists(pen)) return;

  CEntity *penTarget = FindExtEntity(ulTarget);

  if (penTarget == NULL) return;

  pen->InflictDirectDamage(penTarget, pen, (DamageType)eDamageType, fDamage, vHitPoint, vDirection);

  ExtServerReport(TRANS("Entity %u inflicted %.2f damage to entity %u\n"), pen->en_ulID, fDamage, penTarget->en_ulID);
};

void CExtEntityRangeDamage::Write(CNetworkMessage &nm) {
  CExtEntityDamage::Write(nm);

  INetCompress::Float3D(nm, vCenter);
  INetCompress::Integer(nm, fFallOff * 10.0f);
  INetCompress::Integer(nm, fHotSpot * 10.0f);
};

void CExtEntityRangeDamage::Read(CNetworkMessage &nm) {
  CExtEntityDamage::Read(nm);

  INetDecompress::Float3D(nm, vCenter);

  ULONG ulRange;
  INetDecompress::Integer(nm, ulRange);
  fFallOff = FLOAT(ulRange) * 0.1f;

  INetDecompress::Integer(nm, ulRange);
  fHotSpot = FLOAT(ulRange) * 0.1f;
};

void CExtEntityRangeDamage::Process(void) {
  CEntity *pen = GetEntity();

  if (!EntityExists(pen)) return;

  pen->InflictRangeDamage(pen, (DamageType)eDamageType, fDamage, vCenter, fHotSpot, fFallOff);

  ExtServerReport(TRANS("Entity %u inflicted %.2f damage in a %.1f range\n"), pen->en_ulID, fDamage, fFallOff);
};

void CExtEntityBoxDamage::Write(CNetworkMessage &nm) {
  CExtEntityDamage::Write(nm);

  INetCompress::Float3D(nm, boxArea.minvect);
  INetCompress::Float3D(nm, boxArea.maxvect);
};

void CExtEntityBoxDamage::Read(CNetworkMessage &nm) {
  CExtEntityDamage::Read(nm);

  INetDecompress::Float3D(nm, boxArea.minvect);
  INetDecompress::Float3D(nm, boxArea.maxvect);
};

void CExtEntityBoxDamage::Process(void) {
  CEntity *pen = GetEntity();

  if (!EntityExists(pen)) return;

  pen->InflictBoxDamage(pen, (DamageType)eDamageType, fDamage, boxArea);

  ExtServerReport(TRANS("Entity %u inflicted %.2f damage in a [%.2f, %.2f, %.2f] sized area\n"),
    pen->en_ulID, fDamage, boxArea.Size()(1), boxArea.Size()(2), boxArea.Size()(3));
};

#endif // _PATCHCONFIG_EXT_PACKETS
