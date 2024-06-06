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

#if _PATCHCONFIG_EXT_PACKETS

void CExtEntityProp::Write(CNetworkMessage &nm) {
  WriteEntity(nm);

  nm.WriteBits(&bName, 1);
  nm << ulProp;
  nm.WriteBits(&bString, 1);

  if (bString) {
    nm << strValue;
  } else {
    INetCompress::Double(nm, fValue);
  }
};

void CExtEntityProp::Read(CNetworkMessage &nm) {
  ReadEntity(nm);

  bName = FALSE;
  nm.ReadBits(&bName, 1);

  nm >> ulProp;

  bString = FALSE;
  nm.ReadBits(&bString, 1);

  if (bString) {
    nm >> strValue;
  } else {
    INetDecompress::Double(nm, fValue);
  }
};

void CExtEntityProp::Process(void) {
  CEntity *pen = GetEntity();

  if (!EntityExists(pen)) return;

  CEntityProperty *pep = NULL;

  if (bName) {
    pep = IWorld::PropertyForHash(pen, ulProp);
  } else {
    pep = IWorld::PropertyForId(pen, ulProp);
  }

  if (pep == NULL) return;

  INDEX iType = IProperties::ConvertType(pep->ep_eptType);

  if (bString) {
    if (iType == CEntityProperty::EPT_STRING) {
      IProperties::SetPropValue(pen, pep, &strValue);
    } else {
      ExtServerReport(TRANS("Expected string property type but got %d\n"), iType);
    }

  } else if (iType == CEntityProperty::EPT_FLOAT) {
    FLOAT fFloatProp = fValue;
    IProperties::SetPropValue(pen, pep, &fFloatProp);

  } else if (iType == CEntityProperty::EPT_INDEX) {
    INDEX iIntProp = fValue;
    IProperties::SetPropValue(pen, pep, &iIntProp);

  } else {
    ExtServerReport(TRANS("Expected number property type but got %d\n"), iType);
  }
};

#endif // _PATCHCONFIG_EXT_PACKETS
