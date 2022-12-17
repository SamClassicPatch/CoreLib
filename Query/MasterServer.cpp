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

#include "MasterServer.h"
#include "QueryMgr.h"
#include "Networking/CommInterface.h"

// When the last heartbeat has been sent
static TIME _tmLastHeartbeat = -1.0f;

// Get current master server protocol
INDEX IMasterServer::GetProtocol(void) {
  extern INDEX ms_iProtocol;

  if (ms_iProtocol < E_MS_LEGACY || ms_iProtocol >= E_MS_MAX) {
    return E_MS_LEGACY;
  }

  return ms_iProtocol;
};

// Start the server
void IMasterServer::OnServerStart(void) {
  if (ms_bDebugOutput) {
    CPutString("  IMasterServer::OnServerStart()\n");
  }

  // Initialize as a server
  IQuery::bServer = TRUE;
  IQuery::bInitialized = TRUE;

  // Send opening packet to the master server
  switch (GetProtocol()) {
    case E_MS_LEGACY: {
      CTString strPacket;
      strPacket.PrintF("\\heartbeat\\%hu\\gamename\\%s", (_piNetPort.GetIndex() + 1), SAM_MS_NAME);

      IQuery::SendPacket(strPacket);
    } break;

    case E_MS_DARKPLACES: {
      CTString strPacket;
      strPacket.PrintF("\xFF\xFF\xFF\xFFheartbeat DarkPlaces\x0A");

      IQuery::SendPacket(strPacket);
    } break;

    case E_MS_GAMEAGENT: {
      IQuery::SendPacket("q");
    } break;
  }
};

// Stop the server
void IMasterServer::OnServerEnd(void) {
  // Not initialized
  if (!IQuery::bInitialized) {
    return;
  }

  if (ms_bDebugOutput) {
    CPutString("  IMasterServer::OnServerEnd()\n");
  }

  const INDEX iProtocol = GetProtocol();

  // Send double heartbeat for Dark Places
  if (iProtocol == E_MS_DARKPLACES) {
    SendHeartbeat(0);
    SendHeartbeat(0);

  // Send server closing packet to anything but GameAgent
  } else if (iProtocol == E_MS_LEGACY) {
    CTString strPacket;
    strPacket.PrintF("\\heartbeat\\%hu\\gamename\\%s\\statechanged", (_piNetPort.GetIndex() + 1), SAM_MS_NAME);
    IQuery::SendPacket(strPacket);

    if (ms_bDebugOutput) {
      CPrintF("Server end:\n%s\n", strPacket);
    }
  }

  // Close the socket
  IQuery::CloseWinsock();
  IQuery::bInitialized = FALSE;
};

// Server update step
void IMasterServer::OnServerUpdate(void) {
  // Not usable
  if (!IQuery::IsSocketUsable()) {
    return;
  }

  // Receive new packet
  memset(&IQuery::pBuffer[0], 0, 2050);
  INDEX iLength = IQuery::ReceivePacket();

  // If there's any data
  if (iLength > 0) {
    if (ms_bDebugOutput) {
      CPrintF("Received packet (%d bytes)\n", iLength);
    }

    // Parse received packet
    static void (*apParsePacket[E_MS_MAX])(INDEX) = {
      &CLegacyQuery::ServerParsePacket,
      &CDarkPlacesQuery::ServerParsePacket,
      &CGameAgentQuery::ServerParsePacket,
    };

    (*apParsePacket[GetProtocol()])(iLength);
  }

  // Send a heartbeat every 150 seconds
  if (_pTimer->GetRealTimeTick() - _tmLastHeartbeat >= 150.0f) {
    SendHeartbeat(0);
  }
};

// Server state has changed
void IMasterServer::OnServerStateChanged(void) {
  // Not initialized
  if (!IQuery::bInitialized) {
    return;
  }

  if (ms_bDebugOutput) {
    CPutString("  IMasterServer::OnServerStateChanged()\n");
  }

  // Notify master server about the state change
  switch (GetProtocol()) {
    // Legacy
    case E_MS_LEGACY: {
      CTString strPacket;
      strPacket.PrintF("\\heartbeat\\%hu\\gamename\\%s\\statechanged", (_piNetPort.GetIndex() + 1), SAM_MS_NAME);

      IQuery::SendPacket(strPacket);

      if (ms_bDebugOutput) {
        CPrintF("Sending state change:\n%s\n", strPacket);
      }
    } break;

    // Nothing for Dark Places

    // GameAgent
    case E_MS_GAMEAGENT: {
      IQuery::SendPacket("u");
    } break;
  }
};

// Send heartbeat to the master server
void IMasterServer::SendHeartbeat(INDEX iChallenge) {
  CTString strPacket;

  // Build heartbeat packet for a specific master server
  switch (GetProtocol()) {
    case E_MS_LEGACY:
      CLegacyQuery::BuildHearthbeatPacket(strPacket);
      break;

    case E_MS_DARKPLACES:
      CDarkPlacesQuery::BuildHearthbeatPacket(strPacket);
      break;

    case E_MS_GAMEAGENT:
      CGameAgentQuery::BuildHearthbeatPacket(strPacket, iChallenge);
      break;
  }

  if (ms_bDebugOutput) {
    CPrintF("Sending heartbeat:\n%s\n", strPacket);
  }

  // Send heartbeat to the master server
  IQuery::SendPacket(strPacket);
  _tmLastHeartbeat = _pTimer->GetRealTimeTick();
};

// Request server list enumeration
void IMasterServer::EnumTrigger(BOOL bInternet) {
  // The list has changed
  if (_pNetwork->ga_bEnumerationChange) {
    return;
  }

  // Request for a specific master server
  static void (*apEnumTrigger[E_MS_MAX])(BOOL) = {
    &CLegacyQuery::EnumTrigger,
    &CDarkPlacesQuery::EnumTrigger,
    &CGameAgentQuery::EnumTrigger,
  };

  (*apEnumTrigger[GetProtocol()])(bInternet);
};

// Replacement for CNetworkLibrary::EnumSessions()
void IMasterServer::EnumSessions(BOOL bInternet) {
  // Clear old sessions
  FORDELETELIST(CNetworkSession, ns_lnNode, _pNetwork->ga_lhEnumeratedSessions, itns) {
    delete &*itns;
  }

  if (!GetComm().IsNetworkEnabled()) {
    // Have to enumerate as server
    GetComm().PrepareForUse(TRUE, FALSE);
  }

  // Enumerate sessions using new query manager
  EnumTrigger(bInternet);
};

// Update enumerations from the server
void IMasterServer::EnumUpdate(void) {
  // Not usable
  if (!IQuery::IsSocketUsable()) {
    return;
  }

  // Call update method for a specific master server
  static void (*apEnumUpdate[E_MS_MAX])(void) = {
    &CLegacyQuery::EnumUpdate,
    &CDarkPlacesQuery::EnumUpdate,
    &CGameAgentQuery::EnumUpdate,
  };

  (*apEnumUpdate[GetProtocol()])();
};

// Cancel master server enumeration
void IMasterServer::EnumCancel(void) {
  // Not initialized
  if (!IQuery::bInitialized) {
    return;
  }

  // Delete server requests and close the socket
  IQuery::aRequests.Clear();
  IQuery::CloseWinsock();
};
