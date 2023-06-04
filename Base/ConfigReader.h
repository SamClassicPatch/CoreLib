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

#ifndef CECIL_INCL_CONFIGREADER_H
#define CECIL_INCL_CONFIGREADER_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// Interface with functions for reading simple configuration files
namespace IConfig {

// Config property as a key-value pair
struct SConfigPair {
  CTString strKey;
  CTString strVal;
};

// List of config properties
typedef CStaticStackArray<SConfigPair> CProperties;

// Read config values from a file using initialized engine streams or STL streams
BOOL ReadConfig_t(CProperties &aProps, const CTString &strFile, BOOL bEngineStreams);

}; // namespace

#endif
