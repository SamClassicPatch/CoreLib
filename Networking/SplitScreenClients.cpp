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

#include "SplitScreenClients.h"
#include "NetworkFunctions.h"

// How many local players are allowed per client
INDEX ser_iMaxPlayersPerClient = 4;

// Check for connecting clients with split-screen
BOOL CheckSplitScreenClients(INDEX iClient, CNetworkMessage &nmMessage)
{
  // Any amount of local players is allowed
  if (ser_iMaxPlayersPerClient < 1) {
    return TRUE;
  }

  nmMessage.nm_pubPointer = nmMessage.nm_pubMessage + 13;

  // Skip mod name and password
  CTString strGivenMod;
  CTString strGivenPassword;
  nmMessage >> strGivenMod >> strGivenPassword;

  // Read wanted local player count
  INDEX ctWantedLocalPlayers;
  nmMessage >> ctWantedLocalPlayers;

  // If client has more local players than allowed
  if (ctWantedLocalPlayers > ser_iMaxPlayersPerClient)
  {
    // Kick the client
    CTString strExplanation;

    // No split-screen allowed
    if (ser_iMaxPlayersPerClient == 1) {
      strExplanation.PrintF(TRANS("Split-screen isn't allowed in this session!"));

    // Too many split-screen players
    } else {
      strExplanation.PrintF(TRANS("Too many split-screen players (more than %d)!"), ser_iMaxPlayersPerClient);
    }

    INetwork::SendDisconnectMessage(iClient, strExplanation, TRUE);
    return FALSE;
  }

  // Client is good, rewind to the beginning
  nmMessage.Rewind();

  return TRUE;
};
