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

#include "Networking/ExtPackets.h"

#if CLASSICSPATCH_EXT_PACKETS

// Dummy event for initialization
class EInitEvent : public CEntityEvent {
  public:
    // Accommodate for multiple fields of varying data
    ULONG aulDummy[64];

    EInitEvent() : CEntityEvent(EVENTCODE_EVoid) {
      // Fill fields with NULL, FALSE, 0, 0.0f etc.
      memset(aulDummy, 0, sizeof(aulDummy));
    };

    CEntityEvent *MakeCopy(void) {
      return new EInitEvent(*this);
    };
};

void CExtEntityInit::Write(CNetworkMessage &nm) {
  WriteEntity(nm);
  
  static const UBYTE ubNoMore = 0;
  static const UBYTE ubMore = 1;

  for (INDEX i = 0; i < 16; i++) {
    ULONG ulValue = aulEventValues[i];

    // No more values written afterwards
    if (ulValue == (ULONG)-1) {
      nm.WriteBits(&ubNoMore, 1);
      break;
    }

    // Write another value
    nm.WriteBits(&ubMore, 1);
    INetCompress::Integer(nm, ulValue);
  }
};

void CExtEntityInit::Read(CNetworkMessage &nm) {
  ReadEntity(nm);
  memset(aulEventValues, 0, sizeof(aulEventValues));

  UBYTE ubState = 0;

  for (INDEX i = 0; i < 16; i++) {
    nm.ReadBits(&ubState, 1);

    // Read value afterwards
    if (ubState == 1) {
      ULONG &ulValue = aulEventValues[i];
      INetDecompress::Integer(nm, ulValue);

      // Convert to entity pointer, if possible
      CEntity *pen = FindExtEntity(ulValue);

      if (pen != NULL) {
        ulValue = (ULONG)pen;
      }

      // Next value
      continue;
    }

    // No more values
    break;
  }
};

void CExtEntityInit::Process(void) {
  CEntity *pen = GetEntity();

  if (pen == NULL) return;

  // Copy event arguments
  EInitEvent eInit;
  memcpy(eInit.aulDummy, aulEventValues, sizeof(aulEventValues));

  if (pen->GetRenderType() == CEntity::RT_NONE) {
    pen->Initialize(eInit);
    ExtServerReport(TRANS("Initialized %u entity\n"), pen->en_ulID);

  } else {
    // Not using CEntity::Reinitialize() because it doesn't take initialization events
    pen->End_internal();
    pen->Initialize_internal(eInit);

    ExtServerReport(TRANS("Reinitialized %u entity\n"), pen->en_ulID);
  }
};

#endif // CLASSICSPATCH_EXT_PACKETS
