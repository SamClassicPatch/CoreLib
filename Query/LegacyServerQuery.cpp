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

#define MSPORT      28900
#define BUFFSZ      8192
#define BUFFSZSTR   4096

extern unsigned char *gsseckey(u_char *secure, u_char *key, int enctype);

#define PCKQUERY    "\\gamename\\%s" \
                    "\\gamever\\%s" \
                    "\\location\\%s" \
                    "\\hostname\\%s" \
                    "\\hostport\\%hu" \
                    "\\mapname\\%s" \
                    "\\gametype\\%s" \
                    "\\activemod\\" \
                    "\\numplayers\\%d" \
                    "\\maxplayers\\%d" \
                    "\\gamemode\\openplaying" \
                    "\\difficulty\\Normal" \
                    "\\friendlyfire\\%d" \
                    "\\weaponsstay\\%d" \
                    "\\ammosstay\\%d" \
                    "\\healthandarmorstays\\%d" \
                    "\\allowhealth\\%d" \
                    "\\allowarmor\\%d" \
                    "\\infiniteammo\\%d" \
                    "\\respawninplace\\%d" \
                    "\\password\\0" \
                    "\\vipplayers\\1"

#define PCKINFO     "\\hostname\\%s" \
                    "\\hostport\\%hu" \
                    "\\mapname\\%s" \
                    "\\gametype\\%s" \
                    "\\numplayers\\%d" \
                    "\\maxplayers\\%d" \
                    "\\gamemode\\openplaying" \
                    "\\final\\" \
                    "\\queryid\\8.1"

#define PCKBASIC    "\\gamename\\%s" \
                    "\\gamever\\%s" \
                    "\\location\\EU" \
                    "\\final\\" \
                    "\\queryid\\1.1"

extern CTString _getCurrentGameTypeName();

// [Cecil] Use query data here
using namespace QueryData;

extern void _initializeWinsock(void);
extern void _uninitWinsock();
extern void _sendPacket(const char* szBuffer);
extern void _sendPacket(const char* pubBuffer, INDEX iLen);
extern void _sendPacketTo(const char* szBuffer, sockaddr_in* addsin);
extern void _sendPacketTo(const char* pubBuffer, INDEX iLen, sockaddr_in* sin);
extern void _setStatus(const CTString &strStatus);

// Builds hearthbeat packet.
void CLegacyQuery::BuildHearthbeatPacket(CTString &strPacket)
{
  strPacket.PrintF("\\heartbeat\\%hu\\gamename\\%s", (_pShell->GetINDEX("net_iPort") + 1), SERIOUSSAMSTR);
}

void CLegacyQuery::ServerParsePacket(INDEX iLength)
{
  // [Cecil] Moved here from MS_OnServerUpdate()
  _szBuffer[iLength] = 0;

  unsigned char *data = (unsigned char*)&_szBuffer[0];

  char *sPch1 = NULL, *sPch2 = NULL, *sPch3 = NULL, *sPch4 = NULL, *sPch5;

  sPch1 = strstr(_szBuffer, "\\status\\");
  sPch2 = strstr(_szBuffer, "\\info\\");
  sPch3 = strstr(_szBuffer, "\\basic\\");
  sPch4 = strstr(_szBuffer, "\\players\\");

  sPch5 = strstr(_szBuffer, "\\secure\\"); // [SSE] [ZCaliptium] Validation Fix.
  
  if (ms_bDebugOutput) {
    CPrintF("Received data[%d]:\n%s\n", iLength, _szBuffer);
  }

  // status request
  if (sPch1) {
    CTString strPacket;
    CTString strLocation;
    strLocation = _pShell->GetString("net_strLocalHost");

    if (strLocation == ""){
      strLocation = "Heartland";
    }

    strPacket.PrintF( PCKQUERY,
      sam_strGameName,
      _SE_VER_STRING,
      //_pShell->GetString("net_strLocalHost"),
      strLocation,
      _pPatchAPI->GetSessionName(),
      _pShell->GetINDEX("net_iPort"),
      _pNetwork->ga_World.wo_strName,
      _getCurrentGameTypeName(),
      GetPlayerCount(),
      _pNetwork->ga_sesSessionState.ses_ctMaxPlayers,
      _pShell->GetINDEX("gam_bFriendlyFire"),
      _pShell->GetINDEX("gam_bWeaponsStay"),
      _pShell->GetINDEX("gam_bAmmoStays"),
      _pShell->GetINDEX("gam_bHealthArmorStays"),
      _pShell->GetINDEX("gam_bAllowHealth"),
      _pShell->GetINDEX("gam_bAllowArmor"),
      _pShell->GetINDEX("gam_bInfiniteAmmo"),
      _pShell->GetINDEX("gam_bRespawnInPlace"));

      for (INDEX i=0; i<GetPlayerCount(); i++)
      {
        CPlayerBuffer &plb = _pNetwork->ga_srvServer.srv_aplbPlayers[i];
        CPlayerTarget &plt = _pNetwork->ga_sesSessionState.ses_apltPlayers[i];
        if (plt.plt_bActive) {
          CTString strPlayer;

          // [Cecil] 'GetMSLegacyPlayerInf' -> 'plt_penPlayerEntity->GetGameSpyPlayerInfo'
          plt.plt_penPlayerEntity->GetGameSpyPlayerInfo(plb.plb_Index, strPlayer);

          // if we don't have enough space left for the next player
          if (strPacket.Length() + strPlayer.Length() > 2048) {
            // send the packet
            _sendPacketTo(strPacket, &_sinFrom);
            strPacket = "";
          }
          strPacket += strPlayer;
        }
      }

    strPacket += "\\final\\\\queryid\\333.1";
    _sendPacketTo(strPacket, &_sinFrom);

    if (ms_bDebugOutput) {
      CPrintF("Sending status answer:\n%s\n", strPacket);
    }

  // info request
  } else if (sPch2){

    CTString strPacket;
    strPacket.PrintF( PCKINFO,
      _pPatchAPI->GetSessionName(),
      _pShell->GetINDEX("net_iPort"),
      _pNetwork->ga_World.wo_strName,
      _getCurrentGameTypeName(),
      GetPlayerCount(),
      _pNetwork->ga_sesSessionState.ses_ctMaxPlayers);
    _sendPacketTo(strPacket, &_sinFrom);

    if (ms_bDebugOutput) {
      CPrintF("Sending info answer:\n%s\n", strPacket);
    }

  // basis request
  } else if (sPch3){

    CTString strPacket;
    CTString strLocation;
    strLocation = _pShell->GetString("net_strLocalHost");
    if (strLocation == ""){
      strLocation = "Heartland";
    }
    strPacket.PrintF( PCKBASIC,
      sam_strGameName,
      _SE_VER_STRING,
      //_pShell->GetString("net_strLocalHost"));
      strLocation);
    _sendPacketTo(strPacket, &_sinFrom);

    if (ms_bDebugOutput) {
      CPrintF("Sending basic answer:\n%s\n", strPacket);
    }

  // players request
  } else if (sPch4){

    // send the player status response
    CTString strPacket;
    strPacket = "";

    for (INDEX i=0; i<GetPlayerCount(); i++) {
      CPlayerBuffer &plb = _pNetwork->ga_srvServer.srv_aplbPlayers[i];
      CPlayerTarget &plt = _pNetwork->ga_sesSessionState.ses_apltPlayers[i];
      if (plt.plt_bActive) {
        CTString strPlayer;

        // [Cecil] 'GetMSLegacyPlayerInf' -> 'plt_penPlayerEntity->GetGameSpyPlayerInfo'
        plt.plt_penPlayerEntity->GetGameSpyPlayerInfo(plb.plb_Index, strPlayer);

        // if we don't have enough space left for the next player
        if (strlen(strPacket) + strlen(strPlayer) > 2048) {
          // send the packet
          _sendPacketTo(strPacket, &_sinFrom);
          strPacket = "";
        }

        strPacket += strPlayer;
      }
    }

    strPacket += "\\final\\\\queryid\\6.1";
    _sendPacketTo(strPacket, &_sinFrom);

    if (ms_bDebugOutput) {
      CPrintF("Sending players answer:\n%s\n", strPacket);
    }
  
  // [SSE] [ZCaliptium] '/validate/' - Validation request.
  } else if (sPch5) {
    //CPrintF("Received 'validate' request from MS.\n");
    data += 8;
    
    //CPrintF("SecureKey: %s\n", data);
    
    u_char  ucGamekey[]          = {SERIOUSSAMKEY};
    //u_char  ucReckey[]          = {"XUCXHC"};
    //CPrintF("SecureKey: %s\n", ucReckey);
    unsigned char *pValidateKey = NULL;
    pValidateKey = gsseckey((u_char*)data, ucGamekey, 0);
    
    CTString strPacket;
    strPacket.PrintF("\\validate\\%s\\final\\%s\\queryid\\2.1", pValidateKey, "");
    _sendPacketTo(strPacket, &_sinFrom);

    if (ms_bDebugOutput) {
      CPrintF("Sending validation answer:\n%s\n", strPacket);
    }

  } else {
    CPrintF("Unknown query server command!\n");
    CPrintF("%s\n", _szBuffer);
    return;
  }
}
