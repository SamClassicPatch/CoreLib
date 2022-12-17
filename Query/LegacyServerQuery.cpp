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

// Builds hearthbeat packet.
void CLegacyQuery::BuildHearthbeatPacket(CTString &strPacket)
{
  strPacket.PrintF("\\heartbeat\\%hu\\gamename\\%s", (_piNetPort.GetIndex() + 1), SAM_MS_NAME);
}

void CLegacyQuery::ServerParsePacket(INDEX iLength)
{
  // [Cecil] Moved here from MS_OnServerUpdate()
  IQuery::pBuffer[iLength] = 0;

  unsigned char *data = (unsigned char*)IQuery::pBuffer;

  char *sPch1 = NULL, *sPch2 = NULL, *sPch3 = NULL, *sPch4 = NULL, *sPch5;

  sPch1 = strstr(IQuery::pBuffer, "\\status\\");
  sPch2 = strstr(IQuery::pBuffer, "\\info\\");
  sPch3 = strstr(IQuery::pBuffer, "\\basic\\");
  sPch4 = strstr(IQuery::pBuffer, "\\players\\");

  sPch5 = strstr(IQuery::pBuffer, "\\secure\\"); // [SSE] [ZCaliptium] Validation Fix.
  
  if (ms_bDebugOutput) {
    CPrintF("Received data[%d]:\n%s\n", iLength, IQuery::pBuffer);
  }

  // [Cecil] Player count
  const INDEX ctPlayers = INetwork::CountPlayers(FALSE);

  // status request
  if (sPch1) {
    CTString strPacket;
    CTString strLocation;
    strLocation = _pstrLocalHost.GetString();

    if (strLocation == ""){
      strLocation = "Heartland";
    }

    // [Cecil] Retrieve symbols once
    static CSymbolPtr symptrFF("gam_bFriendlyFire");
    static CSymbolPtr symptrWeap("gam_bWeaponsStay");
    static CSymbolPtr symptrAmmo("gam_bAmmoStays");
    static CSymbolPtr symptrVital("gam_bHealthArmorStays");
    static CSymbolPtr symptrHP("gam_bAllowHealth");
    static CSymbolPtr symptrAR("gam_bAllowArmor");
    static CSymbolPtr symptrIA("gam_bInfiniteAmmo");
    static CSymbolPtr symptrResp("gam_bRespawnInPlace");

    strPacket.PrintF( PCKQUERY,
      sam_strGameName,
      _SE_VER_STRING,
      //_pstrLocalHost.GetString(),
      strLocation,
      GetGameAPI()->GetSessionName(),
      _piNetPort.GetIndex(),
      _pNetwork->ga_World.wo_strName,
      GetGameAPI()->GetCurrentGameTypeNameSS(),
      ctPlayers,
      _pNetwork->ga_sesSessionState.ses_ctMaxPlayers,
      symptrFF.GetIndex(),
      symptrWeap.GetIndex(),
      symptrAmmo.GetIndex(),
      symptrVital.GetIndex(),
      symptrHP.GetIndex(),
      symptrAR.GetIndex(),
      symptrIA.GetIndex(),
      symptrResp.GetIndex());

      for (INDEX i = 0; i < ctPlayers; i++)
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
            IQuery::SendReply(strPacket);
            strPacket = "";
          }
          strPacket += strPlayer;
        }
      }

    strPacket += "\\final\\\\queryid\\333.1";
    IQuery::SendReply(strPacket);

    if (ms_bDebugOutput) {
      CPrintF("Sending status answer:\n%s\n", strPacket);
    }

  // info request
  } else if (sPch2){

    CTString strPacket;
    strPacket.PrintF( PCKINFO,
      GetGameAPI()->GetSessionName(),
      _piNetPort.GetIndex(),
      _pNetwork->ga_World.wo_strName,
      GetGameAPI()->GetCurrentGameTypeNameSS(),
      ctPlayers,
      _pNetwork->ga_sesSessionState.ses_ctMaxPlayers);
    IQuery::SendReply(strPacket);

    if (ms_bDebugOutput) {
      CPrintF("Sending info answer:\n%s\n", strPacket);
    }

  // basis request
  } else if (sPch3){

    CTString strPacket;
    CTString strLocation;
    strLocation = _pstrLocalHost.GetString();
    if (strLocation == ""){
      strLocation = "Heartland";
    }
    strPacket.PrintF( PCKBASIC,
      sam_strGameName,
      _SE_VER_STRING,
      //_pstrLocalHost.GetString());
      strLocation);
    IQuery::SendReply(strPacket);

    if (ms_bDebugOutput) {
      CPrintF("Sending basic answer:\n%s\n", strPacket);
    }

  // players request
  } else if (sPch4){

    // send the player status response
    CTString strPacket;
    strPacket = "";

    for (INDEX i = 0; i < ctPlayers; i++) {
      CPlayerBuffer &plb = _pNetwork->ga_srvServer.srv_aplbPlayers[i];
      CPlayerTarget &plt = _pNetwork->ga_sesSessionState.ses_apltPlayers[i];
      if (plt.plt_bActive) {
        CTString strPlayer;

        // [Cecil] 'GetMSLegacyPlayerInf' -> 'plt_penPlayerEntity->GetGameSpyPlayerInfo'
        plt.plt_penPlayerEntity->GetGameSpyPlayerInfo(plb.plb_Index, strPlayer);

        // if we don't have enough space left for the next player
        if (strlen(strPacket) + strlen(strPlayer) > 2048) {
          // send the packet
          IQuery::SendReply(strPacket);
          strPacket = "";
        }

        strPacket += strPlayer;
      }
    }

    strPacket += "\\final\\\\queryid\\6.1";
    IQuery::SendReply(strPacket);

    if (ms_bDebugOutput) {
      CPrintF("Sending players answer:\n%s\n", strPacket);
    }
  
  // [SSE] [ZCaliptium] '/validate/' - Validation request.
  } else if (sPch5) {
    //CPutString("Received 'validate' request from MS.\n");
    data += 8;
    
    //CPrintF("SecureKey: %s\n", data);
    
    u_char  ucGamekey[]          = {SAM_MS_NAME};
    //u_char  ucReckey[]          = {"XUCXHC"};
    //CPrintF("SecureKey: %s\n", ucReckey);
    unsigned char *pValidateKey = NULL;
    pValidateKey = gsseckey((u_char*)data, ucGamekey, 0);
    
    CTString strPacket;
    strPacket.PrintF("\\validate\\%s\\final\\%s\\queryid\\2.1", pValidateKey, "");
    IQuery::SendReply(strPacket);

    if (ms_bDebugOutput) {
      CPrintF("Sending validation answer:\n%s\n", strPacket);
    }

  } else {
    CPutString("Unknown query server command!\n");
    CPrintF("%s\n", IQuery::pBuffer);
    return;
  }
}
