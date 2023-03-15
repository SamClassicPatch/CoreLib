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

#ifndef CECIL_INCL_DATAFUNCTIONS_H
#define CECIL_INCL_DATAFUNCTIONS_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// Interface of useful methods for data manipulation
class IData {
  public:
    // Replace characters in a string
    static inline void ReplaceChar(char *str, char chOld, char chNew) {
      while (*str != '\0') {
        if (*str == chOld) {
          *str = chNew;
        }
        ++str;
      }
    };

    // Find character in a string
    static inline ULONG FindChar(const char *str, char ch, ULONG ulFrom = 0) {
      // Iteration position
      const char *pData = str + ulFrom;

      // Go until the string end
      while (*pData != '\0') {
        // If found the character
        if (*pData == ch) {
          // Return current position relative to the beginning
          return (pData - str);
        }
        ++pData;
      }

      // None found
      return -1;
    };

    // Extract substring at a specific position
    static inline CTString ExtractSubstr(const char *str, ULONG ulFrom, ULONG ulChars) {
      // Limit character amount
      ulChars = ClampUp(ulChars, ULONG(strlen(str)) - ulFrom);

      // Copy substring into a null-terminated buffer
      char *strBuffer = new char[ulChars + 1];
      memcpy(strBuffer, str + ulFrom, ulChars);
      strBuffer[ulChars] = '\0';

      // Set a new string and discard the buffer
      CTString strSubstr(strBuffer);
      delete[] strBuffer;

      return strSubstr;
    };

    // Get position of a decorated character in a decorated string (doesn't count color tags)
    static inline INDEX GetDecoratedChar(const CTString &str, INDEX iChar) {
      // Start at the beginning of a string
      const char *pchSrc = str.str_String;
      INDEX ctTags = 0;

      // Repeat until the desired character index
      while (--iChar >= 0) {
        // String end
        if (pchSrc[0] == '\0') {
          break;
        }

        // If the source char is not escape char
        if (pchSrc[0] != '^') {
          // Count it normally
          pchSrc++;
          continue;
        }

        // Skip however many tag characters there are
        switch (pchSrc[1]) {
          case 'c': pchSrc += 2 + FindZero((UBYTE *)pchSrc + 2, 6); break;
          case 'a': pchSrc += 2 + FindZero((UBYTE *)pchSrc + 2, 2); break;
          case 'f': pchSrc += 2 + FindZero((UBYTE *)pchSrc + 2, 1); break;

          case 'b': case 'i': case 'r': case 'o':
          case 'C': case 'A': case 'F': case 'B': case 'I':
            pchSrc += 2;
            break;

          case '^':
            pchSrc++;
            break;
        }

        // Skipped one full tag
        ctTags++;
      }

      // Difference between the string beginning and where it stopped
      return INDEX(pchSrc - str.str_String) + ctTags;
    };

    // Check if a string matches any line of the string mask
    static inline BOOL MatchesMask(const CTString &strString, CTString strMask) {
      CTString strLine;

      // If there's still something in the mask
      while (strMask != "") {
        // Get first line of the mask
        strLine = strMask;
        strLine.OnlyFirstLine();

        // Remove this line from the mask including the line break
        strMask.TrimLeft(strMask.Length() - strLine.Length() + 1);

        // Check if the string matches the line
        if (strString.Matches(strLine)) {
          return TRUE;
        }
      }

      // No matching lines found
      return FALSE;
    };

    // Print out specific time in details (years, days, hours, minutes, seconds)
    static inline void PrintDetailedTime(CTString &strOut, CTimerValue tvTime) {
      // Get precise seconds
      __int64 iSeconds = (tvTime.tv_llValue / _pTimer->tm_llPerformanceCounterFrequency);

      // Limit down to 0 seconds
      iSeconds = ClampDn(iSeconds, __int64(0));

      // Timeout
      if (iSeconds == 0) {
        strOut = "0s";
        return;
      }

      // Display seconds
      const ULONG ulSec = iSeconds % 60;

      if (ulSec > 0) {
        strOut.PrintF("%us", ulSec);
      }

      // Display minutes
      const ULONG ulMin = (iSeconds / 60) % 60;

      if (ulMin > 0) {
        strOut.PrintF("%um %s", ulMin, strOut);
      }

      // Display hours
      const ULONG ulHours = (iSeconds / 3600) % 24;

      if (ulHours > 0) {
        strOut.PrintF("%uh %s", ulHours, strOut);
      }

      // Display days
      const ULONG ulDaysTotal = iSeconds / 3600 / 24;
      const ULONG ulDays = ulDaysTotal % 365;

      if (ulDays > 0) {
        strOut.PrintF("%ud %s", ulDays, strOut);
      }

      const ULONG ulYearsTotal = (ulDaysTotal / 365);

      if (ulYearsTotal > 0) {
        strOut.PrintF("%uy %s", ulYearsTotal, strOut);
      }
    };
};

#endif
