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

// Available chunks
static const CChunkID _cidExclusive0("PXG0"); // Patch-Exclusive Gameplay = false
static const CChunkID _cidExclusive1("PXG1"); // Patch-Exclusive Gameplay = true

// Read one chunk and process its data
static void ReadOneServerInfoChunk(CTMemoryStream &strm) {
  // Get chunk ID and compare it
  CChunkID cid = strm.GetID_t();

  // Vanilla gameplay
  if (cid == _cidExclusive0) {
    CCoreAPI::varData.bGameplayExt = FALSE;

  // Patched gameplay
  } else if (cid == _cidExclusive1) {
    CCoreAPI::varData.bGameplayExt = TRUE;
  }
};

// Append extra info about the server to the session state for patched clients
void AppendServerInfoToSessionState(CTMemoryStream &strm) {
  // Patch identification tag and the release version
  strm.Write_t(_aSessionStatePatchTag, sizeof(_aSessionStatePatchTag));
  strm << (ULONG)CCoreAPI::ulCoreVersion;

  // Write amount of info chunks
  strm << (INDEX)1;

  // Patch-exclusive gameplay (synchronize with the server)
  strm.WriteID_t(CCoreAPI::varData.bGameplayExt ? _cidExclusive1 : _cidExclusive0);
};

// Reset data before starting any session
void IProcessPacket::ResetSessionData(BOOL bServer) {
  // Reset for server
  if (bServer) {
    // Toggle gameplay logic extensions
    CCoreAPI::varData.bGameplayExt = _bGameplayExt;

  // Reset for client
  } else {
    // No patched gameplay for vanilla servers
    CCoreAPI::varData.bGameplayExt = FALSE;
  }
};

// Read extra info about the patched server
void IProcessPacket::ReadServerInfoFromSessionState(CTMemoryStream &strm) {
  // Tag length and server version
  const ULONG ctTagLen = sizeof(_aSessionStatePatchTag);
  ULONG ulServerVer;

  // Check if there's enough data
  const SLONG slRemaining = strm.GetStreamSize() - strm.GetPos_t();

  if (slRemaining < ctTagLen + sizeof(ulServerVer)) return;

  // Read server tag and version
  char aServerTag[ctTagLen];

  strm.Read_t(aServerTag, ctTagLen);
  strm >> ulServerVer;

  // Skip if server tag cannot be verified
  if (memcmp(aServerTag, _aSessionStatePatchTag, ctTagLen) != 0) return;

  // Read amount of written chunks
  INDEX ctChunks;
  strm >> ctChunks;

  // Read chunks themselves
  while (--ctChunks >= 0) {
    ReadOneServerInfoChunk(strm);
  }
};
