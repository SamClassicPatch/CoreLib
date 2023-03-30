/* Copyright (c) 2022-2023 Dreamy Cecil
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
BOOL IProcessPacket::OnClientDisconnect(INDEX iClient, CNetworkMessage &nmMessage) {
  CSessionSocket &sso = _pNetwork->ga_srvServer.srv_assoSessions[iClient];
  sso.sso_iDisconnectedState = 2;

  // Make client inactive
  ASSERT(!GetComm().Server_IsClientLocal(iClient));
  _aActiveClients[iClient].Reset();

  return FALSE;
};

// Client requesting the session state
BOOL IProcessPacket::OnConnectRemoteSessionStateRequest(INDEX iClient, CNetworkMessage &nmMessage)
{
  // Get client identity
  CClientIdentity *pci = IClientLogging::GetIdentity(iClient);

  // Check if the client is banned
  CClientRestriction *pcr = CClientRestriction::IsBanned(pci);
  BOOL bBanned = (pcr != NULL);

  static CSymbolPtr pbWhiteList("ser_bInverseBanning");

  // Not allowed on the server
  if (bBanned == !pbWhiteList.GetIndex()) {
    // No specific ban record
    if (!bBanned) {
      INetwork::SendDisconnectMessage(iClient, TRANS("You are not allowed on this server!"), TRUE);
      return FALSE;
    }

    CTString strTime, strReason;
    pcr->PrintBanTime(strTime);

    strReason.PrintF(TRANS("You have been banned for %s!"), strTime);
    INetwork::SendDisconnectMessage(iClient, strReason, TRUE);
    return FALSE;
  }

  // Check for connecting clients with split-screen
  if (!CheckSplitScreenClients(iClient, nmMessage)) {
    return FALSE;
  }

  return TRUE;
};

// Client requesting the connection to the server
BOOL IProcessPacket::OnPlayerConnectRequest(INDEX iClient, CNetworkMessage &nmMessage)
{
  // Read character data
  CPlayerCharacter pcCharacter;
  nmMessage >> pcCharacter;

  // Get client identity
  CClientIdentity *pci = IClientLogging::GetIdentity(iClient);

  // Add new character to the identity
  pci->AddNewCharacter(pcCharacter);

  // Check for blacklisted/whitelisted character names
  if (!GetComm().Server_IsClientLocal(iClient)) {
    static CSymbolPtr pstrNameMask("ser_strNameMask");
    static CSymbolPtr pbWhiteList("ser_bInverseBanning");

    // Character name is banned
    if (IData::MatchesMask(pcCharacter.GetName(), pstrNameMask.GetString()) == !pbWhiteList.GetIndex()) {
      INetwork::SendDisconnectMessage(iClient, LOCALIZE("You are banned from this server"), FALSE);
      return FALSE;
    }
  }

  CServer &srv = _pNetwork->ga_srvServer;
  CSessionSocket &sso = srv.srv_assoSessions[iClient];

  // Check if someone's connecting with too many players
  if (iClient > 0 && INetwork::CountClientPlayers(iClient) >= sso.sso_ctLocalPlayers) {
    INetwork::SendDisconnectMessage(iClient, LOCALIZE("Protocol violation"), FALSE);
    return FALSE;
  }

  // Find inactive player for the client
  CPlayerBuffer *pplbNew = INetwork::FirstInactivePlayer();

  // If there's a used character already
  if (INetwork::IsCharacterUsed(pcCharacter)) {
    // Refuse connection
    CTString strMessage;
    strMessage.PrintF(LOCALIZE("Player character '%s' already exists in this session."), pcCharacter.GetName());

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

    // Add new player to the active client
    CActiveClient &acClient = _aActiveClients[iClient];
    ASSERT(acClient.IsActive() && acClient.pClient == pci);

    acClient.AddPlayer(pplbNew);

    // Notify master server that a player is connecting
    static CSymbolPtr symptr("ser_bEnumeration");

    if (symptr.GetIndex()) {
      IMasterServer::OnServerStateChanged();
    }

  // If too many players
  } else {
    // Refuse connection
    INetwork::SendDisconnectMessage(iClient, LOCALIZE("Too many players in session."), FALSE);
  }

  return FALSE;
};

// Client changing the character
BOOL IProcessPacket::OnCharacterChangeRequest(INDEX iClient, CNetworkMessage &nmMessage)
{
  // Read character data
  INDEX iPlayer;
  CPlayerCharacter pcCharacter;
  nmMessage >> iPlayer >> pcCharacter;

  // Get client identity
  CClientIdentity *pci = IClientLogging::GetIdentity(iClient);

  // Add new character to the identity
  pci->AddNewCharacter(pcCharacter);

  // Skip character changes blocked by the anti-flood system
  if (IAntiFlood::HandleCharacterChange(iClient)) {
    return FALSE;
  }

  CServer &srv = _pNetwork->ga_srvServer;

  // Invalid player
  if (iPlayer < 0 || iPlayer > srv.srv_aplbPlayers.Count() ) {
    return FALSE;
  }

  CPlayerBuffer &plb = srv.srv_aplbPlayers[iPlayer];

  // Wrong client or character
  if (plb.plb_iClient != iClient || !(plb.plb_pcCharacter == pcCharacter)) {
    return FALSE;
  }

  // Remember the character
  plb.plb_pcCharacter = pcCharacter;

  // Send character change to all clients
  CNetStreamBlock nsbChangeChar(MSG_SEQ_CHARACTERCHANGE, ++srv.srv_iLastProcessedSequence);
  nsbChangeChar << iPlayer;
  nsbChangeChar << pcCharacter;

  INetwork::AddBlockToAllSessions(nsbChangeChar);

  return FALSE;
};

// Receive action packet from one player of a client
static void ReceiveActionsForPlayer(CPlayerBuffer &plb, CNetworkMessage *pnm, INDEX iMaxBuffer) {
  ASSERT(plb.plb_Active);

  // Receive new action
  CPlayerAction pa;
  *pnm >> pa;

  // Buffer it
  plb.plb_abReceived.AddAction(pa);

  INDEX iSendBehind = 0;
  pnm->ReadBits(&iSendBehind, 2);

  // Add resent actions
  for (INDEX i = 0; i < iSendBehind; i++) {
    CPlayerAction paOld;
    *pnm >> paOld;

    if (paOld.pa_llCreated > plb.plb_paLastAction.pa_llCreated) {
      plb.plb_abReceived.AddAction(paOld);
    }
  }

  // If there are too many actions buffered
  while (plb.plb_abReceived.GetCount() > iMaxBuffer) {
    // Purge the oldest one
    plb.plb_abReceived.RemoveOldest();
  }
};

// Client sending player actions
BOOL IProcessPacket::OnPlayerAction(INDEX iClient, CNetworkMessage &nmMessage)
{
  CServer &srv = _pNetwork->ga_srvServer;
  CSessionSocket &sso = srv.srv_assoSessions[iClient];

  // For each possible player on that client
  for (INDEX i = 0; i < NET_MAXLOCALPLAYERS; i++) {
    // See if saved in the message
    BOOL bSaved = 0;
    nmMessage.ReadBits(&bSaved, 1);

    if (!bSaved) continue;

    // Read client index
    INDEX iPlayer = 0;
    nmMessage.ReadBits(&iPlayer, 4);

    CPlayerBuffer &plb = srv.srv_aplbPlayers[iPlayer];

    // If there is no player on that client
    if (plb.plb_iClient != iClient) {
      // Consider the entire message invalid
      CPrintF("Wrong Client!\n");
      break;
    }

    // Read ping
    plb.plb_iPing = 0;
    nmMessage.ReadBits(&plb.plb_iPing, 10);

    // Let the corresponding client buffer receive the message
    INDEX iMaxBuffer = sso.sso_sspParams.ssp_iBufferActions;

    static CSymbolPtr symptr("cli_bPredictIfServer");

    if (iClient == 0 && !symptr.GetIndex()) {
      iMaxBuffer = 1;
    }

    ReceiveActionsForPlayer(plb, &nmMessage, iMaxBuffer);
  }

  return FALSE;
};

// Client sending a chat message
BOOL IProcessPacket::OnChatInRequest(INDEX iClient, CNetworkMessage &nmMessage)
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
