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

#define VANILLA_EVENTS_ENTITY_ID
#include "Compatibility/VanillaEvents.h"

// Declare various extra symbols for use with extension packets
void DeclareExtraSymbolsForExtPackets(void)
{
  #define DECLARE_SYMBOL(_Symbol, _Value) _pShell->DeclareSymbol("const INDEX " _Symbol ";", (void *)&_Value)

  // Vanilla event types
  DECLARE_SYMBOL("EVENTCODE_EStop",                 EVENTCODE_VNL_EStop);
  DECLARE_SYMBOL("EVENTCODE_EStart",                EVENTCODE_VNL_EStart);
  DECLARE_SYMBOL("EVENTCODE_EActivate",             EVENTCODE_VNL_EActivate);
  DECLARE_SYMBOL("EVENTCODE_EDeactivate",           EVENTCODE_VNL_EDeactivate);
  DECLARE_SYMBOL("EVENTCODE_EEnvironmentStart",     EVENTCODE_VNL_EEnvironmentStart);
  DECLARE_SYMBOL("EVENTCODE_EEnvironmentStop",      EVENTCODE_VNL_EEnvironmentStop);
  DECLARE_SYMBOL("EVENTCODE_EEnd",                  EVENTCODE_VNL_EEnd);
  DECLARE_SYMBOL("EVENTCODE_ETrigger",              EVENTCODE_VNL_ETrigger);
  DECLARE_SYMBOL("EVENTCODE_ETeleportMovingBrush",  EVENTCODE_VNL_ETeleportMovingBrush);
  DECLARE_SYMBOL("EVENTCODE_EReminder",             EVENTCODE_VNL_EReminder);
  DECLARE_SYMBOL("EVENTCODE_EStartAttack",          EVENTCODE_VNL_EStartAttack);
  DECLARE_SYMBOL("EVENTCODE_EStopAttack",           EVENTCODE_VNL_EStopAttack);
  DECLARE_SYMBOL("EVENTCODE_EStopBlindness",        EVENTCODE_VNL_EStopBlindness);
  DECLARE_SYMBOL("EVENTCODE_EStopDeafness",         EVENTCODE_VNL_EStopDeafness);
  DECLARE_SYMBOL("EVENTCODE_EReceiveScore",         EVENTCODE_VNL_EReceiveScore);
  DECLARE_SYMBOL("EVENTCODE_EKilledEnemy",          EVENTCODE_VNL_EKilledEnemy);
  DECLARE_SYMBOL("EVENTCODE_ESecretFound",          EVENTCODE_VNL_ESecretFound);

  DECLARE_SYMBOL("EVENTCODE_ESound",                EVENTCODE_VNL_ESound);
  DECLARE_SYMBOL("EVENTCODE_EScroll",               EVENTCODE_VNL_EScroll);
  DECLARE_SYMBOL("EVENTCODE_ETextFX",               EVENTCODE_VNL_ETextFX);
  DECLARE_SYMBOL("EVENTCODE_EHudPicFX",             EVENTCODE_VNL_EHudPicFX);
  DECLARE_SYMBOL("EVENTCODE_ECredits",              EVENTCODE_VNL_ECredits);
  DECLARE_SYMBOL("EVENTCODE_ECenterMessage",        EVENTCODE_VNL_ECenterMessage);
  DECLARE_SYMBOL("EVENTCODE_EComputerMessage",      EVENTCODE_VNL_EComputerMessage);
  DECLARE_SYMBOL("EVENTCODE_EVoiceMessage",         EVENTCODE_VNL_EVoiceMessage);
  DECLARE_SYMBOL("EVENTCODE_EHitBySpaceShipBeam",   EVENTCODE_VNL_EHitBySpaceShipBeam);

  DECLARE_SYMBOL("EVENTCODE_EAmmoItem",             EVENTCODE_VNL_EAmmoItem);
  DECLARE_SYMBOL("EVENTCODE_EAmmoPackItem",         EVENTCODE_VNL_EAmmoPackItem);
  DECLARE_SYMBOL("EVENTCODE_EArmor",                EVENTCODE_VNL_EArmor);
  DECLARE_SYMBOL("EVENTCODE_EHealth",               EVENTCODE_VNL_EHealth);
  DECLARE_SYMBOL("EVENTCODE_EKey",                  EVENTCODE_VNL_EKey);
  DECLARE_SYMBOL("EVENTCODE_EMessageItem",          EVENTCODE_VNL_EMessageItem);
  DECLARE_SYMBOL("EVENTCODE_EPowerUp",              EVENTCODE_VNL_EPowerUp);
  DECLARE_SYMBOL("EVENTCODE_EWeaponItem",           EVENTCODE_VNL_EWeaponItem);

  DECLARE_SYMBOL("EVENTCODE_ERestartAttack",        EVENTCODE_VNL_ERestartAttack);
  DECLARE_SYMBOL("EVENTCODE_EReconsiderBehavior",   EVENTCODE_VNL_EReconsiderBehavior);
  DECLARE_SYMBOL("EVENTCODE_EForceWound",           EVENTCODE_VNL_EForceWound);
  DECLARE_SYMBOL("EVENTCODE_ESelectWeapon",         EVENTCODE_VNL_ESelectWeapon);
  DECLARE_SYMBOL("EVENTCODE_EBoringWeapon",         EVENTCODE_VNL_EBoringWeapon);
  DECLARE_SYMBOL("EVENTCODE_EFireWeapon",           EVENTCODE_VNL_EFireWeapon);
  DECLARE_SYMBOL("EVENTCODE_EReleaseWeapon",        EVENTCODE_VNL_EReleaseWeapon);
  DECLARE_SYMBOL("EVENTCODE_EReloadWeapon",         EVENTCODE_VNL_EReloadWeapon);
  DECLARE_SYMBOL("EVENTCODE_EWeaponChanged",        EVENTCODE_VNL_EWeaponChanged);

  DECLARE_SYMBOL("EVENTCODE_EAirShockwave",         EVENTCODE_VNL_EAirShockwave);
  DECLARE_SYMBOL("EVENTCODE_EAirWave",              EVENTCODE_VNL_EAirWave);
  DECLARE_SYMBOL("EVENTCODE_ESpawnEffect",          EVENTCODE_VNL_ESpawnEffect);
  DECLARE_SYMBOL("EVENTCODE_ESpawnSpray",           EVENTCODE_VNL_ESpawnSpray);
  DECLARE_SYMBOL("EVENTCODE_EBulletInit",           EVENTCODE_VNL_EBulletInit);
  DECLARE_SYMBOL("EVENTCODE_ELaunchCannonBall",     EVENTCODE_VNL_ELaunchCannonBall);
  DECLARE_SYMBOL("EVENTCODE_ECyborgBike",           EVENTCODE_VNL_ECyborgBike);
  DECLARE_SYMBOL("EVENTCODE_ESpawnDebris",          EVENTCODE_VNL_ESpawnDebris);
  DECLARE_SYMBOL("EVENTCODE_EDevilProjectile",      EVENTCODE_VNL_EDevilProjectile);
  DECLARE_SYMBOL("EVENTCODE_ESpawnEffector",        EVENTCODE_VNL_ESpawnEffector);
  DECLARE_SYMBOL("EVENTCODE_EFlame",                EVENTCODE_VNL_EFlame);
  DECLARE_SYMBOL("EVENTCODE_ELaunchLarvaOffspring", EVENTCODE_VNL_ELaunchLarvaOffspring);
  DECLARE_SYMBOL("EVENTCODE_EAnimatorInit",         EVENTCODE_VNL_EAnimatorInit);
  DECLARE_SYMBOL("EVENTCODE_EViewInit",             EVENTCODE_VNL_EViewInit);
  DECLARE_SYMBOL("EVENTCODE_EWeaponsInit",          EVENTCODE_VNL_EWeaponsInit);
  DECLARE_SYMBOL("EVENTCODE_EWeaponEffectInit",     EVENTCODE_VNL_EWeaponEffectInit);
  DECLARE_SYMBOL("EVENTCODE_ELaunchProjectile",     EVENTCODE_VNL_ELaunchProjectile);
  DECLARE_SYMBOL("EVENTCODE_EReminderInit",         EVENTCODE_VNL_EReminderInit);
  DECLARE_SYMBOL("EVENTCODE_ESeriousBomb",          EVENTCODE_VNL_ESeriousBomb);
  DECLARE_SYMBOL("EVENTCODE_ESpawnerProjectile",    EVENTCODE_VNL_ESpawnerProjectile);
  DECLARE_SYMBOL("EVENTCODE_ESpinnerInit",          EVENTCODE_VNL_ESpinnerInit);
  DECLARE_SYMBOL("EVENTCODE_ETwister",              EVENTCODE_VNL_ETwister);
  DECLARE_SYMBOL("EVENTCODE_EWatcherInit",          EVENTCODE_VNL_EWatcherInit);
  DECLARE_SYMBOL("EVENTCODE_EWater",                EVENTCODE_VNL_EWater);

  // Entity flags
  static const INDEX iENF_SELECTED          = ENF_SELECTED;
  static const INDEX iENF_ZONING            = ENF_ZONING;
  static const INDEX iENF_DELETED           = ENF_DELETED;
  static const INDEX iENF_ALIVE             = ENF_ALIVE;
  static const INDEX iENF_INRENDERING       = ENF_INRENDERING;
  static const INDEX iENF_VALIDSHADINGINFO  = ENF_VALIDSHADINGINFO;
  static const INDEX iENF_SEETHROUGH        = ENF_SEETHROUGH;
  static const INDEX iENF_FOUNDINGRIDSEARCH = ENF_FOUNDINGRIDSEARCH;
  static const INDEX iENF_CLUSTERSHADOWS    = ENF_CLUSTERSHADOWS;
  static const INDEX iENF_BACKGROUND        = ENF_BACKGROUND;
  static const INDEX iENF_ANCHORED          = ENF_ANCHORED;
  static const INDEX iENF_HASPARTICLES      = ENF_HASPARTICLES;
  static const INDEX iENF_INVISIBLE         = ENF_INVISIBLE;
  static const INDEX iENF_DYNAMICSHADOWS    = ENF_DYNAMICSHADOWS;
  static const INDEX iENF_NOTIFYLEVELCHANGE = ENF_NOTIFYLEVELCHANGE;
  static const INDEX iENF_CROSSESLEVELS     = ENF_CROSSESLEVELS;
  static const INDEX iENF_PREDICTABLE       = ENF_PREDICTABLE;
  static const INDEX iENF_PREDICTOR         = ENF_PREDICTOR;
  static const INDEX iENF_PREDICTED         = ENF_PREDICTED;
  static const INDEX iENF_WILLBEPREDICTED   = ENF_WILLBEPREDICTED;
  static const INDEX iENF_TEMPPREDICTOR     = ENF_TEMPPREDICTOR;
  static const INDEX iENF_HIDDEN            = ENF_HIDDEN;
  static const INDEX iENF_NOSHADINGINFO     = ENF_NOSHADINGINFO;

  DECLARE_SYMBOL("ENF_SELECTED",          iENF_SELECTED);
  DECLARE_SYMBOL("ENF_ZONING",            iENF_ZONING);
  DECLARE_SYMBOL("ENF_DELETED",           iENF_DELETED);
  DECLARE_SYMBOL("ENF_ALIVE",             iENF_ALIVE);
  DECLARE_SYMBOL("ENF_INRENDERING",       iENF_INRENDERING);
  DECLARE_SYMBOL("ENF_VALIDSHADINGINFO",  iENF_VALIDSHADINGINFO);
  DECLARE_SYMBOL("ENF_SEETHROUGH",        iENF_SEETHROUGH);
  DECLARE_SYMBOL("ENF_FOUNDINGRIDSEARCH", iENF_FOUNDINGRIDSEARCH);
  DECLARE_SYMBOL("ENF_CLUSTERSHADOWS",    iENF_CLUSTERSHADOWS);
  DECLARE_SYMBOL("ENF_BACKGROUND",        iENF_BACKGROUND);
  DECLARE_SYMBOL("ENF_ANCHORED",          iENF_ANCHORED);
  DECLARE_SYMBOL("ENF_HASPARTICLES",      iENF_HASPARTICLES);
  DECLARE_SYMBOL("ENF_INVISIBLE",         iENF_INVISIBLE);
  DECLARE_SYMBOL("ENF_DYNAMICSHADOWS",    iENF_DYNAMICSHADOWS);
  DECLARE_SYMBOL("ENF_NOTIFYLEVELCHANGE", iENF_NOTIFYLEVELCHANGE);
  DECLARE_SYMBOL("ENF_CROSSESLEVELS",     iENF_CROSSESLEVELS);
  DECLARE_SYMBOL("ENF_PREDICTABLE",       iENF_PREDICTABLE);
  DECLARE_SYMBOL("ENF_PREDICTOR",         iENF_PREDICTOR);
  DECLARE_SYMBOL("ENF_PREDICTED",         iENF_PREDICTED);
  DECLARE_SYMBOL("ENF_WILLBEPREDICTED",   iENF_WILLBEPREDICTED);
  DECLARE_SYMBOL("ENF_TEMPPREDICTOR",     iENF_TEMPPREDICTOR);
  DECLARE_SYMBOL("ENF_HIDDEN",            iENF_HIDDEN);
  DECLARE_SYMBOL("ENF_NOSHADINGINFO",     iENF_NOSHADINGINFO);

  // Entity physics flags
  static const INDEX iEPF_ORIENTEDBYGRAVITY    = EPF_ORIENTEDBYGRAVITY;
  static const INDEX iEPF_TRANSLATEDBYGRAVITY  = EPF_TRANSLATEDBYGRAVITY;
  static const INDEX iEPF_PUSHABLE             = EPF_PUSHABLE;
  static const INDEX iEPF_STICKYFEET           = EPF_STICKYFEET;
  static const INDEX iEPF_RT_SYNCHRONIZED      = EPF_RT_SYNCHRONIZED;
  static const INDEX iEPF_ABSOLUTETRANSLATE    = EPF_ABSOLUTETRANSLATE;
  static const INDEX iEPF_NOACCELERATION       = EPF_NOACCELERATION;
  static const INDEX iEPF_HASLUNGS             = EPF_HASLUNGS;
  static const INDEX iEPF_HASGILLS             = EPF_HASGILLS;
  static const INDEX iEPF_MOVABLE              = EPF_MOVABLE;
  static const INDEX iEPF_NOIMPACT             = EPF_NOIMPACT;
  static const INDEX iEPF_NOIMPACTTHISTICK     = EPF_NOIMPACTTHISTICK;
  static const INDEX iEPF_CANFADESPINNING      = EPF_CANFADESPINNING;
  static const INDEX iEPF_ONSTEEPSLOPE         = EPF_ONSTEEPSLOPE;
  static const INDEX iEPF_ORIENTINGTOGRAVITY   = EPF_ORIENTINGTOGRAVITY;
  static const INDEX iEPF_FLOATING             = EPF_FLOATING;
  static const INDEX iEPF_FORCEADDED           = EPF_FORCEADDED;
  static const INDEX iEPF_ONBLOCK_MASK         = EPF_ONBLOCK_MASK;
  static const INDEX iEPF_ONBLOCK_STOP         = EPF_ONBLOCK_STOP;
  static const INDEX iEPF_ONBLOCK_SLIDE        = EPF_ONBLOCK_SLIDE;
  static const INDEX iEPF_ONBLOCK_CLIMBORSLIDE = EPF_ONBLOCK_CLIMBORSLIDE;
  static const INDEX iEPF_ONBLOCK_BOUNCE       = EPF_ONBLOCK_BOUNCE;
  static const INDEX iEPF_ONBLOCK_PUSH         = EPF_ONBLOCK_PUSH;
  static const INDEX iEPF_ONBLOCK_STOPEXACT    = EPF_ONBLOCK_STOPEXACT;

  DECLARE_SYMBOL("EPF_ORIENTEDBYGRAVITY",    iEPF_ORIENTEDBYGRAVITY);
  DECLARE_SYMBOL("EPF_TRANSLATEDBYGRAVITY",  iEPF_TRANSLATEDBYGRAVITY);
  DECLARE_SYMBOL("EPF_PUSHABLE",             iEPF_PUSHABLE);
  DECLARE_SYMBOL("EPF_STICKYFEET",           iEPF_STICKYFEET);
  DECLARE_SYMBOL("EPF_RT_SYNCHRONIZED",      iEPF_RT_SYNCHRONIZED);
  DECLARE_SYMBOL("EPF_ABSOLUTETRANSLATE",    iEPF_ABSOLUTETRANSLATE);
  DECLARE_SYMBOL("EPF_NOACCELERATION",       iEPF_NOACCELERATION);
  DECLARE_SYMBOL("EPF_HASLUNGS",             iEPF_HASLUNGS);
  DECLARE_SYMBOL("EPF_HASGILLS",             iEPF_HASGILLS);
  DECLARE_SYMBOL("EPF_MOVABLE",              iEPF_MOVABLE);
  DECLARE_SYMBOL("EPF_NOIMPACT",             iEPF_NOIMPACT);
  DECLARE_SYMBOL("EPF_NOIMPACTTHISTICK",     iEPF_NOIMPACTTHISTICK);
  DECLARE_SYMBOL("EPF_CANFADESPINNING",      iEPF_CANFADESPINNING);
  DECLARE_SYMBOL("EPF_ONSTEEPSLOPE",         iEPF_ONSTEEPSLOPE);
  DECLARE_SYMBOL("EPF_ORIENTINGTOGRAVITY",   iEPF_ORIENTINGTOGRAVITY);
  DECLARE_SYMBOL("EPF_FLOATING",             iEPF_FLOATING);
  DECLARE_SYMBOL("EPF_FORCEADDED",           iEPF_FORCEADDED);
  DECLARE_SYMBOL("EPF_ONBLOCK_MASK",         iEPF_ONBLOCK_MASK);
  DECLARE_SYMBOL("EPF_ONBLOCK_STOP",         iEPF_ONBLOCK_STOP);
  DECLARE_SYMBOL("EPF_ONBLOCK_SLIDE",        iEPF_ONBLOCK_SLIDE);
  DECLARE_SYMBOL("EPF_ONBLOCK_CLIMBORSLIDE", iEPF_ONBLOCK_CLIMBORSLIDE);
  DECLARE_SYMBOL("EPF_ONBLOCK_BOUNCE",       iEPF_ONBLOCK_BOUNCE);
  DECLARE_SYMBOL("EPF_ONBLOCK_PUSH",         iEPF_ONBLOCK_PUSH);
  DECLARE_SYMBOL("EPF_ONBLOCK_STOPEXACT",    iEPF_ONBLOCK_STOPEXACT);
};