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

#ifndef CECIL_INCL_CONFIGFUNCTIONS_H
#define CECIL_INCL_CONFIGFUNCTIONS_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include <CoreLib/Interfaces/DataFunctions.h>

// Interface with functions for reading simple configuration files
namespace IConfig {

// Config property as a key-value pair
struct SConfigPair {
  CTString strKey;
  CTString strVal;
};

// List of config properties
typedef CStaticStackArray<SConfigPair> CProperties;

inline BOOL ReadConfig_t(CProperties &aProps, const CTString &strFile) {
  // Opened and read at least one property
  BOOL bRead = FALSE;

  try {
    CTFileStream strm;
    strm.Open_t(strFile);

    // Line separated into a property and its value
    CTString strLine;
    char strProp[1024];
    char strValue[1024];

    INDEX iLine = 0;

    while (!strm.AtEOF()) {
      // Read non-empty line
      IData::GetLineFromStream_t(strm, strProp, sizeof(strProp));
      iLine++;

      strLine = strProp;
      strLine.TrimSpacesLeft();
      strLine.TrimSpacesRight();

      // Skip empty lines
      if (strLine == "") continue;

      // Try to read property with a string value
      if (strLine.ScanF("%1024[^=]=%1024s", strProp, strValue) != 2) {
        // Invalid line
        throw iLine;
      }

      // Add new property
      SConfigPair &pair = aProps.Push();
      pair.strKey = strProp;
      pair.strVal = strValue;

      bRead = TRUE;
    }

  // Ignore reading errors
  } catch (char *strError) {
    (void)strError;

  // Throw line error as a string
  } catch (INDEX iError) {
    ThrowF_t(TRANS("Cannot read config file '%s':\nInvalid key-value pair on line %d"), strFile, iError);
  }

  return bRead;
};

}; // namespace

#endif
