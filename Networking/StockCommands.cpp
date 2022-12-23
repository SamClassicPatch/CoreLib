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

#include "StockCommands.h"
#include "Interfaces/WorldFunctions.h"

// Display name of the current map
BOOL IStockCommands::CurrentMap(CTString &strResult, INDEX, const CTString &) {
  CWorld &wo = *IWorld::GetWorld();
  strResult.PrintF("Current map: \"%s^r\"\n- %s", wo.wo_strName, wo.wo_fnmFileName.str_String);

  return TRUE;
};
