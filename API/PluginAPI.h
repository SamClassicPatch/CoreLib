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

#ifndef CECIL_INCL_PLUGINAPI_H
#define CECIL_INCL_PLUGINAPI_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// Declare certain classes | Which files to include to define classes
class CPluginModule; // #include <CoreLib/Modules/PluginModule.h>
class CPluginStock;  // #include <CoreLib/Modules/PluginStock.h>

// API for handling plugin modules
class CPluginAPI {
  public:
    CPluginStock *pPluginStock; // Stock of plugin modules

  public:
    // Constructor
    CPluginAPI();
    
    // Obtain pointer to a plugin module
    virtual CPluginModule *ObtainPlugin_t(const CTFileName &fnmModule);
};

#endif
