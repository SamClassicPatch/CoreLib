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

void CExtEntityParent::Write(CNetworkMessage &nm) {
  WriteEntity(nm);
  INetCompress::Integer(nm, ulParent);
};

void CExtEntityParent::Read(CNetworkMessage &nm) {
  ReadEntity(nm);
  INetDecompress::Integer(nm, ulParent);
};

void CExtEntityParent::Process(void) {
  CEntity *pen = GetEntity();

  if (!EntityExists(pen)) return;

  CEntity *penParent = FindExtEntity(ulParent);
  pen->SetParent(penParent);

  if (pen->GetParent() == NULL) {
    ExtServerReport(TRANS("Unparented %u entity\n"), pen->en_ulID);

  } else {
    ExtServerReport(TRANS("Parented %u entity to %u\n"), pen->en_ulID, penParent->en_ulID);
  }
};

#endif // CLASSICSPATCH_EXT_PACKETS
