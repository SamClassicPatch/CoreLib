/* Copyright (c) 2024 Dreamy Cecil
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

#include "VotingSystem.h"
#include "VoteTypes.h"
#include "ChatCommands.h"
#include "ClientLogging.h"
#include "Interfaces/FileFunctions.h"

#define INVALID_MAP_MESSAGE        TRANS("Invalid map index!")
#define INVALID_CLIENT_MESSAGE     TRANS("Invalid client index!")
#define VOTING_IN_PROGRESS_MESSAGE TRANS("There is already voting in progress!")

static INDEX ser_bVotingSystem       = FALSE; // Enable voting system
static INDEX ser_bPlayersStartVote   = TRUE;  // Allow players to initiate voting
static INDEX ser_bPlayersCanVote     = TRUE;  // Allow players to vote
static INDEX ser_bObserversStartVote = FALSE; // Allow spectators to initiate voting
static INDEX ser_bObserversCanVote   = FALSE; // Allow spectators to vote
static FLOAT ser_fVotingTime = 30.0f; // How long to vote for

static INDEX ser_bVoteMap  = TRUE; // Allow voting to change a map
static INDEX ser_bVoteKick = TRUE; // Allow voting to kick a client
FLOAT ser_fVoteKickTime = 300.0f; // How long to kick clients for (5 minutes)

namespace IVotingSystem {

// Current map pool
static CDynamicStackArray<SVoteMap> _aVoteMapPool;

// Current voting in progress
static CGenericVote *_pvtCurrentVote = NULL;

// Display current map pool
static void VoteMapPool(void) {
  CTString strPool;
  PrintMapPool(strPool);
  CPutString(strPool + "\n");
};

// Add map to the pool
static void VoteMapAdd(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  const CTString &strMap = *NEXT_ARG(CTString *);
  AddMapToPool(strMap);
};

// Remove map from the pool
static void VoteMapRemove(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iMap = NEXT_ARG(INDEX);

  if (iMap < 1 || iMap > _aVoteMapPool.Count()) {
    CPutString(INVALID_MAP_MESSAGE);
    CPutString("\n");
    return;
  }

  SVoteMap &map = _aVoteMapPool[iMap - 1];
  CPrintF(TRANS("Removed '%s' from the map pool!\n"), map.strName.Undecorated());

  _aVoteMapPool.Delete(&map);
};

// Load map pool from a file
static void VoteMapLoad(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  const CTString &strMap = *NEXT_ARG(CTString *);
  LoadMapPool(strMap);
};

// Initialize voting system
void Initialize(void) {
  // Custom symbols
  _pShell->DeclareSymbol("persistent user INDEX ser_bVotingSystem;",       &ser_bVotingSystem);
  _pShell->DeclareSymbol("persistent user INDEX ser_bPlayersStartVote;",   &ser_bPlayersStartVote);
  _pShell->DeclareSymbol("persistent user INDEX ser_bPlayersCanVote;",     &ser_bPlayersCanVote);
  _pShell->DeclareSymbol("persistent user INDEX ser_bObserversStartVote;", &ser_bObserversStartVote);
  _pShell->DeclareSymbol("persistent user INDEX ser_bObserversCanVote;",   &ser_bObserversCanVote);
  _pShell->DeclareSymbol("persistent user FLOAT ser_fVotingTime;",         &ser_fVotingTime);

  _pShell->DeclareSymbol("persistent user INDEX ser_bVoteMap;",            &ser_bVoteMap);
  _pShell->DeclareSymbol("persistent user INDEX ser_bVoteKick;",           &ser_bVoteKick);
  _pShell->DeclareSymbol("persistent user FLOAT ser_fVoteKickTime;",       &ser_fVoteKickTime);

  _pShell->DeclareSymbol("user void VoteMapPool(void);",     &VoteMapPool);
  _pShell->DeclareSymbol("user void VoteMapAdd(CTString);",  &VoteMapAdd);
  _pShell->DeclareSymbol("user void VoteMapRemove(INDEX);",  &VoteMapRemove);
  _pShell->DeclareSymbol("user void VoteMapLoad(CTString);", &VoteMapLoad);

  LoadMapPool(CTString("Data\\ClassicsPatch\\VoteMapPool.lst"));
};

static CTString VoteYesCommand(void) {
  return "^c00ff00" + ser_strCommandPrefix + "y";
};

static CTString VoteNoCommand(void) {
  return "^cff0000" + ser_strCommandPrefix + "n";
};

// Check if current server is running
static BOOL IsServerRunning(void) {
  // Non-local game; running a server; with more than one player
  return _pNetwork->IsNetworkEnabled() && _pNetwork->IsServer() && _pNetwork->ga_sesSessionState.ses_ctMaxPlayers > 1;
};

// Check if voting is available
static BOOL IsVotingAvailable(void) {
  // Setting is on; server is running
  return ser_bVotingSystem && IsServerRunning();
};

// Initiate voting for a specific thing by some client
static BOOL InitiateVoting(INDEX iClient, CGenericVote *pvt) {
  // Unavailable or already voting in progress
  if (!IsVotingAvailable() || _pvtCurrentVote != NULL) return FALSE;

  _pvtCurrentVote = pvt->MakeCopy();
  DOUBLE dTimeLeft = ceil(_pvtCurrentVote->GetTimeLeft().GetSeconds());

  // Notify everyone about the voting
  CTString strPlayers;
  CActiveClient &ac = _aActiveClients[iClient];

  if (ac.cPlayers.Count() == 0) {
    strPlayers.PrintF(TRANS("Client %d"), iClient);
  } else {
    strPlayers = ac.ListPlayers().Undecorated();
  }

  CTString strChatMessage(0, TRANS("%s has initiated a vote:\n"), strPlayers);
  strChatMessage += "  " + _pvtCurrentVote->VoteMessage() + "\n";

  strChatMessage += CTString(0, TRANS("^CYou have ^cffffff%d^C seconds to vote. Type %s^C or %s^C to vote for or against it!"),
    (INDEX)dTimeLeft, VoteYesCommand(), VoteNoCommand());

  _pNetwork->SendChat(0, -1, strChatMessage);
  return TRUE;
};

// Update current vote (needs to be synced with the game loop)
void UpdateVote(void) {
  // No voting in progress
  if (_pvtCurrentVote == NULL) return;

  // Terminate voting if the server isn't running anymore
  if (!IsServerRunning()) {
    delete _pvtCurrentVote;
    _pvtCurrentVote = NULL;
    return;
  }

  DOUBLE dTimeLeft = ceil(_pvtCurrentVote->GetTimeLeft().GetSeconds());
  DOUBLE dNextReport = ceil(_pvtCurrentVote->GetReportTimeLeft().GetSeconds());

  const INDEX ctYes = _pvtCurrentVote->vt_Yes.Count();
  const INDEX ctNo = _pvtCurrentVote->vt_No.Count();
  const BOOL bVotePassed = (ctYes > 0 && ctYes > ctNo);

  // Wait after voting ends
  if (_pvtCurrentVote->vt_bOver)
  {
    if (dTimeLeft <= 0.0) {
      // Perform an action
      if (bVotePassed) {
        _pvtCurrentVote->VotingOver();
      }

      delete _pvtCurrentVote;
      _pvtCurrentVote = NULL;
    }
    return;
  }

  // Voting time expired
  if (dTimeLeft <= 0.0) {
    CTString strChatMessage(0, TRANS("Voting is over! ^c00ff00Yes: %d^C / ^cff0000No: %d"), ctYes, ctNo);

    // Describe action that's about to be performed
    if (bVotePassed) {
      strChatMessage += "\n  " + _pvtCurrentVote->ResultMessage();
    }

    _pNetwork->SendChat(0, -1, strChatMessage);

    // Wait before performing an action
    _pvtCurrentVote->vt_bOver = TRUE;
    _pvtCurrentVote->SetTime(3.0);
    return;
  }

  // Make a report
  if (dNextReport <= 0.0) {
    CTString strChatMessage(0, TRANS("^cffffff%d^C seconds left to vote"), (INDEX)dTimeLeft);
    strChatMessage += CTString(0, " -- %s %d^C / %s %d", VoteYesCommand(), ctYes, VoteNoCommand(), ctNo);
    _pNetwork->SendChat(0, -1, strChatMessage);

    // Set next report
    _pvtCurrentVote->SetReportTime();
  }
};

// Load map pool from a file
void LoadMapPool(const CTFileName &fnmMapPool) {
  // Load new pool list
  CFileList aMapPool;
  if (!IFiles::LoadStringList(aMapPool, fnmMapPool)) return;

  // Clear current pool
  _aVoteMapPool.Clear();

  const INDEX ct = aMapPool.Count();

  for (INDEX i = 0; i < ct; i++) {
    CTFileName &fnm = aMapPool[i];

    // TFE-specific maps
    if (fnm.RemovePrefix("TFE:")) {
      #if SE1_GAME == SS_TFE
        AddMapToPool(fnm);
      #endif
      continue;

    // TSE-specific maps
    } else if (fnm.RemovePrefix("TSE:")) {
      #if SE1_GAME == SS_TSE
        AddMapToPool(fnm);
      #endif
      continue;
    }

    AddMapToPool(fnm);
  }
};

// Add world file to the map pool
BOOL AddMapToPool(const CTFileName &fnmWorldFile) {
  SVoteMap map;
  map.fnmWorld = fnmWorldFile;

  try {
    // Open the world file
    CTFileStream strm;
    strm.Open_t(fnmWorldFile);

    // Skip a bunch of initial chunks
    strm.ExpectID_t("BUIV");

    INDEX iDummy;
    strm >> iDummy;

    strm.ExpectID_t("WRLD");
    strm.ExpectID_t("WLIF");

    static const CChunkID chnkDTRS(CTString("DT") + "RS");

    if (strm.PeekID_t() == chnkDTRS) {
      strm.ExpectID_t(chnkDTRS);
    }

    // Two SSR chunks
    if (strm.PeekID_t() == CChunkID("LDRB")) {
      strm.ExpectID_t("LDRB");

      CTString strDummy;
      strm >> strDummy;
    }

    if (strm.PeekID_t() == CChunkID("Plv0")) {
      strm.ExpectID_t("Plv0");

      UBYTE aDummy[12];
      strm.Read_t(aDummy, sizeof(aDummy));
    }

    // Read the name
    strm >> map.strName;

    // Add map to the pool
    _aVoteMapPool.Push() = map;
    return TRUE;

  } catch (char *strError) {
    CPrintF(TRANS("Cannot add '%s' to the map pool:\n%s\n"), fnmWorldFile.str_String, strError);
  }

  return FALSE;
};

// Print current map pool
void PrintMapPool(CTString &str) {
  str = TRANS("^cffffffAvailable maps:");
  const INDEX ct = _aVoteMapPool.Count();

  for (INDEX i = 0; i < ct; i++) {
    const CTString &strMap = _aVoteMapPool[i].strName;
    str += CTString(0, "\n%d. %s", i + 1, strMap.Undecorated());
  }
};

// Print current clients
void PrintClientList(CTString &str) {
  str = TRANS("^cffffffAvailable clients:");
  const INDEX ct = _aActiveClients.Count();

  for (INDEX i = 0; i < ct; i++) {
    CActiveClient &ac = _aActiveClients[i];

    // Inactive
    if (ac.pClient == NULL) continue;

    str += CTString(0, "\n%d. ", i);

    // No active players
    if (ac.cPlayers.Count() == 0) {
      str += "<observer>";

    } else {
      str += ac.ListPlayers();
    }
  }
};

// Check if can initiate voting
static BOOL CanInitiateVoting(CTString &strResult, INDEX iClient) {
  // Unavailable
  if (!IsVotingAvailable()) {
    strResult = "";
    return FALSE;
  }

  CActiveClient &ac = _aActiveClients[iClient];
  const BOOL bRealPlayer = ac.cPlayers.Count() != 0;

  // Players can't initiate voting
  if (bRealPlayer && !ser_bPlayersStartVote) {
    strResult = TRANS("Players aren't allowed to initiate voting!");
    return FALSE;

  // Spectators can't initiate voting
  } else if (!bRealPlayer && !ser_bObserversStartVote) {
    strResult = TRANS("Observers aren't allowed to initiate voting!");
    return FALSE;
  }

  return TRUE;
};

// Initiate voting to change a map
BOOL Chat::VoteMap(CTString &strResult, INDEX iClient, const CTString &strArguments) {
  // Disabled
  if (!ser_bVoteMap) return FALSE;

  // Can't vote (reply to the client if there's a message)
  if (!CanInitiateVoting(strResult, iClient)) {
    return (strResult != "");
  }

  const INDEX ct = _aVoteMapPool.Count();

  // No maps
  if (ct == 0) {
    strResult = TRANS("There are no maps in the current pool!");
    return TRUE;
  }

  INDEX iMap = -1;
  INDEX iScan = const_cast<CTString &>(strArguments).ScanF("%d", &iMap);

  // Display current map pool
  if (iScan != 1) {
    PrintMapPool(strResult);
    strResult += "\n\n" + CTString(0, TRANS("To initiate a vote, type \"%svotemap <map index>\""), ser_strCommandPrefix);
    return TRUE;
  }

  if (iMap < 1 || iMap > ct) {
    strResult = INVALID_MAP_MESSAGE;
    return TRUE;
  }

  // Create map vote
  CMapVote vt(_aVoteMapPool[iMap - 1]);
  vt.SetTime((DOUBLE)ser_fVotingTime);

  if (!InitiateVoting(iClient, &vt)) {
    strResult = VOTING_IN_PROGRESS_MESSAGE;
  }

  return TRUE;
};

// Initiate voting to kick a client
BOOL Chat::VoteKick(CTString &strResult, INDEX iClient, const CTString &strArguments) {
  // Disabled
  if (!ser_bVoteKick) return FALSE;

  // Can't vote (reply to the client if there's a message)
  if (!CanInitiateVoting(strResult, iClient)) {
    return (strResult != "");
  }

  const INDEX ct = _aActiveClients.Count();

  INDEX iKick = -1;
  INDEX iScan = const_cast<CTString &>(strArguments).ScanF("%d", &iKick);

  // Display current clients
  if (iScan != 1) {
    PrintClientList(strResult);
    strResult += "\n\n" + CTString(0, TRANS("To initiate a vote, type \"%svotekick <client index>\""), ser_strCommandPrefix);
    return TRUE;
  }

  if (iKick < 0 || iKick >= ct) {
    strResult = INVALID_CLIENT_MESSAGE;
    return TRUE;
  }

  CActiveClient &acKick = _aActiveClients[iKick];

  if (acKick.pClient == NULL) {
    strResult = INVALID_CLIENT_MESSAGE;
    return TRUE;
  }

  // Create kick vote
  CKickVote vt(acKick);
  vt.SetTime((DOUBLE)ser_fVotingTime);

  if (!InitiateVoting(iClient, &vt)) {
    strResult = VOTING_IN_PROGRESS_MESSAGE;
  }

  return TRUE;
};

// Make a vote as a client
static BOOL CheckVote(CTString &strResult, INDEX iClient, BOOL bVoteYes) {
  // Unavailable or no voting in progress
  if (!IsVotingAvailable() || _pvtCurrentVote == NULL) return FALSE;

  CActiveClient &ac = _aActiveClients[iClient];
  const BOOL bRealPlayer = ac.cPlayers.Count() != 0;

  // Players can't vote
  if (bRealPlayer && !ser_bPlayersCanVote) {
    strResult = TRANS("Players aren't allowed to vote!");
    return TRUE;

  // Spectators can't vote
  } else if (!bRealPlayer && !ser_bObserversCanVote) {
    strResult = TRANS("Observers aren't allowed to vote!");
    return TRUE;
  }

  CGenericVote &vt = *_pvtCurrentVote;

  // Already voted
  if (vt.vt_Yes.IsMember(&ac) || vt.vt_No.IsMember(&ac)) {
    return FALSE;
  }

  if (bVoteYes) {
    vt.vt_Yes.Add(&ac);
  } else {
    vt.vt_No.Add(&ac);
  }

  return FALSE;
};

// Vote yes in the current vote
BOOL Chat::VoteYes(CTString &strResult, INDEX iClient, const CTString &strArguments) {
  return CheckVote(strResult, iClient, TRUE);
};

// Vote no in the current vote
BOOL Chat::VoteNo(CTString &strResult, INDEX iClient, const CTString &strArguments) {
  return CheckVote(strResult, iClient, FALSE);
};

}; // namespace
