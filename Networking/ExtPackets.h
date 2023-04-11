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
      EXT_ENTITY_INIT,     // (Re)initialize an entity
      EXT_ENTITY_TELEPORT, // Teleport an entity
      EXT_ENTITY_PARENT,   // Set entity's parent
      EXT_ENTITY_PROP,     // Change property value of an entity

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

class CExtEntityPacket : public CExtPacket {
  public:
    ULONG ulEntity; // Entity ID in the world (31 bits)

  public:
    CExtEntityPacket() : ulEntity(0x7FFFFFFF)
    {
    };

    // Write entity ID
    void WriteEntity(CNetworkMessage &nm) {
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

class CExtEntityInit : public CExtEntityPacket {
  public:
    ULONG aulEventValues[16];

  public:
    CExtEntityInit() : CExtEntityPacket()
    {
      // Invalid/empty values
      for (INDEX i = 0; i < 16; i++) {
        aulEventValues[i] = (ULONG)-1;
      }
    };

  public:
    virtual EType GetType(void) const {
      return EXT_ENTITY_INIT;
    };

    virtual void Write(CNetworkMessage &nm);
    virtual void Read(CNetworkMessage &nm);
    virtual void Process(void);
};

class CExtEntityTeleport : public CExtEntityPacket {
  public:
    CPlacement3D plPos;
    BOOL bRelative;

  public:
    CExtEntityTeleport() : CExtEntityPacket(),
      plPos(FLOAT3D(0, 0, 0), ANGLE3D(0, 0, 0)), bRelative(FALSE)
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
    ULONG ulPropHash; // Property name hash

  public:
    BOOL bName; // Using a name
    CTString strProp; // Property name
    ULONG ulPropID; // Property ID

    BOOL bString; // Using a string value
    CTString strValue;
    FLOAT fValue;

  public:
    CExtEntityProp() : CExtEntityPacket(),
      ulPropHash(0), bName(FALSE), ulPropID(0), bString(FALSE), fValue(0.0f)
    {
    };

  public:
    virtual EType GetType(void) const {
      return EXT_ENTITY_PROP;
    };

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
