/* Copyright (c) 2021-2022 by ZCaliptium.
   Copyright (c) 2022 Dreamy Cecil

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

// Abstract class that represents loaded plugin library
class CPluginModule : public CSerial
{
  public:
    // Plugin method types
    typedef void (*CVoidFunc)(void); // Simple method
    typedef void (*CDrawFunc)(CDrawPort *pdp); // Draw method
    typedef void (*CInfoFunc)(CPluginAPI::PluginInfo *pInfo); // Plugin info method

  public:
    HINSTANCE _hiLibrary; // Library handle
    BOOL _bInitialized; // Plugin has been initialized
    CPluginAPI::PluginInfo _info; // Plugin information

  public:
    // Constructor
    CPluginModule();

    // Destructor
    virtual ~CPluginModule();

    // Return library handle
    inline HINSTANCE GetHandle(void) {
      return _hiLibrary;
    };

    // Check if plugin has been initialized
    inline BOOL IsInitialized(void) {
      return _bInitialized;
    };

    // Get plugin information
    inline const CPluginAPI::PluginInfo &GetInfo(void) {
      return _info;
    };

    // Module initialization
    virtual void Initialize(void);

    // Module cleanup
    virtual void Clear(void);

    // Reset class fields
    virtual void ResetFields(void);

    // Write to stream (obsolete method)
    virtual void Write_t(CTStream *ostrFile);

    // Read from stream (obsolete method)
    virtual void Read_t(CTStream *istrFile);

    // Load plugin module (override non-virtual CSerial::Load_t)
    virtual void Load_t(const CTFileName &fnmDLL);
    
    // Return amount of used memory in bytes
    virtual SLONG GetUsedMemory(void);

    // Check if this kind of objects is can be freed automatically
    virtual BOOL IsAutoFreed(void) {
      return FALSE;
    };

    // Get specific symbol from the module (must be a pointer to the pointer variable)
    template<class Type> void GetSymbol_t(Type *ppSymbol, const char *strSymbolName) {
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

  // Plugin methods
  public:
    CVoidFunc pOnStartupFunc; // Entry point for the plugin
    CVoidFunc pOnShutdownFunc; // Plugin cleanup before releasing it
    CInfoFunc pGetInfoFunc; // Retrieve information about the plugin

    CVoidFunc pOnStepFunc; // Ñalled every simulation tick for executing synchronized logic
    CDrawFunc pOnPreDrawFunc; // Called before every game redraw
    CDrawFunc pOnPostDrawFunc; // Called after every game redraw
    CDrawFunc pOnFrameFunc; // Ñalled every render frame

    // Call startup method
    virtual void OnStartup(void);

    // Call shutdown method
    virtual void OnShutdown(void);

    // Call step method
    virtual void OnStep(void);

    // Call pre-draw method
    virtual void OnPreDraw(CDrawPort *pdp);

    // Call post-draw method
    virtual void OnPostDraw(CDrawPort *pdp);

    // Call frame method
    virtual void OnFrame(CDrawPort *pdp);
};

#endif
