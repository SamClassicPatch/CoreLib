/* Copyright (c) 2022 Dreamy Cecil
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

#ifndef CECIL_INCL_SYMBOLPTR_H
#define CECIL_INCL_SYMBOLPTR_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// Class to be utilized statically that searches for shell symbols
class CSymbolPtr {
  public:
    CShellSymbol *_pss;

  public:
    // Default constructor
    CSymbolPtr() : _pss(NULL)
    {
    };

    // Constructor with a symbol name
    CSymbolPtr(const char *strSymbolName) {
      Find(strSymbolName);
    };

    // Find symbol under a specific name
    void Find(const char *strSymbolName) {
      _pss = _pShell->GetSymbol(strSymbolName, TRUE);
    };

    // Check if symbol has been found
    BOOL Exists(void) const {
      return (_pss != NULL);
    };

    // Get index value
    const INDEX GetIndex(void) const {
      ASSERT(_pss != NULL);
      return *(INDEX *)_pss->ss_pvValue;
    };

    // Get float value
    const FLOAT GetFloat(void) const {
      ASSERT(_pss != NULL);
      return *(FLOAT *)_pss->ss_pvValue;
    };

    // Get string value
    const CTString &GetString(void) const {
      ASSERT(_pss != NULL);
      return *(CTString *)_pss->ss_pvValue;
    };
};

#endif