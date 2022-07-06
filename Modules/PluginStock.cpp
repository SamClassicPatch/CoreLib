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
