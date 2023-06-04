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

#include "ConfigReader.h"
#include "Interfaces/DataFunctions.h"

#include <STLIncludesBegin.h>
#include <fstream>
#include <sstream>
#include <STLIncludesEnd.h>

namespace IConfig {

// Parse config line into key and value
static inline BOOL ParseConfigLine(const std::string &strLine, SConfigPair &pair) {
  // Skip empty lines
  if (strLine.find_first_not_of(" \r\n\t") == std::string::npos) {
    return FALSE;
  }

  size_t iAssign = strLine.find('=');

  // No value
  if (iAssign == std::string::npos) return FALSE;

  // Get key before the assignment
  pair.strKey = strLine.substr(0, iAssign).c_str();

  // Get value after the assignment
  pair.strVal = strLine.substr(iAssign + 1).c_str();

  return TRUE;
};

// Read config values from a file using initialized engine streams or STL streams
BOOL ReadConfig_t(CProperties &aProps, const CTString &strFile, BOOL bEngineStreams) {
  // Opened and read at least one property
  BOOL bRead = FALSE;

  // Line separated into key and value
  std::string strLine;
  SConfigPair pair;

  // Use Serious Engine streams (can load from GRO packages)
  if (bEngineStreams) {
    CTFileStream strm;
    strm.Open_t(strFile);

    while (!strm.AtEOF()) {
      // Read non-empty line
      char strBuffer[1024];
      IData::GetLineFromStream_t(strm, strBuffer, sizeof(strBuffer));

      strLine = strBuffer;

      // Parse the line and add the pair
      if (ParseConfigLine(strBuffer, pair)) {
        aProps.Push() = pair;
        bRead = TRUE;
      }
    }

    strm.Close();

  // Use STL streams (can be used before engine initialization)
  } else {
    std::ifstream strm((CCoreAPI::GetAppPath() + strFile).str_String);

    if (strm.fail()) {
      ThrowF_t(LOCALIZE("Cannot open file `%s' (%s)"), strFile.str_String, strerror(errno));
    }

    while (std::getline(strm, strLine)) {
      // Parse the line and add the pair
      if (ParseConfigLine(strLine, pair)) {
        aProps.Push() = pair;
        bRead = TRUE;
      }
    }

    strm.close();
  }

  return bRead;
};

}; // namespace
