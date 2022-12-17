/* Copyright (c) 2002-2012 Croteam Ltd. 
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

#include "QueryMgr.h"
#include "Networking/NetworkFunctions.h"

// Builds hearthbeat packet.
void CGameAgentQuery::BuildHearthbeatPacket(CTString &strPacket, INDEX iChallenge)
{
  strPacket.PrintF("0;challenge;%d;players;%d;maxplayers;%d;level;%s;gametype;%s;version;%s;product;%s",
      iChallenge,
      INetwork::CountPlayers(FALSE),
      _pNetwork->ga_sesSessionState.ses_ctMaxPlayers,
      _pNetwork->ga_World.wo_strName,
      GetGameAPI()->GetCurrentGameTypeNameSS(),
      _SE_VER_STRING,
      sam_strGameName);
}

void CGameAgentQuery::ServerParsePacket(INDEX iLength)
{
  // [Cecil] Player count
  const INDEX ctPlayers = INetwork::CountPlayers(FALSE);

  // check the received packet ID
  switch (IQuery::pBuffer[0])
  {
    case 1: // server join response
    {
      int iChallenge = *(INDEX*)(IQuery::pBuffer + 1);
      // send the challenge
      IMasterServer::SendHeartbeat(iChallenge);
      break;
    }

    case 2: // server status request
    {
      // send the status response
      CTString strPacket;
      strPacket.PrintF("0;players;%d;maxplayers;%d;level;%s;gametype;%s;version;%s;gamename;%s;sessionname;%s",
        ctPlayers,
        _pNetwork->ga_sesSessionState.ses_ctMaxPlayers,
        _pNetwork->ga_World.wo_strName,
        GetGameAPI()->GetCurrentGameTypeNameSS(),
        _SE_VER_STRING,
        sam_strGameName,
        GetGameAPI()->GetSessionName());
      IQuery::SendReply(strPacket);
      break;
    }

    case 3: // player status request
    {
      // send the player status response
      CTString strPacket;
      strPacket.PrintF("\x01players\x02%d\x03", ctPlayers);
      for (INDEX i = 0; i < ctPlayers; i++) {
        CPlayerBuffer &plb = _pNetwork->ga_srvServer.srv_aplbPlayers[i];
        CPlayerTarget &plt = _pNetwork->ga_sesSessionState.ses_apltPlayers[i];
        if (plt.plt_bActive) {
          CTString strPlayer;

          // [Cecil] 'GetGameAgentPlayerInfo' -> 'plt_penPlayerEntity->GetGameSpyPlayerInfo'
          plt.plt_penPlayerEntity->GetGameSpyPlayerInfo(plb.plb_Index, strPlayer);

          // if we don't have enough space left for the next player
          if (strPacket.Length() + strPlayer.Length() > 2048) {
            // send the packet
            IQuery::SendReply(strPacket);
            strPacket = "";
          }

          strPacket += strPlayer;
        }
      }

      strPacket += "\x04";
      IQuery::SendReply(strPacket);
      break;
    }

    case 4: // ping
    {
      // just send back 1 byte and the amount of players in the server (this could be useful in some cases for external scripts)
      CTString strPacket;
      strPacket.PrintF("\x04%d", ctPlayers);
      IQuery::SendReply(strPacket);
      break;
    }
  }
}

void CGameAgentQuery::EnumTrigger(BOOL bInternet)
{
  // Make sure that there are no requests still stuck in buffer.
  IQuery::aRequests.Clear();

  // We're not a server.
  IQuery::bServer = FALSE;
  // Initialization.
  IQuery::bInitialized = TRUE;
  // Send enumeration packet to masterserver.
  IQuery::SendPacket("e");
  IQuery::SetStatus(".");
}

static void ClientParsePacket(INDEX iLength)
{
  switch (IQuery::pBuffer[0])
  {
    case 's':
    {
      struct sIPPort {
        UBYTE bFirst;
        UBYTE bSecond;
        UBYTE bThird;
        UBYTE bFourth;
        USHORT iPort;
      };

      _pNetwork->ga_strEnumerationStatus = "";
  
      sIPPort* pServers = (sIPPort*)(IQuery::pBuffer + 1);

      while(iLength - ((CHAR*)pServers - IQuery::pBuffer) >= sizeof(sIPPort)) {
        sIPPort ip = *pServers;
  
        CTString strIP;
        strIP.PrintF("%d.%d.%d.%d", ip.bFirst, ip.bSecond, ip.bThird, ip.bFourth);
  
        sockaddr_in sinServer;
        sinServer.sin_family = AF_INET;
        sinServer.sin_addr.s_addr = inet_addr(strIP);
        sinServer.sin_port = htons(ip.iPort + 1);
  
        // insert server status request into container
        SServerRequest::AddRequest(sinServer);
  
        // send packet to server
        IQuery::SendPacketTo(&sinServer, "\x02", 1);
  
        pServers++;
      }
    } break;
    
    case '0':
    {
      CTString strPlayers;
      CTString strMaxPlayers;
      CTString strLevel;
      CTString strGameType;
      CTString strVersion;
      CTString strGameName;
      CTString strSessionName;
  
      CHAR* pszPacket = IQuery::pBuffer + 2; // we do +2 because the first character is always ';', which we don't care about.
  
      BOOL bReadValue = FALSE;
      CTString strKey;
      CTString strValue;
  
      while(*pszPacket != 0)
      {
        switch (*pszPacket)
        {
          case ';': {
            if (strKey != "sessionname") {
              if (bReadValue) {
                // we're done reading the value, check which key it was
                if (strKey == "players") {
                  strPlayers = strValue;
                } else if (strKey == "maxplayers") {
                  strMaxPlayers = strValue;
                } else if (strKey == "level") {
                  strLevel = strValue;
                } else if (strKey == "gametype") {
                  strGameType = strValue;
                } else if (strKey == "version") {
                  strVersion = strValue;
                } else if (strKey == "gamename") {
                  strGameName = strValue;
                } else {
                  CPrintF("Unknown GameAgent parameter key '%s'!", strKey);
                }
    
                // reset temporary holders
                strKey = "";
                strValue = "";
              }
            }

            bReadValue = !bReadValue;
          } break;
  
          default: {
            // read into the value or into the key, depending where we are
            if (bReadValue) {
              strValue.InsertChar(strValue.Length(), *pszPacket);
            } else {
              strKey.InsertChar(strKey.Length(), *pszPacket);
            }
          } break;
        }
  
        // move to next character
        pszPacket++;
      }
  
      // check if we still have a sessionname to back up
      if (strKey == "sessionname") {
        strSessionName = strValue;
      }
  
      // insert the server into the serverlist
      CNetworkSession &ns = *new CNetworkSession;
      _pNetwork->ga_lhEnumeratedSessions.AddTail(ns.ns_lnNode);
  
      CTimerValue tvPing = SServerRequest::PopRequestTime(IQuery::sinFrom);
  
      if (tvPing.tv_llValue == -1) {
        // server status was never requested
        break;
      }

      __int64 tmPing = (_pTimer->GetHighPrecisionTimer() - tvPing).GetMilliseconds();
  
      // add the server to the serverlist
      ns.ns_strSession = strSessionName;
      ns.ns_strAddress = inet_ntoa(IQuery::sinFrom.sin_addr) + CTString(":") + CTString(0, "%d", htons(IQuery::sinFrom.sin_port) - 1);
      ns.ns_tmPing = (tmPing / 1000.0f);
      ns.ns_strWorld = strLevel;
      ns.ns_ctPlayers = atoi(strPlayers);
      ns.ns_ctMaxPlayers = atoi(strMaxPlayers);
      ns.ns_strGameType = strGameType;
      ns.ns_strMod = strGameName;
      ns.ns_strVer = strVersion;
    } break;
    
    default: {
      CPrintF("Unknown enum packet ID %x!\n", IQuery::pBuffer[0]);
    } break;
  }
}

void CGameAgentQuery::EnumUpdate(void)
{
  int iLength = IQuery::ReceivePacket();

  if (iLength == -1) {
    return;
  }

  IQuery::pBuffer[iLength] = 0; // Terminate the buffer with NULL.

  ClientParsePacket(iLength);
}