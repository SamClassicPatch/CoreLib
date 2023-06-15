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

#include "PacketCommands.h"
#include "Networking/ExtPackets.h"

#define VANILLA_EVENTS_ENTITY_ID
#include "Compatibility/VanillaEvents.h"

namespace IPacketCommands {

// Preconfigured event for packet commands
static EExtEntityEvent _eePacketEvent;
static ULONG _ctPacketEventFields = 0;

// Begin event setup of a specific type
void SetupEvent(INDEX iEventType) {
  // Reset event
  _eePacketEvent.Reset();
  _ctPacketEventFields = 0;

  // Set new type
  _eePacketEvent.ee_slEvent = iEventType;
};

// Set event field to an integer
void EventFieldIndex(INDEX iField, INDEX iValue) {
  ASSERT(iField >= 0 && iField < 64);
  iField = Clamp(iField, (INDEX)0, (INDEX)63);

  // Use as many fields as the set one
  (INDEX &)_eePacketEvent.aulFields[iField] = iValue;
  _ctPacketEventFields = Max(_ctPacketEventFields, ULONG(iField + 1));
};

// Set event field to a float
void EventFieldFloat(INDEX iField, FLOAT fValue) {
  ASSERT(iField >= 0 && iField < 64);
  iField = Clamp(iField, (INDEX)0, (INDEX)63);

  // Use as many fields as the set one
  (FLOAT &)_eePacketEvent.aulFields[iField] = fValue;
  _ctPacketEventFields = Max(_ctPacketEventFields, ULONG(iField + 1));
};

// Set three event fields to vector values
void EventFieldVector(INDEX iField, FLOAT fX, FLOAT fY, FLOAT fZ) {
  ASSERT(iField >= 0 && iField < 62);
  iField = Clamp(iField, (INDEX)0, (INDEX)61);

  // Use as many fields as the set one
  (FLOAT3D &)_eePacketEvent.aulFields[iField] = FLOAT3D(fX, fY, fZ);
  _ctPacketEventFields = Max(_ctPacketEventFields, ULONG(iField + 1));
};

// Create entity from the list
void EntityCreate(const CTString &strClass) {
  CExtEntityCreate pck;
  pck.fnmClass = "Classes\\" + strClass + ".ecl";
  pck.SendPacket();
};

// Destroy entities
void EntityDelete(INDEX iEntity, INDEX iSameClass) {
  CExtEntityDelete pck;
  pck.ulEntity = iEntity;
  pck.bSameClass = (iSameClass != 0);
  pck.SendPacket();
};

// Copy entity
void EntityCopy(INDEX iEntity, INDEX iCopies) {
  CExtEntityCopy pck;
  pck.ulEntity = iEntity;
  pck.ubCopies = Clamp(iCopies, (INDEX)0, (INDEX)31);
  pck.SendPacket();
};

// Send set up event to an entity
void EntityEvent(INDEX iEntity) {
  CExtEntityEvent pck;
  pck.ulEntity = iEntity;
  pck.Copy(_eePacketEvent, _ctPacketEventFields);
  pck.SendPacket();
};

// Receive item by an entity via a set up event
void EntityItem(INDEX iEntity) {
  CExtEntityItem pck;
  pck.ulEntity = iEntity;
  pck.Copy(_eePacketEvent, _ctPacketEventFields);
  pck.SendPacket();
};

// Initialize entity
void EntityInit(INDEX iEntity) {
  CExtEntityInit pck;
  pck.ulEntity = iEntity;
  pck.SetEvent(EVoid(), sizeof(EVoid));
  pck.SendPacket();
};

// Initialize entity with preset event
void EntityInitEvent(INDEX iEntity) {
  CExtEntityInit pck;
  pck.ulEntity = iEntity;
  pck.Copy(_eePacketEvent, _ctPacketEventFields);
  pck.SendPacket();
};

// Set new entity position
void EntitySetPos(INDEX iEntity, FLOAT fX, FLOAT fY, FLOAT fZ, INDEX iRelative) {
  CExtEntityPosition pck;
  pck.ulEntity = iEntity;
  pck.vSet = FLOAT3D(fX, fY, fZ);
  pck.bRotation = FALSE;
  pck.bRelative = (iRelative != 0);
  pck.SendPacket();
};

// Set new entity rotation
void EntitySetRot(INDEX iEntity, FLOAT fH, FLOAT fP, FLOAT fB, INDEX iRelative) {
  CExtEntityPosition pck;
  pck.ulEntity = iEntity;
  pck.vSet = FLOAT3D(fH, fP, fB);
  pck.bRotation = TRUE;
  pck.bRelative = (iRelative != 0);
  pck.SendPacket();
};

// Set new entity placement
void EntityTeleport(INDEX iEntity, FLOAT fX, FLOAT fY, FLOAT fZ, FLOAT fH, FLOAT fP, FLOAT fB, INDEX iRelative) {
  CExtEntityTeleport pck;
  pck.ulEntity = iEntity;
  pck.plSet = CPlacement3D(FLOAT3D(fX, fY, fZ), ANGLE3D(fH, fP, fB));
  pck.bRelative = (iRelative != 0);
  pck.SendPacket();
};

// Parent entity
void EntityParent(INDEX iEntity, INDEX iParent) {
  CExtEntityParent pck;
  pck.ulEntity = iEntity;
  pck.ulParent = iParent;
  pck.SendPacket();
};

// Change number property by name or ID
void EntityNumberProp(INDEX iEntity, const CTString &strProp, INDEX iPropID, FLOAT fValue) {
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
void EntityStringProp(INDEX iEntity, const CTString &strProp, INDEX iPropID, const CTString &strValue) {
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
void EntityHealth(INDEX iEntity, FLOAT fHealth) {
  CExtEntityHealth pck;
  pck.ulEntity = iEntity;
  pck.fHealth = fHealth;
  pck.SendPacket();
};

// Change entity flags
void EntityFlags(INDEX iEntity, INDEX iFlags, INDEX iRemove) {
  CExtEntityFlags pck;
  pck.ulEntity = iEntity;
  pck.EntityFlags(iFlags, (iRemove != 0));
  pck.SendPacket();
};

// Change physical flags
void EntityPhysicalFlags(INDEX iEntity, INDEX iFlags, INDEX iRemove) {
  CExtEntityFlags pck;
  pck.ulEntity = iEntity;
  pck.PhysicalFlags(iFlags, (iRemove != 0));
  pck.SendPacket();
};

// Change collision flags
void EntityCollisionFlags(INDEX iEntity, INDEX iFlags, INDEX iRemove) {
  CExtEntityFlags pck;
  pck.ulEntity = iEntity;
  pck.CollisionFlags(iFlags, (iRemove != 0));
  pck.SendPacket();
};

// Set movement speed
void EntityMove(INDEX iEntity, FLOAT fX, FLOAT fY, FLOAT fZ) {
  CExtEntityMove pck;
  pck.ulEntity = iEntity;
  pck.vSpeed = FLOAT3D(fX, fY, fZ);
  pck.SendPacket();
};

// Set rotation speed
void EntityRotate(INDEX iEntity, FLOAT fH, FLOAT fP, FLOAT fB) {
  CExtEntityRotate pck;
  pck.ulEntity = iEntity;
  pck.vSpeed = FLOAT3D(fH, fP, fB);
  pck.SendPacket();
};

// Give impulse in an absolute direction
void EntityImpulse(INDEX iEntity, FLOAT fX, FLOAT fY, FLOAT fZ) {
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
void SetupDamage(INDEX iInflictor, INDEX iType, FLOAT fDamage) {
  _iDamageSetup = -1;

  _ulDamageInflictor = iInflictor;
  _ulDamageType = iType;
  _fDamageAmount = fDamage;
};

// Setup direct damage (target, hit point, direction)
void SetDirectDamage(INDEX iTarget, FLOAT fHitX, FLOAT fHitY, FLOAT fHitZ, FLOAT fDirX, FLOAT fDirY, FLOAT fDirZ) {
  _iDamageSetup = 0;

  _ulDamageTarget = iTarget;
  _vDamageVec1 = FLOAT3D(fHitX, fHitY, fHitZ);
  _vDamageVec2 = FLOAT3D(fDirX, fDirY, fDirZ);
};

// Setup range damage (hit center, fall off, hot spot)
void SetRangeDamage(FLOAT fX, FLOAT fY, FLOAT fZ, FLOAT fFallOff, FLOAT fHotSpot) {
  _iDamageSetup = 1;

  _vDamageVec1 = FLOAT3D(fX, fY, fZ);
  _vDamageVec2 = FLOAT3D(fFallOff, fHotSpot, 0.0f);
};

// Setup box damage (min corner, max corner)
void SetBoxDamage(FLOAT fX1, FLOAT fY1, FLOAT fZ1, FLOAT fX2, FLOAT fY2, FLOAT fZ2) {
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

}; // namespace
