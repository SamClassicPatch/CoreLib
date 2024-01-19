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

#ifndef CECIL_INCL_VOTINGTYPES_H
#define CECIL_INCL_VOTINGTYPES_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include "ActiveClients.h"

// One map in the pool
struct SVoteMap {
  CTFileName fnmWorld;
  CTString strName;

  // Clear map
  void Clear(void) {
    fnmWorld.Clear();
    strName.Clear();
  };

  // Assignment operator
  SVoteMap &operator=(const SVoteMap &mapOther) {
    fnmWorld = mapOther.fnmWorld;
    strName = mapOther.strName;
    return *this;
  };
};

// Current vote
class CORE_API CGenericVote {
  public:
    CTimerValue vt_tvUntil; // When voting ends
    CTimerValue vt_tvReport; // When to remind about voting

    CActiveClient::List vt_Yes; // Clients who voted for
    CActiveClient::List vt_No;  // Clients who voted against

    BOOL vt_bOver; // Voting is over

  public:
    // Constructor
    CGenericVote() {
      vt_tvUntil.Clear();
      vt_tvReport.Clear();
      vt_bOver = FALSE;
    };

    // Copy constructor
    CGenericVote(const CGenericVote &vtOther) :
      vt_tvUntil(vtOther.vt_tvUntil), vt_tvReport(vtOther.vt_tvReport), vt_bOver(vtOther.vt_bOver)
    {
    };

    // Destructor
    virtual ~CGenericVote()
    {
    };

    // Check how much time is left to vote
    CTimerValue GetTimeLeft(void) const;

    // Check how much time is left before the next report
    CTimerValue GetReportTimeLeft(void) const;

    // Set how long the vote is going to go for
    void SetTime(CTimerValue tvTime);

    // Set when to remind about voting next time
    void SetReportTime(void);

  public:
    // Make copy of this class
    virtual CGenericVote *MakeCopy(void) const = 0;

    // Vote description
    virtual CTString VoteMessage(void) const = 0;

    // Vote result
    virtual CTString ResultMessage(void) const = 0;

    // Perform action after voting
    virtual void VotingOver(void) = 0;
};

// Current map vote
class CORE_API CMapVote : public CGenericVote {
  public:
    SVoteMap vt_map; // Voting for this map from the pool

  public:
    // Constructor from a pool map
    CMapVote(const SVoteMap &map = SVoteMap()) : CGenericVote(), vt_map(map)
    {
    };

    // Copy constructor
    CMapVote(const CMapVote &vtOther) : CGenericVote(vtOther), vt_map(vtOther.vt_map)
    {
    };

    // Make copy of this class
    virtual CGenericVote *MakeCopy(void) const {
      return new CMapVote(*this);
    };

    // Vote description
    virtual CTString VoteMessage(void) const;

    // Vote result
    virtual CTString ResultMessage(void) const;

    // Perform action after voting
    virtual void VotingOver(void);
};

// Current kick vote
class CORE_API CKickVote : public CGenericVote {
  public:
    CClientIdentity *vt_pciIdentity; // Client identity to ban
    CTString vt_strPlayers; // Client's players

  public:
    // Constructor from an active client
    CKickVote(CActiveClient &ac) : CGenericVote(), vt_pciIdentity(ac.pClient)
    {
      if (ac.cPlayers.Count() == 0) {
        vt_strPlayers.PrintF(TRANS("Client %d"), _aActiveClients.Index(&ac));
      } else {
        vt_strPlayers = ac.ListPlayers().Undecorated();
      }
    };

    // Copy constructor
    CKickVote(const CKickVote &vtOther) : CGenericVote(vtOther),
      vt_pciIdentity(vtOther.vt_pciIdentity), vt_strPlayers(vtOther.vt_strPlayers)
    {
    };

    // Make copy of this class
    virtual CGenericVote *MakeCopy(void) const {
      return new CKickVote(*this);
    };

    // Vote description
    virtual CTString VoteMessage(void) const;

    // Vote result
    virtual CTString ResultMessage(void) const;

    // Perform action after voting
    virtual void VotingOver(void);
};

#endif
