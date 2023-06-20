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

#include "ActiveClients.h"
#include "Networking/CommInterface.h"

// Active clients by client IDs on the server
CStaticArray<CActiveClient> _aActiveClients;

// Setup the client to be active
void CActiveClient::Set(CClientIdentity *pci, const SClientAddress &addrSet) {
  pClient = pci;
  addr = addrSet;
};

// Reset the client to be inactive
void CActiveClient::Reset(void) {
  pClient = NULL;
  cPlayers.Clear();
  addr.SetIP(0);

  ResetPacketCounters();
};

// Reset anti-flood counters
void CActiveClient::ResetPacketCounters(void) {
  ctLastSecPackets = 0;
  ctLastSecMessages = 0;
};

// Check if client is active right now
BOOL CActiveClient::IsActive(void) const {
  // Client identity exists
  return pClient != NULL;
};

// Add a new player
void CActiveClient::AddPlayer(CPlayerBuffer *pplb) {
  cPlayers.Add(pplb);
};

// Get active clients with a specific identity
void CActiveClient::GetActiveClients(CActiveClient::List &cClients, CClientIdentity *pci) {
  // Go through active clients
  FOREACHINSTATICARRAY(_aActiveClients, CActiveClient, itac)
  {
    // If found matching identity
    if (itac->pClient == pci) {
      // Add it to the list
      cClients.Add(itac);
    }
  }
};

// Deactivate some client
void CActiveClient::DeactivateClient(INDEX iClient) {
  // Make sure the server client isn't being deactivated
  ASSERT(!GetComm().Server_IsClientLocal(iClient));
  _aActiveClients[iClient].Reset();
};

// Reset all clients to be inactive
void CActiveClient::ResetAll(void)
{
  FOREACHINSTATICARRAY(_aActiveClients, CActiveClient, itac) {
    itac->Reset();
  }
};
