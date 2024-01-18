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
#include "ChatCommands.h"
#include "ClientLogging.h"
#include "Interfaces/FileFunctions.h"

namespace IVotingSystem {

// Current voting in progress
static CVote *_pvtCurrentVote = NULL;

static INDEX ser_bPlayersStartVote   = TRUE;  // Allow players to initiate voting
static INDEX ser_bPlayersCanVote     = TRUE;  // Allow players to vote
static INDEX ser_bObserversStartVote = FALSE; // Allow spectators to initiate voting
static INDEX ser_bObserversCanVote   = FALSE; // Allow spectators to vote
static FLOAT ser_fVotingTime = 30.0f; // How long to vote for

// Initialize voting system
void Initialize(void) {
  // Custom symbols
  _pShell->DeclareSymbol("persistent user INDEX ser_bPlayersStartVote;",   &ser_bPlayersStartVote);
  _pShell->DeclareSymbol("persistent user INDEX ser_bPlayersCanVote;",     &ser_bPlayersCanVote);
  _pShell->DeclareSymbol("persistent user INDEX ser_bObserversStartVote;", &ser_bObserversStartVote);
  _pShell->DeclareSymbol("persistent user INDEX ser_bObserversCanVote;",   &ser_bObserversCanVote);
  _pShell->DeclareSymbol("persistent user FLOAT ser_fVotingTime;",         &ser_fVotingTime);
};

static CTString VoteYesCommand(void) {
  return "^c00ff00" + ser_strCommandPrefix + "y";
};

static CTString VoteNoCommand(void) {
  return "^cff0000" + ser_strCommandPrefix + "n";
};

// Check if voting is available
static BOOL IsVotingAvailable(void) {
  // Non-local game; running a server; with more than one player
  return _pNetwork->IsNetworkEnabled() && _pNetwork->IsServer() && _pNetwork->ga_sesSessionState.ses_ctMaxPlayers > 1;
};

// Initiate voting for a specific thing by some client
static BOOL InitiateVoting(INDEX iClient, CVote *pvt) {
  // Unavailable or already voting in progress
  if (!IsVotingAvailable() || _pvtCurrentVote != NULL) return FALSE;

  _pvtCurrentVote = pvt->MakeCopy();
  DOUBLE dTimeLeft = ceil(_pvtCurrentVote->GetTimeLeft().GetSeconds());

  // Notify everyone about the voting
  CTString strChatMessage(0, TRANS("Client %d has initiated a vote for:\n"), iClient);
  strChatMessage += "  " + _pvtCurrentVote->VoteMessage() + "\n";

  strChatMessage += CTString(0, TRANS("^CYou have ^cffffff%d^C seconds to vote. Type %s^C or %s^C to vote for or against it!"),
    (INDEX)dTimeLeft, VoteYesCommand(), VoteNoCommand());

  _pNetwork->SendChat(0, -1, strChatMessage);
  return TRUE;
};

// Update current vote
void UpdateVote(void) {
  // Unavailable or no voting in progress
  if (!IsVotingAvailable() || _pvtCurrentVote == NULL) return;

  DOUBLE dTimeLeft = ceil(_pvtCurrentVote->GetTimeLeft().GetSeconds());
  DOUBLE dNextReport = ceil(_pvtCurrentVote->GetReportLeft().GetSeconds());

  const INDEX ctYes = _pvtCurrentVote->vt_Yes.Count();
  const INDEX ctNo = _pvtCurrentVote->vt_No.Count();

  // Voting time expired
  if (dTimeLeft <= 0.0) {
    CTString strChatMessage(0, TRANS("Voting is over! ^c00ff00Yes: %d^C / ^cff0000No: %d"), ctYes, ctNo);
    _pNetwork->SendChat(0, -1, strChatMessage);

    if (ctYes > 0 && ctYes > ctNo) {
      _pvtCurrentVote->VotingOver();
    }

    delete _pvtCurrentVote;
    _pvtCurrentVote = NULL;
    return;
  }

  // Make a report
  if (dNextReport <= 0.0) {
    CTString strChatMessage(0, TRANS("^cffffff%d^C seconds left to vote"), (INDEX)dTimeLeft);
    strChatMessage += CTString(0, " -- %s %d^C / %s %d", VoteYesCommand(), ctYes, VoteNoCommand(), ctNo);
    _pNetwork->SendChat(0, -1, strChatMessage);

    // Set next report
    _pvtCurrentVote->SetNextReport();
  }
};

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

  CVote &vt = *_pvtCurrentVote;

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
