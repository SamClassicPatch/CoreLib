/* Copyright (c) 2021-2022 by ZCaliptium.

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

#ifndef SE_INCL_PLUGINMODULE_H
#define SE_INCL_PLUGINMODULE_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

//! Abstract class that represents loaded plugin library.
class CPluginModule : public CSerial 
{
  public:
    HINSTANCE _hiLibrary;

  public:
    // [Cecil] Plugin method types
    typedef void (*CVoidFunc)(void); // Simple method

    //! Pointer to initialization routines, called after LoadLibrary.
    CVoidFunc pOnStartupFunc;
    
    //! Pointer to deinit routines, called before FreeLibrary.
    CVoidFunc pOnShutdownFunc;

  public:
    //! Constructor.
    CPluginModule();

    //! Destructor
    virtual ~CPluginModule();

    //! Clear module. 
    virtual void Clear(void);

    //! Read from stream.
    virtual void Read_t(CTStream *istrFile); // throw char *
    
    //! Write to stream.
    virtual void Write_t(CTStream *ostrFile); // throw char *

    // [Cecil] Load plugin module manually
    virtual void LoadPlugin_t(const CTFileName &fnmDLL); // throw char *
    
    //! Returns amount of used memory in bytes.
    virtual SLONG GetUsedMemory(void);

    // [Cecil] 'GetModuleHandle' -> 'GetHandle' to avoid the macro
    //! Returns loaded library handle.
    inline HINSTANCE GetHandle()
    {
      return _hiLibrary;
    }

    //! Check if this kind of objects is auto-freed.
    virtual BOOL IsAutoFreed(void)
    {
      return FALSE;
    };

    // [Cecil] Get specific symbol from the module (must be a pointer to the pointer variable)
    template<class Type> void GetSymbol_t(Type *ppSymbol, const char *strSymbolName)
    {
      // No module
      if (GetHandle() == NULL) {
        ThrowF_t(TRANS("Plugin module has not been loaded yet!"));
      }

      *ppSymbol = (Type)GetProcAddress(GetHandle(), strSymbolName);

      // No symbol
      if (*ppSymbol == NULL) {
        ThrowF_t(TRANS("Cannot find '%s' symbol in '%s'!"), strSymbolName, GetName());
      }
    };
};

#endif  /* include-once check. */