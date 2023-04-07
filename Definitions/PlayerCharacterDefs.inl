/* Copyright (c) 2002-2012 Croteam Ltd. 
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

// [Cecil] Definitions of unexported CPlayerCharacter methods from the engine

#ifndef CECIL_INCL_PLAYERCHARACTER_DEFS_H
#define CECIL_INCL_PLAYERCHARACTER_DEFS_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#if CLASSICSPATCH_FUNC_DEFINITIONS

// Write player character into a network packet
CNetworkMessage &operator<<(CNetworkMessage &nm, CPlayerCharacter &pc) {
  nm << pc.pc_strName;
  nm << pc.pc_strTeam;

  nm.Write(pc.pc_aubGUID, PLAYERGUIDSIZE);
  nm.Write(pc.pc_aubAppearance, MAX_PLAYERAPPEARANCE);

  return nm;
};

// Read player character from a network packet
CNetworkMessage &operator>>(CNetworkMessage &nm, CPlayerCharacter &pc) {
  nm >> pc.pc_strName;
  nm >> pc.pc_strTeam;

  nm.Read(pc.pc_aubGUID, PLAYERGUIDSIZE);
  nm.Read(pc.pc_aubAppearance, MAX_PLAYERAPPEARANCE);

  return nm;
};

#endif // CLASSICSPATCH_FUNC_DEFINITIONS

#endif
