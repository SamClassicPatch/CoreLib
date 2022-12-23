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

#include "MessageProcessing.h"
#include "NetworkFunctions.h"
#include "Modules.h"

#include "Interfaces/DataFunctions.h"
#include "Query/QueryManager.h"

// Client confirming the disconnection
BOOL OnClientDisconnect(INDEX iClient, CNetworkMessage &nmMessage) {
  CSessionSocket &sso = _pNetwork->ga_srvServer.srv_assoSessions[iClient];
  sso.sso_iDisconnectedState = 2;

  return FALSE;
};

// Client requesting the session state
BOOL OnConnectRemoteSessionStateRequest(INDEX iClient, CNetworkMessage &nmMessage)
{
  // Check for connecting clients with split-screen
  if (!CheckSplitScreenClients(iClient, nmMessage)) {
    return FALSE;
  }

  return TRUE;
};

// Client requesting the connection to the server
BOOL OnPlayerConnectRequest(INDEX iClient, CNetworkMessage &nmMessage)
{
  // Read character data
  CPlayerCharacter pcCharacter;
  nmMessage >> pcCharacter;

  static CSymbolPtr pstrNameMask("ser_strNameMask");
  static CSymbolPtr pbWhiteList("ser_bInverseBanning");

  // Character name is banned
  if (IData::MatchesMask(pcCharacter.GetName(), pstrNameMask.GetString()) == !pbWhiteList.GetIndex()) {
    INetwork::SendDisconnectMessage(iClient, TRANS("You are banned from this server"), TRUE);
    return FALSE;
  }

  CServer &srv = _pNetwork->ga_srvServer;
  CSessionSocket &sso = srv.srv_assoSessions[iClient];

  // Check if someone's connecting with too many players
  if (iClient > 0 && INetwork::CountClientPlayers(iClient) >= sso.sso_ctLocalPlayers) {
    INetwork::SendDisconnectMessage(iClient, TRANS("Protocol violation"), FALSE);
    return FALSE;
  }

  // Find inactive player for the client
  CPlayerBuffer *pplbNew = INetwork::FirstInactivePlayer();

  // If there's a used character already
  if (INetwork::IsCharacterUsed(pcCharacter)) {
    // Refuse connection
    CTString strMessage;
    strMessage.PrintF(TRANS("Player character '%s' already exists in this session."), pcCharacter.GetName());

    INetwork::SendDisconnectMessage(iClient, strMessage, FALSE);

  // If able to add new players
  } else if (pplbNew != NULL) {
    // Activate new player
    pplbNew->Activate(iClient);
    INDEX iNewPlayer = pplbNew->plb_Index;

    // Remember the character
    pplbNew->plb_pcCharacter = pcCharacter;

    // Send message to all clients about adding a new player
    CNetStreamBlock nsbAddClientData(MSG_SEQ_ADDPLAYER, ++srv.srv_iLastProcessedSequence);
    nsbAddClientData << iNewPlayer;
    nsbAddClientData << pcCharacter;

    INetwork::AddBlockToAllSessions(nsbAddClientData);

    // Don't wait for any more players
    _pShell->Execute("ser_bWaitFirstPlayer = 0;");

    // Reply to this client about adding a new player
    CNetworkMessage nmPlayerRegistered(MSG_REP_CONNECTPLAYER);
    nmPlayerRegistered << iNewPlayer;

    _pNetwork->SendToClientReliable(iClient, nmPlayerRegistered);

    // Notify master server that a player is connecting
    static CSymbolPtr symptr("ser_bEnumeration");

    if (symptr.GetIndex()) {
      IMasterServer::OnServerStateChanged();
    }

  // If too many players
  } else {
    // Refuse connection
    INetwork::SendDisconnectMessage(iClient, TRANS("Too many players in session."), FALSE);
  }

  return FALSE;
};

// Client changing the character
BOOL OnCharacterChangeRequest(INDEX iClient, CNetworkMessage &nmMessage)
{
  // Skip character changes blocked by the anti-flood system
  if (IAntiFlood::HandleCharacterChange(iClient)) {
    return FALSE;
  }

  return TRUE;
};

// Client sending a chat message
BOOL OnChatInRequest(INDEX iClient, CNetworkMessage &nmMessage)
{
  // Skip messages blocked by the anti-flood system
  if (IAntiFlood::HandleChatMessage(iClient)) {
    return FALSE;
  }

  ULONG ulFrom, ulTo;
  CTString strMessage;

  nmMessage >> ulFrom >> ulTo >> strMessage;
  nmMessage.Rewind();

  // Handle chat command if the message starts with a command prefix
  if (strMessage.HasPrefix(ser_strCommandPrefix)) {
    return IChatCommands::HandleCommand(iClient, strMessage);
  }

  return TRUE;
};
