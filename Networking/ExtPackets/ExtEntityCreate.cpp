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
#include "Interfaces/WorldFunctions.h"

#if CLASSICSPATCH_EXT_PACKETS

CTString CExtEntityCreate::aBaseClasses[255] = {
  "Acid",
  "AirElemental",
  "AirShockwave",
  "AirWave",
  "AmmoItem",
  "AmmoPack",
  "AnimationChanger",
  "AnimationHub",
  "AreaMarker",
  "ArmorItem",
  "BackgroundViewer",
  "BasicEffect",
  "Beast",
  "BigHead",
  "BlendController",
  "BloodSpray",
  "Boneman",
  "Bouncer",
  "Bullet",
  "Camera",
  "CameraMarker",
  "CannonBall",
  "CannonRotating",
  "CannonStatic",
  "Catman",
  "ChainSawFreak",
  "Copier",
  "Counter",
  "CrateBus",
  "CrateRider",
  "CreditsHolder",
  "Cyborg",
  "CyborgBike",
  "Damager",
  "Debris",
  "Demon",
  "DestroyableArchitecture",
  "Devil",
  "DevilMarker",
  "DevilProjectile",
  "DoorController",
  "Dragonman",
  "EffectMarker",
  "Effector",
  "Elemental",
  "EnemyBase",
  "EnemyCounter",
  "EnemyDive",
  "EnemyFly",
  "EnemyMarker",
  "EnemyRunInto",
  "EnemySpawner",
  "EntityStateDisplay",
  "EnvironmentBase",
  "EnvironmentMarker",
  "EnvironmentParticlesHolder",
  "Eruptor",
  "ExotechLarva",
  "ExotechLarvaBattery",
  "ExotechLarvaCharger",
  "Eyeman",
  "Fish",
  "Fishman",
  "Flame",
  "FogMarker",
  "GhostBusterRay",
  "Gizmo",
  "GradientMarker",
  "GravityMarker",
  "GravityRouter",
  "Grunt",
  "Guffy",
  "HazeMarker",
  "Headman",
  "HealthItem",
  "Huanman",
  "HudPicHolder",
  "Item",
  "KeyItem",
  "LarvaOffspring",
  "Light",
  "Lightning",
  "Mamut",
  "Mamutman",
  "Mantaman",
  "Marker",
  "MessageHolder",
  "MessageItem",
  "MeteorShower",
  "MirrorMarker",
  "ModelDestruction",
  "ModelHolder",
  "ModelHolder2",
  "ModelHolder3",
  "MovingBrush",
  "MovingBrushMarker",
  "MusicChanger",
  "MusicHolder",
  "NavigationMarker",
  "ParticlesHolder",
  "Pendulum",
  "PhotoAlbum",
  "Pipebomb",
  "Player",
  "PlayerActionMarker",
  "PlayerAnimator",
  "PlayerMarker",
  "PlayerView",
  "PlayerWeapons",
  "PlayerWeaponsEffects",
  "PowerUpItem",
  "Projectile",
  "PyramidSpaceShip",
  "PyramidSpaceShipMarker",
  "Reminder",
  "RobotDriving",
  "RobotFixed",
  "RobotFlying",
  "RollingStone",
  "Santa",
  "Scorpman",
  "ScrollHolder",
  "SeriousBomb",
  "Ship",
  "ShipMarker",
  "Shooter",
  "SoundHolder",
  "SpawnerProjectile",
  "Spinner",
  "StormController",
  "Summoner",
  "SummonerMarker",
  "Switch",
  "TacticsChanger",
  "TacticsHolder",
  "Teleport",
  "Terrain",
  "TextFXHolder",
  "TimeController",
  "TouchField",
  "Trigger",
  "Twister",
  "VoiceHolder",
  "Walker",
  "Watcher",
  "WatchPlayers",
  "Water",
  "WeaponItem",
  "Werebull",
  "Woman",
  "WorldBase",
  "WorldLink",
  "WorldSettingsController",
  // Reserved for external classes
  "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
  "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
  "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
};

CEntity *CExtEntityCreate::penLast = NULL;

void CExtEntityCreate::Write(CNetworkMessage &nm) {
  ubClass = 0xFF;
  CTFileName fnmCheck = fnmClass;

  // If class file matches base classes
  if (fnmCheck.RemovePrefix("Classes\\")) {
    if (fnmCheck.Matches("*.ecl")) {
      fnmCheck = fnmCheck.NoExt();

      // Find its index in the dictionary
      for (UBYTE i = 0; i < 0xFF; i++) {
        // Empty base class
        if (aBaseClasses[i] == "") continue;

        // Compare filename with a base class regardless of case
        if (stricmp(fnmCheck.str_String, aBaseClasses[i]) == 0) {
          ubClass = i;
          break;
        }
      }
    }
  }

  nm << ubClass;

  // Write extra class filename (assume ".ecl" extension)
  if (ubClass == 0xFF) {
    const CTString strClassName = fnmClass.NoExt();

    UBYTE ubLength = strClassName.Length();
    nm << ubLength;

    for (UBYTE i = 0; i < ubLength; i++) {
      INetCompress::PathChar(nm, strClassName[i]);
    }
  }

  INetCompress::Placement(nm, plPos);
};

void CExtEntityCreate::Read(CNetworkMessage &nm) {
  nm >> ubClass;

  // Read extra class filename
  if (ubClass == 0xFF) {
    UBYTE ubLength;
    nm >> ubLength;

    // Allocate enough characters
    char *strAlloc = new char[ubLength + 1];
    strAlloc[ubLength] = '\0';

    // Read each character
    for (UBYTE i = 0; i < ubLength; i++) {
      strAlloc[i] = INetDecompress::PathChar(nm);
    }

    // Assign path to the class and free the allocated buffer
    fnmClass = strAlloc + CTString(".ecl");
    delete[] strAlloc;

  // Get class from the dictionary
  } else {
    fnmClass = "Classes\\" + aBaseClasses[ubClass] + ".ecl";
  }

  INetDecompress::Placement(nm, plPos);
};

void CExtEntityCreate::Process(void) {
  // Reset last entity
  penLast = NULL;

  try {
    penLast = IWorld::GetWorld()->CreateEntity_t(plPos, fnmClass);
    ExtServerReport(TRANS("Created '%s' entity (%u)\n"), penLast->GetClass()->ec_pdecDLLClass->dec_strName, penLast->en_ulID);

  } catch (char *strError) {
    ExtServerReport(TRANS("Cannot create entity: %s\n"), strError);
  }
};

#endif // CLASSICSPATCH_EXT_PACKETS
