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

// Base class for each extension packet
class CExtPacket {
  public:
    // Built-in extension packets
    enum EType {
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

// Report packet actions to the server
inline void ExtServerReport(const char *strFormat, ...) {
  if (!_pNetwork->IsServer()) return;

  va_list arg;
  va_start(arg, strFormat);

  CTString str;
  str.VPrintF(strFormat, arg);

  CPutString(str);
  va_end(arg);
};

#endif
