/* Copyright (c) 2023-2024 Dreamy Cecil
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

#include "Networking/ExtPackets.h"

#if CLASSICSPATCH_EXT_PACKETS

// Copy event bytes (iEventSize = sizeof(ee))
void CExtEntityEvent::SetEvent(CEntityEvent &ee, size_t iEventSize) {
  ASSERT(iEventSize <= sizeof(eEvent));
  ctFields = 0;

  // Copy event type
  eEvent.ee_slEvent = ee.ee_slEvent;

  // Skip bytes before event data
  const size_t iSkip = offsetof(CEntityEvent, ee_slEvent) + sizeof(ee.ee_slEvent);
  iEventSize -= iSkip;

  // No data to copy
  if (iEventSize == 0) return;

  // Count 4-byte fields
  ctFields = ceilf(FLOAT(iEventSize) * 0.25f);

  // [Cecil] NOTE: If some event has CEntityPointer fields, they need to contain entity IDs as 4-byte integers
  // instead of pointers to entities directly. When setting entity IDs for CEntityPointer fields, do it like this:
  //   (ULONG &)ee.pen = iEntityID;
  // DO NOT FORGET to do this at the end of the function to avoid crashes upon calling the pointer destructor:
  //   (ULONG &)ee.pen = NULL;
  void *pEventData = ((UBYTE *)&ee) + iSkip;
  memcpy(eEvent.aulFields, pEventData, iEventSize);
};

// Copy event data from another event
void CExtEntityEvent::Copy(const EExtEntityEvent &eeOther, ULONG ctSetFields) {
  eEvent.ee_slEvent = eeOther.ee_slEvent;
  memcpy(eEvent.aulFields, eeOther.aulFields, sizeof(eEvent.aulFields));

  ctFields = ctSetFields;
};

void CExtEntityEvent::Write(CNetworkMessage &nm) {
  WriteEntity(nm);
  eEvent.Write(nm, ctFields);
};

void CExtEntityEvent::Read(CNetworkMessage &nm) {
  ReadEntity(nm);
  ctFields = eEvent.Read(nm);

  // Convert fields of a specific event
  if (ctFields > 0) {
    eEvent.ConvertTypes();
  }
};

void CExtEntityEvent::Process(void) {
  CEntity *pen = GetEntity();

  if (!EntityExists(pen)) return;

  pen->SendEvent(eEvent);
};

void CExtEntityItem::Process(void) {
  CEntity *pen = GetEntity();

  if (!EntityExists(pen)) return;

  pen->ReceiveItem(eEvent);
};

#endif // CLASSICSPATCH_EXT_PACKETS
