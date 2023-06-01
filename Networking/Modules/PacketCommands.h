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

#ifndef CECIL_INCL_PACKETCOMMANDS_H
#define CECIL_INCL_PACKETCOMMANDS_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// Interface for commands for sending extension packets
namespace IPacketCommands {

// Event setup
void SetupEvent(INDEX iEventType);
void EventFieldIndex(INDEX iField, INDEX iValue);
void EventFieldFloat(INDEX iField, FLOAT fValue);
void EventFieldVector(INDEX iField, FLOAT fX, FLOAT fY, FLOAT fZ);

// Entity instantiation
void EntityCreate(const CTString &strClass);
void EntityDelete(INDEX iEntity, INDEX iSameClass);
void EntityCopy(INDEX iEntity, INDEX iCopies);

// Entity logic
void EntityEvent(INDEX iEntity, INDEX iID);
void EntityInit(INDEX iEntity);
void EntityInitEvent(INDEX iEntity);

// Entity placement
void EntityTeleport(INDEX iEntity, FLOAT fX, FLOAT fY, FLOAT fZ, FLOAT fH, FLOAT fP, FLOAT fB, INDEX iRelative);
void EntitySetPos(INDEX iEntity, FLOAT fX, FLOAT fY, FLOAT fZ, INDEX iRelative);
void EntitySetRot(INDEX iEntity, FLOAT fH, FLOAT fP, FLOAT fB, INDEX iRelative);

// Entity properties
void EntityParent(INDEX iEntity, INDEX iParent);
void EntityNumberProp(INDEX iEntity, const CTString &strProp, INDEX iPropID, FLOAT fValue);
void EntityStringProp(INDEX iEntity, const CTString &strProp, INDEX iPropID, const CTString &strValue);
void EntityHealth(INDEX iEntity, FLOAT fHealth);

// Entity flags
void EntityFlags(INDEX iEntity, INDEX iFlags, INDEX iRemove);
void EntityPhysicalFlags(INDEX iEntity, INDEX iFlags, INDEX iRemove);
void EntityCollisionFlags(INDEX iEntity, INDEX iFlags, INDEX iRemove);

// Entity movement
void EntityMove(INDEX iEntity, FLOAT fX, FLOAT fY, FLOAT fZ);
void EntityRotate(INDEX iEntity, FLOAT fH, FLOAT fP, FLOAT fB);
void EntityImpulse(INDEX iEntity, FLOAT fX, FLOAT fY, FLOAT fZ);

}; // namespace

#endif
