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

#include "CommInterface.h"
#include "StreamBlock.h"

// Interface of network methods
class INetwork {
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
};

#endif
