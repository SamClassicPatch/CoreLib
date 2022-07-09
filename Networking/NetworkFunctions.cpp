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

// Handle packets coming from a client (CServer::Handle alternative)
BOOL INetwork::ServerHandle(CMessageDispatcher *pmd, INDEX iClient, CNetworkMessage &nmMessage) {
  // Let CServer::Handle process packets of other types
  if (nmMessage.GetType() != PCK_EXTENSION) return TRUE;

  CServer &srv = _pNetwork->ga_srvServer;

  // Handle specific packet types
  ULONG ulType;
  nmMessage >> ulType;

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
