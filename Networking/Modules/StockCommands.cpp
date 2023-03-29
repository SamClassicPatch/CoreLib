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

#include "StockCommands.h"
#include "Networking/NetworkFunctions.h"
#include "ClientLogging.h"
#include "Interfaces/DataFunctions.h"
#include "Interfaces/WorldFunctions.h"

// Parse command as a normal chat message for non-server clients
#define IGNORE_CLIENTS { if (!GetComm().Server_IsClientLocal(iClient)) return FALSE; }

// Display name of the current map
BOOL IStockCommands::CurrentMap(CTString &strResult, INDEX, const CTString &) {
  CWorld &wo = *IWorld::GetWorld();
  strResult.PrintF("Current map: \"%s^r\"\n- %s", wo.wo_strName, wo.wo_fnmFileName.str_String);

  return TRUE;
};

// Display log of all clients
BOOL IStockCommands::ClientLog(CTString &strResult, INDEX iClient, const CTString &) {
  IGNORE_CLIENTS;

  // No clients
  if (_aClientIdentities.Count() == 0) {
    strResult = "No clients have been logged!";
    return TRUE;
  }

  strResult = "--- Client log ---\n";

  // Go through every client identity
  const INDEX ctIdentities = _aClientIdentities.Count();

  for (INDEX iIdentity = 0; iIdentity < ctIdentities; iIdentity++) {
    CClientIdentity &ci = _aClientIdentities[iIdentity];

    // List addresses
    strResult += CTString(0, "   ^c00ffffClient %d:^C\nAddresses:\n", iIdentity);

    for (INDEX iAddr = 0; iAddr < ci.aAddresses.Count(); iAddr++) {
      strResult += CTString(0, "%d. %s\n", iAddr + 1, ci.aAddresses[iAddr].GetHost());
    }

    // List characters
    strResult += "Characters:\n";

    for (INDEX iChar = 0; iChar < ci.aCharacters.Count(); iChar++) {
      const CPlayerCharacter &pc = ci.aCharacters[iChar];
      const UBYTE *pGUID = pc.pc_aubGUID;

      // GUID and name
      strResult += CTString(0, "%d. %02X%02X%02X%02X %02X%02X%02X%02X %02X%02X%02X%02X %02X%02X%02X%02X : %s\n",
        iChar + 1, pGUID[0], pGUID[1], pGUID[2], pGUID[3], pGUID[4], pGUID[5], pGUID[6], pGUID[7],
        pGUID[8], pGUID[9], pGUID[10], pGUID[11], pGUID[12], pGUID[13], pGUID[14], pGUID[15], pc.GetNameForPrinting());
    }

    // Print relevant information
    {
      CTString strInfo = "";

      // Check current activity
      CActiveClient::List cClients;
      CActiveClient::GetActiveClients(cClients, &ci);

      FOREACHINDYNAMICCONTAINER(cClients, CActiveClient, itac) {
        CDynamicContainer<CPlayerBuffer> &aPlayers = itac->cPlayers;
        strInfo += CTString(0, " ^cffff00Active %d: %s (%d players)\n",
                            itac.GetIndex(), itac->addr.GetHost(), aPlayers.Count());

        // List active characters
        FOREACHINDYNAMICCONTAINER(aPlayers, CPlayerBuffer, itplb) {
          const CPlayerCharacter &pc = itplb->plb_pcCharacter;
          const UBYTE *pGUID = pc.pc_aubGUID;

          // GUID and name
          strInfo += CTString(0, "  %02X%02X%02X%02X %02X%02X%02X%02X %02X%02X%02X%02X %02X%02X%02X%02X : %s\n",
            pGUID[0], pGUID[1], pGUID[2], pGUID[3], pGUID[4], pGUID[5], pGUID[6], pGUID[7],
            pGUID[8], pGUID[9], pGUID[10], pGUID[11], pGUID[12], pGUID[13], pGUID[14], pGUID[15], pc.GetName().Undecorated());
        }
      }

      // Check for a ban
      CClientRestriction *pcr = CClientRestriction::IsBanned(&ci);

      if (pcr != NULL) {
        CTString strTime;
        pcr->PrintBanTime(strTime);

        strInfo += CTString(0, " ^cff0000BANNED for %s!\n", strTime);
      }

      // Check for a mute
      pcr = CClientRestriction::IsMuted(&ci);

      if (pcr != NULL) {
        CTString strTime;
        pcr->PrintMuteTime(strTime);

        strInfo += CTString(0, " ^c7f7f7fMUTED for %s!\n", strTime);
      }

      // Display relevant information
      if (strInfo != "") {
        strResult += "^caaaaaaRelevant information:^r\n" + strInfo;
      }
    }
  }

  // Remove last line break
  strResult.DeleteChar(strResult.Length() - 1);
  return TRUE;
};

// Parse arguments of a timed action aimed at some client
static CTString TimedClientAction(INDEX &iIdentity, FLOAT &fTime, CTString strArgs) {
  // Get identity index and time from the arguments
  INDEX iScan = strArgs.ScanF("%d %f", &iIdentity, &fTime);

  if (iScan < 1) {
    return "Couldn't parse the client index!";
  }

  // Invalid index
  if (iIdentity < 0 || iIdentity >= _aClientIdentities.Count()) {
    return "Invalid client index!";
  }

  // Set default time (5 minutes)
  if (iScan < 2) {
    fTime = 300.0f;
  }

  return "";
};

// Parse arguments of a reasoned action aimed at some client
static CTString ReasonedClientAction(INDEX &iIdentity, CTString &strReason, CTString strArgs) {
  // Get identity index from the arguments
  if (strArgs.ScanF("%d", &iIdentity) < 1) {
    return "Couldn't parse the client index!";
  }

  // Invalid index
  if (iIdentity < 0 || iIdentity >= _aClientIdentities.Count()) {
    return "Invalid client index!";
  }

  // Find first whitespace
  const ULONG ulWhitespace = IData::FindChar(strArgs, ' ');

  // If found
  if (ulWhitespace != -1) {
    // Remove everything before the whitespace and trim the rest
    strArgs.TrimLeft(strArgs.Length() - ulWhitespace);
    strArgs.TrimSpacesLeft();

    // Use first 100 characters as a reason
    strArgs.TrimRight(100);
    strReason = strArgs;

  } else {
    // Otherwise no reason
    strReason = "";
  }

  return "";
};

// Ban a specific client
BOOL IStockCommands::BanClient(CTString &strResult, INDEX iClient, const CTString &strArguments) {
  IGNORE_CLIENTS;

  // Get arguments
  INDEX iIdentity;
  FLOAT fTime;

  strResult = TimedClientAction(iIdentity, fTime, strArguments);

  // Some error has occurred
  if (strResult != "") {
    return TRUE;
  }

  // Ban client
  strResult = CClientRestriction::BanClient(iIdentity, fTime);
  return TRUE;
};

// Mute a specific client
BOOL IStockCommands::MuteClient(CTString &strResult, INDEX iClient, const CTString &strArguments) {
  IGNORE_CLIENTS;

  // Get arguments
  INDEX iIdentity;
  FLOAT fTime;

  strResult = TimedClientAction(iIdentity, fTime, strArguments);

  // Some error has occurred
  if (strResult != "") {
    return TRUE;
  }

  // Mute client
  strResult = CClientRestriction::MuteClient(iIdentity, fTime);
  return TRUE;
};

// Kick a specific client
BOOL IStockCommands::KickClient(CTString &strResult, INDEX iClient, const CTString &strArguments) {
  IGNORE_CLIENTS;

  // Get arguments
  INDEX iIdentity;
  CTString strReason;

  strResult = ReasonedClientAction(iIdentity, strReason, strArguments);

  // Some error has occurred
  if (strResult != "") {
    return TRUE;
  }

  // Ban client
  strResult = CClientRestriction::KickClient(iIdentity, strReason);
  return TRUE;
};
