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

#include "ExtPackets.h"
#include "NetworkFunctions.h"
#include "Modules/PacketCommands.h"

#define VANILLA_EVENTS_ENTITY_ID
#include <Extras/XGizmo/Vanilla/EntityEvents.h>

#if CLASSICSPATCH_EXT_PACKETS

// Report packet actions to the server
INDEX ser_bReportExtPacketLogic = TRUE;

// Report packet actions to the server
void CExtPacket::ExtServerReport(const char *strFormat, ...) {
  // Ignore reports
  if (!_pNetwork->IsServer() || !ser_bReportExtPacketLogic) return;

  va_list arg;
  va_start(arg, strFormat);

  CTString str;
  str.VPrintF(strFormat, arg);

  // Append packet name in the beginning
  CPrintF("[%s] %s", GetName(), str);
  va_end(arg);
};

// Send extension packet from server to all clients
void CExtPacket::SendPacket(void) {
  // Not running a server
  if (!_pNetwork->IsServer()) return;

  CNetStreamBlock nsbExt = INetwork::CreateServerPacket(GetType());
  Write(nsbExt);

  INetwork::AddBlockToAllSessions(nsbExt);
};

// Make sure to return some entity from the ID
CEntity *CExtEntityPacket::GetEntity(void) {
  if (!IsEntityValid()) {
    ExtServerReport(TRANS("Received invalid entity ID!\n"));
    return NULL;
  }

  CEntity *pen = FindExtEntity(ulEntity);

  if (pen == NULL) {
    ExtServerReport(TRANS("Received invalid entity ID!\n"));
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
      case EXT_ENTITY_ITEM:     return new CExtEntityItem();
      case EXT_ENTITY_INIT:     return new CExtEntityInit();
      case EXT_ENTITY_TELEPORT: return new CExtEntityTeleport();
      case EXT_ENTITY_POSITION: return new CExtEntityPosition();
      case EXT_ENTITY_PARENT:   return new CExtEntityParent();
      case EXT_ENTITY_PROP:     return new CExtEntityProp();
      case EXT_ENTITY_HEALTH:   return new CExtEntityHealth();
      case EXT_ENTITY_FLAGS:    return new CExtEntityFlags();
      case EXT_ENTITY_MOVE:     return new CExtEntityMove();
      case EXT_ENTITY_ROTATE:   return new CExtEntityRotate();
      case EXT_ENTITY_IMPULSE:  return new CExtEntityImpulse();
      case EXT_ENTITY_DIRDMG:   return new CExtEntityDirectDamage();
      case EXT_ENTITY_RADDMG:   return new CExtEntityRangeDamage();
      case EXT_ENTITY_BOXDMG:   return new CExtEntityBoxDamage();

      case EXT_CHANGE_LEVEL:    return new CExtChangeLevel();
      case EXT_CHANGE_WORLD:    return new CExtChangeWorld();
      case EXT_SESSION_PROPS:   return new CExtSessionProps();
      case EXT_GAMEPLAY_EXT:    return new CExtGameplayExt();
    }
  }

  ASSERT(FALSE);
  return NULL;
};

// [Cecil] TEMP: Get entity of a specific class under a certain index
static INDEX GetEntity(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  const CTString &strClass = *NEXT_ARG(CTString *);
  INDEX iEntity = NEXT_ARG(INDEX);

  CEntities cen;
  IWorld::FindClasses(IWorld::GetWorld()->wo_cenEntities, cen, strClass);

  if (iEntity < cen.Count()) {
    return cen[iEntity].en_ulID;
  }

  return -1;
};

// Register the module
void CExtPacket::RegisterExtPackets(void)
{
  _pShell->DeclareSymbol("persistent user INDEX ser_bReportExtPacketLogic;", &ser_bReportExtPacketLogic);

  // [Cecil] TEMP: Get entity of a specific class under a certain index
  _pShell->DeclareSymbol("user INDEX GetEntity(CTString, INDEX);", &GetEntity);

  // Declare extra symbols
  void DeclareExtraSymbolsForExtPackets(void);
  DeclareExtraSymbolsForExtPackets();

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
  _pShell->DeclareSymbol("user void pck_EntityEvent(INDEX);", &IPacketCommands::EntityEvent);
  _pShell->DeclareSymbol("user void pck_EntityItem(INDEX);", &IPacketCommands::EntityItem);
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
  _pShell->DeclareSymbol("user void pck_EntityImpulse(INDEX, FLOAT, FLOAT, FLOAT);", &IPacketCommands::EntityImpulse);

  // Entity damage
  _pShell->DeclareSymbol("user void pck_SetupDamage(INDEX, INDEX, FLOAT);", &IPacketCommands::SetupDamage);
  _pShell->DeclareSymbol("user void pck_SetDirectDamage(INDEX, FLOAT, FLOAT, FLOAT, FLOAT, FLOAT, FLOAT);", &IPacketCommands::SetDirectDamage);
  _pShell->DeclareSymbol("user void pck_SetRangeDamage(FLOAT, FLOAT, FLOAT, FLOAT, FLOAT);", &IPacketCommands::SetRangeDamage);
  _pShell->DeclareSymbol("user void pck_SetBoxDamage(FLOAT, FLOAT, FLOAT, FLOAT, FLOAT, FLOAT);", &IPacketCommands::SetBoxDamage);

  _pShell->DeclareSymbol("user void pck_EntityDamage(void);", &IPacketCommands::EntityDamage);

  // World change
  _pShell->DeclareSymbol("user void pck_ChangeLevel(CTString);", &IPacketCommands::ChangeLevel);
  _pShell->DeclareSymbol("user void pck_ChangeWorld(CTString);", &IPacketCommands::ChangeWorld);

  // Session properties
  _pShell->DeclareSymbol("user void pck_StartSesProps(INDEX);",    &IPacketCommands::StartSesProps);
  _pShell->DeclareSymbol("user void pck_SesPropIndex(INDEX);",     &IPacketCommands::SesPropIndex);
  _pShell->DeclareSymbol("user void pck_SesPropFloat(FLOAT);",     &IPacketCommands::SesPropFloat);
  _pShell->DeclareSymbol("user void pck_SesPropString(CTString);", &IPacketCommands::SesPropString);
  _pShell->DeclareSymbol("user void pck_SeekSesProp(INDEX);",      &IPacketCommands::SeekSesProp);
  _pShell->DeclareSymbol("user void pck_SendSesProps(void);",      &IPacketCommands::SendSesProps);

  // Gameplay extensions
  _pShell->DeclareSymbol("user void pck_GameplayExtNumber(CTString, FLOAT);",    &IPacketCommands::GameplayExtNumber);
  _pShell->DeclareSymbol("user void pck_GameplayExtString(CTString, CTString);", &IPacketCommands::GameplayExtString);
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
    case EVENTCODE_VNL_EStart:
    case EVENTCODE_VNL_ETrigger:
    case EVENTCODE_VNL_EAirShockwave:
    case EVENTCODE_VNL_EAirWave:
    case EVENTCODE_VNL_EBulletInit:
    case EVENTCODE_VNL_ELaunchCannonBall:
    case EVENTCODE_VNL_ELaunchLarvaOffspring:
    case EVENTCODE_VNL_EAnimatorInit:
    case EVENTCODE_VNL_EWeaponsInit:
    case EVENTCODE_VNL_EWeaponEffectInit:
    case EVENTCODE_VNL_ELaunchProjectile:
    case EVENTCODE_VNL_EReminderInit:
    case EVENTCODE_VNL_ESeriousBomb:
    case EVENTCODE_VNL_EWatcherInit:
    case EVENTCODE_VNL_EWater: {
      VNL_EStart &ee = (VNL_EStart &)*this;
      ee.penCaused = EntityFromID(0);
    } break;

    // Second field is an entity
    case EVENTCODE_VNL_ESound:
    case EVENTCODE_VNL_EScroll:
    case EVENTCODE_VNL_ETextFX:
    case EVENTCODE_VNL_EHudPicFX:
    case EVENTCODE_VNL_ECredits: {
      VNL_ESound &ee = (VNL_ESound &)*this;
      ee.penTarget = EntityFromID(1);
    } break;

    // Two first fields are entities
    case EVENTCODE_VNL_EDevilProjectile:
    case EVENTCODE_VNL_EFlame:
    case EVENTCODE_VNL_EViewInit:
    case EVENTCODE_VNL_ESpinnerInit: {
      VNL_EFlame &ee = (VNL_EFlame &)*this;
      ee.penOwner = EntityFromID(0);
      ee.penAttach = EntityFromID(1);
    } break;

    // Same ID as ETwister
    case EVENTCODE_VNL_ESpawnerProjectile: {
      VNL_ESpawnerProjectile &ee = (VNL_ESpawnerProjectile &)*this;
      ee.penOwner = EntityFromID(0);
      ee.penTemplate = MaybeEntity(1); // Preserves 'ETwister::fSize'
    } break;

    // Skips: sptType, fDamagePower, fSizeMultiplier, vDirection
    case EVENTCODE_VNL_ESpawnSpray: {
      VNL_ESpawnSpray &ee = (VNL_ESpawnSpray &)*this;
      ee.penOwner = EntityFromID(6);
    } break;

    case EVENTCODE_VNL_ESpawnDebris: {
      VNL_ESpawnDebris &ee = (VNL_ESpawnDebris &)*this;
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
    case EVENTCODE_VNL_ESpawnEffector: {
      VNL_ESpawnEffector &ee = (VNL_ESpawnEffector &)*this;
      ee.penModel  = EntityFromID(10);
      ee.penModel2 = EntityFromID(11);
    } break;
  }
};

#endif // CLASSICSPATCH_EXT_PACKETS
