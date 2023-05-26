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

#ifndef CECIL_INCL_EXTPACKETS_H
#define CECIL_INCL_EXTPACKETS_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include "MessageCompression.h"
#include <CoreLib/Interfaces/WorldFunctions.h>

#if CLASSICSPATCH_EXT_PACKETS

// Report packet actions to the server
extern INDEX ser_bReportExtPacketLogic;

// Base class for each extension packet
class CExtPacket {
  public:
    // Built-in extension packets
    enum EType {
      // Server to all clients
      EXT_ENTITY_CREATE,   // Create new entity
      EXT_ENTITY_DELETE,   // Delete an entity
      EXT_ENTITY_COPY,     // Copy an entity
      EXT_ENTITY_EVENT,    // Send event to an entity
      EXT_ENTITY_INIT,     // (Re)initialize an entity
      EXT_ENTITY_TELEPORT, // Teleport an entity
      EXT_ENTITY_POSITION, // Set position or rotation of an entity
      EXT_ENTITY_PARENT,   // Set entity's parent
      EXT_ENTITY_PROP,     // Change property value of an entity
      EXT_ENTITY_HEALTH,   // Set entity health
      EXT_ENTITY_FLAGS,    // Change various entity flags
      EXT_ENTITY_MOVE,     // Set absolute movement speed of an entity
      EXT_ENTITY_ROTATE,   // Set absolute rotation speed of an entity

      // Maximum amount of built-in packets
      EXT_MAX_PACKETS,
    };

  public:
    // Extension packet index
    virtual EType GetType(void) const = 0;

    // Write extension packet
    virtual void Write(CNetworkMessage &nm) = 0;

    // Read extension packet
    virtual void Read(CNetworkMessage &nm) = 0;

    // Process read packet as a client
    virtual void Process(void) = 0;

    // Send extension packet from server to all clients
    void SendPacket(void);

  public:
    // Create new packet from type
    static CExtPacket *CreatePacket(EType ePacket, BOOL bClient);

    // Register the module
    static void RegisterExtPackets(void);
};

// Entity packets

class CExtEntityCreate : public CExtPacket {
  private:
    // Dictionary of base class names, the index of which can be one byte
    static CTString aBaseClasses[255];
    UBYTE ubClass; // Index in the dictionary (0-254)

  public:
    // Class file to create an entity from
    CTFileName fnmClass; // Packed as an extra if index is 255
    CPlacement3D plPos; // Place to create an entity at

    // Last created entity
    static CEntity *penLast;

  public:
    CExtEntityCreate() : ubClass(0xFF), plPos(FLOAT3D(0, 0, 0), ANGLE3D(0, 0, 0))
    {
    };

  public:
    virtual EType GetType(void) const {
      return EXT_ENTITY_CREATE;
    };

    virtual void Write(CNetworkMessage &nm);
    virtual void Read(CNetworkMessage &nm);
    virtual void Process(void);
};

// Base for entity manipulation packets
class CExtEntityPacket : public CExtPacket {
  public:
    ULONG ulEntity; // Entity ID in the world (31 bits)

  public:
    CExtEntityPacket() : ulEntity(0x7FFFFFFF)
    {
    };

    // Write entity ID
    void WriteEntity(CNetworkMessage &nm) {
      ulEntity = ClampUp(ulEntity, 0x7FFFFFFFUL);
      nm.WriteBits(&ulEntity, 31);
    };

    // Read entity ID
    void ReadEntity(CNetworkMessage &nm) {
      ulEntity = 0;
      nm.ReadBits(&ulEntity, 31);
    };

    // Check for invalid ID
    inline BOOL IsEntityValid(void) {
      // 0x7FFFFFFF - 0xFFFFFFFF are invalid
      return ulEntity < 0x7FFFFFFF;
    };

    // Make sure to return some entity from the ID
    CEntity *GetEntity(void);
};

// Holder for event fields
class EExtEntityEvent : public CEntityEvent {
  public:
    // Accommodate for multiple fields of varying data
    ULONG aulFields[64];

  public:
    EExtEntityEvent() : CEntityEvent(EVENTCODE_EVoid) {
      Reset();
    };

    CEntityEvent *MakeCopy(void) {
      return new EExtEntityEvent(*this);
    };

  public:
    // Reset event fields
    inline void Reset(void) {
      ee_slEvent = EVENTCODE_EVoid;

      // Fill fields with NULL, FALSE, 0, 0.0f etc.
      memset(aulFields, 0, sizeof(aulFields));
    };

    // Convert entity ID into a pointer
    inline ULONG EntityFromID(INDEX i) {
      return (ULONG)IWorld::FindEntityByID(IWorld::GetWorld(), aulFields[i]);
    };

    // Convert entity ID into a pointer, if possible
    inline ULONG MaybeEntity(INDEX i) {
      // Return entity if found any
      ULONG ulPtr = EntityFromID(i);
      if (ulPtr != NULL) return ulPtr;

      // Return value as is
      return aulFields[i];
    };

    // Write event into a network packet
    void Write(CNetworkMessage &nm, ULONG ctFields);

    // Read event from a network packet
    ULONG Read(CNetworkMessage &nm);

    // Convert fields according to the event type
    void ConvertTypes(void);
};

class CExtEntityDelete : public CExtEntityPacket {
  public:
    BOOL bSameClass; // Delete all entities of the same class

  public:
    CExtEntityDelete() : CExtEntityPacket(), bSameClass(FALSE)
    {
    };

  public:
    virtual EType GetType(void) const {
      return EXT_ENTITY_DELETE;
    };

    virtual void Write(CNetworkMessage &nm);
    virtual void Read(CNetworkMessage &nm);
    virtual void Process(void);
};

class CExtEntityCopy : public CExtEntityPacket {
  public:
    UBYTE ubCopies;

  public:
    CExtEntityCopy() : CExtEntityPacket(), ubCopies(1)
    {
    };

  public:
    virtual EType GetType(void) const {
      return EXT_ENTITY_COPY;
    };

    virtual void Write(CNetworkMessage &nm);
    virtual void Read(CNetworkMessage &nm);
    virtual void Process(void);
};

class CExtEntityEvent : public CExtEntityPacket {
  protected:
    EExtEntityEvent eEvent; // Data holder
    ULONG ctFields; // Amount of used fields

  public:
    CExtEntityEvent() : CExtEntityPacket(), ctFields(0)
    {
    };

    // Copy event bytes (iEventSize = sizeof(ee))
    void SetEvent(CEntityEvent &ee, size_t iEventSize);

  public:
    virtual EType GetType(void) const {
      return EXT_ENTITY_EVENT;
    };

    virtual void Write(CNetworkMessage &nm);
    virtual void Read(CNetworkMessage &nm);
    virtual void Process(void);
};

class CExtEntityInit : public CExtEntityEvent {
  public:
    CExtEntityInit() : CExtEntityEvent()
    {
    };

  public:
    virtual EType GetType(void) const {
      return EXT_ENTITY_INIT;
    };

    virtual void Process(void);
};

class CExtEntityTeleport : public CExtEntityPacket {
  public:
    CPlacement3D plSet; // Placement to set
    BOOL bRelative; // Relative to the current placement (oriented)

  public:
    CExtEntityTeleport() : CExtEntityPacket(),
      plSet(FLOAT3D(0, 0, 0), ANGLE3D(0, 0, 0)), bRelative(FALSE)
    {
    };

  public:
    virtual EType GetType(void) const {
      return EXT_ENTITY_TELEPORT;
    };

    virtual void Write(CNetworkMessage &nm);
    virtual void Read(CNetworkMessage &nm);
    virtual void Process(void);
};

class CExtEntityPosition : public CExtEntityPacket {
  public:
    FLOAT3D vSet; // Position or rotation to set
    BOOL bRotation; // Set rotation instead of position
    BOOL bRelative; // Relative to the current placement (axis-aligned)

  public:
    CExtEntityPosition() : CExtEntityPacket(), vSet(0, 0, 0),
      bRotation(FALSE), bRelative(FALSE)
    {
    };

  public:
    virtual EType GetType(void) const {
      return EXT_ENTITY_POSITION;
    };

    virtual void Write(CNetworkMessage &nm);
    virtual void Read(CNetworkMessage &nm);
    virtual void Process(void);
};

class CExtEntityParent : public CExtEntityPacket {
  public:
    ULONG ulParent;

  public:
    CExtEntityParent() : CExtEntityPacket(), ulParent(-1)
    {
    };

  public:
    virtual EType GetType(void) const {
      return EXT_ENTITY_PARENT;
    };

    virtual void Write(CNetworkMessage &nm);
    virtual void Read(CNetworkMessage &nm);
    virtual void Process(void);
};

class CExtEntityProp : public CExtEntityPacket {
  private:
    BOOL bName; // Using a name
    ULONG ulProp; // Property ID or name hash

    BOOL bString; // Using a string value
    CTString strValue;
    DOUBLE fValue;

  public:
    CExtEntityProp() : CExtEntityPacket(),
      bName(FALSE), ulProp(0), bString(FALSE), fValue(0.0)
    {
    };

    // Set property name
    inline void SetProperty(const CTString &strName) {
      bName = TRUE;
      ulProp = strName.GetHash();
    };

    // Set property ID
    inline void SetProperty(ULONG ulID) {
      bName = FALSE;
      ulProp = ulID;
    };

    // Set string value
    inline void SetValue(const CTString &str) {
      bString = TRUE;
      strValue = str;
    };

    // Set number value
    inline void SetValue(DOUBLE f) {
      bString = FALSE;
      fValue = f;
    };

  public:
    virtual EType GetType(void) const {
      return EXT_ENTITY_PROP;
    };

    virtual void Write(CNetworkMessage &nm);
    virtual void Read(CNetworkMessage &nm);
    virtual void Process(void);
};

class CExtEntityHealth : public CExtEntityPacket {
  public:
    FLOAT fHealth; // Health to set

  public:
    CExtEntityHealth() : CExtEntityPacket(), fHealth(0.0f)
    {
    };

  public:
    virtual EType GetType(void) const {
      return EXT_ENTITY_HEALTH;
    };

    virtual void Write(CNetworkMessage &nm);
    virtual void Read(CNetworkMessage &nm);
    virtual void Process(void);
};

class CExtEntityFlags : public CExtEntityPacket {
  protected:
    ULONG ulFlags; // Flags to apply
    UBYTE ubType; // Type of flags
    BOOL bRemove; // Disable flags instead of enabling

  public:
    CExtEntityFlags() : CExtEntityPacket(), ulFlags(0), ubType(0), bRemove(FALSE)
    {
    };

    // Set normal flags
    inline void EntityFlags(ULONG ul, BOOL bRemoveFlags) {
      ulFlags = ul;
      ubType = 0;
      bRemove = bRemoveFlags;
    };

    // Set physical flags
    inline void PhysicalFlags(ULONG ul, BOOL bRemoveFlags) {
      ulFlags = ul;
      ubType = 1;
      bRemove = bRemoveFlags;
    };

    // Set collision flags
    inline void CollisionFlags(ULONG ul, BOOL bRemoveFlags) {
      ulFlags = ul;
      ubType = 2;
      bRemove = bRemoveFlags;
    };

  public:
    virtual EType GetType(void) const {
      return EXT_ENTITY_FLAGS;
    };

    virtual void Write(CNetworkMessage &nm);
    virtual void Read(CNetworkMessage &nm);
    virtual void Process(void);
};

class CExtEntityMove : public CExtEntityPacket {
  public:
    FLOAT3D vSpeed; // Desired speed

  public:
    CExtEntityMove() : CExtEntityPacket(), vSpeed(0.0f, 0.0f, 0.0f)
    {
    };

  public:
    virtual EType GetType(void) const {
      return EXT_ENTITY_MOVE;
    };

    virtual void Write(CNetworkMessage &nm);
    virtual void Read(CNetworkMessage &nm);
    virtual void Process(void);
};

class CExtEntityRotate : public CExtEntityMove {
  public:
    virtual EType GetType(void) const {
      return EXT_ENTITY_ROTATE;
    };

    virtual void Process(void);
};

// Retrieve an entity from an ID
inline CEntity *FindExtEntity(ULONG ulID) {
  // Take last created entity if ID is 0
  CEntity *pen = CExtEntityCreate::penLast;

  if (ulID != 0) {
    pen = IWorld::FindEntityByID(IWorld::GetWorld(), ulID);
  }

  return pen;
};

// Report packet actions to the server
inline void ExtServerReport(const char *strFormat, ...) {
  // Ignore reports
  if (!_pNetwork->IsServer() || !ser_bReportExtPacketLogic) return;

  va_list arg;
  va_start(arg, strFormat);

  CTString str;
  str.VPrintF(strFormat, arg);

  CPutString(str);
  va_end(arg);
};

#endif // CLASSICSPATCH_EXT_PACKETS

#endif
