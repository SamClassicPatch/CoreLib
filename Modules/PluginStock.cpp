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

#include "PluginStock.h"

#define TYPE CPluginModule
#define CStock_TYPE CStock_CPluginModule
#define CNameTable_TYPE CNameTable_CPluginModule
#define CNameTableSlot_TYPE CNameTableSlot_CPluginModule

#include <Engine/Templates/NameTable.cpp>
#include <Engine/Templates/Stock.cpp>

#undef CNameTableSlot_TYPE
#undef CNameTable_TYPE
#undef CStock_TYPE
#undef TYPE

// Let plugin module load itself from a file
CPluginModule *CPluginStock::Obtain_t(const CTFileName &fnmFileName) {
  // Find stocked plugin with the same name
  CPluginModule *pExisting = st_ntObjects.Find(fnmFileName);

  // If found
  if (pExisting != NULL) {
    // Use it once more
    pExisting->MarkUsed();

    return pExisting;
  }

  // Add new plugin module
  CPluginModule *pNewPlugin = new CPluginModule;
  pNewPlugin->ser_FileName = fnmFileName;

  st_ctObjects.Add(pNewPlugin);
  st_ntObjects.Add(pNewPlugin);

  try {
    // Try to load the plugin
    pNewPlugin->Load_t(fnmFileName);

  } catch (char *) {
    // Release the plugin if couldn't load it
    st_ctObjects.Remove(pNewPlugin);
    st_ntObjects.Remove(pNewPlugin);
    delete pNewPlugin;

    throw;
  }

  // Mark as used for the first time
  pNewPlugin->MarkUsed();

  // Return it
  return pNewPlugin;
};

// Forcefully release a plugin
void CPluginStock::ForceRelease(CPluginModule *pPlugin) {
  // Use once less
  pPlugin->MarkUnused();

  // Forcefully release it if not used anymore
  if (!pPlugin->IsUsed()) {
    st_ctObjects.Remove(pPlugin);
    st_ntObjects.Remove(pPlugin);

    delete pPlugin;
  }
};
