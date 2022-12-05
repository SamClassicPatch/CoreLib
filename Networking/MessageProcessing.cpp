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

#include "AntiFlood.h"
#include "ChatCommands.h"
#include "SplitScreenClients.h"

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
  return TRUE;
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
