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

#include "StdH.h"

// Constructor
CPluginAPI::CPluginAPI() {
  // Create stock of plugin modules
  pPluginStock = new CPluginStock;
};

// Obtain pointer to a plugin module
CPluginModule *CPluginAPI::ObtainPlugin_t(const CTFileName &fnmModule)
{
  CPluginModule *pPlugin = pPluginStock->Obtain_t(fnmModule);

  // Check used API version
  ULONG ulPluginVer = pPlugin->GetInfo().apiVer;

  if (ulPluginVer != CORE_API_VERSION) {
    CPrintF("'%s' load cancelled: Wrong API version (%u)\n", fnmModule.str_String, ulPluginVer);

    // Release it
    pPluginStock->ForcePlugin(pPlugin);

    // No plugin has been loaded
    return NULL;
  }

  // Initialize the plugin and return it
  pPlugin->Initialize();

  return pPlugin;
};

// Load plugin module without safety checks
CPluginModule *CPluginAPI::LoadPlugin_t(const CTFileName &fnmModule)
{
  CPluginModule *pPlugin = pPluginStock->Obtain_t(fnmModule);
  pPlugin->Initialize();

  return pPlugin;
};

// Get loaded plugins
CDynamicContainer<CPluginModule> &CPluginAPI::GetPlugins(void) {
  return pPluginStock->st_ctObjects;
};
