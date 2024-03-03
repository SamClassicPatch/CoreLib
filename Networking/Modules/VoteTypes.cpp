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

#include "VoteTypes.h"
#include "VotingSystem.h"

// Check how much time is left to vote
CTimerValue CGenericVote::GetTimeLeft(void) const {
  return vt_tvUntil - _pTimer->GetHighPrecisionTimer();
};

// Check how much time is left before the next report
CTimerValue CGenericVote::GetReportTimeLeft(void) const {
  return vt_tvReport - _pTimer->GetHighPrecisionTimer();
};

// Set how long the vote is going to go for
void CGenericVote::SetTime(CTimerValue tvTime) {
  vt_tvUntil = _pTimer->GetHighPrecisionTimer() + tvTime;
  SetReportTime();
};

// Set when to remind about voting next time
void CGenericVote::SetReportTime(void) {
  // Every five seconds under ten
  if (GetTimeLeft().GetSeconds() <= 10.5) {
    vt_tvReport = _pTimer->GetHighPrecisionTimer() + CTimerValue(5.0);

  // Every ten seconds
  } else {
    vt_tvReport = _pTimer->GetHighPrecisionTimer() + CTimerValue(10.0);
  }
};

CTString CMapVote::VoteMessage(void) const {
  return CTString(0, TRANS("^cffff00Change current map to: %s^r"), vt_map.strName);
};

CTString CMapVote::ResultMessage(void) const {
  return CTString(0, TRANS("^cffff00Changing current map to: %s^r"), vt_map.strName);
};

void CMapVote::VotingOver(void) {
  // Force server to switch to another map
  if (GetAPI()->IsServerApp()) {
    _pShell->SetString("ded_strForceLevelChange", vt_map.fnmWorld);

  // Start new game on a new map
  } else {
    _pGame->StopGame();

    CSesPropsContainer sp;
    _pGame->SetMultiPlayerSession((CSessionProperties &)sp);
    GetGameAPI()->NewGame(GetGameAPI()->GetSessionName(), vt_map.fnmWorld, (CSessionProperties &)sp);
  }
};

CTString CKickVote::VoteMessage(void) const {
  return CTString(0, TRANS("^cffff00Kick %s from the server"), vt_strPlayers);
};

CTString CKickVote::ResultMessage(void) const {
  return CTString(0, TRANS("^cffff00Kicking %s from the server..."), vt_strPlayers);
};

void CKickVote::VotingOver(void) {
  INDEX iIdentity = _aClientIdentities.GetIndex(vt_pciIdentity);

  if (iIdentity != -1) {
    CClientRestriction::BanClient(iIdentity, ser_fVoteKickTime);

  } else {
    CPutString(TRANS("Couldn't find client identity in the log for kicking!\n"));
  }
};

CTString CMuteVote::VoteMessage(void) const {
  return CTString(0, TRANS("^cffff00Prevent %s from chatting"), vt_strPlayers);
};

CTString CMuteVote::ResultMessage(void) const {
  return CTString(0, TRANS("^cffff00Muting %s..."), vt_strPlayers);
};

void CMuteVote::VotingOver(void) {
  INDEX iIdentity = _aClientIdentities.GetIndex(vt_pciIdentity);

  if (iIdentity != -1) {
    CClientRestriction::MuteClient(iIdentity, ser_fVoteMuteTime);

  } else {
    CPutString(TRANS("Couldn't find client identity in the log for muting!\n"));
  }
};

CTString CSkipRoundVote::VoteMessage(void) const {
  return TRANS("^cffff00Skip current round");
};

CTString CSkipRoundVote::ResultMessage(void) const {
  return TRANS("^cffff00Skipping current round...");
};

void CSkipRoundVote::VotingOver(void) {
  // Force restart server
  if (GetAPI()->IsServerApp()) {
    _pShell->Execute("Restart();");
  }
};
