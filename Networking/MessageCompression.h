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

#ifndef CECIL_INCL_MESSAGECOMPRESSION_H
#define CECIL_INCL_MESSAGECOMPRESSION_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include <Engine/Network/Network.h>
#include <Engine/Network/NetworkMessage.h>

#include <cctype>

// Up to 64 possible characters in a path string (least used are last)
static const char *_achCompressedPathCharacters = "_ABCDEFGHIJKLMNOPQRSTUVWXYZ.\\0123456789 -()!+='&,;[]{}`$%#@~";

// Interface with methods for compressing data into network packets
class INetCompress {
  public:
    // Compress 32-bit integer
    static inline void Integer(CNetworkMessage &nm, ULONG ul) {
      if (ul == 0x0) {
        UBYTE ub = 1;
        nm.WriteBits(&ub, 1);

      } else if (ul == 0x1) {
        UBYTE ub = 2;
        nm.WriteBits(&ub, 2);

      } else if (ul <= 0x3) {
        UBYTE ub = 4;
        nm.WriteBits(&ub, 3);
        nm.WriteBits(&ul, 1);

      } else if (ul <= 0xF) {
        UBYTE ub = 8;
        nm.WriteBits(&ub, 4);
        nm.WriteBits(&ul, 4);

      } else if (ul <= 0xFF) {
        UBYTE ub = 16;
        nm.WriteBits(&ub, 5);
        nm.WriteBits(&ul, 8);

      } else if (ul <= 0xFFFF) {
        UBYTE ub = 32;
        nm.WriteBits(&ub, 6);
        nm.WriteBits(&ul, 16);

      } else {
        UBYTE ub = 0;
        nm.WriteBits(&ub, 6);
        nm.WriteBits(&ul, 32);
      }
    };

    // Compress path character
    static inline char PathChar(CNetworkMessage &nm, char ch) {
      ch = toupper(ch);
      UBYTE ubChar = 0; // First character can act as invalid one

      for (UBYTE i = 0; i < 61; i++) {
        if (ch == _achCompressedPathCharacters[i]) {
          ubChar = i;
          break;
        }
      }

      nm.WriteBits(&ubChar, 6);
      return _achCompressedPathCharacters[ubChar];
    };

    // Compress float value
    static inline void Float(CNetworkMessage &nm, FLOAT f) {
      if (f > -0.001f && f < 0.001f) {
        UBYTE ub = 0;
        nm.WriteBits(&ub, 1);

      } else {
        UBYTE ub = 1;
        nm.WriteBits(&ub, 1);
        nm.WriteBits(&f, 32);
      }
    };

    // Compress angle in the 0-360 range
    static inline void Angle(CNetworkMessage &nm, ANGLE f) {
      f = WrapAngle(f);

      if (f < 0.001f) {
        UBYTE ub = 0;
        nm.WriteBits(&ub, 1);

      } else {
        UBYTE ub = 1;
        nm.WriteBits(&ub, 1);

        // Compress angle into a 15-bit integer
        UWORD uwAngle = Clamp((DOUBLE)f * (32767.0 / 360.0), 0.0, 32767.0);
        nm.WriteBits(&uwAngle, 15);
      }
    };
};

// Interface with methods for decompressing data from network packets
class INetDecompress {
  public:
    // Decompress 32-bit integer
    static inline void Integer(CNetworkMessage &nm, ULONG &ul) {
      ul = 0;

      // Find number of zero bits for flags
      INDEX iZeros = 0;

      for (; iZeros < 6; iZeros++) {
        UBYTE ub = 0;
        nm.ReadBits(&ub, 1);

        if (ub != 0) break;
      }

      // Now read flags according to the number of bits
      switch (iZeros) {
        case 0:  ul = 0; break;
        case 1:  ul = 1; break;
        case 2:  nm.ReadBits(&ul, 1); ul |= 2; break;
        case 3:  nm.ReadBits(&ul, 4); break;
        case 4:  nm.ReadBits(&ul, 8); break;
        case 5:  nm.ReadBits(&ul, 16); break;
        default: nm.ReadBits(&ul, 32); break;
      }
    };

    // Decompress path character
    static inline char PathChar(CNetworkMessage &nm) {
      UBYTE ubChar = 0;
      nm.ReadBits(&ubChar, 6);

      return _achCompressedPathCharacters[ubChar];
    };

    // Decompress float value
    static inline void Float(CNetworkMessage &nm, FLOAT &f) {
      UBYTE ub = 0;
      nm.ReadBits(&ub, 1);

      f = 0.0f;

      if (ub == 1) {
        nm.ReadBits(&f, 32);
      }
    };

    // Decompress angle in the 0-360 range
    static inline void Angle(CNetworkMessage &nm, ANGLE &f) {
      UBYTE ub = 0;
      nm.ReadBits(&ub, 1);

      f = 0.0f;

      if (ub == 1) {
        UWORD uwAngle = 0;
        nm.ReadBits(&uwAngle, 15);

        // Decompress angle from a 15-bit integer
        f = (DOUBLE)uwAngle * (360.0 / 32767.0);
      }
    };
};

#endif
