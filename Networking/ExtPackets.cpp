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
#include "Modules/PacketCommands.h"

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
      case EXT_ENTITY_HEALTH:   return new CExtEntityHealth();
      case EXT_ENTITY_FLAGS:    return new CExtEntityFlags();
      case EXT_ENTITY_MOVE:     return new CExtEntityMove();
      case EXT_ENTITY_ROTATE:   return new CExtEntityRotate();
    }
  }

  ASSERT(FALSE);
  return NULL;
};

// Register the module
void CExtPacket::RegisterExtPackets(void)
{
  _pShell->DeclareSymbol("persistent user INDEX ser_bReportExtPacketLogic;", &ser_bReportExtPacketLogic);

  // Vanilla event types
  _pShell->DeclareSymbol("const INDEX EVENTCODE_EStop;",                 (void *)&EVENTCODE_EStop);
  _pShell->DeclareSymbol("const INDEX EVENTCODE_EStart;",                (void *)&EVENTCODE_EStart);
  _pShell->DeclareSymbol("const INDEX EVENTCODE_EActivate;",             (void *)&EVENTCODE_EActivate);
  _pShell->DeclareSymbol("const INDEX EVENTCODE_EDeactivate;",           (void *)&EVENTCODE_EDeactivate);
  _pShell->DeclareSymbol("const INDEX EVENTCODE_EEnvironmentStart;",     (void *)&EVENTCODE_EEnvironmentStart);
  _pShell->DeclareSymbol("const INDEX EVENTCODE_EEnvironmentStop;",      (void *)&EVENTCODE_EEnvironmentStop);
  _pShell->DeclareSymbol("const INDEX EVENTCODE_EEnd;",                  (void *)&EVENTCODE_EEnd);
  _pShell->DeclareSymbol("const INDEX EVENTCODE_ETrigger;",              (void *)&EVENTCODE_ETrigger);
  _pShell->DeclareSymbol("const INDEX EVENTCODE_ETeleportMovingBrush;",  (void *)&EVENTCODE_ETeleportMovingBrush);
  _pShell->DeclareSymbol("const INDEX EVENTCODE_EReminder;",             (void *)&EVENTCODE_EReminder);
  _pShell->DeclareSymbol("const INDEX EVENTCODE_EStartAttack;",          (void *)&EVENTCODE_EStartAttack);
  _pShell->DeclareSymbol("const INDEX EVENTCODE_EStopAttack;",           (void *)&EVENTCODE_EStopAttack);
  _pShell->DeclareSymbol("const INDEX EVENTCODE_EStopBlindness;",        (void *)&EVENTCODE_EStopBlindness);
  _pShell->DeclareSymbol("const INDEX EVENTCODE_EStopDeafness;",         (void *)&EVENTCODE_EStopDeafness);
  _pShell->DeclareSymbol("const INDEX EVENTCODE_EReceiveScore;",         (void *)&EVENTCODE_EReceiveScore);
  _pShell->DeclareSymbol("const INDEX EVENTCODE_EKilledEnemy;",          (void *)&EVENTCODE_EKilledEnemy);
  _pShell->DeclareSymbol("const INDEX EVENTCODE_ESecretFound;",          (void *)&EVENTCODE_ESecretFound);
  _pShell->DeclareSymbol("const INDEX EVENTCODE_ERestartAttack;",        (void *)&EVENTCODE_ERestartAttack);
  _pShell->DeclareSymbol("const INDEX EVENTCODE_EReconsiderBehavior;",   (void *)&EVENTCODE_EReconsiderBehavior);
  _pShell->DeclareSymbol("const INDEX EVENTCODE_EForceWound;",           (void *)&EVENTCODE_EForceWound);
  _pShell->DeclareSymbol("const INDEX EVENTCODE_ESelectWeapon;",         (void *)&EVENTCODE_ESelectWeapon);
  _pShell->DeclareSymbol("const INDEX EVENTCODE_EBoringWeapon;",         (void *)&EVENTCODE_EBoringWeapon);
  _pShell->DeclareSymbol("const INDEX EVENTCODE_EFireWeapon;",           (void *)&EVENTCODE_EFireWeapon);
  _pShell->DeclareSymbol("const INDEX EVENTCODE_EReleaseWeapon;",        (void *)&EVENTCODE_EReleaseWeapon);
  _pShell->DeclareSymbol("const INDEX EVENTCODE_EReloadWeapon;",         (void *)&EVENTCODE_EReloadWeapon);
  _pShell->DeclareSymbol("const INDEX EVENTCODE_EWeaponChanged;",        (void *)&EVENTCODE_EWeaponChanged);
  _pShell->DeclareSymbol("const INDEX EVENTCODE_EAirShockwave;",         (void *)&EVENTCODE_EAirShockwave);
  _pShell->DeclareSymbol("const INDEX EVENTCODE_EAirWave;",              (void *)&EVENTCODE_EAirWave);
  _pShell->DeclareSymbol("const INDEX EVENTCODE_ESpawnEffect;",          (void *)&EVENTCODE_ESpawnEffect);
  _pShell->DeclareSymbol("const INDEX EVENTCODE_ESpawnSpray;",           (void *)&EVENTCODE_ESpawnSpray);
  _pShell->DeclareSymbol("const INDEX EVENTCODE_EBulletInit;",           (void *)&EVENTCODE_EBulletInit);
  _pShell->DeclareSymbol("const INDEX EVENTCODE_ELaunchCannonBall;",     (void *)&EVENTCODE_ELaunchCannonBall);
  _pShell->DeclareSymbol("const INDEX EVENTCODE_ECyborgBike;",           (void *)&EVENTCODE_ECyborgBike);
  _pShell->DeclareSymbol("const INDEX EVENTCODE_ESpawnDebris;",          (void *)&EVENTCODE_ESpawnDebris);
  _pShell->DeclareSymbol("const INDEX EVENTCODE_EDevilProjectile;",      (void *)&EVENTCODE_EDevilProjectile);
  _pShell->DeclareSymbol("const INDEX EVENTCODE_ESpawnEffector;",        (void *)&EVENTCODE_ESpawnEffector);
  _pShell->DeclareSymbol("const INDEX EVENTCODE_EFlame;",                (void *)&EVENTCODE_EFlame);
  _pShell->DeclareSymbol("const INDEX EVENTCODE_ELaunchLarvaOffspring;", (void *)&EVENTCODE_ELaunchLarvaOffspring);
  _pShell->DeclareSymbol("const INDEX EVENTCODE_EAnimatorInit;",         (void *)&EVENTCODE_EAnimatorInit);
  _pShell->DeclareSymbol("const INDEX EVENTCODE_EViewInit;",             (void *)&EVENTCODE_EViewInit);
  _pShell->DeclareSymbol("const INDEX EVENTCODE_EWeaponsInit;",          (void *)&EVENTCODE_EWeaponsInit);
  _pShell->DeclareSymbol("const INDEX EVENTCODE_EWeaponEffectInit;",     (void *)&EVENTCODE_EWeaponEffectInit);
  _pShell->DeclareSymbol("const INDEX EVENTCODE_ELaunchProjectile;",     (void *)&EVENTCODE_ELaunchProjectile);
  _pShell->DeclareSymbol("const INDEX EVENTCODE_EReminderInit;",         (void *)&EVENTCODE_EReminderInit);
  _pShell->DeclareSymbol("const INDEX EVENTCODE_ESeriousBomb;",          (void *)&EVENTCODE_ESeriousBomb);
  _pShell->DeclareSymbol("const INDEX EVENTCODE_ESpawnerProjectile;",    (void *)&EVENTCODE_ESpawnerProjectile);
  _pShell->DeclareSymbol("const INDEX EVENTCODE_ESpinnerInit;",          (void *)&EVENTCODE_ESpinnerInit);
  _pShell->DeclareSymbol("const INDEX EVENTCODE_ETwister;",              (void *)&EVENTCODE_ETwister);
  _pShell->DeclareSymbol("const INDEX EVENTCODE_EWatcherInit;",          (void *)&EVENTCODE_EWatcherInit);
  _pShell->DeclareSymbol("const INDEX EVENTCODE_EWater;",                (void *)&EVENTCODE_EWater);

  // Event setup
  _pShell->DeclareSymbol("user void pck_SetupEvent(INDEX);", &IPacketCommands::SetupEvent);
  _pShell->DeclareSymbol("user void pck_EventFieldIndex(INDEX, INDEX);", &IPacketCommands::EventFieldIndex);
  _pShell->DeclareSymbol("user void pck_EventFieldFloat(INDEX, FLOAT);", &IPacketCommands::EventFieldFloat);
  _pShell->DeclareSymbol("user void pck_EventFieldVector(INDEX, FLOAT, FLOAT, FLOAT);", &IPacketCommands::EventFieldVector);

  // Entity instantiation
  _pShell->DeclareSymbol("user void pck_EntityCreate(CTString);", &IPacketCommands::EntityCreate);
  _pShell->DeclareSymbol("user void pck_EntityDelete(INDEX, INDEX);", &IPacketCommands::EntityDelete);
  _pShell->DeclareSymbol("user void pck_EntityCopy(INDEX, INDEX);", &IPacketCommands::EntityCopy);

  // Entity logic
  _pShell->DeclareSymbol("user void pck_EntityEvent(INDEX, INDEX);", &IPacketCommands::EntityEvent);
  _pShell->DeclareSymbol("user void pck_EntityInit(INDEX);", &IPacketCommands::EntityInit);
  _pShell->DeclareSymbol("user void pck_EntityInitEvent(INDEX);", &IPacketCommands::EntityInitEvent);

  // Entity placement
  _pShell->DeclareSymbol("user void pck_EntitySetPos(INDEX, FLOAT, FLOAT, FLOAT, INDEX);", &IPacketCommands::EntitySetPos);
  _pShell->DeclareSymbol("user void pck_EntitySetRot(INDEX, FLOAT, FLOAT, FLOAT, INDEX);", &IPacketCommands::EntitySetRot);
  _pShell->DeclareSymbol("user void pck_EntityTeleport(INDEX, FLOAT, FLOAT, FLOAT, FLOAT, FLOAT, FLOAT, INDEX);", &IPacketCommands::EntityTeleport);

  // Entity properties
  _pShell->DeclareSymbol("user void pck_EntityParent(INDEX, INDEX);", &IPacketCommands::EntityParent);
  _pShell->DeclareSymbol("user void pck_EntityNumberProp(INDEX, CTString, INDEX, FLOAT);", &IPacketCommands::EntityNumberProp);
  _pShell->DeclareSymbol("user void pck_EntityStringProp(INDEX, CTString, INDEX, CTString);", &IPacketCommands::EntityStringProp);
  _pShell->DeclareSymbol("user void pck_EntityHealth(INDEX, FLOAT);", &IPacketCommands::EntityHealth);

  // Entity flags
  _pShell->DeclareSymbol("user void pck_EntityFlags(INDEX, INDEX, INDEX);", &IPacketCommands::EntityFlags);
  _pShell->DeclareSymbol("user void pck_EntityPhysicalFlags(INDEX, INDEX, INDEX);", &IPacketCommands::EntityPhysicalFlags);
  _pShell->DeclareSymbol("user void pck_EntityCollisionFlags(INDEX, INDEX, INDEX);", &IPacketCommands::EntityCollisionFlags);

  // Entity movement
  _pShell->DeclareSymbol("user void pck_EntityMove(INDEX, FLOAT, FLOAT, FLOAT);", &IPacketCommands::EntityMove);
  _pShell->DeclareSymbol("user void pck_EntityRotate(INDEX, FLOAT, FLOAT, FLOAT);", &IPacketCommands::EntityRotate);
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
