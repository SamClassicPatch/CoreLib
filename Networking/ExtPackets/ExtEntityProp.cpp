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

#include "Networking/ExtPackets.h"
#include "Interfaces/PropertyFunctions.h"

#if CLASSICSPATCH_EXT_PACKETS

void CExtEntityProp::Write(CNetworkMessage &nm) {
  WriteEntity(nm);

  nm.WriteBits(&bName, 1);

  if (bName) {
    nm << ulProp;

  } else {
    INetCompress::Integer(nm, ulProp);
  }

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

  if (bName) {
    nm >> ulProp;

  } else {
    INetDecompress::Integer(nm, ulProp);
  }

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

  if (pen == NULL) return;

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
    IProperties::SetPropValue(pen, pep, &fValue);

  } else if (iType == CEntityProperty::EPT_INDEX) {
    INDEX iValue = fValue;
    IProperties::SetPropValue(pen, pep, &iValue);

  } else {
    ExtServerReport(TRANS("Expected number property type but got %d\n"), iType);
  }
};

#endif // CLASSICSPATCH_EXT_PACKETS
