/* Copyright (c) 2018-2021 Dreamy Cecil
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

// [Cecil] 2019-06-04: This file is for functions primarily used by PlayerBot class
#pragma once

#include "EntitiesMP/Player.h"
#include "EntitiesMP/PlayerBot.h"

#include "BotWeapons.h"

// Copied player actions (renamed prefix to avoid conflicts)
#define PLRA_FIRE            (1L<<0)
#define PLRA_RELOAD          (1L<<1)
#define PLRA_USE             (1L<<5)
#define PLRA_3RD_PERSON_VIEW (1L<<7)
#define PLRA_USE_HELD        (1L<<9)
#define PLRA_SNIPER_ZOOMIN   (1L<<10)
#define PLRA_SNIPER_ZOOMOUT  (1L<<11)
#define PLRA_SNIPER_USE      (1L<<12)
#define PLRA_FIREBOMB        (1L<<13)

// Player speed
extern FLOAT plr_fSpeedForward;
extern FLOAT plr_fSpeedBackward;
extern FLOAT plr_fSpeedSide;
extern FLOAT plr_fSpeedUp;

// [Cecil] 2019-05-28: Find nearest NavMesh point
CBotPathPoint *NearestNavMeshPoint(CPlayer *pen, const FLOAT3D &vCheck, CBotPathPoint *pbppExclude);

// Write and read bot properties
void BotWrite(CPlayerBot *pen, CTStream *strm);
void BotRead(CPlayerBot *pen, CTStream *strm);

// [Cecil] 2019-06-05: Check if this entity is important for a path point
BOOL ImportantForNavMesh(CPlayer *penBot, CEntity *penEntity);

// [Cecil] 2021-06-14: Bot logic flags
#define BLF_ENEMYEXISTS  (1 << 0)
#define BLF_SEEENEMY     (1 << 1)
#define BLF_CANSHOOT     (1 << 2)
#define BLF_ITEMEXISTS   (1 << 3)
#define BLF_FOLLOWPLAYER (1 << 4)
#define BLF_FOLLOWING    (1 << 5)
#define BLF_SEEPLAYER    (1 << 6)

// [Cecil] 2021-06-14: Bot logic settings
struct SBotLogic {
  UBYTE ubFlags; // things bot is thinking about
  EntityInfo *peiTarget; // entity info of the looking target
  SBotWeaponConfig *aWeapons; // selected weapon config

  CPlacement3D plBotView; // bot's viewpoint
  ANGLE3D aAim;  // in which direction bot needs to aim

  // Constructor
  SBotLogic(void) : ubFlags(0), peiTarget(NULL), aAim(0.0f, 0.0f, 0.0f) {
    aWeapons = PickWeaponConfig();
    plBotView = CPlacement3D(FLOAT3D(0.0f, 0.0f, 0.0f), ANGLE3D(0.0f, 0.0f, 0.0f));
  };

  // Viewpoint functions
  inline FLOAT3D &ViewPos(void) { return plBotView.pl_PositionVector; };
  inline ANGLE3D &ViewAng(void) { return plBotView.pl_OrientationAngle; };

  // Check flags
  inline BOOL EnemyExists(void)  { return ubFlags & BLF_ENEMYEXISTS; };
  inline BOOL SeeEnemy(void)     { return ubFlags & BLF_SEEENEMY; };
  inline BOOL CanShoot(void)     { return ubFlags & BLF_CANSHOOT; };
  inline BOOL ItemExists(void)   { return ubFlags & BLF_ITEMEXISTS; };
  inline BOOL FollowPlayer(void) { return ubFlags & BLF_FOLLOWPLAYER; };
  inline BOOL Following(void)    { return ubFlags & BLF_FOLLOWING; };
  inline BOOL SeePlayer(void)    { return ubFlags & BLF_SEEPLAYER; };
};

// [Cecil] Cast bot view ray
BOOL CastBotRay(CPlayerBot *pen, CEntity *penTarget, SBotLogic &sbl, BOOL bPhysical);

#define MAX_ITEM_DIST 256.0f

// [Cecil] 2021-06-13: Check if it's an enemy player
BOOL IsEnemyPlayer(CEntity *pen);

// [Cecil] 2021-06-19: Check if it's a monster enemy
BOOL IsEnemyMonster(CEntity *pen);

// [Cecil] 2021-06-17: Search for an item
void BotItemSearch(CPlayerBot *pen, SBotLogic &sbl);

// [Cecil] 2021-06-14: Determine the closest item
CEntity *GetClosestItem(CPlayerBot *pen, FLOAT &fItemDist, SBotLogic &sbl);

// [Cecil] 2018-10-11: Bot enemy searching
CEntity *ClosestEnemy(CPlayerBot *pen, FLOAT &fLastDist, SBotLogic &sbl);
// [Cecil] Closest item entity
CEntity *ClosestItemType(CPlayerBot *pen, const CDLLEntityClass &decClass, FLOAT &fDist, SBotLogic &sbl);
// [Cecil] 2019-05-30: Find closest real player
CEntity *ClosestRealPlayer(CPlayerBot *pen, FLOAT3D vCheckPos, FLOAT &fDist);
