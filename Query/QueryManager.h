/* Copyright (c) 2022-2024 Dreamy Cecil
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

#ifndef CECIL_INCL_QUERYMANAGER_H
#define CECIL_INCL_QUERYMANAGER_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include "MasterServer.h"
#include "ServerRequest.h"

// Master server protocols
enum EMasterServers {
  E_MS_LEGACY     = 0, // GameSpy emulation (default)
  E_MS_DARKPLACES = 1, // Dark Places
  E_MS_GAMEAGENT  = 2, // GameAgent from 1.10

  E_MS_MAX,
};

// Debug output for query
CORE_API extern INDEX ms_bDebugOutput;

// Commonly used symbols
extern CSymbolPtr _piNetPort;
extern CSymbolPtr _pstrLocalHost;

// Internal query functionality
namespace IQuery {

#pragma pack(push, 1)

// Structure housing an IP address with a port
struct Address {
  union {
    UBYTE aIP[4]; // IP address byte by byte
    ULONG ulIP; // Full IP address
  };
  UWORD uwPort; // Port

  // Add new server request from a received address
  void AddServerRequest(const char **ppBuffer, INDEX &iLength, const UWORD uwSetPort, const char *strPacket, SOCKET iSocketUDP = NULL);
};

#pragma pack(pop)

extern sockaddr_in sinFrom;
extern char *pBuffer;

extern BOOL bServer;
extern BOOL bInitialized;

extern CDynamicStackArray<SServerRequest> aRequests;

// Initialize the socket
void InitWinsock(void);

// Close the socket
void CloseWinsock();

// Check if the socket is usable
BOOL IsSocketUsable(void);

// Send data packet
void SendPacket(const char *pBuffer, int iLength = -1);

// Send data packet to a specific socket address
void SendPacketTo(sockaddr_in *psin, const char *pBuffer, int iLength, SOCKET iSocket = NULL);

// Send reply packet with a message
void SendReply(const CTString &strMessage);

// Receive some packet
int ReceivePacket(void);

// Set enumeration status
void SetStatus(const CTString &strStatus);

// GameAgent protocol
namespace GameAgent {
  void BuildHearthbeatPacket(CTString &strPacket, INDEX iChallenge);
  void EnumTrigger(BOOL bInternet);
  void EnumUpdate(void);
  void ServerParsePacket(INDEX iLength);
};

// Legacy protocol
namespace Legacy {
  void BuildHearthbeatPacket(CTString &strPacket);
  void EnumTrigger(BOOL bInternet);
  void EnumUpdate(void);
  void ServerParsePacket(INDEX iLength);
};

// DarkPlaces protocol
namespace DarkPlaces {
  void BuildHearthbeatPacket(CTString &strPacket);
  void EnumTrigger(BOOL bInternet);
  void EnumUpdate(void);
  void ServerParsePacket(INDEX iLength);
};

}; // namespace

// Game key and game name for the master server
#define SAM_MS_KEY "AKbna4\0"
#define SAM_MS_NAME CHOOSE_FOR_GAME("serioussam", "serioussamse", "serioussamse")

#endif
