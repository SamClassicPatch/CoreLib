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

#ifndef CECIL_INCL_UNZIP_H
#define CECIL_INCL_UNZIP_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// Interface with functions for getting files out of ZIP archives
class CORE_API IUnzip {
  public:
    // Add one zip archive to the currently active set
    static void AddArchive(const CTFileName &fnm);

    // Read directories of all currently added archives in reverse alphabetical order
    static void ReadDirectoriesReverse_t(void);

  public:
    // Enumeration for all files in all zips
    static INDEX GetFileCount(void);

    // Get file at a specific position
    static const CTFileName &GetFileAtIndex(INDEX i);

    // Check if specific file is from a mod
    static BOOL IsFileAtIndexMod(INDEX i);

    // Get index of a specific file (-1 if no file)
    static INDEX GetFileIndex(const CTFileName &fnm);

    // Get info of a zip file entry
    static void GetFileInfo(INDEX iHandle, CTFileName &fnmZip,
      SLONG &slOffset, SLONG &slSizeCompressed, SLONG &slSizeUncompressed, 
      BOOL &bCompressed);

    // Check if a file entry exists
    static inline BOOL FileExists(const CTFileName &fnm) {
      return (GetFileIndex(fnm) != -1);
    };

  public:
    // Open a zip file entry for reading
    static INDEX Open_t(const CTFileName &fnm);

    // Get uncompressed size of a file
    static SLONG GetSize(INDEX iHandle);

    // Get CRC of a file
    static ULONG GetCRC(INDEX iHandle);

    // Read a block from ZIP file
    static void ReadBlock_t(INDEX iHandle, UBYTE *pub, SLONG slStart, SLONG slLen);

    // Close a ZIP file entry
    static void Close(INDEX iHandle);
};

#endif
