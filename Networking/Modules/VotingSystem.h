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

#ifndef CECIL_INCL_VOTINGSYSTEM_H
#define CECIL_INCL_VOTINGSYSTEM_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include "ActiveClients.h"

// Current vote
class CORE_API CVote {
  public:
    CTimerValue vt_tvUntil; // When voting ends
    CTimerValue vt_tvReport; // When to remind about voting

    CActiveClient::List vt_Yes; // Clients who voted for
    CActiveClient::List vt_No;  // Clients who voted against

  public:
    // Constructor
    CVote() {
      vt_tvUntil.Clear();
      vt_tvReport.Clear();
    };

    // Copy constructor
    CVote(const CVote &vtOther) : vt_tvUntil(vtOther.vt_tvUntil), vt_tvReport(vtOther.vt_tvReport)
    {
    };

    // Destructor
    virtual ~CVote()
    {
    };

    // Check how much time is left to vote
    CTimerValue GetTimeLeft(void) const {
      return vt_tvUntil - _pTimer->GetHighPrecisionTimer();
    };

    // Check how much time is left before the next report
    CTimerValue GetReportLeft(void) const {
      return vt_tvReport - _pTimer->GetHighPrecisionTimer();
    };

    // Set how long the vote is going to go for
    void SetVoteTime(CTimerValue tvTime) {
      vt_tvUntil = _pTimer->GetHighPrecisionTimer() + tvTime;
      SetNextReport();
    };

    // Set when to remind about voting next time
    void SetNextReport(void) {
      // Every five seconds under ten
      if (GetTimeLeft().GetSeconds() <= 10.5) {
        vt_tvReport = _pTimer->GetHighPrecisionTimer() + CTimerValue(5.0);

      // Every ten seconds
      } else {
        vt_tvReport = _pTimer->GetHighPrecisionTimer() + CTimerValue(10.0);
      }
    };

    // Make copy of this class
    virtual CVote *MakeCopy(void) const = 0;

    // Vote description
    virtual CTString VoteMessage(void) const = 0;

    // Perform action after voting
    virtual void VotingOver(void) = 0;
};

// Interface for voting via chat
namespace IVotingSystem {

// Initialize voting system
void Initialize(void);

// Update current vote (needs to be synced with the game loop)
CORE_API void UpdateVote(void);

// Chat commands
struct Chat {
  // Vote yes in the current vote
  static BOOL VoteYes(CTString &strResult, INDEX iClient, const CTString &strArguments);

  // Vote no in the current vote
  static BOOL VoteNo(CTString &strResult, INDEX iClient, const CTString &strArguments);
};

}; // namespace

#endif
