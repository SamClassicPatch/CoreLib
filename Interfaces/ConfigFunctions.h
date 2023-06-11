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

#include <STLIncludesBegin.h>
#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <STLIncludesEnd.h>

// INI config structure
class CIniConfig {
  protected:
    // STL-styled comparator of CTString classes for sorting
    struct IniCompareCTString
    {
      __forceinline bool operator()(const CTString &lhs, const CTString &rhs) const {
        return strcmp(lhs, rhs) < 0;
      };
    };

  public:
    // Property pairs (key, value) and groups (group name, pairs)
    typedef std::map<CTString, CTString, IniCompareCTString> CPairs;
    typedef std::map<CTString, CPairs, IniCompareCTString> CGroups;

  public:
    CGroups aConfig;

  public:
    // Clear the config
    inline void Clear(void) {
      aConfig.clear();
    };

    // Check if some group exists
    inline BOOL GroupExists(const CTString &strGroup) {
      return aConfig.find(strGroup) != aConfig.end();
    };

    // Delete some group
    inline BOOL DeleteGroup(const CTString &strGroup) {
      return aConfig.erase(strGroup) != 0;
    };

    // Delete key under some group
    inline BOOL DeleteKey(const CTString &strGroup, const CTString &strKey) {
      // Find group
      CGroups::iterator it = aConfig.find(strGroup);
      if (it == aConfig.end()) return FALSE;

      return it->second.erase(strKey) != 0;
    };

  protected:
    // Parse config line and set key and value in a specific group, if it's not a group line
    BOOL ParseLine(const std::string &strLine, CTString &strGroup) {
      // Skip empty lines
      size_t iFirst = strLine.find_first_not_of(" \r\n\t");

      if (iFirst == std::string::npos) return FALSE;

      // Parse a group name if it's enclosed in square brackets
      if (strLine[iFirst] == '[') {
        // Skip opening bracket and search for the closing one
        iFirst++;
        size_t iLast = strLine.find_last_not_of(" \r\n\t");

        if (strLine[iLast] == ']') {
          // Just change the group
          strGroup = strLine.substr(iFirst, iLast - iFirst).c_str();
          return FALSE;
        }
      }

      // Get key and value separator
      size_t iSeparator = strLine.find('=');
      if (iSeparator == std::string::npos) return FALSE;

      // Get key and value around the assignment operator
      SetValue(strGroup, strLine.substr(0, iSeparator).c_str(), strLine.substr(iSeparator + 1).c_str());

      return TRUE;
    };

  public:
    // Set value to a property under some group
    void SetValue(const CTString &strGroup, const CTString &strKey, const CTString &strValue) {
      // STL should create groups and keys that don't already exist
      aConfig[strGroup][strKey] = strValue;
    };

    // Get value from a property under some group
    CTString GetValue(const CTString &strGroup, const CTString &strKey, const CTString &strDefValue = "") const {
      // Find group
      CGroups::const_iterator itGroup = aConfig.find(strGroup);
      if (itGroup == aConfig.end()) return strDefValue;

      // Find pair
      CPairs::const_iterator itPair = itGroup->second.find(strKey);
      if (itPair == itGroup->second.end()) return strDefValue;

      return itPair->second;
    };

    // Save config into a file
    void Save_t(const CTString &strFile) {
      CGroups::const_iterator itGroup;
      CPairs::const_iterator itPair;

      // Create new file
      CTFileStream strmConfig;
      strmConfig.Create_t(strFile);

      for (itGroup = aConfig.begin(); itGroup != aConfig.end(); itGroup++) {
        // Write group
        if (itGroup->first != "") {
          strmConfig.FPrintF_t("[%s]\n", itGroup->first.str_String);
        }

        // Write every key-value pair from it
        const CPairs &aProps = itGroup->second;

        for (itPair = aProps.begin(); itPair != aProps.end(); itPair++)
        {
          strmConfig.FPrintF_t("%s=%s\n", itPair->first.str_String, itPair->second.str_String);
        }
      }

      strmConfig.Close();
    };

    // Load config from a file
    void Load_t(const CTString &strFile, BOOL bEngineStreams) {
      // Current group and a key-value pair to add to it
      CTString strGroup = "";
      CTString strKey = "";
      CTString strValue = "";

      // Use Serious Engine streams (can load from GRO packages)
      if (bEngineStreams) {
        CTFileStream strm;
        strm.Open_t(strFile);

        while (!strm.AtEOF()) {
          // Read non-empty line
          char strBuffer[1024];
          IData::GetLineFromStream_t(strm, strBuffer, sizeof(strBuffer));

          ParseLine(strBuffer, strGroup);
        }

        strm.Close();

      // Use STL streams (can be used before engine initialization)
      } else {
        std::ifstream strm((CCoreAPI::GetAppPath() + strFile).str_String);

        if (strm.fail()) {
          ThrowF_t(LOCALIZE("Cannot open file `%s' (%s)"), strFile.str_String, strerror(errno));
        }

        std::string strLine;

        while (std::getline(strm, strLine)) {
          ParseLine(strLine, strGroup);
        }

        strm.close();
      }
    };
};

#endif
