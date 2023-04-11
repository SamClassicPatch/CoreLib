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

#include "ExtPackets.h"
#include "NetworkFunctions.h"

#if CLASSICSPATCH_EXT_PACKETS

// Report packet actions to the server
INDEX ser_bReportExtPacketLogic = TRUE;

void CExtPacket::SendPacket(void) {
  CNetStreamBlock nsbExt = INetwork::CreateServerPacket(GetType());
  Write(nsbExt);

  INetwork::AddBlockToAllSessions(nsbExt);
};

// Make sure to return some entity from the ID
CEntity *CExtEntityPacket::GetEntity(void) {
  if (!IsEntityValid()) {
    CPutString(TRANS("Received invalid entity ID!\n"));
    return NULL;
  }

  CEntity *pen = FindExtEntity(ulEntity);

  if (pen == NULL) {
    CPutString(TRANS("Received invalid entity ID!\n"));
    return NULL;
  }

  return pen;
};

// Create new packet from type
CExtPacket *CExtPacket::CreatePacket(EType ePacket, BOOL bClient) {
  // Invalid packet
  if (ePacket < 0 || ePacket >= EXT_MAX_PACKETS) {
    ASSERT(FALSE);
    return NULL;
  }

  // From server to client
  if (bClient) {
    switch (ePacket) {
      case EXT_ENTITY_CREATE:   return new CExtEntityCreate();
      case EXT_ENTITY_DELETE:   return new CExtEntityDelete();
      case EXT_ENTITY_COPY:     return new CExtEntityCopy();
      case EXT_ENTITY_INIT:     return new CExtEntityInit();
      case EXT_ENTITY_TELEPORT: return new CExtEntityTeleport();
      case EXT_ENTITY_PARENT:   return new CExtEntityParent();
      case EXT_ENTITY_PROP:     return new CExtEntityProp();
    }
  }

  ASSERT(FALSE);
  return NULL;
};

// Register the module
void CExtPacket::RegisterExtPackets(void)
{
  _pShell->DeclareSymbol("persistent user INDEX ser_bReportExtPacketLogic;", &ser_bReportExtPacketLogic);
};

#endif // CLASSICSPATCH_EXT_PACKETS
