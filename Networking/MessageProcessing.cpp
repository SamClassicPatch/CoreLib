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

// Which client sent last packet to the server
INDEX IProcessPacket::_iHandlingClient = IProcessPacket::CLT_NONE;

#if CLASSICSPATCH_GUID_MASKING

// Arrays of sync checks per client
CStaticArray<IProcessPacket::CSyncCheckArray> IProcessPacket::_aClientChecks;

// Should mask player GUIDs or not
BOOL IProcessPacket::_bMaskGUIDs = TRUE;

// Clear arrays with sync checks
void IProcessPacket::ClearSyncChecks(void)
{
  FOREACHINSTATICARRAY(IProcessPacket::_aClientChecks, IProcessPacket::CSyncCheckArray, itar) {
    itar->Clear();
  }
};

#endif // CLASSICSPATCH_GUID_MASKING

// Buffer sync check for the server
void IProcessPacket::AddSyncCheck(const INDEX iClient, const CSyncCheck &sc)
{
#if CLASSICSPATCH_GUID_MASKING
  // Use the first array if not masking
  CSyncCheckArray &aChecks = _aClientChecks[_bMaskGUIDs ? iClient : 0];
#else
  CSyncCheckArray &aChecks = _pNetwork->ga_srvServer.srv_ascChecks;
#endif

  // Recreate the buffer if the size differs
  static CSymbolPtr symptr("ser_iSyncCheckBuffer");
  INDEX &iBuffer = symptr.GetIndex();

  iBuffer = ClampDn(iBuffer, (INDEX)1);

  if (aChecks.Count() != iBuffer) {
    aChecks.Clear();
    aChecks.New(iBuffer);
  }

  // Find the oldest one
  INDEX iOldest = 0;

  for (INDEX i = 1; i < aChecks.Count(); i++) {
    if (aChecks[i].sc_tmTick < aChecks[iOldest].sc_tmTick) {
      iOldest = i;
    }
  }

  // Overwrite it
  aChecks[iOldest] = sc;
};

// Find buffered sync check for a given tick
INDEX IProcessPacket::FindSyncCheck(const INDEX iClient, TIME tmTick, CSyncCheck &sc)
{
#if CLASSICSPATCH_GUID_MASKING
  // Use the first array if not masking
  CSyncCheckArray &aChecks = _aClientChecks[_bMaskGUIDs ? iClient : 0];
#else
  CSyncCheckArray &aChecks = _pNetwork->ga_srvServer.srv_ascChecks;
#endif

  BOOL bHasEarlier = FALSE;
  BOOL bHasLater = FALSE;

  for (INDEX i = 0; i < aChecks.Count(); i++) {
    TIME tmInTable = aChecks[i].sc_tmTick;

    if (tmInTable == tmTick) {
      sc = aChecks[i];
      return 0;

    } else if (tmInTable < tmTick) {
      bHasEarlier = TRUE;

    } else if (tmInTable > tmTick) {
      bHasLater = TRUE;
    }
  }

  if (!bHasEarlier) {
    ASSERT(bHasLater);
    return -1;

  } else if (!bHasLater) {
    ASSERT(bHasEarlier);
    return +1;
  }

  // Cannot have earlier, later and not found all at once
  ASSERT(FALSE);
  return +1;
};

// Mask player GUID using data from the player buffer
void IProcessPacket::MaskGUID(UBYTE *aubGUID, CPlayerBuffer &plb) {
  // Clear GUID
  memset(aubGUID, 0, 16);

  // Use client and buffer indices for uniqueness
  aubGUID[0] = plb.plb_iClient;
  aubGUID[1] = plb.plb_Index;
};

// Client confirming the disconnection
BOOL IProcessPacket::OnClientDisconnect(INDEX iClient, CNetworkMessage &nmMessage) {
  CSessionSocket &sso = _pNetwork->ga_srvServer.srv_assoSessions[iClient];
  sso.sso_iDisconnectedState = 2;

  // Make client inactive
  CActiveClient::DeactivateClient(iClient);

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

  // If there's a used character already
  if (INetwork::IsCharacterUsed(pcCharacter)) {
    // Refuse connection
    CTString strMessage;
    strMessage.PrintF(LOCALIZE("Player character '%s' already exists in this session."), pcCharacter.GetName());

    INetwork::SendDisconnectMessage(iClient, strMessage, FALSE);
    return FALSE;
  }

  // Find inactive player for the client
  CPlayerBuffer *pplbNew = INetwork::FirstInactivePlayer();

  // If able to add new players
  if (pplbNew != NULL) {
    // Activate new player
    pplbNew->Activate(iClient);
    INDEX iNewPlayer = pplbNew->plb_Index;

    // Remember the character
    pplbNew->plb_pcCharacter = pcCharacter;

    const INDEX iLastSequence = ++srv.srv_iLastProcessedSequence;

  #if CLASSICSPATCH_GUID_MASKING
    if (_bMaskGUIDs) {
      // Send message back to this client about adding a new player
      if (iClient == 0 || sso.sso_bActive) {
        CNetStreamBlock nsbClientPlayer(MSG_SEQ_ADDPLAYER, iLastSequence);
        nsbClientPlayer << iNewPlayer;
        nsbClientPlayer << pcCharacter;

        INetwork::AddBlockToSession(nsbClientPlayer, iClient);
      }

      // Mask player GUID for other clients
      MaskGUID(pcCharacter.pc_aubGUID, *pplbNew);
    }
  #endif // CLASSICSPATCH_GUID_MASKING

    // Send message to other clients about adding a new player
    CNetStreamBlock nsbAddClientData(MSG_SEQ_ADDPLAYER, iLastSequence);
    nsbAddClientData << iNewPlayer;
    nsbAddClientData << pcCharacter;

  #if CLASSICSPATCH_GUID_MASKING
    // Send to other clients
    if (_bMaskGUIDs) {
      for (INDEX iSession = 0; iSession < srv.srv_assoSessions.Count(); iSession++) {
        CSessionSocket &ssoBlock = srv.srv_assoSessions[iSession];

        if (iSession == iClient || (iSession > 0 && !ssoBlock.sso_bActive)) {
          continue;
        }

        INetwork::AddBlockToSession(nsbAddClientData, iSession);
      }

    } else
  #endif // CLASSICSPATCH_GUID_MASKING
    {
      // Send to everyone
      INetwork::AddBlockToAllSessions(nsbAddClientData);
    }

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

  const INDEX iLastSequence = ++srv.srv_iLastProcessedSequence;

#if CLASSICSPATCH_GUID_MASKING
  if (_bMaskGUIDs) {
    // Send character change back to this client
    if (iClient == 0 || srv.srv_assoSessions[iClient].sso_bActive) {
      CNetStreamBlock nsbClientChar(MSG_SEQ_CHARACTERCHANGE, iLastSequence);
      nsbClientChar << iPlayer;
      nsbClientChar << pcCharacter;

      INetwork::AddBlockToSession(nsbClientChar, iClient);
    }

    // Mask player GUID for other clients
    MaskGUID(pcCharacter.pc_aubGUID, plb);
  }
#endif // CLASSICSPATCH_GUID_MASKING

  // Send character change to other clients
  CNetStreamBlock nsbChangeChar(MSG_SEQ_CHARACTERCHANGE, iLastSequence);
  nsbChangeChar << iPlayer;
  nsbChangeChar << pcCharacter;

#if CLASSICSPATCH_GUID_MASKING
  // Send to other clients
  if (_bMaskGUIDs) {
    for (INDEX iSession = 0; iSession < srv.srv_assoSessions.Count(); iSession++) {
      CSessionSocket &ssoBlock = srv.srv_assoSessions[iSession];

      if (iSession == iClient || (iSession > 0 && !ssoBlock.sso_bActive)) {
        continue;
      }

      INetwork::AddBlockToSession(nsbChangeChar, iSession);
    }

  } else
#endif // CLASSICSPATCH_GUID_MASKING
  {
    // Send to everyone
    INetwork::AddBlockToAllSessions(nsbChangeChar);
  }

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

// Client sends a CRC check
BOOL IProcessPacket::OnSyncCheck(INDEX iClient, CNetworkMessage &nmMessage) {
  static CSymbolPtr pbReportSyncOK("ser_bReportSyncOK");
  static CSymbolPtr pbReportSyncBad("ser_bReportSyncBad");
  static CSymbolPtr pbReportSyncLate("ser_bReportSyncLate");
  static CSymbolPtr pbReportSyncEarly("ser_bReportSyncEarly");
  static CSymbolPtr pbPauseOnSyncBad("ser_bPauseOnSyncBad");
  static CSymbolPtr piKickOnSyncBad("ser_iKickOnSyncBad");

  CServer &srv = _pNetwork->ga_srvServer;

  // Read sync check from the packet
  CSyncCheck scRemote;
  nmMessage.Read(&scRemote, sizeof(scRemote));

  const TIME tmTick = scRemote.sc_tmTick;

  // Try to find it in the buffer
  CSyncCheck scLocal;
  INDEX iFound = FindSyncCheck(iClient, tmTick, scLocal);

  CSessionSocket &sso = srv.srv_assoSessions[iClient];
  TIME &tmLastSync = sso.sso_tmLastSyncReceived;

  // Sync on time
  if (iFound == 0) {
    // Flush stream buffer up to that sequence
    ((CNetStream &)sso.sso_nsBuffer).RemoveOlderBlocksBySequence(scRemote.sc_iSequence);

    // Disconnect if the level has changed
    if (scLocal.sc_iLevel != scRemote.sc_iLevel) {
      INetwork::SendDisconnectMessage(iClient, LOCALIZE("Level change in progress. Please retry."), FALSE);

    // Wrong CRC
    } else if (scLocal.sc_ulCRC != scRemote.sc_ulCRC) {
      sso.sso_ctBadSyncs++;

      if (pbReportSyncBad.GetIndex()) {
        CPrintF(LOCALIZE("SYNCBAD: Client '%s', Sequence %d Tick %.2f - bad %d\n"), 
          GetComm().Server_GetClientName(iClient), scRemote.sc_iSequence, tmTick, sso.sso_ctBadSyncs);
      }

      // Kick from too many bad sync
      if (piKickOnSyncBad.GetIndex() > 0) {
        if (sso.sso_ctBadSyncs >= piKickOnSyncBad.GetIndex()) {
          INetwork::SendDisconnectMessage(iClient, LOCALIZE("Too many bad syncs"), FALSE);
        }

      // Pause on any bad sync
      } else if (pbPauseOnSyncBad.GetIndex()) {
        _pNetwork->ga_sesSessionState.ses_bWantPause = TRUE;
      }

    // Clear bad syncs
    } else {
      sso.sso_ctBadSyncs = 0;

      if (pbReportSyncOK.GetIndex()) {
        CPrintF(LOCALIZE("SYNCOK: Client '%s', Tick %.2f\n"), GetComm().Server_GetClientName(iClient), tmTick);
      }
    }

    // Remember that this sync is for this tick
    if (tmLastSync < tmTick) tmLastSync = tmTick;

  // Too old
  } else if (iFound < 0) {
    // Only report if syncs are okay (to avoid late syncs on level change)
    if (pbReportSyncLate.GetIndex() && tmLastSync > 0) {
      CPrintF(LOCALIZE("SYNCLATE: Client '%s', Tick %.2f\n"), GetComm().Server_GetClientName(iClient), tmTick);
    }

  // Too new
  } else {
    if (pbReportSyncEarly.GetIndex()) {
      CPrintF(LOCALIZE("SYNCEARLY: Client '%s', Tick %.2f\n"), GetComm().Server_GetClientName(iClient), tmTick);
    }

    // Remember that this sync was ahead of time
    if (tmLastSync < tmTick) tmLastSync = tmTick;
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
