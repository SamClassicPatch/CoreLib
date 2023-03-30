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

#ifndef CECIL_INCL_MESSAGEPROCESSING_H
#define CECIL_INCL_MESSAGEPROCESSING_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// Interface with custom message processing methods
class IProcessPacket {
  public:
    // Client confirming the disconnection
    static BOOL OnClientDisconnect(INDEX iClient, CNetworkMessage &nmMessage);

    // Client requesting the session state
    static BOOL OnConnectRemoteSessionStateRequest(INDEX iClient, CNetworkMessage &nmMessage);

    // Client requesting the connection to the server
    static BOOL OnPlayerConnectRequest(INDEX iClient, CNetworkMessage &nmMessage);

    // Client changing the character
    static BOOL OnCharacterChangeRequest(INDEX iClient, CNetworkMessage &nmMessage);

    // Client sending player actions
    static BOOL OnPlayerAction(INDEX iClient, CNetworkMessage &nmMessage);

    // Client sending a chat message
    static BOOL OnChatInRequest(INDEX iClient, CNetworkMessage &nmMessage);
};

#endif
