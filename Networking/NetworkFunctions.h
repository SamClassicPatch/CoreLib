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

#ifndef CECIL_INCL_NETWORKPACKETS_H
#define CECIL_INCL_NETWORKPACKETS_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include "CommInterface.h"
#include "StreamBlock.h"

// Interface of network methods
class CORE_API INetwork {
  public:
    // New network packet types
    enum ENetPackets {
      // Start with 49 to continue the MESSAGETYPE / NetworkMessageType list
      PCK_DUMMY_NETWORK_PACKET = 49,

      // Allow 49-62 range for mod packets

      // Occupy the last index for new packets (cannot go above 63)
      PCK_EXTENSION = 63,
    };

    // Create network message packet of a custom type
    #define NETWORK_PACKET(Variable, PacketType) \
      MESSAGETYPE Variable##_##PacketType = MESSAGETYPE(PacketType); \
      CNetworkMessage Variable(Variable##_##PacketType)

  public:
    // Create packet to send to a server
    static inline CNetworkMessage CreateClientPacket(const ULONG ulType) {
      NETWORK_PACKET(nmClient, PCK_EXTENSION);
      nmClient << ulType; // Packet type

      return nmClient;
    };

    // Create packet to send to all clients
    static inline CNetStreamBlock CreateServerPacket(const ULONG ulType) {
      CServer &srv = _pNetwork->ga_srvServer;

      CNetStreamBlock nsbServer(PCK_EXTENSION, ++srv.srv_iLastProcessedSequence);
      nsbServer << ulType; // Packet type

      return nsbServer;
    };

    // Add block to streams for all sessions
    static inline void AddBlockToAllSessions(CNetStreamBlock &nsb) {
      CServer &srv = _pNetwork->ga_srvServer;

      // For each active session
      for (INDEX iSession = 0; iSession < srv.srv_assoSessions.Count(); iSession++) {
        CSessionSocket &sso = srv.srv_assoSessions[iSession];

        if (iSession > 0 && !sso.sso_bActive) {
          continue;
        }

        // Add the block to the buffer
        ((CNetStream &)sso.sso_nsBuffer).AddBlock(nsb);
      }
    };

    // Handle packets coming from a client (CServer::Handle alternative)
    static BOOL ServerHandle(CMessageDispatcher *pmd, INDEX iClient, CNetworkMessage &nmReceived);

    // Handle packets coming from a server
    static BOOL ClientHandle(CSessionState *pses, CNetworkMessage &nmMessage);

    // Send disconnect message to a client (CServer::SendDisconnectMessage reimplementation)
    static void SendDisconnectMessage(INDEX iClient, const char *strExplanation, BOOL bStream);

    // Send chat message to a client with custom name of a sender
    static void SendChatToClient(INDEX iClient, const CTString &strFromName, const CTString &strMessage);

  // CServer method reimplementations
  public:

    // Get number of active players
    static inline INDEX CountPlayers(BOOL bOnlyVIP) {
      CServer &srv = _pNetwork->ga_srvServer;
      INDEX ctPlayers = 0;

      FOREACHINSTATICARRAY(srv.srv_aplbPlayers, CPlayerBuffer, itplb)
      {
        // Skip inactive players
        if (!itplb->IsActive()) continue;

        // Any or VIP
        if (!bOnlyVIP || srv.srv_assoSessions[itplb->plb_iClient].sso_bVIP) {
          ctPlayers++;
        }
      }

      return ctPlayers;
    };

    // Get number of active clients
    static inline INDEX CountClients(BOOL bOnlyVIP) {
      CServer &srv = _pNetwork->ga_srvServer;

      const INDEX ctSessions = srv.srv_assoSessions.Count();
      INDEX ctClients = 0;

      for (INDEX i = 0; i < ctSessions; i++) {
        CSessionSocket &sso = srv.srv_assoSessions[i];

        // Skip inactive clients
        if (i > 0 && !sso.sso_bActive) continue;

        // Any or VIP
        if (!bOnlyVIP || sso.sso_bVIP) {
          ctClients++;
        }
      }

      return ctClients;
    };

    // Get number of active observers
    static inline INDEX CountObservers(void) {
      CServer &srv = _pNetwork->ga_srvServer;

      const INDEX ctSessions = srv.srv_assoSessions.Count();
      INDEX ctClients = 0;

      for (INDEX i = 0; i < ctSessions; i++) {
        CSessionSocket &sso = srv.srv_assoSessions[i];

        // Skip inactive clients
        if (i > 0 && !sso.sso_bActive) continue;

        // Client with no players
        if (sso.sso_ctLocalPlayers == 0) {
          ctClients++;
        }
      }

      return ctClients;
    };

    // Get number of active players of a specific client
    static inline INDEX CountClientPlayers(INDEX iClient) {
      CServer &srv = _pNetwork->ga_srvServer;
      INDEX ctPlayers = 0;

      FOREACHINSTATICARRAY(srv.srv_aplbPlayers, CPlayerBuffer, itplb)
      {
        // Active player with a matching client index
        if (itplb->IsActive() && itplb->plb_iClient == iClient) {
          ctPlayers++;
        }
      }

      return ctPlayers;
    };

    // Find first inactive client
    static inline CPlayerBuffer *FirstInactivePlayer(void) {
      CServer &srv = _pNetwork->ga_srvServer;

      FOREACHINSTATICARRAY(srv.srv_aplbPlayers, CPlayerBuffer, itplb)
      {
        // Found inactive player
        if (!itplb->IsActive()) {
          return itplb;
        }
      }

      // No inactive players
      return NULL;
    };

    // Check if some character already exists in this session
    static inline BOOL IsCharacterUsed(const CPlayerCharacter &pc) {
      CServer &srv = _pNetwork->ga_srvServer;

      FOREACHINSTATICARRAY(srv.srv_aplbPlayers, CPlayerBuffer, itplb)
      {
        // Active player with a matching character
        if (itplb->IsActive() && itplb->plb_pcCharacter == pc) {
          return TRUE;
        }
      }

      // No matching character found
      return FALSE;
    };

    // Compose a bit mask of all players of a specific client
    static inline ULONG MaskOfClientPlayers(INDEX iClient) {
      CServer &srv = _pNetwork->ga_srvServer;

      const INDEX ctPlayers = srv.srv_aplbPlayers.Count();
      ULONG ulMask = 0;

      for (INDEX i = 0; i < ctPlayers; i++) {
        CPlayerBuffer &plb = srv.srv_aplbPlayers[i];

        // Active player with a matching client index
        if (plb.IsActive() && plb.plb_iClient == iClient) {
          ulMask |= (1UL << i);
        }
      }

      return ulMask;
    };
};

#endif
