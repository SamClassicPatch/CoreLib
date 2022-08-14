/* Copyright (c) 2002-2012 Croteam Ltd. 
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

// [Cecil] Definitions of unexported CPlayerAction methods from the engine

#ifndef CECIL_INCL_PLAYERACTION_DEFS_H
#define CECIL_INCL_PLAYERACTION_DEFS_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// Write player action into a network packet
CNetworkMessage &operator<<(CNetworkMessage &nm, const CPlayerAction &pa) {
  nm.Write(&pa.pa_llCreated, sizeof(pa.pa_llCreated));

  const ULONG *pul = (const ULONG *)&pa.pa_vTranslation;

  for (INDEX i = 0; i < 9; i++)
  {
    if (*pul == 0) {
      UBYTE ub = 0;
      nm.WriteBits(&ub, 1);

    } else {
      UBYTE ub = 1;
      nm.WriteBits(&ub, 1);
      nm.WriteBits(pul, 32);
    }

    pul++;
  }

  ULONG ulFlags = pa.pa_ulButtons;

  if (ulFlags == 0) {
    UBYTE ub = 1;
    nm.WriteBits(&ub, 1);

  } else if (ulFlags == 1) {
    UBYTE ub = 2;
    nm.WriteBits(&ub, 2);

  } else if (ulFlags <= 3) {
    UBYTE ub = 4;
    nm.WriteBits(&ub, 3);
    nm.WriteBits(&ulFlags, 1);

  } else if (ulFlags <= 15) {
    UBYTE ub = 8;
    nm.WriteBits(&ub, 4);
    nm.WriteBits(&ulFlags, 4);

  } else if (ulFlags <= 255) {
    UBYTE ub = 16;
    nm.WriteBits(&ub, 5);
    nm.WriteBits(&ulFlags, 8);

  } else if (ulFlags <= 65535) {
    UBYTE ub = 32;
    nm.WriteBits(&ub, 6);
    nm.WriteBits(&ulFlags, 16);

  } else {
    UBYTE ub = 0;
    nm.WriteBits(&ub, 6);
    nm.WriteBits(&ulFlags, 32);
  }

  return nm;
};

// Read player action from a network packet
CNetworkMessage &operator>>(CNetworkMessage &nm, CPlayerAction &pa) {
  nm.Read(&pa.pa_llCreated, sizeof(pa.pa_llCreated));

  ULONG *pul = (ULONG *)&pa.pa_vTranslation;

  for (INDEX i = 0; i < 9; i++) {
    UBYTE ub = 0;
    nm.ReadBits(&ub, 1);

    if (ub == 0) {
      *pul = 0;
    } else {
      nm.ReadBits(pul, 32);
    }

    pul++;
  }

  // Find number of zero bits for flags
  INDEX iZeros = 0;

  for(; iZeros < 6; iZeros++) {
    UBYTE ub = 0;
    nm.ReadBits(&ub, 1);

    if (ub != 0) {
      break;
    }
  }

  ULONG ulFlags = 0;

  // Now read flags according to the number of bits
  switch (iZeros) {
    case 0: {
      ulFlags = 0;
    } break;

    case 1: {
      ulFlags = 1;
    } break;

    case 2: {
      ulFlags = 0;
      nm.ReadBits(&ulFlags, 1);
      ulFlags |= 2;
    } break;

    case 3: {
      ulFlags = 0;
      nm.ReadBits(&ulFlags, 4);
    } break;

    case 4: {
      ulFlags = 0;
      nm.ReadBits(&ulFlags, 8);
    } break;

    case 5: {
      ulFlags = 0;
      nm.ReadBits(&ulFlags, 16);
    } break;

    default: {
      ulFlags = 0;
      nm.ReadBits(&ulFlags, 32);
    } break;
  }

  pa.pa_ulButtons = ulFlags;
  return nm;
};

// Write player action into a stream
CTStream &operator<<(CTStream &strm, const CPlayerAction &pa) {
  strm.Write_t(&pa, sizeof(pa));
  return strm;
};

// Read player action from a stream
CTStream &operator>>(CTStream &strm, CPlayerAction &pa) {
  strm.Read_t(&pa, sizeof(pa));
  return strm;
};

#endif
