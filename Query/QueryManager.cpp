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

#include "StdH.h"

#include "QueryManager.h"

#pragma comment(lib, "wsock32.lib")

// Internal query fields
sockaddr_in IQuery::sinFrom;
char *IQuery::pBuffer = NULL;

BOOL IQuery::bServer = FALSE;
BOOL IQuery::bInitialized = FALSE;

CDynamicStackArray<SServerRequest> IQuery::aRequests;

static WSADATA *_wsaData = NULL;
static sockaddr_in *_sin = NULL;
static sockaddr_in *_sinLocal = NULL;

static SOCKET _socket = NULL;

// Master server addresses
extern CTString ms_strLegacyMS = "master.333networks.com";
static CTString ms_strGameAgentMS = "master.333networks.com";
static CTString ms_strDarkPlacesMS = "192.168.1.4";

// Current master server protocol
extern INDEX ms_iProtocol = E_MS_LEGACY;

// Debug output for query
INDEX ms_bDebugOutput = FALSE;

// Commonly used symbols
CSymbolPtr _piNetPort;
CSymbolPtr _pstrLocalHost;

// Initialize query manager
extern void InitQuery(void) {
  // Custom symbols
  _pShell->DeclareSymbol("persistent user CTString ms_strLegacyMS;",     &ms_strLegacyMS);
  _pShell->DeclareSymbol("persistent user CTString ms_strDarkPlacesMS;", &ms_strDarkPlacesMS);
  _pShell->DeclareSymbol("persistent user CTString ms_strGameAgentMS;",  &ms_strGameAgentMS);
  _pShell->DeclareSymbol("persistent user INDEX ms_iProtocol;",          &ms_iProtocol);
  _pShell->DeclareSymbol("persistent user INDEX ms_bDebugOutput;",       &ms_bDebugOutput);

  // Master server protocol types
  static const INDEX iLegacyMS     = E_MS_LEGACY;
  static const INDEX iDarkPlacesMS = E_MS_DARKPLACES;
  static const INDEX iGameAgentMS  = E_MS_GAMEAGENT;
  _pShell->DeclareSymbol("const INDEX MS_LEGACY;",     (void *)&iLegacyMS);
  _pShell->DeclareSymbol("const INDEX MS_DARKPLACES;", (void *)&iDarkPlacesMS);
  _pShell->DeclareSymbol("const INDEX MS_GAMEAGENT;",  (void *)&iGameAgentMS);

  // Retrieve commonly used symbols
  _piNetPort.Find("net_iPort");
  _pstrLocalHost.Find("net_strLocalHost");
};

// Initialize the socket
void IQuery::InitWinsock(void) {
  // Already initialized
  if (_wsaData != NULL && _socket != NULL) {
    return;
  }

  // Create new socket address
  _wsaData = new WSADATA;
  _socket = NULL;

  // Create a buffer for packets
  if (IQuery::pBuffer != NULL) {
    delete[] IQuery::pBuffer;
  }
  IQuery::pBuffer = new char[2050];

  // Start socket address
  if (WSAStartup(MAKEWORD(2, 2), _wsaData) != 0) {
    // Something went wrong
    CPutString("Error initializing winsock!\n");
    CloseWinsock();
    return;
  }

  // Master server addresses
  static const CTString astrIPs[E_MS_MAX] = {
    ms_strLegacyMS,
    ms_strDarkPlacesMS,
    ms_strGameAgentMS,
  };

  // Get host from the address
  const CTString &strMasterServerIP = astrIPs[IMasterServer::GetProtocol()];
  hostent* phe = gethostbyname(strMasterServerIP);

  // Couldn't resolve the hostname
  if (phe == NULL) {
    CPrintF("Couldn't resolve the host server '%s'\n", strMasterServerIP);
    CloseWinsock();
    return;
  }

  // Create destination address
  _sin = new sockaddr_in;
  _sin->sin_family = AF_INET;
  _sin->sin_addr.s_addr = *(ULONG *)phe->h_addr_list[0];

  // Master server ports
  static const UWORD aiPorts[E_MS_MAX] = {
    27900,
    27950,
    9005,
  };

  // Select master server port
  _sin->sin_port = htons(aiPorts[IMasterServer::GetProtocol()]);

  // Create the socket
  _socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  // If it's a server
  if (IQuery::bServer) {
    // Create local socket source address
    _sinLocal = new sockaddr_in;
    _sinLocal->sin_family = AF_INET;
    _sinLocal->sin_addr.s_addr = inet_addr("0.0.0.0");
    _sinLocal->sin_port = htons(_piNetPort.GetIndex() + 1);

    // Allow receiving UDP broadcast
    int iOpt = 1;

    if (setsockopt(_socket, SOL_SOCKET, SO_BROADCAST, (char *)&iOpt, sizeof(iOpt)) != 0) {
      CPutString("Couldn't allow receiving UDP broadcast for the socket!\n");
      CloseWinsock();
      return;
    }

    // Bind the socket
    bind(_socket, (sockaddr *)_sinLocal, sizeof(*_sinLocal));
  }

  // Set socket to be non-blocking
  DWORD dwNonBlocking = 1;

  if (ioctlsocket(_socket, FIONBIO, &dwNonBlocking) != 0) {
    CPutString("Couldn't make socket non-blocking!\n");
    CloseWinsock();
  }
};

// Close the socket
void IQuery::CloseWinsock(void) {
  // If socket address exists
  if (_wsaData != NULL) {
    // Close the socket
    closesocket(_socket);

    // Delete socket address
    delete _wsaData;
    _wsaData = NULL;
  }

  // Reset socket
  _socket = NULL;
};

// Check if the socket is usable
BOOL IQuery::IsSocketUsable(void) {
  return (_socket != NULL && bInitialized);
};

// Send packet with data from a buffer
void IQuery::SendPacket(const char *pBuffer, int iLength) {
  // Initialize the socket in case it's not
  InitWinsock();

  // Calculate buffer length
  if (iLength == -1) {
    iLength = strlen(pBuffer);
  }

  SendPacketTo(_sin, pBuffer, iLength);
};

// Send data packet through a specific socket
void IQuery::SendPacketTo(sockaddr_in *psin, const char *pBuffer, int iLength) {
  sendto(_socket, pBuffer, iLength, 0, (sockaddr *)psin, sizeof(sockaddr_in));
};

// Send reply packet with a message
void IQuery::SendReply(const CTString &strMessage) {
  SendPacketTo(&IQuery::sinFrom, strMessage.str_String, strMessage.Length());
};

// Receive some packet
int IQuery::ReceivePacket(void) {
  int ctFrom = sizeof(IQuery::sinFrom);
  return recvfrom(_socket, IQuery::pBuffer, 2048, 0, (sockaddr *)&IQuery::sinFrom, &ctFrom);
};

// Set enumeration status
void IQuery::SetStatus(const CTString &strStatus) {
  _pNetwork->ga_bEnumerationChange = TRUE;
  _pNetwork->ga_strEnumerationStatus = strStatus;
};