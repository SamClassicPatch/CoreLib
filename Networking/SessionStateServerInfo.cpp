/* Copyright (c) 2023 Dreamy Cecil
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

// Reset data before starting any session
void IProcessPacket::ResetSessionData(BOOL bNewSetup) {
  // Set gameplay extensions
  CCoreAPI::varData.bGameplayExt = (bNewSetup && _bGameplayExt);

  // Set new data
  if (CCoreAPI::varData.bGameplayExt) {
    CCoreAPI::varData.bFixTimers = _bFixTimers;

  // Reset to vanilla
  } else {
    CCoreAPI::varData.bFixTimers = FALSE;
  }
};

// Available data chunks
static const CChunkID _cidTimers0("TMR0"); // Fix logic timers = false
static const CChunkID _cidTimers1("TMR1"); // Fix logic timers = true

// Read one chunk and process its data
static void ReadOneServerInfoChunk(CTStream &strm) {
  // Get chunk ID and compare it
  CChunkID cid = strm.GetID_t();

  if (cid == _cidTimers0) {
    CCoreAPI::varData.bFixTimers = FALSE;

  } else if (cid == _cidTimers1) {
    CCoreAPI::varData.bFixTimers = TRUE;
  }
};

// Append extra info about the patched server
void IProcessPacket::WriteServerInfoToSessionState(CTStream &strm) {
  // No gameplay extensions
  if (!CCoreAPI::varData.bGameplayExt) return;

  // Write patch tag
  IProcessPacket::WritePatchTag(strm);

  // Write amount of info chunks
  strm << (INDEX)1;

  // Fix logic timers
  strm.WriteID_t(CCoreAPI::varData.bFixTimers ? _cidTimers1 : _cidTimers0);
};

// Read extra info about the patched server
void IProcessPacket::ReadServerInfoFromSessionState(CTStream &strm) {
  // Skip if patch tag cannot be verified
  if (!ReadPatchTag(strm, NULL)) return;

  // Gameplay extensions are active
  CCoreAPI::varData.bGameplayExt = TRUE;

  // Read amount of written chunks
  INDEX ctChunks;
  strm >> ctChunks;

  // Read chunks themselves
  while (--ctChunks >= 0) {
    ReadOneServerInfoChunk(strm);
  }
};
