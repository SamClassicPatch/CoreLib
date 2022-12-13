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

// [Cecil] Definitions of unexported methods for working with ZIP archives from the engine

#ifndef CECIL_INCL_UNZIP_DEFS_H
#define CECIL_INCL_UNZIP_DEFS_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#ifndef CORE_NO_ZLIB

#include <Extras/zlib/zlib.h>

#include <Engine/Base/Unzip.h>
#include <CoreLib/Interfaces/DataFunctions.h>

// [Cecil] Pointer to 'zip_csLock' in the engine
static CTCriticalSection *_pcsZipLockEngine = ADDR_UNZIP_CRITSEC;

#pragma pack(1)

// Before each file in the zip
#define SIGNATURE_LFH 0x04034b50

struct LocalFileHeader {
  SWORD lfh_swVersionToExtract;
  SWORD lfh_swGPBFlag;
  SWORD lfh_swCompressionMethod;
  SWORD lfh_swModFileTime;
  SWORD lfh_swModFileDate;
  SLONG lfh_slCRC32;
  SLONG lfh_slCompressedSize;
  SLONG lfh_slUncompressedSize;
  SWORD lfh_swFileNameLen;
  SWORD lfh_swExtraFieldLen;

// follows:
//  filename (variable size)
//  extra field (variable size)
};

// After file data, only if compressed from a non-seekable stream
// this exists only if bit 3 in GPB flag is set
#define SIGNATURE_DD 0x08074b50

struct DataDescriptor {
  SLONG dd_slCRC32;
  SLONG dd_slCompressedSize;
  SLONG dd_slUncompressedSize;
};

// One file in central dir
#define SIGNATURE_FH 0x02014b50

struct FileHeader {
  SWORD fh_swVersionMadeBy;
  SWORD fh_swVersionToExtract;
  SWORD fh_swGPBFlag;
  SWORD fh_swCompressionMethod;
  SWORD fh_swModFileTime;
  SWORD fh_swModFileDate;
  SLONG fh_slCRC32;
  SLONG fh_slCompressedSize;
  SLONG fh_slUncompressedSize;
  SWORD fh_swFileNameLen;
  SWORD fh_swExtraFieldLen;
  SWORD fh_swFileCommentLen;
  SWORD fh_swDiskNoStart;
  SWORD fh_swInternalFileAttributes;
  SLONG fh_swExternalFileAttributes;
  SLONG fh_slLocalHeaderOffset;

// follows:
//  filename (variable size)
//  extra field (variable size)
//  file comment (variable size)
};

// At the end of entire zip file
#define SIGNATURE_EOD 0x06054b50

struct EndOfDir {
  SWORD eod_swDiskNo;
  SWORD eod_swDirStartDiskNo;
  SWORD eod_swEntriesInDirOnThisDisk;
  SWORD eod_swEntriesInDir;
  SLONG eod_slSizeOfDir;
  SLONG eod_slDirOffsetInFile;
  SWORD eod_swCommentLenght;

// follows: 
//  zipfile comment (variable size)
};

#pragma pack()

// One entry (a zipped file) in a zip archive
class CZipEntry {
  public:
    CTFileName *ze_pfnmArchive;  // Path of the archive
    CTFileName ze_fnm;           // File name with path inside archive
    SLONG ze_slCompressedSize;   // Size of file in the archive
    SLONG ze_slUncompressedSize; // Size when uncompressed
    SLONG ze_slDataOffset;       // Position of compressed data inside archive
    ULONG ze_ulCRC;              // Checksum of the file
    BOOL ze_bStored;             // Set if file is not compressed, but stored
    BOOL ze_bMod;                // Set if from a mod's archive

    void Clear(void) {
      ze_pfnmArchive = NULL;
      ze_fnm.Clear();
    };
};

#define BUF_SIZE 1024

// An open instance of a file inside a zip
class CZipHandle {
  public:
    BOOL zh_bOpen;        // Set if the handle is used
    CZipEntry zh_zeEntry; // The entry itself
    z_stream zh_zstream;  // zlib filestream for decompression
    FILE *zh_fFile;       // Open handle of the archive
    UBYTE *zh_pubBufIn;   // Input buffer

    CZipHandle(void);
    void Clear(void);
    void ThrowZLIBError_t(int ierr, const CTString &strDescription);
};

// Get error string for a zlib error
CTString GetZlibError(int ierr)
{
  switch (ierr) {
    case Z_OK:            return TRANS("Z_OK           ");
    case Z_STREAM_END:    return TRANS("Z_STREAM_END   ");
    case Z_NEED_DICT:     return TRANS("Z_NEED_DICT    ");
    case Z_STREAM_ERROR:  return TRANS("Z_STREAM_ERROR ");
    case Z_DATA_ERROR:    return TRANS("Z_DATA_ERROR   ");
    case Z_MEM_ERROR:     return TRANS("Z_MEM_ERROR    ");
    case Z_BUF_ERROR:     return TRANS("Z_BUF_ERROR    ");
    case Z_VERSION_ERROR: return TRANS("Z_VERSION_ERROR");

    case Z_ERRNO: {
      CTString strError;
      strError.PrintF(TRANS("Z_ERRNO: %s"), strerror(errno));
      return strError; 
    }

    default: {
      CTString strError;
      strError.PrintF(TRANS("Unknown ZLIB error: %d"), ierr);
      return strError;
    }
  }
};

CZipHandle::CZipHandle(void)
{
  zh_bOpen = FALSE;
  zh_fFile = NULL;
  zh_pubBufIn = NULL;

  memset(&zh_zstream, 0, sizeof(zh_zstream));
};

void CZipHandle::Clear(void)
{
  zh_bOpen = FALSE;
  zh_zeEntry.Clear();

  // Clear the zlib stream
  CTSingleLock slZip(_pcsZipLockEngine, TRUE);

  inflateEnd(&zh_zstream);
  memset(&zh_zstream, 0, sizeof(zh_zstream));

  // Free buffers
  if (zh_pubBufIn != NULL) {
    FreeMemory(zh_pubBufIn);
    zh_pubBufIn = NULL;
  }

  // Close the zip archive file
  if (zh_fFile != NULL) {
    fclose(zh_fFile);
    zh_fFile = NULL;
  }
};

void CZipHandle::ThrowZLIBError_t(int ierr, const CTString &strDescription)
{
  ThrowF_t(TRANS("(%s/%s) %s - ZLIB error: %s - %s"), zh_zeEntry.ze_pfnmArchive->str_String,
    zh_zeEntry.ze_fnm.str_String, strDescription, GetZlibError(ierr), zh_zstream.msg);
};

// [Cecil] Pointer to '_azhHandles' in the engine
static CStaticStackArray<CZipHandle> &_aHandlesEngine = *(CStaticStackArray<CZipHandle> *)ADDR_UNZIP_HANDLES;

// [Cecil] Pointer to '_azeFiles' in the engine
static CStaticStackArray<CZipEntry> &_aFilesEngine = *(CStaticStackArray<CZipEntry> *)ADDR_UNZIP_ENTRIES;

// [Cecil] Pointer to '_afnmArchives' in the engine
static CStaticStackArray<CTFileName> &_aArchivesEngine = *(CStaticStackArray<CTFileName> *)ADDR_UNZIP_ARCHIVES;

// [Cecil] Handle verification (to avoid code duplication)
static inline BOOL VerifyHandle(INDEX iHandle) {
  // Check handle number
  if (iHandle < 0 || iHandle >= _aHandlesEngine.Count()) {
    ASSERT(FALSE);
    return FALSE;
  }

  // Check the handle
  CZipHandle &zh = _aHandlesEngine[iHandle];

  if (!zh.zh_bOpen) {
    ASSERT(FALSE);
    return FALSE;
  }

  return TRUE;
};

// Add one zip archive to the currently active set
void UNZIPAddArchive(const CTFileName &fnm)
{
  _aArchivesEngine.Push() = fnm;
};

// Read directory of a zip archive and add all files in it to active set
static void ReadZIPDirectory_t(CTFileName *pfnmZip)
{
  // Open the archive
  FILE *f = fopen(pfnmZip->str_String, "rb");

  if (f == NULL) {
    ThrowF_t(TRANS("%s: Cannot open file (%s)"), pfnmZip->str_String, strerror(errno));
  }

  // Start at the end of file, minus expected minimum overhead
  fseek(f, 0, SEEK_END);

  int iPos = ftell(f) - sizeof(long) - sizeof(EndOfDir) + 2;

  // Do not search more than 128k (should be around 65k at most)
  int iMinPos = iPos - 128 * 1024;

  if (iMinPos < 0) {
    iMinPos = 0;
  }

  EndOfDir eod;
  BOOL bEODFound = FALSE;

  // While not at the beginning
  for(; iPos > iMinPos; iPos--) {
    // Read signature
    fseek(f, iPos, SEEK_SET);

    int iSignature;
    fread(&iSignature, sizeof(iSignature), 1, f);

    // Found the signature
    if (iSignature == SIGNATURE_EOD) {
      // Read directory end
      fread(&eod, sizeof(eod), 1, f);

      // Cannot have a multi-volume zip
      if (eod.eod_swDiskNo != 0 || eod.eod_swDirStartDiskNo != 0
       || eod.eod_swEntriesInDirOnThisDisk != eod.eod_swEntriesInDir) {
        ThrowF_t(TRANS("%s: Multi-volume zips are not supported"), (CTString&)*pfnmZip);
      }

      // Cannot have an empty zip
      if (eod.eod_swEntriesInDir <= 0) {
        ThrowF_t(TRANS("%s: Empty zip"), (CTString&)*pfnmZip);
      }

      bEODFound = TRUE;
      break;
    }
  }

  // EOD is not found
  if (!bEODFound) {
    ThrowF_t(TRANS("%s: Cannot find 'end of central directory'"), (CTString&)*pfnmZip);
  }

  // Check if the zip is from a mod
  BOOL bMod = pfnmZip->HasPrefix(_fnmApplicationPath + "Mods\\")
           || pfnmZip->HasPrefix(_fnmCDPath + "Mods\\");

  // Go to the beginning of the central dir
  fseek(f, eod.eod_slDirOffsetInFile, SEEK_SET);

  INDEX ctFiles = 0;

  // For each file
  for (INDEX iFile = 0; iFile < eod.eod_swEntriesInDir; iFile++) {
    // Read the signature
    int slSig;
    fread(&slSig, sizeof(slSig), 1, f);

    // Unexpected signature
    if (slSig != SIGNATURE_FH) {
      ThrowF_t(TRANS("%s: Wrong signature for 'file header' number %d'"), pfnmZip->str_String, iFile);
    }

    // Read its header
    FileHeader fh;
    fread(&fh, sizeof(fh), 1, f);

    // Check the filename
    const SLONG slMaxFileName = 512;

    if (fh.fh_swFileNameLen > slMaxFileName) {
      ThrowF_t(TRANS("%s: Too long filepath in zip"), pfnmZip->str_String);
    }

    if (fh.fh_swFileNameLen <= 0) {
      ThrowF_t(TRANS("%s: Invalid filepath length in zip"), pfnmZip->str_String);
    }
    
    // Read the filename
    char strBuffer[slMaxFileName + 1];
    memset(strBuffer, 0, sizeof(strBuffer));

    fread(strBuffer, fh.fh_swFileNameLen, 1, f);

    // Skip eventual comment and extra fields
    if (fh.fh_swFileCommentLen + fh.fh_swExtraFieldLen > 0) {
      fseek(f, fh.fh_swFileCommentLen+fh.fh_swExtraFieldLen, SEEK_CUR);
    }

    // If it's a directory
    if (strBuffer[strlen(strBuffer) - 1] == '/') {
      // Check the size
      if (fh.fh_slUncompressedSize != 0 || fh.fh_slCompressedSize != 0) {
        ThrowF_t(TRANS("%s/%s: Invalid directory"), pfnmZip->str_String, strBuffer);
      }

    // If it's a file
    } else {
      ctFiles++;

      // Convert slashes in the filename
      IData::ConvertSlashes(strBuffer);

      // Create a new entry
      CZipEntry &ze = _aFilesEngine.Push();

      // Remember file data
      ze.ze_fnm = CTString(strBuffer);
      ze.ze_pfnmArchive = pfnmZip;
      ze.ze_slCompressedSize = fh.fh_slCompressedSize;
      ze.ze_slUncompressedSize = fh.fh_slUncompressedSize;
      ze.ze_slDataOffset = fh.fh_slLocalHeaderOffset;
      ze.ze_ulCRC = fh.fh_slCRC32;
      ze.ze_bMod = bMod;

      // Check for compressopn
      if (fh.fh_swCompressionMethod == 0) {
        ze.ze_bStored = TRUE;

      } else if (fh.fh_swCompressionMethod == 8) {
        ze.ze_bStored = FALSE;

      } else {
        ThrowF_t(TRANS("%s/%s: Only 'deflate' compression is supported"),
          ze.ze_pfnmArchive->str_String, ze.ze_fnm.str_String);
      }
    }
  }

  // Some error has occurred
  if (ferror(f)) {
    ThrowF_t(TRANS("%s: Error reading central directory"), (CTString&)*pfnmZip);
  }

  // Report that the file has been read
  CPrintF(TRANS("  %s: %d files\n"), pfnmZip->str_String, ctFiles++);
};

// Read one directory of an archive
static void ReadOneArchiveDir_t(CTFileName &fnm)
{
  // Remember current number of files
  INDEX ctOrgFiles = _aFilesEngine.Count();

  // Try to read the directory and add all files
  try {
    ReadZIPDirectory_t(&fnm);

  } catch (char *) {
    // Remove added files
    if (ctOrgFiles < _aFilesEngine.Count())
    {
      if (ctOrgFiles == 0) {
        _aFilesEngine.PopAll();
      } else {
        _aFilesEngine.PopUntil(ctOrgFiles - 1);
      }
    }

    // Pass the error further up
    throw;
  }
};

int qsort_ArchiveCTFileName_reverse(const void *pElement1, const void *pElement2)
{                   
  // get the filenames
  const CTFileName &fnm1 = *(const CTFileName *)pElement1;
  const CTFileName &fnm2 = *(const CTFileName *)pElement2;

  // Calculate priorities based on the location of a GRO file
  BOOL bMod1 = fnm1.HasPrefix(_fnmApplicationPath + "Mods\\");
  BOOL bCD1 = fnm1.HasPrefix(_fnmCDPath);
  BOOL bModCD1 = fnm1.HasPrefix(_fnmCDPath + "Mods\\");

  INDEX iPriority1 = 1;

  if (bMod1) {
    iPriority1 = 3;

  } else if (bModCD1) {
    iPriority1 = 2;

  } else if (bCD1) {
    iPriority1 = 0;
  }

  BOOL bMod2 = fnm2.HasPrefix(_fnmApplicationPath + "Mods\\");
  BOOL bCD2 = fnm2.HasPrefix(_fnmCDPath);
  BOOL bModCD2 = fnm2.HasPrefix(_fnmCDPath + "Mods\\");

  INDEX iPriority2 = 1;

  if (bMod2) {
    iPriority2 = 3;

  } else if (bModCD2) {
    iPriority2 = 2;

  } else if (bCD2) {
    iPriority2 = 0;
  }

  // find sorting order
  if (iPriority1 < iPriority2) {
    return +1;
  } else if (iPriority1 > iPriority2) {
    return -1;
  }

  return -stricmp(fnm1.str_String, fnm2.str_String);
};

// read directories of all currently added archives, in reverse alphabetical order
void UNZIPReadDirectoriesReverse_t(void)
{
  // No archives
  if (_aArchivesEngine.Count() == 0) return;

  // Sort the archive filenames reversely
  qsort(&_aArchivesEngine[0], _aArchivesEngine.Count(), sizeof(CTFileName), qsort_ArchiveCTFileName_reverse);

  CTString strAllErrors = "";

  // Go through the archives
  for (INDEX iArchive = 0; iArchive < _aArchivesEngine.Count(); iArchive++) {
    // Try to read the archive directory
    try {
      ReadOneArchiveDir_t(_aArchivesEngine[iArchive]);

    // Write the error
    } catch (char *strError) {
      strAllErrors += strError;
      strAllErrors += "\n";
    }
  }

  // Report any errors
  if (strAllErrors != "") {
    ThrowF_t(strAllErrors.str_String);
  }
};

// Check if a file entry exists
BOOL UNZIPFileExists(const CTFileName &fnm) {
  // [Cecil] Check for the index to avoid code duplication
  return (UNZIPGetFileIndex(fnm) != -1);
};

// Enumeration for all files in all zips
INDEX UNZIPGetFileCount(void) {
  return _aFilesEngine.Count();
};

// Get file at a specific position
const CTFileName &UNZIPGetFileAtIndex(INDEX i) {
  return _aFilesEngine[i].ze_fnm;
};

// Check if specific file is from a mod
BOOL UNZIPIsFileAtIndexMod(INDEX i) {
  return _aFilesEngine[i].ze_bMod;
};

// Get index of a specific file (-1 if no file)
INDEX UNZIPGetFileIndex(const CTFileName &fnm)
{
  for (INDEX iFile = 0; iFile < _aFilesEngine.Count(); iFile++) {
    // Filename matches
    if (_aFilesEngine[iFile].ze_fnm == fnm) {
      return iFile;
    }
  }

  return -1;
};

// Get info of a zip file entry
void UNZIPGetFileInfo(INDEX iHandle, CTFileName &fnmZip, 
  SLONG &slOffset, SLONG &slSizeCompressed, SLONG &slSizeUncompressed, BOOL &bCompressed)
{
  if (!VerifyHandle(iHandle)) return;
  
  // Get parameters of the entry
  const CZipEntry &ze = _aHandlesEngine[iHandle].zh_zeEntry;

  fnmZip = *ze.ze_pfnmArchive;
  bCompressed = !ze.ze_bStored;
  slOffset = ze.ze_slDataOffset;
  slSizeCompressed = ze.ze_slCompressedSize;
  slSizeUncompressed = ze.ze_slUncompressedSize;
};

// Open a zip file entry for reading
INDEX UNZIPOpen_t(const CTFileName &fnm)
{
  CZipEntry *pze = NULL;

  for (INDEX iFile = 0; iFile < _aFilesEngine.Count(); iFile++)
  {
    // Stop searching if it's that one
    if (_aFilesEngine[iFile].ze_fnm == fnm) {
      pze = &_aFilesEngine[iFile];
      break;
    }
  }

  // Not found
  if (pze == NULL) {
    ThrowF_t(TRANS("File not found: %s"), fnm.str_String);
  }

  // Go through each existing handle
  BOOL bHandleFound = FALSE;
  INDEX iHandle = 1;

  for (; iHandle < _aHandlesEngine.Count(); iHandle++) {
    // Found unused one
    if (!_aHandlesEngine[iHandle].zh_bOpen) {
      bHandleFound = TRUE;
      break;
    }
  }

  // If no free handle found
  if (!bHandleFound) {
    // Create a new one
    iHandle = _aHandlesEngine.Count();
    _aHandlesEngine.Push(1);
  }
  
  // Get the handle
  CZipHandle &zh = _aHandlesEngine[iHandle];

  ASSERT(!zh.zh_bOpen);
  zh.zh_zeEntry = *pze;

  // Open zip archive for reading
  zh.zh_fFile = fopen(pze->ze_pfnmArchive->str_String, "rb");

  // If failed to open it
  if (zh.zh_fFile == NULL) {
    // Clear the handle
    zh.Clear();

    // Report error
    ThrowF_t(TRANS("Cannot open '%s': %s"), pze->ze_pfnmArchive->str_String, strerror(errno));
  }

  // Seek to the local header of the entry
  fseek(zh.zh_fFile, zh.zh_zeEntry.ze_slDataOffset, SEEK_SET);

  // Read the signature
  int slSig;
  fread(&slSig, sizeof(slSig), 1, zh.zh_fFile);

  // Unexpected signature
  if (slSig != SIGNATURE_LFH) {
    ThrowF_t(TRANS("%s/%s: Wrong signature for 'local file header'"), 
      zh.zh_zeEntry.ze_pfnmArchive->str_String, zh.zh_zeEntry.ze_fnm.str_String);
  }

  // Read the header
  LocalFileHeader lfh;
  fread(&lfh, sizeof(lfh), 1, zh.zh_fFile);

  // Determine exact compressed data position
  zh.zh_zeEntry.ze_slDataOffset = ftell(zh.zh_fFile) + lfh.lfh_swFileNameLen + lfh.lfh_swExtraFieldLen;

  // Seek there
  fseek(zh.zh_fFile, zh.zh_zeEntry.ze_slDataOffset, SEEK_SET);

  // Allocate buffers
  zh.zh_pubBufIn = (UBYTE *)AllocMemory(BUF_SIZE);

  // Initialize zlib stream
  CTSingleLock slZip(_pcsZipLockEngine, TRUE);

  zh.zh_zstream.next_out  = NULL;
  zh.zh_zstream.avail_out = 0;
  zh.zh_zstream.next_in   = NULL;
  zh.zh_zstream.avail_in  = 0;
  zh.zh_zstream.zalloc = (alloc_func)Z_NULL;
  zh.zh_zstream.zfree = (free_func)Z_NULL;

  int err = inflateInit2(&zh.zh_zstream, -15);

  // If failed
  if (err != Z_OK) {
    // Clean up what is possible
    FreeMemory(zh.zh_pubBufIn);
    zh.zh_pubBufIn = NULL;

    fclose(zh.zh_fFile);
    zh.zh_fFile = NULL;

    zh.ThrowZLIBError_t(err, TRANS("Cannot init inflation"));
  }

  // Return the handle successfully
  zh.zh_bOpen = TRUE;

  return iHandle;
};

// Get uncompressed size of a file
SLONG UNZIPGetSize(INDEX iHandle)
{
  if (!VerifyHandle(iHandle)) return 0;

  return _aHandlesEngine[iHandle].zh_zeEntry.ze_slUncompressedSize;
};

// Get CRC of a file
ULONG UNZIPGetCRC(INDEX iHandle)
{
  if (!VerifyHandle(iHandle)) return 0;

  return _aHandlesEngine[iHandle].zh_zeEntry.ze_ulCRC;
};

// Read a block from ZIP file
void UNZIPReadBlock_t(INDEX iHandle, UBYTE *pub, SLONG slStart, SLONG slLen)
{
  if (!VerifyHandle(iHandle)) return;

  CZipHandle &zh = _aHandlesEngine[iHandle];

  // Over the end of file
  if (slStart >= zh.zh_zeEntry.ze_slUncompressedSize) {
    return;
  }

  // Clamp length to end of the entry data
  slLen = Min(slLen, zh.zh_zeEntry.ze_slUncompressedSize - slStart);

  // If not compressed
  if (zh.zh_zeEntry.ze_bStored) {
    // Just read from file
    fseek(zh.zh_fFile, zh.zh_zeEntry.ze_slDataOffset+slStart, SEEK_SET);
    fread(pub, 1, slLen, zh.zh_fFile);
    return;
  }

  CTSingleLock slZip(_pcsZipLockEngine, TRUE);

  // If behind the current pointer
  if (slStart < zh.zh_zstream.total_out) {
    // Reset the zlib stream to beginning
    inflateReset(&zh.zh_zstream);
    zh.zh_zstream.avail_in = 0;
    zh.zh_zstream.next_in = NULL;

    // Seek to start of zip entry data inside archive
    fseek(zh.zh_fFile, zh.zh_zeEntry.ze_slDataOffset, SEEK_SET);
  }

  // While ahead of the current pointer
  while (slStart > zh.zh_zstream.total_out)
  {
    // If zlib has no more input
    while (zh.zh_zstream.avail_in == 0) {
      // Read more to it
      SLONG slRead = fread(zh.zh_pubBufIn, 1, BUF_SIZE, zh.zh_fFile);

      if (slRead <= 0) {
        return;
      }

      // Tell zlib that there is more to read
      zh.zh_zstream.next_in = zh.zh_pubBufIn;
      zh.zh_zstream.avail_in = slRead;
    }

    // Read dummy data from the output
    #define DUMMY_SIZE 256
    UBYTE aubDummy[DUMMY_SIZE];

    // Decode to output
    zh.zh_zstream.avail_out = Min(SLONG(slStart-zh.zh_zstream.total_out), SLONG(DUMMY_SIZE));
    zh.zh_zstream.next_out = aubDummy;

    int ierr = inflate(&zh.zh_zstream, Z_SYNC_FLUSH);

    if (ierr != Z_OK && ierr != Z_STREAM_END) {
      zh.ThrowZLIBError_t(ierr, TRANS("Error seeking in zip"));
    }
  }

  // If not streaming continuously
  if (slStart != zh.zh_zstream.total_out) {
    // This should not happen
    ASSERT(FALSE);

    // Read empty
    memset(pub, 0, slLen);

    return;
  }

  // Set zlib for writing to the block
  zh.zh_zstream.avail_out = slLen;
  zh.zh_zstream.next_out = pub;

  // While there is something to write to given block
  while (zh.zh_zstream.avail_out > 0)
  {
    // If zlib has no more input
    while (zh.zh_zstream.avail_in == 0) {
      // Read more to it
      SLONG slRead = fread(zh.zh_pubBufIn, 1, BUF_SIZE, zh.zh_fFile);

      if (slRead <= 0) {
        return;
      }

      // Tell zlib that there is more to read
      zh.zh_zstream.next_in = zh.zh_pubBufIn;
      zh.zh_zstream.avail_in = slRead;
    }

    // Decode to output
    int ierr = inflate(&zh.zh_zstream, Z_SYNC_FLUSH);

    if (ierr != Z_OK && ierr != Z_STREAM_END) {
      zh.ThrowZLIBError_t(ierr, TRANS("Error reading from zip"));
    }
  }
};

// Close a ZIP file entry
void UNZIPClose(INDEX iHandle)
{
  if (!VerifyHandle(iHandle)) return;

  // Clear it
  _aHandlesEngine[iHandle].Clear();
};

#endif

#endif
