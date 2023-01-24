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

#ifndef CECIL_INCL_PLUGINSYMBOLS_H
#define CECIL_INCL_PLUGINSYMBOLS_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// Class for defining shell symbols within plugins
class CPluginSymbol
{
  public:
    // Available symbol types
    enum ESymbolType {
      E_INDEX  = 0,
      E_FLOAT  = 1,
      E_STRING = 2,
    };

  private:
    ESymbolType _type; // Variable type
    ULONG _ulFlags; // Symbol flags
    CTString _strValue; // Default value to assign

    // Eventual pointer to the registered symbol
    CShellSymbol *_pss;

  public:
    // Constructor for an index type
    CPluginSymbol(const ULONG ulSetFlags, const INDEX iDefault) :
      _type(E_INDEX), _ulFlags(ulSetFlags)
    {
      // Print integer value
      _strValue.PrintF("%d", iDefault);
    };

    // Constructor for a float type
    CPluginSymbol(const ULONG ulSetFlags, const FLOAT fDefault) :
      _type(E_FLOAT), _ulFlags(ulSetFlags)
    {
      // Print float value
      _strValue.PrintF("%f", fDefault);
    };

    // Constructor for a string type
    CPluginSymbol(const ULONG ulSetFlags, const char *strDefault) :
      _type(E_STRING), _ulFlags(ulSetFlags)
    {
      _strValue = strDefault;
      const INDEX iLast = _strValue.Length() - 1;

      // Go from the end
      for (INDEX i = iLast; i >= 0; i--) {
        // Double all backslashes and add them to quotes
        if (strDefault[i] == '\\'
         || strDefault[i] == '"') {
          _strValue.InsertChar(i, '\\');
        }
      }

      // Surround with quotes
      _strValue = "\"" + _strValue + "\"";
    };

    // Get symbol type
    ESymbolType GetType(void) const {
      return _type;
    };

    // Get symbol flags
    ULONG GetFlags(void) const {
      return _ulFlags;
    };

    // Get default value
    const CTString &GetDefaultValue(void) const {
      return _strValue;
    };

    // Set pointer to the shell symbol
    void SetSymbol(CShellSymbol *pssSet) {
      _pss = pssSet;
    };

    // Get index value
    INDEX GetIndex(void) const {
      ASSERT(_type == E_INDEX);
      ASSERT(_pss != NULL);

      return *(INDEX *)_pss->ss_pvValue;
    };

    // Get float value
    FLOAT GetFloat(void) const {
      ASSERT(_type == E_FLOAT);
      ASSERT(_pss != NULL);

      return *(FLOAT *)_pss->ss_pvValue;
    };

    // Get string value
    const CTString &GetString(void) const {
      ASSERT(_type == E_STRING);
      ASSERT(_pss != NULL);

      return *(CTString *)_pss->ss_pvValue;
    };

    // Register this symbol through the API
    inline void Register(const char *strSymbolName, const char *strPreFunc, const char *strPostFunc);
};

#endif
