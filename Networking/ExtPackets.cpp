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

#define VANILLA_EVENTS_ENTITY_ID
#include "Compatibility/VanillaEvents.h"

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
      case EXT_ENTITY_EVENT:    return new CExtEntityEvent();
      case EXT_ENTITY_INIT:     return new CExtEntityInit();
      case EXT_ENTITY_TELEPORT: return new CExtEntityTeleport();
      case EXT_ENTITY_POSITION: return new CExtEntityPosition();
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

// Write event into a network packet
void EExtEntityEvent::Write(CNetworkMessage &nm, ULONG ctFields) {
  nm << ee_slEvent;

  // Write data
  UBYTE ubData = (ctFields != 0);
  nm.WriteBits(&ubData, 1);

  if (ubData) {
    // Fit 64 fields by writing the 0-63 range
    ubData = UBYTE(ctFields - 1);
    nm.WriteBits(&ubData, 6);

    nm.Write(&aulFields[0], ctFields * sizeof(ULONG));
  }
};

// Read event from a network packet
ULONG EExtEntityEvent::Read(CNetworkMessage &nm) {
  Reset();
  nm >> ee_slEvent;

  // Read data
  UBYTE ubData = 0;
  nm.ReadBits(&ubData, 1);

  if (ubData) {
    ULONG ctFields = 0;

    // Interpret read size as being in the 1-64 range
    nm.ReadBits(&ctFields, 6);
    ctFields++;

    nm.Read(&aulFields[0], ctFields * sizeof(ULONG));
    return ctFields;
  }

  return 0;
};

// Convert fields according to the event type
void EExtEntityEvent::ConvertTypes(void)
{
  // Convert entity IDs into pointers
  switch (ee_slEvent) {
    // First field is an entity
    case EVENTCODE_EStart:
    case EVENTCODE_ETrigger:
    case EVENTCODE_EAirShockwave:
    case EVENTCODE_EAirWave:
    case EVENTCODE_EBulletInit:
    case EVENTCODE_ELaunchCannonBall:
    case EVENTCODE_ELaunchLarvaOffspring:
    case EVENTCODE_EAnimatorInit:
    case EVENTCODE_EWeaponsInit:
    case EVENTCODE_EWeaponEffectInit:
    case EVENTCODE_ELaunchProjectile:
    case EVENTCODE_EReminderInit:
    case EVENTCODE_ESeriousBomb:
    case EVENTCODE_EWatcherInit:
    case EVENTCODE_EWater: {
      EStart &ee = (EStart &)*this;
      ee.penCaused = EntityFromID(0);
    } break;

    // Two first fields are entities
    case EVENTCODE_EDevilProjectile:
    case EVENTCODE_EFlame:
    case EVENTCODE_EViewInit:
    case EVENTCODE_ESpinnerInit: {
      EFlame &ee = (EFlame &)*this;
      ee.penOwner = EntityFromID(0);
      ee.penAttach = EntityFromID(1);
    } break;

    // Same ID as ETwister
    case EVENTCODE_ESpawnerProjectile: {
      ESpawnerProjectile &ee = (ESpawnerProjectile &)*this;
      ee.penOwner = EntityFromID(0);
      ee.penTemplate = MaybeEntity(1); // Preserves 'ETwister::fSize'
    } break;

    // Skips: sptType, fDamagePower, fSizeMultiplier, vDirection
    case EVENTCODE_ESpawnSpray: {
      ESpawnSpray &ee = (ESpawnSpray &)*this;
      ee.penOwner = EntityFromID(6);
    } break;

    case EVENTCODE_ESpawnDebris: {
      ESpawnDebris &ee = (ESpawnDebris &)*this;
      ee.penFallFXPapa = EntityFromID(22);

      if (ee.penFallFXPapa != NULL) {
        CEntity *pen = (CEntity *)ee.penFallFXPapa;

        ee.pmd = pen->GetModelObject()->GetData(); // 1
        ee.ptd = (CTextureData *)pen->GetModelObject()->mo_toTexture.GetData(); // 3
        ee.ptdRefl = (CTextureData *)pen->GetModelObject()->mo_toReflection.GetData(); // 4
        ee.ptdSpec = (CTextureData *)pen->GetModelObject()->mo_toSpecular.GetData(); // 5
        ee.ptdBump = (CTextureData *)pen->GetModelObject()->mo_toBump.GetData(); // 6
      }
    } break;

    // Skips: eetType, vDamageDir, vDestination, tmLifeTime, fSize, ctCount
    case EVENTCODE_ESpawnEffector: {
      ESpawnEffector &ee = (ESpawnEffector &)*this;
      ee.penModel  = EntityFromID(10);
      ee.penModel2 = EntityFromID(11);
    } break;
  }
};

#endif // CLASSICSPATCH_EXT_PACKETS
