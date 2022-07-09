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

#ifndef CECIL_INCL_PLUGINSTOCK_H
#define CECIL_INCL_PLUGINSTOCK_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include "PluginModule.h"

// ENGINE_API is useless in this context, since it's an out-of-Engine stock
#undef ENGINE_API
#define ENGINE_API virtual // Workaround for marking certain methods as virtual

#define TYPE CPluginModule
#define CStock_TYPE CStock_CPluginModule
#define CNameTable_TYPE CNameTable_CPluginModule
#define CNameTableSlot_TYPE CNameTableSlot_CPluginModule

#include <Engine/Templates/NameTable.h>
#include <Engine/Templates/Stock.h>

#undef CNameTableSlot_TYPE
#undef CNameTable_TYPE
#undef CStock_TYPE
#undef TYPE

// Restore engine API
#undef ENGINE_API
#define ENGINE_API __declspec(dllimport)

// Enhanced stock class
class CPluginStock : public CStock_CPluginModule
{
  public:
    // Let plugin module load itself from a file
    virtual CPluginModule *Obtain_t(const CTFileName &fnmFileName);

    // Release plugin manually
    virtual void ReleasePlugin(CPluginModule *pPlugin);
};

#endif
