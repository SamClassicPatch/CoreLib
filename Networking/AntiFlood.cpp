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

#include "AntiFlood.h"
#include "NetworkFunctions.h"

// Kick clients for attempted packet flood
INDEX ser_bEnableAntiFlood = TRUE;

// Allowed messages threshold before treating it as packet flood
INDEX ser_iPacketFloodThreshold = 10;

// Allowed messages from client per second
INDEX ser_iMaxMessagesPerSecond = 2;

// Individual data about packet flood
struct SClientFloodData {
  INDEX ctLastSecPackets; // Packets sent in the past second
  INDEX ctLastSecMessages; // Chat messages sent in the past second

  // Constructor
  SClientFloodData() : ctLastSecPackets(0), ctLastSecMessages(0)
  {
  };
};

// Anti-flood data per server client
static SClientFloodData _aClientFloodData[SERVER_CLIENTS];

// See if should check for packet flooding for this client
static BOOL CheckForPacketFlood(INDEX iClient) {
  // Don't use anti-flood system
  if (!ser_bEnableAntiFlood) {
    return FALSE;
  }

  // Ignore server clients
  if (GetComm().Server_IsClientLocal(iClient)) {
    return FALSE;
  }

  return TRUE;
};

// Detect potential packet flood and deal with it
static BOOL DetectPacketFlood(INDEX iClient)
{
  INDEX &ctPackets = _aClientFloodData[iClient].ctLastSecPackets;

  // Client haven't exceeded the flood threshold
  if (ser_iPacketFloodThreshold < 0 || ctPackets <= ser_iPacketFloodThreshold) {
    return FALSE;
  }

  // Kick for attempted packet flood
  CSessionSocket &sso = _pNetwork->ga_srvServer.srv_assoSessions[iClient];
  sso.sso_iDisconnectedState = 2; // Force disconnect

  CTString strChatMessage;
  strChatMessage.PrintF("^cFF0000 Client %d has been kicked for a packet flood attempt!", iClient);
  _pNetwork->SendChat(0, -1, strChatMessage);

  // Detected
  return TRUE;
};

// Handle character changes from a client
BOOL IAntiFlood::HandleCharacterChange(INDEX iClient)
{
  if (!CheckForPacketFlood(iClient)) {
    return FALSE;
  }

  // Count one packet from the client
  _aClientFloodData[iClient].ctLastSecPackets++;

  // Deal with packet flood
  return DetectPacketFlood(iClient);
};

// Handle chat messages from a client
BOOL IAntiFlood::HandleChatMessage(INDEX iClient)
{
  if (!CheckForPacketFlood(iClient)) {
    return FALSE;
  }

  // Count one chat message from the client
  _aClientFloodData[iClient].ctLastSecPackets++;
  _aClientFloodData[iClient].ctLastSecMessages++;

  // If detected packet flood
  if (DetectPacketFlood(iClient)) {
    // Don't show the message in chat
    return TRUE;
  }

  // Quit if no further message limit
  if (ser_iMaxMessagesPerSecond < 0) {
    return FALSE;
  }

  // If client sent too many messages in the past second
  if (_aClientFloodData[iClient].ctLastSecMessages > ser_iMaxMessagesPerSecond) {
    // Notify the client about spam
    CTString strWarning;

    if (ser_iMaxMessagesPerSecond > 0) {
      strWarning = "Too many chat messages at once!";
    } else {
      strWarning = "Chatting is disabled in this session!";
    }

    if (ser_iPacketFloodThreshold >= 0) {
      strWarning += " Further attempts may lead to a kick!";
    }

    INetwork::SendChatToClient(iClient, "Server", strWarning);

    // Don't show the message in chat
    return TRUE;
  }

  // No packet flood
  return FALSE;
};

// Reset packet counter for each client
void IAntiFlood::ResetCounters(void)
{
  for (INDEX i = 0; i < SERVER_CLIENTS; i++) {
    _aClientFloodData[i].ctLastSecPackets = 0;
  }
};
