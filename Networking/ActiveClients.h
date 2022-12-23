/* Copyright (c) 2022 Dreamy Cecil
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

#ifndef CECIL_INCL_ACTIVECLIENTS_H
#define CECIL_INCL_ACTIVECLIENTS_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include "ClientIdentity.h"

// Currently active client
class CORE_API CActiveClient {
  public:
    // List of pointers to active clients
    typedef CDynamicContainer<CActiveClient> List;

  public:
    // Client identity that's using this client (none, if inactive)
    CClientIdentity *pClient;

    // Players of this client (none, if observer)
    CDynamicContainer<CPlayerBuffer> cPlayers;

    // Address the client is playing from
    SClientAddress addr;

    // Anti-flood system
    INDEX ctLastSecPackets; // Packets sent in the past second
    INDEX ctLastSecMessages; // Chat messages sent in the past second

  public:
    // Default constructor
    CActiveClient() : pClient(NULL)
    {
      ResetPacketCounters();
    };

    // Setup the client to be active
    void Set(CClientIdentity *pci, const SClientAddress &addrSet);

    // Reset the client to be inactive
    void Reset(void);

    // Reset anti-flood counters
    void ResetPacketCounters(void);

    // Check if client is active right now
    BOOL IsActive(void) const;

    // Add a new player
    void AddPlayer(CPlayerBuffer *pplb);

    // Get active clients with a specific identity
    static void GetActiveClients(CActiveClient::List &cClients, CClientIdentity *pci);
};

// Active clients by client IDs on the server
extern CStaticArray<CActiveClient> _aActiveClients;

#endif
