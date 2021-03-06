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

#ifndef CECIL_INCL_WORLDENTITIES_H
#define CECIL_INCL_WORLDENTITIES_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include <Engine/Entities/Entity.h>
#include <Engine/World/World.h>

// Containers of entities
typedef CDynamicContainer<CEntity>             CEntities;
typedef CDynamicContainer<CLiveEntity>         CLiveEntities;
typedef CDynamicContainer<CRationalEntity>     CRationalEntities;
typedef CDynamicContainer<CMovableEntity>      CMovableEntities;
typedef CDynamicContainer<CMovableModelEntity> CMovableModelEntities;
typedef CDynamicContainer<CMovableBrushEntity> CMovableBrushEntities;
typedef CDynamicContainer<CPlayerEntity>       CPlayerEntities;

// Interface of useful methods for world and entity manipulation
class IWorld {
  public:
    // Get current game world
    static inline CWorld *GetWorld(void) {
      return &_pNetwork->ga_World;
    };

    // Find entity in a world by its ID
    static inline CEntity *FindEntityByID(CWorld *pwo, const ULONG ulEntityID) {
      FOREACHINDYNAMICCONTAINER(GetWorld()->wo_cenEntities, CEntity, iten) {
        CEntity *pen = &*iten;

        if (pen->GetFlags() & ENF_DELETED) {
          continue;
        }

        // Same ID
        if (pen->en_ulID == ulEntityID) {
          return pen;
        }
      }

      return NULL;
    };

    // Find entity property by its ID or offset
    static inline CEntityProperty *FindProperty(CEntity *pen, const ULONG ulID, const SLONG slOffset, const INDEX iType) {
      CDLLEntityClass *pdec = pen->en_pecClass->ec_pdecDLLClass;

      while (pdec != NULL) {
        // For each property
        for (INDEX iProp = 0; iProp < pdec->dec_ctProperties; iProp++) {
          CEntityProperty &ep = pdec->dec_aepProperties[iProp];

          // Only check the matching type
          if (ep.ep_eptType != iType) {
            continue;
          }

          // Matching ID or offset (ID is more likely to remain the same)
          if (ep.ep_ulID == ulID || ep.ep_slOffset == slOffset) {
            return &ep;
          }
        }

        // Next class in the hierarchy
        pdec = pdec->dec_pdecBase;
      }

      return NULL;
    };

    // Find WorldSettingsController in a world
    static inline CEntity *GetWSC(CWorld *pwo) {
      CEntity *penBack = pwo->GetBackgroundViewer();

      if (penBack != NULL) {
        // Get property offset only once
        static SLONG slPointerOffset = -1;
    
        if (slPointerOffset == -1) {
          // Obtain entity pointer property
          CEntityProperty *pep = penBack->PropertyForName("World settings controller");

          // No entity pointer
          if (pep == NULL) return NULL;

          slPointerOffset = pep->ep_slOffset;
        }
    
        // Obtain WorldSettingsController
        return (CEntity *)ENTITYPROPERTY(penBack, slPointerOffset, CEntityPointer);
      }

      return NULL;
    };

    // Find entities of a specific class
    static inline void FindClasses(CEntities &cInput, CEntities &cOutput, const char *strClass) {
      ASSERT(cOutput.Count() == 0);

      FOREACHINDYNAMICCONTAINER(cInput, CEntity, iten) {
        CEntity *penCheck = iten;

        if (!IsDerivedFromClass(penCheck, strClass)) {
          continue;
        }

        cOutput.Add(penCheck);
      }
    };

    // Get pointers to local player entities
    static inline void GetLocalPlayers(CPlayerEntities &cOutput) {
      ASSERT(cOutput.Count() == 0);

      const INDEX ctPlayers = _pNetwork->ga_aplsPlayers.Count();

      // Go through all local players
      for (INDEX iLocalPlayer = 0; iLocalPlayer < ctPlayers; iLocalPlayer++) {
        INDEX iPlayer = _pNetwork->ga_aplsPlayers[iLocalPlayer].pls_Index;

        // Player hasn't been added
        if (iPlayer < 0) {
          continue;
        }

        // Add player entity
        CPlayerTarget &plt = _pNetwork->ga_sesSessionState.ses_apltPlayers[iPlayer];
        cOutput.Add(plt.plt_penPlayerEntity);
      }
    };
};

#endif
