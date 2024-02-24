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

#if CLASSICSPATCH_EXT_PACKETS

#include "PacketCommands.h"
#include "Networking/ExtPackets.h"

#define VANILLA_EVENTS_ENTITY_ID
#include "Compatibility/VanillaEvents.h"

namespace IPacketCommands {

// Preconfigured event for packet commands
static EExtEntityEvent _eePacketEvent;
static ULONG _ctPacketEventFields = 0;

// Begin event setup of a specific type
void SetupEvent(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iEventType = NEXT_ARG(INDEX);

  // Reset event
  _eePacketEvent.Reset();
  _ctPacketEventFields = 0;

  // Set new type
  _eePacketEvent.ee_slEvent = iEventType;
};

// Set event field to an integer
void EventFieldIndex(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iField = NEXT_ARG(INDEX);
  INDEX iValue = NEXT_ARG(INDEX);

  ASSERT(iField >= 0 && iField < 64);
  iField = Clamp(iField, (INDEX)0, (INDEX)63);

  // Use as many fields as the set one
  (INDEX &)_eePacketEvent.aulFields[iField] = iValue;
  _ctPacketEventFields = Max(_ctPacketEventFields, ULONG(iField + 1));
};

// Set event field to a float
void EventFieldFloat(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iField = NEXT_ARG(INDEX);
  FLOAT fValue = NEXT_ARG(FLOAT);

  ASSERT(iField >= 0 && iField < 64);
  iField = Clamp(iField, (INDEX)0, (INDEX)63);

  // Use as many fields as the set one
  (FLOAT &)_eePacketEvent.aulFields[iField] = fValue;
  _ctPacketEventFields = Max(_ctPacketEventFields, ULONG(iField + 1));
};

// Set three event fields to vector values
void EventFieldVector(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iField = NEXT_ARG(INDEX);
  FLOAT fX = NEXT_ARG(FLOAT);
  FLOAT fY = NEXT_ARG(FLOAT);
  FLOAT fZ = NEXT_ARG(FLOAT);

  ASSERT(iField >= 0 && iField < 62);
  iField = Clamp(iField, (INDEX)0, (INDEX)61);

  // Use as many fields as the set one
  (FLOAT3D &)_eePacketEvent.aulFields[iField] = FLOAT3D(fX, fY, fZ);
  _ctPacketEventFields = Max(_ctPacketEventFields, ULONG(iField + 1));
};

// Create entity from the list
void EntityCreate(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  const CTString &strClass = *NEXT_ARG(CTString *);

  CExtEntityCreate pck;
  pck.fnmClass = "Classes\\" + strClass + ".ecl";
  pck.SendPacket();
};

// Destroy entities
void EntityDelete(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iEntity = NEXT_ARG(INDEX);
  INDEX iSameClass = NEXT_ARG(INDEX);

  CExtEntityDelete pck;
  pck.ulEntity = iEntity;
  pck.bSameClass = (iSameClass != 0);
  pck.SendPacket();
};

// Copy entity
void EntityCopy(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iEntity = NEXT_ARG(INDEX);
  INDEX iCopies = NEXT_ARG(INDEX);

  CExtEntityCopy pck;
  pck.ulEntity = iEntity;
  pck.ubCopies = Clamp(iCopies, (INDEX)0, (INDEX)31);
  pck.SendPacket();
};

// Send set up event to an entity
void EntityEvent(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iEntity = NEXT_ARG(INDEX);

  CExtEntityEvent pck;
  pck.ulEntity = iEntity;
  pck.Copy(_eePacketEvent, _ctPacketEventFields);
  pck.SendPacket();
};

// Receive item by an entity via a set up event
void EntityItem(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iEntity = NEXT_ARG(INDEX);

  CExtEntityItem pck;
  pck.ulEntity = iEntity;
  pck.Copy(_eePacketEvent, _ctPacketEventFields);
  pck.SendPacket();
};

// Initialize entity
void EntityInit(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iEntity = NEXT_ARG(INDEX);

  CExtEntityInit pck;
  pck.ulEntity = iEntity;
  pck.SetEvent(EVoid(), sizeof(EVoid));
  pck.SendPacket();
};

// Initialize entity with preset event
void EntityInitEvent(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iEntity = NEXT_ARG(INDEX);

  CExtEntityInit pck;
  pck.ulEntity = iEntity;
  pck.Copy(_eePacketEvent, _ctPacketEventFields);
  pck.SendPacket();
};

// Set new entity position
void EntitySetPos(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iEntity = NEXT_ARG(INDEX);
  FLOAT fX = NEXT_ARG(FLOAT);
  FLOAT fY = NEXT_ARG(FLOAT);
  FLOAT fZ = NEXT_ARG(FLOAT);
  INDEX iRelative = NEXT_ARG(INDEX);

  CExtEntityPosition pck;
  pck.ulEntity = iEntity;
  pck.vSet = FLOAT3D(fX, fY, fZ);
  pck.bRotation = FALSE;
  pck.bRelative = (iRelative != 0);
  pck.SendPacket();
};

// Set new entity rotation
void EntitySetRot(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iEntity = NEXT_ARG(INDEX);
  FLOAT fH = NEXT_ARG(FLOAT);
  FLOAT fP = NEXT_ARG(FLOAT);
  FLOAT fB = NEXT_ARG(FLOAT);
  INDEX iRelative = NEXT_ARG(INDEX);

  CExtEntityPosition pck;
  pck.ulEntity = iEntity;
  pck.vSet = FLOAT3D(fH, fP, fB);
  pck.bRotation = TRUE;
  pck.bRelative = (iRelative != 0);
  pck.SendPacket();
};

// Set new entity placement
void EntityTeleport(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iEntity = NEXT_ARG(INDEX);
  FLOAT fX = NEXT_ARG(FLOAT);
  FLOAT fY = NEXT_ARG(FLOAT);
  FLOAT fZ = NEXT_ARG(FLOAT);
  FLOAT fH = NEXT_ARG(FLOAT);
  FLOAT fP = NEXT_ARG(FLOAT);
  FLOAT fB = NEXT_ARG(FLOAT);
  INDEX iRelative = NEXT_ARG(INDEX);

  CExtEntityTeleport pck;
  pck.ulEntity = iEntity;
  pck.plSet = CPlacement3D(FLOAT3D(fX, fY, fZ), ANGLE3D(fH, fP, fB));
  pck.bRelative = (iRelative != 0);
  pck.SendPacket();
};

// Parent entity
void EntityParent(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iEntity = NEXT_ARG(INDEX);
  INDEX iParent = NEXT_ARG(INDEX);

  CExtEntityParent pck;
  pck.ulEntity = iEntity;
  pck.ulParent = iParent;
  pck.SendPacket();
};

// Change number property by name or ID
void EntityNumberProp(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iEntity = NEXT_ARG(INDEX);
  const CTString &strProp = *NEXT_ARG(CTString *);
  INDEX iPropID = NEXT_ARG(INDEX);
  FLOAT fValue = NEXT_ARG(FLOAT);

  CExtEntityProp pck;
  pck.ulEntity = iEntity;

  if (strProp != "") {
    pck.SetProperty(strProp);
  } else {
    pck.SetProperty(iPropID);
  }

  pck.SetValue(fValue);
  pck.SendPacket();
};

// Change string property by name or ID
void EntityStringProp(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iEntity = NEXT_ARG(INDEX);
  const CTString &strProp = *NEXT_ARG(CTString *);
  INDEX iPropID = NEXT_ARG(INDEX);
  const CTString &strValue = *NEXT_ARG(CTString *);

  CExtEntityProp pck;
  pck.ulEntity = iEntity;

  if (strProp != "") {
    pck.SetProperty(strProp);
  } else {
    pck.SetProperty(iPropID);
  }

  pck.SetValue(strValue);
  pck.SendPacket();
};

// Change entity health
void EntityHealth(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iEntity = NEXT_ARG(INDEX);
  FLOAT fHealth = NEXT_ARG(FLOAT);

  CExtEntityHealth pck;
  pck.ulEntity = iEntity;
  pck.fHealth = fHealth;
  pck.SendPacket();
};

// Change entity flags
void EntityFlags(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iEntity = NEXT_ARG(INDEX);
  INDEX iFlags = NEXT_ARG(INDEX);
  INDEX iRemove = NEXT_ARG(INDEX);

  CExtEntityFlags pck;
  pck.ulEntity = iEntity;
  pck.EntityFlags(iFlags, (iRemove != 0));
  pck.SendPacket();
};

// Change physical flags
void EntityPhysicalFlags(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iEntity = NEXT_ARG(INDEX);
  INDEX iFlags = NEXT_ARG(INDEX);
  INDEX iRemove = NEXT_ARG(INDEX);

  CExtEntityFlags pck;
  pck.ulEntity = iEntity;
  pck.PhysicalFlags(iFlags, (iRemove != 0));
  pck.SendPacket();
};

// Change collision flags
void EntityCollisionFlags(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iEntity = NEXT_ARG(INDEX);
  INDEX iFlags = NEXT_ARG(INDEX);
  INDEX iRemove = NEXT_ARG(INDEX);

  CExtEntityFlags pck;
  pck.ulEntity = iEntity;
  pck.CollisionFlags(iFlags, (iRemove != 0));
  pck.SendPacket();
};

// Set movement speed
void EntityMove(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iEntity = NEXT_ARG(INDEX);
  FLOAT fX = NEXT_ARG(FLOAT);
  FLOAT fY = NEXT_ARG(FLOAT);
  FLOAT fZ = NEXT_ARG(FLOAT);

  CExtEntityMove pck;
  pck.ulEntity = iEntity;
  pck.vSpeed = FLOAT3D(fX, fY, fZ);
  pck.SendPacket();
};

// Set rotation speed
void EntityRotate(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iEntity = NEXT_ARG(INDEX);
  FLOAT fH = NEXT_ARG(FLOAT);
  FLOAT fP = NEXT_ARG(FLOAT);
  FLOAT fB = NEXT_ARG(FLOAT);

  CExtEntityRotate pck;
  pck.ulEntity = iEntity;
  pck.vSpeed = FLOAT3D(fH, fP, fB);
  pck.SendPacket();
};

// Give impulse in an absolute direction
void EntityImpulse(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iEntity = NEXT_ARG(INDEX);
  FLOAT fX = NEXT_ARG(FLOAT);
  FLOAT fY = NEXT_ARG(FLOAT);
  FLOAT fZ = NEXT_ARG(FLOAT);

  CExtEntityImpulse pck;
  pck.ulEntity = iEntity;
  pck.vSpeed = FLOAT3D(fX, fY, fZ);
  pck.SendPacket();
};

static INDEX _iDamageSetup = -1;

static ULONG _ulDamageInflictor = -1;
static ULONG _ulDamageTarget = -1;
static ULONG _ulDamageType = DMT_NONE;
static FLOAT _fDamageAmount = 0.0f;

static FLOAT3D _vDamageVec1(0, 0, 0);
static FLOAT3D _vDamageVec2(0, 0, 0);

// Begin damage setup with an inflictor
void SetupDamage(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iInflictor = NEXT_ARG(INDEX);
  INDEX iType = NEXT_ARG(INDEX);
  FLOAT fDamage = NEXT_ARG(FLOAT);

  _iDamageSetup = -1;

  _ulDamageInflictor = iInflictor;
  _ulDamageType = iType;
  _fDamageAmount = fDamage;
};

// Setup direct damage (target, hit point, direction)
void SetDirectDamage(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iTarget = NEXT_ARG(INDEX);
  FLOAT fHitX = NEXT_ARG(FLOAT);
  FLOAT fHitY = NEXT_ARG(FLOAT);
  FLOAT fHitZ = NEXT_ARG(FLOAT);
  FLOAT fDirX = NEXT_ARG(FLOAT);
  FLOAT fDirY = NEXT_ARG(FLOAT);
  FLOAT fDirZ = NEXT_ARG(FLOAT);

  _iDamageSetup = 0;

  _ulDamageTarget = iTarget;
  _vDamageVec1 = FLOAT3D(fHitX, fHitY, fHitZ);
  _vDamageVec2 = FLOAT3D(fDirX, fDirY, fDirZ);
};

// Setup range damage (hit center, fall off, hot spot)
void SetRangeDamage(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  FLOAT fX = NEXT_ARG(FLOAT);
  FLOAT fY = NEXT_ARG(FLOAT);
  FLOAT fZ = NEXT_ARG(FLOAT);
  FLOAT fFallOff = NEXT_ARG(FLOAT);
  FLOAT fHotSpot = NEXT_ARG(FLOAT);

  _iDamageSetup = 1;

  _vDamageVec1 = FLOAT3D(fX, fY, fZ);
  _vDamageVec2 = FLOAT3D(fFallOff, fHotSpot, 0.0f);
};

// Setup box damage (min corner, max corner)
void SetBoxDamage(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  FLOAT fX1 = NEXT_ARG(FLOAT);
  FLOAT fY1 = NEXT_ARG(FLOAT);
  FLOAT fZ1 = NEXT_ARG(FLOAT);
  FLOAT fX2 = NEXT_ARG(FLOAT);
  FLOAT fY2 = NEXT_ARG(FLOAT);
  FLOAT fZ2 = NEXT_ARG(FLOAT);

  _iDamageSetup = 2;

  _vDamageVec1 = FLOAT3D(fX1, fY1, fZ1);
  _vDamageVec2 = FLOAT3D(fX2, fY2, fZ2);
};

// Inflict set up damage
void EntityDamage(void) {
  switch (_iDamageSetup) {
    case 0: {
      CExtEntityDirectDamage pck;
      pck.ulEntity = _ulDamageInflictor;
      pck.eDamageType = _ulDamageType;
      pck.fDamage = _fDamageAmount;

      pck.ulTarget = _ulDamageTarget;
      pck.vHitPoint = _vDamageVec1;
      pck.vDirection = _vDamageVec2;
      pck.SendPacket();
    } break;

    case 1: {
      CExtEntityRangeDamage pck;
      pck.ulEntity = _ulDamageInflictor;
      pck.eDamageType = _ulDamageType;
      pck.fDamage = _fDamageAmount;

      pck.vCenter = _vDamageVec1;
      pck.fFallOff = _vDamageVec2(1);
      pck.fHotSpot = _vDamageVec2(2);
      pck.SendPacket();
    } break;

    case 2: {
      CExtEntityBoxDamage pck;
      pck.ulEntity = _ulDamageInflictor;
      pck.eDamageType = _ulDamageType;
      pck.fDamage = _fDamageAmount;

      pck.boxArea = FLOATaabbox3D(_vDamageVec1, _vDamageVec2);
      pck.SendPacket();
    } break;
  }
};

// Change level using WorldLink
void ChangeLevel(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  const CTString &strWorld = *NEXT_ARG(CTString *);

  CExtChangeLevel pck;
  pck.strWorld = strWorld;
  pck.SendPacket();
};

// Force immediate world change
void ChangeWorld(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  const CTString &strWorld = *NEXT_ARG(CTString *);

  CExtChangeWorld pck;
  pck.strWorld = strWorld;
  pck.SendPacket();
};

}; // namespace

#endif // CLASSICSPATCH_EXT_PACKETS
