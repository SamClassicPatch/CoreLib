/* Copyright (c) 2023-2024 Dreamy Cecil
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

#include "MessageProcessing.h"

// Write patch identification tag into a stream
void IProcessPacket::WritePatchTag(CTStream &strm) {
  // Write tag and release version
  strm.Write_t(_aSessionStatePatchTag, sizeof(_aSessionStatePatchTag));
  strm << (ULONG)CCoreAPI::ulCoreVersion;
};

// Read patch identification tag from a stream
BOOL IProcessPacket::ReadPatchTag(CTStream &strm, ULONG *pulReadVersion) {
  // Remember stream position
  const SLONG slPos = strm.GetPos_t();

  // Tag length and release version
  const ULONG ctTagLen = sizeof(_aSessionStatePatchTag);
  ULONG ulVer;

  // Check if there's enough data
  const SLONG slRemaining = strm.GetStreamSize() - slPos;

  // No tag if not enough data
  if (slRemaining < ctTagLen + sizeof(ulVer)) {
    return FALSE;
  }

  // Read tag and version
  char aTag[ctTagLen];

  strm.Read_t(aTag, ctTagLen);
  strm >> ulVer;

  // Set read version
  if (pulReadVersion != NULL) {
    *pulReadVersion = ulVer;
  }

  // Return back upon incorrect tag
  if (memcmp(aTag, _aSessionStatePatchTag, ctTagLen) != 0) {
    strm.SetPos_t(slPos);
    return FALSE;
  }

  // Correct tag
  return TRUE;
};

#if CLASSICSPATCH_GAMEPLAY_EXT

// Reset data before starting any session
void IProcessPacket::ResetSessionData(BOOL bNewSetup) {
  // Set new data
  if (bNewSetup && GameplayExtEnabled()) {
    CoreGEX() = _gexSetup;

  // Reset to vanilla
  } else {
    CoreGEX().Reset(TRUE);
  }
};

// Available data chunks
static const CChunkID _cidTimers0("TMR0"); // Fix logic timers = false
static const CChunkID _cidTimers1("TMR1"); // Fix logic timers = true
static const CChunkID _cidAirControl0("UAC0"); // Unlimited air control = false
static const CChunkID _cidAirControl1("UAC1"); // Unlimited air control = true
static const CChunkID _cidMoveSpeed("MOVE"); // Movement speed multiplier
static const CChunkID _cidJumpHeight("JUMP"); // Jump height multiplier
static const CChunkID _cidGravityMod("GRAV"); // Gravity modifiers

// Read one chunk and process its data
static BOOL ReadOneServerInfoChunk(CTStream &strm) {
  // Get chunk ID and compare it
  CChunkID cid = strm.GetID_t();

  // Fix logic timers
  if (cid == _cidTimers0) {
    CoreGEX().bFixTimers = FALSE;

  } else if (cid == _cidTimers1) {
    CoreGEX().bFixTimers = TRUE;

  // Unlimited air control
  } else if (cid == _cidAirControl0) {
    CoreGEX().bUnlimitedAirControl = FALSE;

  } else if (cid == _cidAirControl1) {
    CoreGEX().bUnlimitedAirControl = TRUE;

  // Movement speed and jump height multipliers
  } else if (cid == _cidMoveSpeed) {
    strm >> CoreGEX().fMoveSpeed;

  } else if (cid == _cidJumpHeight) {
    strm >> CoreGEX().fJumpHeight;

  } else if (cid == _cidGravityMod) {
    strm >> CoreGEX().fGravityAcc;

  // Invalid chunk
  } else {
    return FALSE;
  }

  return TRUE;
};

// Append extra info about the patched server
void IProcessPacket::WriteServerInfoToSessionState(CTStream &strm) {
  // No gameplay extensions
  if (!CoreGEX().bGameplayExt) return;

  // Write patch tag
  IProcessPacket::WritePatchTag(strm);

  // Write amount of info chunks
  strm << (INDEX)5;

  // Fix logic timers
  strm.WriteID_t(CoreGEX().bFixTimers ? _cidTimers1 : _cidTimers0);

  // Player settings
  strm.WriteID_t(CoreGEX().bUnlimitedAirControl ? _cidAirControl1 : _cidAirControl0);

  strm.WriteID_t(_cidMoveSpeed);
  strm << CoreGEX().fMoveSpeed;

  strm.WriteID_t(_cidJumpHeight);
  strm << CoreGEX().fJumpHeight;

  strm.WriteID_t(_cidGravityMod);
  strm << CoreGEX().fGravityAcc;
};

// Read extra info about the patched server
void IProcessPacket::ReadServerInfoFromSessionState(CTStream &strm) {
  // Skip if patch tag cannot be verified
  if (!ReadPatchTag(strm, NULL)) return;

  // Gameplay extensions are active
  CoreGEX().bGameplayExt = TRUE;

  // Read amount of written chunks
  INDEX ctChunks;
  strm >> ctChunks;

  // Read chunks themselves
  while (--ctChunks >= 0) {
    // Break the loop upon encountering an invalid chunk
    if (!ReadOneServerInfoChunk(strm)) break;
  }
};

#else

// Reset data before starting any session
void IProcessPacket::ResetSessionData(BOOL bNewSetup)
{
};

// Append extra info about the patched server
void IProcessPacket::WriteServerInfoToSessionState(CTStream &strm)
{
};

// Read extra info about the patched server
void IProcessPacket::ReadServerInfoFromSessionState(CTStream &strm) {
  // Skip if patch tag cannot be verified
  if (!ReadPatchTag(strm, NULL)) return;

  // No data to read after the tag
  ASSERTALWAYS("CLASSICSPATCH_GAMEPLAY_EXT is turned off in this build!");
  ThrowF_t("Server info reading is unavailable in this build!");
};

#endif // CLASSICSPATCH_GAMEPLAY_EXT
