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
#include <CoreLib/Objects/MapStructure.h>

#include <STLIncludesBegin.h>
#include <string>
#include <fstream>
#include <sstream>
#include <STLIncludesEnd.h>

// Uses STL internally for convenience but user input/output is CTString
typedef std::string IniStr;

typedef se1::map<IniStr, IniStr> IniKeys;
typedef se1::map<IniStr, IniKeys> IniSections;

// INI config structure
class CIniConfig : protected IniSections {
  private:
    // Parse config line and set key and value in a specific section, if it isn't a section line
    BOOL ParseLine(const IniStr &strLine, IniStr &strSection) {
      #define FIND_SPACES " \t"

      // Skip empty lines
      size_t iBeg = strLine.find_first_not_of(FIND_SPACES);
      if (iBeg == IniStr::npos) return FALSE;

      // Parse a group name if it's enclosed in square brackets
      if (strLine[iBeg] == '[') {
        // Skip opening bracket and search for the closing one
        size_t iEnd = strLine.find_last_not_of(FIND_SPACES);

        if (strLine[iEnd] == ']') {
          // Just change the group
          strSection = strLine.substr(iBeg + 1, iEnd - iBeg - 1).c_str();
          return FALSE;
        }
      }

      // Get key and value separator
      size_t iSeparator = strLine.find('=');
      if (iSeparator == IniStr::npos) return FALSE;

      // Get key and value around the assignment operator
      IniStr strKey = strLine.substr(0, iSeparator);
      IniStr strVal = strLine.substr(iSeparator + 1);

      {
        // Trim spaces around the key
        size_t iEnd = strKey.find_last_not_of(FIND_SPACES);
        strKey = strKey.substr(iBeg, iEnd - iBeg + 1);

        // Trim spaces around the value
        iBeg = strVal.find_first_not_of(FIND_SPACES);
        iEnd = strVal.find_last_not_of(FIND_SPACES);

        // Remove surrounding quotes
        if (iBeg != iEnd) {
          if (strVal[iBeg] == '\"') iBeg++;
          if (strVal[iEnd] == '\"') iEnd--;
        }

        strVal = strVal.substr(iBeg, iEnd - iBeg + 1);
      }

      SetValue(strSection.c_str(), strKey.c_str(), strVal.c_str());
      return TRUE;
    };

  public:
    // Clear the config
    inline void Clear(void) { clear(); };

    // Check if config is empty
    inline BOOL IsEmpty(void) const { return empty(); };

    // Check if some section exists
    BOOL SectionExists(const char *strSection) const {
      return find(strSection) != end();
    };

    // Check if some key exists under some section
    BOOL KeyExists(const char *strSection, const char *strKey) const {
      const_iterator it = find(strSection);
      if (it == end()) return false;

      IniKeys::const_iterator itPair = it->second.find(strKey);
      return itPair != it->second.end();
    };

    // Delete key under some section or the entire section (if strKey is NULL)
    bool Delete(const char *strSection, const char *strKey = NULL) {
      // No section
      iterator it = find(strSection);
      if (it == end()) return false;

      // Delete section
      if (strKey == NULL) {
        erase(it);
        return true;
      }

      IniKeys &keys = it->second;

      // No key
      IniKeys::iterator itPair = keys.find(strKey);
      if (itPair == keys.end()) return false;

      // Delete key
      keys.erase(itPair);
      return true;
    };

  public:
    // Set value to a key under some section
    void SetValue(const char *strSection, const char *strKey, const char *strValue) {
      // Create new section, if there isn't the one needed
      iterator it = find(strSection);
      if (it == end()) it = insert(value_type(strSection, IniKeys())).first;

      // Insert a new key-value pair or find an existing one
      IniKeys::_Pairib ib = it->second.insert(IniKeys::value_type(strKey, strValue));

      // Update value of the existing key
      if (!ib.second) {
        ib.first->second = strValue;
      }
    };

    // Get value under a key or return a default value, if key or section doesn't exist
    CTString GetValue(const char *strSection, const char *strKey, const char *strDefValue = "") const {
      const_iterator it = find(strSection);
      if (it == end()) return strDefValue;

      const IniKeys &pairs = it->second;

      IniKeys::const_iterator itPair = pairs.find(strKey);
      if (itPair == pairs.end()) return strDefValue;

      return itPair->second.c_str();
    };

    // Get boolean value under a key or return a default value, if key or section doesn't exist
    BOOL GetBoolValue(const char *strSection, const char *strKey, BOOL bDefValue) const {
      const_iterator it = find(strSection);
      if (it == end()) return bDefValue;

      const IniKeys &pairs = it->second;

      IniKeys::const_iterator itPair = pairs.find(strKey);
      if (itPair == pairs.end()) return bDefValue;

      // Determine boolean value from the beginning of a string
      const IniStr &str = itPair->second;
      char ch = toupper(str[0]); // First character

      switch (ch) {
        // True values
        case 'T': case 'Y': case '1': return TRUE;

        // False values
        case 'F': case 'N': case '0': return FALSE;

        // On/Off
        case 'O': {
          // This is either '\0' or another character
          ch = toupper(str[1]); // Second character

          if (ch == 'N') return TRUE;
          if (ch == 'F') return FALSE;
        } break;
      }

      return bDefValue;
    };

    // Get integer value under a key or return a default value, if key or section doesn't exist
    SLONG GetIntValue(const char *strSection, const char *strKey, SLONG iDefValue) const {
      const_iterator it = find(strSection);
      if (it == end()) return iDefValue;

      const IniKeys &pairs = it->second;

      IniKeys::const_iterator itPair = pairs.find(strKey);
      if (itPair == pairs.end()) return iDefValue;

      // Determine integer value from the string
      SLONG iValue;
      if (sscanf(itPair->second.c_str(), "%i", &iValue) == 1) return iValue;

      return iDefValue;
    };

    // Get float value under a key or return a default value, if key or section doesn't exist
    DOUBLE GetDoubleValue(const char *strSection, const char *strKey, DOUBLE dDefValue) const {
      const_iterator it = find(strSection);
      if (it == end()) return dDefValue;

      const IniKeys &pairs = it->second;

      IniKeys::const_iterator itPair = pairs.find(strKey);
      if (itPair == pairs.end()) return dDefValue;

      // Determine float value from the string
      DOUBLE dValue;
      if (sscanf(itPair->second.c_str(), "%lg", &dValue) == 1) return dValue;

      return dDefValue;
    };

  public:
    // Load config from a string
    void LoadData(const IniStr &str) {
      // Parse each line from the string
      std::istringstream strm(str);
      IniStr strLine, strSection;

      while (std::getline(strm, strLine)) {
        ParseLine(strLine, strSection);
      }
    };

    // Load config from a file
    void Load_t(const CTString &strFile, BOOL bEngineStreams)
    {
      // Use Serious Engine streams (can load from GRO packages)
      if (bEngineStreams) {
        CTFileStream strm;
        strm.Open_t(strFile);

        // Read all text from the file
        const size_t iFileSize = strm.GetStreamSize() - strm.GetPos_t();
        IniStr strContents(iFileSize, '\0');

        if (iFileSize > 0) {
          strm.Read_t(&strContents[0], iFileSize);

          // Replace carriage returns with other line breaks
          size_t iReturn = 0;
          while ((iReturn = strContents.find('\r', iReturn)) != IniStr::npos) {
            strContents[iReturn] = '\n';
          }
        }

        // Load data from a string
        LoadData(strContents);

        strm.Close();

      // Use stock loader (can be used before engine initialization)
      } else {
        std::ifstream strm((CCoreAPI::AppPath() + strFile).str_String);

        if (strm.fail()) {
          ThrowF_t(LOCALIZE("Cannot open file `%s' (%s)"), strFile, strerror(errno));
        }

        // Parse each line from the file
        IniStr strLine, strSection;

        while (std::getline(strm, strLine)) {
          ParseLine(strLine, strSection);
        }

        strm.close();
      }
    };

    // Save config into a string
    void SaveData(IniStr &str) const {
      std::ostringstream strm;
      const_iterator it = begin();

      for (; it != end(); it++) {
        // Non-empty section
        if (it->first != "") {
          strm << '[' << it->first << "]\n";
        }

        const IniKeys &pairs = it->second;
        IniKeys::const_iterator itPair = pairs.begin();

        // key = "value"
        for (; itPair != pairs.end(); itPair++) {
          const IniStr &str = itPair->second;

          // Find spaces on either end
          size_t iBegSpace = str.find_first_of(" \t");
          size_t iEndSpace = str.find_last_of(" \t");

          // Surround with quotes if there's a space on either end
          if (iBegSpace == 0 || iEndSpace == str.length() - 1) {
            strm << itPair->first << " = \"" << str << "\"\n";
          } else {
            strm << itPair->first << " = " << str << '\n';
          }
        }
      }

      str = strm.str();
    };

    // Save config into a file
    void Save_t(const CTString &strFile) const {
      CTFileStream strm;
      strm.Create_t(strFile);

      IniStr strSave;
      SaveData(strSave);
      strm.PutString_t(strSave.c_str());

      strm.Close();
    };
};

#endif
