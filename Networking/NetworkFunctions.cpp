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

#include "NetworkFunctions.h"
#include "MessageProcessing.h"

// Handle packets coming from a client (CServer::Handle alternative)
BOOL INetwork::ServerHandle(CMessageDispatcher *pmd, INDEX iClient, CNetworkMessage &nmMessage) {
  // Process some default packets
  switch (nmMessage.GetType()) {
    // Client requesting the session state
    case MSG_REQ_CONNECTREMOTESESSIONSTATE:
      return OnConnectRemoteSessionStateRequest(iClient, nmMessage);

    // Client requesting the connection to the server
    case MSG_REQ_CONNECTPLAYER:
      return OnPlayerConnectRequest(iClient, nmMessage);

    // Client changing the character
    case MSG_REQ_CHARACTERCHANGE:
      return OnCharacterChangeRequest(iClient, nmMessage);

    // Client sending a chat message
    case MSG_CHAT_IN:
      return OnChatInRequest(iClient, nmMessage);
  }

  // Let CServer::Handle process packets of other types
  if (nmMessage.GetType() != PCK_EXTENSION) return TRUE;

  CServer &srv = _pNetwork->ga_srvServer;

  // Handle specific packet types
  ULONG ulType;
  nmMessage >> ulType;

  // Let plugins handle packets
  FOREACHPLUGINHANDLER(GetPluginAPI()->cNetworkEvents, INetworkEvents, pEvents) {
    if ((IAbstractEvents *)pEvents == NULL) continue;

    // Handle packet through this plugin handler
    if (pEvents->OnServerPacket(nmMessage, ulType)) {
      // Quit if packet has been handled
      return FALSE;
    }
  }

  switch (ulType)
  {
    case 0: // [Cecil] TEMP
    
    // Invalid packets
    default: {
      CPrintF("Server received PCK_EXTENSION of an invalid (%u) type!\n", ulType);
      ASSERT(FALSE);
    }
  }
  
  // No extra processing needed
  return FALSE;
};

// Handle packets coming from a server
BOOL INetwork::ClientHandle(CSessionState *pses, CNetworkMessage &nmMessage) {
  // Let default methods handle packets of other types
  if (nmMessage.GetType() != PCK_EXTENSION) return TRUE;
  
  // Handle specific packet types
  ULONG ulType;
  nmMessage >> ulType;

  // Let plugins handle packets
  FOREACHPLUGINHANDLER(GetPluginAPI()->cNetworkEvents, INetworkEvents, pEvents) {
    if ((IAbstractEvents *)pEvents == NULL) continue;

    // Handle packet through this plugin handler
    if (pEvents->OnClientPacket(nmMessage, ulType)) {
      // Quit if packet has been handled
      return FALSE;
    }
  }

  switch (ulType)
  {
    case 0: // [Cecil] TEMP

    // Invalid packets
    default: {
      CPrintF("Client received PCK_EXTENSION of an invalid (%u) type!\n", ulType);
      ASSERT(FALSE);
    }
  }

  // No extra processing needed
  return FALSE;
};

// Send disconnect message to a client (CServer::SendDisconnectMessage reimplementation)
void INetwork::SendDisconnectMessage(INDEX iClient, const char *strExplanation, BOOL bStream) {
  // Not a server
  if (!_pNetwork->IsServer()) {
    return;
  }

  CSessionSocket &sso = _pNetwork->ga_srvServer.srv_assoSessions[iClient];

  if (!bStream) {
    // Compose message
    CNetworkMessage nmDisconnect(MSG_INF_DISCONNECTED);
    nmDisconnect << CTString(strExplanation);

    // Send it
    _pNetwork->SendToClientReliable(iClient, nmDisconnect);

  } else {
    CTMemoryStream strmDisconnect;
    strmDisconnect << INDEX(MSG_INF_DISCONNECTED);
    strmDisconnect << CTString(strExplanation);

    // Send the stream to the remote session state
    _pNetwork->SendToClientReliable(iClient, strmDisconnect);
  }

  // Report that it has gone away
  CPrintF(TRANS("Client '%s' ordered to disconnect: %s\n"), GetComm().Server_GetClientName(iClient), strExplanation);

  // If not disconnected before
  if (sso.sso_iDisconnectedState == 0) {
    // Mark the disconnection
    sso.sso_iDisconnectedState = 1;

  // If the client was already kicked before, but is still hanging here
  } else {
    // Force the disconnection
    CPrintF(TRANS("Forcing client '%s' to disconnect\n"), GetComm().Server_GetClientName(iClient));

    sso.sso_iDisconnectedState = 2;
  }
};

// Send chat message to a client with custom name of a sender
void INetwork::SendChatToClient(INDEX iClient, const CTString &strFromName, const CTString &strMessage) {
  // Not a server
  if (!_pNetwork->IsServer()) {
    return;
  }

  CNetworkMessage nm(MSG_CHAT_OUT);
  nm << (INDEX)0;
  nm << strFromName;
  nm << strMessage;

  _pNetwork->SendToClient(iClient, nm);
};
