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

#ifndef CECIL_INCL_APICONFIG_H
#define CECIL_INCL_APICONFIG_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// Define empty API if not utilizing Core directly
#ifndef CORE_API
  #define CORE_API
  #define CORE_POINTER_API

#else
  #define CORE_POINTER_API "C" CORE_API
#endif

// Current API version
#define CORE_API_VERSION 5

// Current Classics Patch version
#define CORE_PATCH_VERSION CCoreAPI::MakeVersion(1, 6, 0)

// Indication of a build under active development (disabled for releases)
#define CORE_DEV_BUILD 1

// Classics Patch configuration file
#define CORE_CONFIG_FILE CTString("Data\\ClassicsPatch\\Config.ini")

#endif