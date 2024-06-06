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

#ifndef CECIL_INCL_EXTPACKETS_H
#define CECIL_INCL_EXTPACKETS_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include "MessageCompression.h"

#if _PATCHCONFIG_EXT_PACKETS

// Report packet actions to the server
CORE_API extern INDEX ser_bReportExtPacketLogic;

// Base class for each extension packet
class CORE_API CExtPacket {
  public:
    // Built-in extension packets
    enum EType {
      // Server to all clients
      EXT_ENTITY_CREATE,   // Create new entity
      EXT_ENTITY_DELETE,   // Delete an entity
      EXT_ENTITY_COPY,     // Copy an entity
      EXT_ENTITY_EVENT,    // Send event to an entity
      EXT_ENTITY_ITEM,     // Receive item by an entity
      EXT_ENTITY_INIT,     // (Re)initialize an entity
      EXT_ENTITY_TELEPORT, // Teleport an entity
      EXT_ENTITY_POSITION, // Set position or rotation of an entity
      EXT_ENTITY_PARENT,   // Set entity's parent
      EXT_ENTITY_PROP,     // Change property value of an entity
      EXT_ENTITY_HEALTH,   // Set entity health
      EXT_ENTITY_FLAGS,    // Change various entity flags
      EXT_ENTITY_MOVE,     // Set absolute movement speed of an entity
      EXT_ENTITY_ROTATE,   // Set absolute rotation speed of an entity
      EXT_ENTITY_IMPULSE,  // Give impulse to some entity
      EXT_ENTITY_DIRDMG,   // Inflict damage to some entity
      EXT_ENTITY_RADDMG,   // Inflict damage at some point
      EXT_ENTITY_BOXDMG,   // Inflict damage in some area

      EXT_CHANGE_LEVEL,  // Force level change by creating vanilla WorldLink entity, if possible
      EXT_CHANGE_WORLD,  // Force immediate world change regardless of gameplay
      EXT_SESSION_PROPS, // Change data in session properties
      EXT_GAMEPLAY_EXT,  // Change data in gameplay extensions

      // Maximum amount of built-in packets
      EXT_MAX_PACKETS,
    };

  public:
    // Report packet actions to the server
    void ExtServerReport(const char *strFormat, ...);

    // Extension packet index
    virtual EType GetType(void) const = 0;

    // Internal name of the extension packet type
    virtual const char *GetName(void) const = 0;

    // Macro for defining packets of a specific type
    #define DEFINE_EXT_PACKET_FOR_TYPE(_Type) \
      virtual EType GetType(void) const { return _Type; }; \
      virtual const char *GetName(void) const { return #_Type; };

  public:
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

class CORE_API CExtEntityCreate : public CExtPacket {
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
    DEFINE_EXT_PACKET_FOR_TYPE(EXT_ENTITY_CREATE);

    virtual void Write(CNetworkMessage &nm);
    virtual void Read(CNetworkMessage &nm);
    virtual void Process(void);
};

// Base for entity manipulation packets
class CORE_API CExtEntityPacket : public CExtPacket {
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

    // Check if some entity exists
    static inline BOOL EntityExists(CEntity *pen) {
      return (pen != NULL && !(pen->GetFlags() & ENF_DELETED));
    };
};

// Holder for event fields
class CORE_API EExtEntityEvent : public CEntityEvent {
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

class CORE_API CExtEntityDelete : public CExtEntityPacket {
  public:
    BOOL bSameClass; // Delete all entities of the same class

  public:
    CExtEntityDelete() : CExtEntityPacket(), bSameClass(FALSE)
    {
    };

  public:
    DEFINE_EXT_PACKET_FOR_TYPE(EXT_ENTITY_DELETE);

    virtual void Write(CNetworkMessage &nm);
    virtual void Read(CNetworkMessage &nm);
    virtual void Process(void);
};

class CORE_API CExtEntityCopy : public CExtEntityPacket {
  public:
    UBYTE ubCopies;

  public:
    CExtEntityCopy() : CExtEntityPacket(), ubCopies(1)
    {
    };

  public:
    DEFINE_EXT_PACKET_FOR_TYPE(EXT_ENTITY_COPY);

    virtual void Write(CNetworkMessage &nm);
    virtual void Read(CNetworkMessage &nm);
    virtual void Process(void);
};

class CORE_API CExtEntityEvent : public CExtEntityPacket {
  protected:
    EExtEntityEvent eEvent; // Data holder
    ULONG ctFields; // Amount of used fields

  public:
    CExtEntityEvent() : CExtEntityPacket(), ctFields(0)
    {
    };

    // Copy event bytes (iEventSize = sizeof(ee))
    void SetEvent(CEntityEvent &ee, size_t iEventSize);

    // Copy event data from another event
    void Copy(const EExtEntityEvent &eeOther, ULONG ctSetFields);

  public:
    DEFINE_EXT_PACKET_FOR_TYPE(EXT_ENTITY_EVENT);

    virtual void Write(CNetworkMessage &nm);
    virtual void Read(CNetworkMessage &nm);
    virtual void Process(void);
};

class CORE_API CExtEntityItem : public CExtEntityEvent {
  public:
    CExtEntityItem() : CExtEntityEvent()
    {
    };

  public:
    DEFINE_EXT_PACKET_FOR_TYPE(EXT_ENTITY_ITEM);

    virtual void Process(void);
};

class CORE_API CExtEntityInit : public CExtEntityEvent {
  public:
    CExtEntityInit() : CExtEntityEvent()
    {
    };

  public:
    DEFINE_EXT_PACKET_FOR_TYPE(EXT_ENTITY_INIT);

    virtual void Process(void);
};

class CORE_API CExtEntityTeleport : public CExtEntityPacket {
  public:
    CPlacement3D plSet; // Placement to set
    BOOL bRelative; // Relative to the current placement (oriented)

  public:
    CExtEntityTeleport() : CExtEntityPacket(),
      plSet(FLOAT3D(0, 0, 0), ANGLE3D(0, 0, 0)), bRelative(FALSE)
    {
    };

  public:
    DEFINE_EXT_PACKET_FOR_TYPE(EXT_ENTITY_TELEPORT);

    virtual void Write(CNetworkMessage &nm);
    virtual void Read(CNetworkMessage &nm);
    virtual void Process(void);
};

class CORE_API CExtEntityPosition : public CExtEntityPacket {
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
    DEFINE_EXT_PACKET_FOR_TYPE(EXT_ENTITY_POSITION);

    virtual void Write(CNetworkMessage &nm);
    virtual void Read(CNetworkMessage &nm);
    virtual void Process(void);
};

class CORE_API CExtEntityParent : public CExtEntityPacket {
  public:
    ULONG ulParent;

  public:
    CExtEntityParent() : CExtEntityPacket(), ulParent(-1)
    {
    };

  public:
    DEFINE_EXT_PACKET_FOR_TYPE(EXT_ENTITY_PARENT);

    virtual void Write(CNetworkMessage &nm);
    virtual void Read(CNetworkMessage &nm);
    virtual void Process(void);
};

class CORE_API CExtEntityProp : public CExtEntityPacket {
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
    DEFINE_EXT_PACKET_FOR_TYPE(EXT_ENTITY_PROP);

    virtual void Write(CNetworkMessage &nm);
    virtual void Read(CNetworkMessage &nm);
    virtual void Process(void);
};

class CORE_API CExtEntityHealth : public CExtEntityPacket {
  public:
    FLOAT fHealth; // Health to set

  public:
    CExtEntityHealth() : CExtEntityPacket(), fHealth(0.0f)
    {
    };

  public:
    DEFINE_EXT_PACKET_FOR_TYPE(EXT_ENTITY_HEALTH);

    virtual void Write(CNetworkMessage &nm);
    virtual void Read(CNetworkMessage &nm);
    virtual void Process(void);
};

class CORE_API CExtEntityFlags : public CExtEntityPacket {
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
    DEFINE_EXT_PACKET_FOR_TYPE(EXT_ENTITY_FLAGS);

    virtual void Write(CNetworkMessage &nm);
    virtual void Read(CNetworkMessage &nm);
    virtual void Process(void);
};

class CORE_API CExtEntityMove : public CExtEntityPacket {
  public:
    FLOAT3D vSpeed; // Desired speed

  public:
    CExtEntityMove() : CExtEntityPacket(), vSpeed(0.0f, 0.0f, 0.0f)
    {
    };

  public:
    DEFINE_EXT_PACKET_FOR_TYPE(EXT_ENTITY_MOVE);

    virtual void Write(CNetworkMessage &nm);
    virtual void Read(CNetworkMessage &nm);
    virtual void Process(void);
};

class CORE_API CExtEntityRotate : public CExtEntityMove {
  public:
    CExtEntityRotate() : CExtEntityMove()
    {
    };

  public:
    DEFINE_EXT_PACKET_FOR_TYPE(EXT_ENTITY_ROTATE);

    virtual void Process(void);
};

class CORE_API CExtEntityImpulse : public CExtEntityMove {
  public:
    CExtEntityImpulse() : CExtEntityMove()
    {
    };

  public:
    DEFINE_EXT_PACKET_FOR_TYPE(EXT_ENTITY_IMPULSE);

    virtual void Process(void);
};

// Abstract damage packet
class CORE_API CExtEntityDamage : public CExtEntityPacket {
  public:
    ULONG eDamageType; // Damage type to use
    FLOAT fDamage; // Damage to inflict

  public:
    CExtEntityDamage() : CExtEntityPacket(), eDamageType(DMT_NONE), fDamage(0.0f)
    {
    };

    virtual void Write(CNetworkMessage &nm);
    virtual void Read(CNetworkMessage &nm);
};

class CORE_API CExtEntityDirectDamage : public CExtEntityDamage {
  public:
    ULONG ulTarget; // Target entity for damaging
    FLOAT3D vHitPoint; // Where exactly the damage occurred
    FLOAT3D vDirection; // From which direction the damage came from

  public:
    CExtEntityDirectDamage() : CExtEntityDamage(), ulTarget(-1)
    {
    };

  public:
    DEFINE_EXT_PACKET_FOR_TYPE(EXT_ENTITY_DIRDMG);

    virtual void Write(CNetworkMessage &nm);
    virtual void Read(CNetworkMessage &nm);
    virtual void Process(void);
};

class CORE_API CExtEntityRangeDamage : public CExtEntityDamage {
  public:
    FLOAT3D vCenter; // Place to inflict damage from
    FLOAT fFallOff; // Total damage radius
    FLOAT fHotSpot; // Full damage radius

  public:
    CExtEntityRangeDamage() : CExtEntityDamage(), vCenter(0, 0, 0), fFallOff(0.0f), fHotSpot(0.0f)
    {
    };

  public:
    DEFINE_EXT_PACKET_FOR_TYPE(EXT_ENTITY_RADDMG);

    virtual void Write(CNetworkMessage &nm);
    virtual void Read(CNetworkMessage &nm);
    virtual void Process(void);
};

class CORE_API CExtEntityBoxDamage : public CExtEntityDamage {
  public:
    FLOATaabbox3D boxArea; // Area to inflict the damage in

  public:
    CExtEntityBoxDamage() : CExtEntityDamage(), boxArea(FLOAT3D(0, 0, 0), 0.0f)
    {
    };

  public:
    DEFINE_EXT_PACKET_FOR_TYPE(EXT_ENTITY_BOXDMG);

    virtual void Write(CNetworkMessage &nm);
    virtual void Read(CNetworkMessage &nm);
    virtual void Process(void);
};

class CORE_API CExtChangeLevel : public CExtPacket {
  public:
    CTString strWorld; // World file to change to

  public:
    CExtChangeLevel()
    {
    };

  public:
    DEFINE_EXT_PACKET_FOR_TYPE(EXT_CHANGE_LEVEL);

    virtual void Write(CNetworkMessage &nm);
    virtual void Read(CNetworkMessage &nm);
    virtual void Process(void);
};

class CORE_API CExtChangeWorld : public CExtChangeLevel {
  public:
    CExtChangeWorld() : CExtChangeLevel()
    {
    };

  public:
    DEFINE_EXT_PACKET_FOR_TYPE(EXT_CHANGE_WORLD);

    virtual void Process(void);
};

class CORE_API CExtSessionProps : public CExtPacket {
  public:
    CSesPropsContainer sp; // Session properties to set (data that's not processed isn't being zeroed!)
    SLONG slSize; // Amount of bytes to set
    SLONG slOffset; // Starting byte (up to NET_MAXSESSIONPROPERTIES - 1)

  public:
    CExtSessionProps() : slSize(0), slOffset(0)
    {
    };

    // Set new data at the current end and expand session properties size
    BOOL AddData(const void *pData, size_t ctBytes);

  public:
    DEFINE_EXT_PACKET_FOR_TYPE(EXT_SESSION_PROPS);

    virtual void Write(CNetworkMessage &nm);
    virtual void Read(CNetworkMessage &nm);
    virtual void Process(void);
};

class CORE_API CExtGameplayExt : public CExtPacket {
  private:
    UWORD iVar; // Variable in the structure (0 is invalid, starts from 1)
    BOOL bString; // Using a string value
    CTString strValue;
    DOUBLE fValue;

  public:
    CExtGameplayExt() : iVar(0), bString(FALSE), fValue(0.0)
    {
    };

    // Find variable index by its name
    UWORD FindVar(const CTString &strVar);

    // Set string value
    void SetValue(const CTString &strVar, const CTString &str);

    // Set number value
    void SetValue(const CTString &strVar, DOUBLE f);

  public:
    DEFINE_EXT_PACKET_FOR_TYPE(EXT_GAMEPLAY_EXT);

    virtual void Write(CNetworkMessage &nm);
    virtual void Read(CNetworkMessage &nm);
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

#endif // _PATCHCONFIG_EXT_PACKETS

#endif
