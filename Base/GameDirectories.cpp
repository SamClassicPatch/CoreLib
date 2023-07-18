/* Copyright (c) 2023 Dreamy Cecil
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

#include "Interfaces/FileFunctions.h"

// Other game directories
CTString _astrGameDirs[GAME_DIRECTORIES_CT];

// Check if a filename is under a specified game directory
BOOL IsFileFromDir(const CTString &strGameDir, const CTFileName &fnm) {
  if (strGameDir == "") return FALSE;

  CTFileName fnmFull;

  // Try checking the archive path
  if (ExpandFilePath(EFP_READ, fnm, fnmFull) == EFP_BASEZIP) {
    fnmFull = IUnzip::GetFileArchive(fnm);
  }

  return fnmFull.HasPrefix(strGameDir);
};
