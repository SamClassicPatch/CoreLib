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

#include "CoreTimerHandler.h"

#include "Networking/Modules.h"

// Define pointer to the timer handler
CCoreTimerHandler *_pTimerHandler = NULL;

// This is called every CTimer::TickQuantum seconds
void CCoreTimerHandler::HandleTimer(void) {
  // Called every game tick, even if no session was started and while in pause
  static CTimerValue _tvLastSecCheck(-1.0f);

  CTimerValue tvNow = _pTimer->GetHighPrecisionTimer();

  if ((tvNow - _tvLastSecCheck).GetSeconds() >= 1.0f) {
    _tvLastSecCheck = tvNow;

    // Call per-second functions
    OnSecond();
  }

  // Call per-tick functions
  OnTick();
};

// Called every game tick
void CCoreTimerHandler::OnTick(void)
{
  // Update client restriction records
  CClientRestriction::UpdateExpirations();
};

// Called every game second
void CCoreTimerHandler::OnSecond(void)
{
  // Reset anti-flood counters
  IAntiFlood::ResetCounters();
};
