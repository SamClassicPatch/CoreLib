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

void CExtEntityDelete::Write(CNetworkMessage &nm) {
  WriteEntity(nm);
  nm.WriteBits(&bSameClass, 1);
};

void CExtEntityDelete::Read(CNetworkMessage &nm) {
  ReadEntity(nm);

  bSameClass = FALSE;
  nm.ReadBits(&bSameClass, 1);
};

void CExtEntityDelete::Process(void) {
  CEntity *pen = GetEntity();

  if (pen == NULL) return;

  // Reset last entity
  if (CExtEntityCreate::penLast == pen) {
    CExtEntityCreate::penLast = NULL;
  }

  // Delete all entities of the same class
  if (bSameClass) {
    CEntities cenDestroy;
    IWorld::GetEntitiesOfClass(IWorld::GetWorld(), cenDestroy, pen);

    const INDEX ctEntities = cenDestroy.Count();

    ExtServerReport(TRANS("Deleted %d \"%s\" entities\n"), ctEntities, pen->en_pecClass->ec_pdecDLLClass);

    FOREACHINDYNAMICCONTAINER(cenDestroy, CEntity, itenDestroy) {
      itenDestroy->Destroy();
    }

  // Delete this entity
  } else {
    ExtServerReport(TRANS("Deleted %u entity\n"), pen->en_ulID);
    pen->Destroy();
  }
};

#endif // CLASSICSPATCH_EXT_PACKETS