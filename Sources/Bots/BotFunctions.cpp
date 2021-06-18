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
#include "StdH.h"
#include "BotFunctions.h"

// Shortcuts
#define SBS   (pen->m_sbsBot)
#define WORLD (_pNetwork->ga_World)

// [Cecil] 2018-10-12: Bot Beta Weapon Priority
extern SBotWeaponConfig MOD_botDeathmatchWeapons[CT_BOT_WEAPONS] = {
  //    Weapon Type         Min D    Max D     DMG   Accuracy  Strafe  Predict
  { WEAPON_SNIPER,          32.0f,  300.0f,  150.0f,  0.95f,   0.30f,   FALSE}, // 13
  { WEAPON_DOUBLESHOTGUN,    0.0f,   32.0f,  100.0f,  0.60f,   0.50f,   FALSE}, // 5
  { WEAPON_MINIGUN,          0.0f,  300.0f,   30.0f,  0.85f,   0.25f,   FALSE}, // 7
  { WEAPON_SINGLESHOTGUN,    0.0f,   24.0f,   70.0f,  0.70f,   0.25f,   FALSE}, // 4
  { WEAPON_TOMMYGUN,         0.0f,  500.0f,   20.0f,  0.90f,   0.10f,   FALSE}, // 6
  { WEAPON_LASER,            0.0f,  128.0f,   40.0f,  0.60f,   0.20f,    TRUE}, // 12
  { WEAPON_IRONCANNON,      16.0f,  128.0f,  300.0f,  0.40f,   0.20f,    TRUE}, // 14
  { WEAPON_GRENADELAUNCHER, 12.0f,  128.0f,  100.0f,  0.30f,   0.20f,    TRUE}, // 9
  { WEAPON_ROCKETLAUNCHER,  12.0f,   96.0f,   70.0f,  0.20f,   0.20f,    TRUE}, // 8
  { WEAPON_FLAMER,           0.0f,   24.0f,   30.0f,  0.80f,   0.50f,    TRUE}, // 11
  { WEAPON_CHAINSAW,         0.0f,   10.0f,   50.0f,  1.00f,   0.00f,   FALSE}, // 10
  { WEAPON_KNIFE,            0.0f,    8.0f,   50.0f,  1.00f,   0.00f,   FALSE}, // 1
  { WEAPON_DOUBLECOLT,       0.0f,  500.0f,   10.0f,  0.70f,   0.00f,   FALSE}, // 3
  { WEAPON_COLT,             0.0f,  500.0f,   20.0f,  0.85f,   0.00f,   FALSE}, // 2
  { WEAPON_NONE,             0.0f,    0.0f,    0.0f,  0.00f,   0.00f,   FALSE}, // 0
};

extern SBotWeaponConfig MOD_botCooperativeWeapons[CT_BOT_WEAPONS] = {
  //    Weapon Type         Min D    Max D     DMG   Accuracy  Strafe  Predict
  { WEAPON_IRONCANNON,      24.0f,  128.0f,  200.0f,  0.40f,   0.20f,    TRUE}, // 14
  { WEAPON_MINIGUN,          0.0f,  500.0f,  200.0f,  0.85f,   0.25f,   FALSE}, // 7
  { WEAPON_SNIPER,          32.0f,  500.0f,  100.0f,  0.95f,   0.30f,   FALSE}, // 13
  { WEAPON_LASER,            0.0f,  256.0f,  100.0f,  0.60f,   0.20f,    TRUE}, // 12
  { WEAPON_GRENADELAUNCHER, 12.0f,  128.0f,  150.0f,  0.40f,   0.20f,    TRUE}, // 9
  { WEAPON_ROCKETLAUNCHER,  12.0f,   96.0f,  100.0f,  0.20f,   0.20f,    TRUE}, // 8
  { WEAPON_FLAMER,           0.0f,   24.0f,   60.0f,  0.80f,   0.50f,    TRUE}, // 11
  { WEAPON_TOMMYGUN,         0.0f,  500.0f,  100.0f,  0.90f,   0.10f,   FALSE}, // 6
  { WEAPON_DOUBLESHOTGUN,    0.0f,   32.0f,   70.0f,  0.60f,   0.50f,   FALSE}, // 5
  { WEAPON_SINGLESHOTGUN,    0.0f,   24.0f,   40.0f,  0.70f,   0.25f,   FALSE}, // 4
  { WEAPON_CHAINSAW,         0.0f,   10.0f,  100.0f,  1.00f,   0.00f,   FALSE}, // 10
  { WEAPON_KNIFE,            0.0f,    8.0f,   50.0f,  1.00f,   0.00f,   FALSE}, // 1
  { WEAPON_DOUBLECOLT,       0.0f,  500.0f,   10.0f,  0.70f,   0.00f,   FALSE}, // 3
  { WEAPON_COLT,             0.0f,  500.0f,   10.0f,  0.85f,   0.00f,   FALSE}, // 2
  { WEAPON_NONE,             0.0f,    0.0f,    0.0f,  0.00f,   0.00f,   FALSE}, // 0
};

// [Cecil] 2019-05-28: Find nearest NavMesh point
CBotPathPoint *NearestNavMeshPoint(CPlayer *pen, const FLOAT3D &vCheck, CBotPathPoint *pbppExclude) {
  if (_pNavmesh->bnm_cbppPoints.Count() <= 0) {
    return NULL;
  }

  FLOAT fDist = -1.0f;
  CBotPathPoint *pbppNearest = NULL;

  FOREACHINDYNAMICCONTAINER(_pNavmesh->bnm_cbppPoints, CBotPathPoint, itbpp) {
    CBotPathPoint *pbpp = itbpp;

    FLOAT fDiff = ClampDn((pbpp->bpp_vPos - vCheck).Length() - pbpp->bpp_fRange, 0.0f);
    BOOL bNotCurrent = (pen == NULL ? TRUE : !pen->CurrentPoint(pbppExclude));

    if ((fDiff < fDist || fDist < 0.0f) && bNotCurrent) {
      pbppNearest = pbpp;
      fDist = fDiff;
    }
  }

  return pbppNearest;
};

// Write bot properties
void BotWrite(CPlayerBot *pen, CTStream *strm) {
  // write current point
  if (pen->m_pbppCurrent == NULL || !_pNavmesh->bnm_cbppPoints.IsMember(pen->m_pbppCurrent)) {
    *strm << (INDEX)-1;
  } else {
    *strm << pen->m_pbppCurrent->bpp_iIndex;
  }

  // write target point
  if (pen->m_pbppTarget == NULL || !_pNavmesh->bnm_cbppPoints.IsMember(pen->m_pbppTarget)) {
    *strm << (INDEX)-1;
  } else {
    *strm << pen->m_pbppTarget->bpp_iIndex;
  }

  // write point flags
  *strm << pen->m_ulPointFlags;

  // write settings
  *strm << SBS;
};

// Read bot properties
void BotRead(CPlayerBot *pen, CTStream *strm) {
  // add to the list if not already there
  if (!_cenPlayerBots.IsMember(pen)) {
    _cenPlayerBots.Add(pen);
  }

  // read current point
  INDEX iPoint;
  *strm >> iPoint;

  if (iPoint != -1) {
    pen->m_pbppCurrent = _pNavmesh->FindPointByID(iPoint);
  }
  
  // read target point
  *strm >> iPoint;

  if (iPoint != -1) {
    pen->m_pbppTarget = _pNavmesh->FindPointByID(iPoint);
  }

  // read point flags
  *strm >> pen->m_ulPointFlags;

  // read settings
  *strm >> SBS;
};

// [Cecil] 2019-06-05: Check if this entity is important for a path point
BOOL ImportantForNavMesh(CPlayer *penBot, CEntity *penEntity) {
  if (IsDerivedFromDllClass(penEntity, CItem_DLLClass)) {
    return IsItemPickable(penBot, (CItem *)penEntity);
  }

  return FALSE;
};

// [Cecil] Cast bot view ray
BOOL CastBotRay(CPlayerBot *pen, CEntity *penTarget, SBotLogic &sbl, BOOL bPhysical) {
  FLOAT3D vBody = FLOAT3D(0.0f, 0.0f, 0.0f);

  // target's body center
  if (sbl.peiTarget != NULL) {
    FLOAT *v = sbl.peiTarget->vTargetCenter;
    vBody = FLOAT3D(v[0], v[1], v[2]) * penTarget->GetRotationMatrix();
  }

  FLOAT3D vTarget = penTarget->GetPlacement().pl_PositionVector + vBody;
  CCastRay crBot(pen, sbl.ViewPos(), vTarget);

  crBot.cr_ttHitModels = CCastRay::TT_NONE;
  crBot.cr_bHitTranslucentPortals = TRUE;
  crBot.cr_bPhysical = bPhysical;
  CastRayFlags(crBot, pen->GetWorld(), (bPhysical ? BPOF_PASSABLE : 0));

  return (vTarget - crBot.cr_vHit).Length() <= 1.0f
       && crBot.cr_fHitDistance < 1000.0f; // [Cecil] TEMP: Target is too far
};

// [Cecil] 2021-06-17: Search for an item
void BotItemSearch(CPlayerBot *pen, SBotLogic &sbl) {
  // check for nearby items
  FLOAT fItemDist = MAX_ITEM_DIST;

  // need this to determine the distance to the closest one
  CEntity *penItem = ClosestItemType(pen, CItem_DLLClass, fItemDist, sbl);

  if (penItem != NULL) {
    // determine close distance for the item
    FLOATaabbox3D boxItem;
    penItem->GetBoundingBox(boxItem);
    FLOAT3D vItemSize = boxItem.Size();

    FLOAT fCloseItemDist = Max(Abs(vItemSize(1)), Abs(vItemSize(3))) / 2.0f;

    // check if bot wants an item (if it's not too close)
    BOOL bWantItem = (fItemDist > fCloseItemDist);

    // check if item is really needed (because going for an important point)
    BOOL bNeedItem = (!pen->m_bImportantPoint || fItemDist < 8.0f);

    if (bWantItem && bNeedItem) {
      // determine the closest item
      penItem = GetClosestItem(pen, fItemDist, sbl);

      // put searching on cooldown if selected some item
      if (penItem != NULL) {
        pen->m_penLastItem = penItem;
        pen->m_tmLastItemSearch = _pTimer->CurrentTick() + SBS.fItemSearchCD;
      }
    }
  }

  // has some item
  if (pen->m_penLastItem != NULL) {
    // item is pickable
    if (IsItemPickable(pen, (CItem *)&*pen->m_penLastItem)) {
      sbl.ubFlags |= BLF_ITEMEXISTS;
      pen->m_penFollow = pen->m_penLastItem;

    // not pickable anymore
    } else {
      pen->m_penLastItem = NULL;
      pen->m_tmLastItemSearch = 0.0f;
    }
  }
};

// [Cecil] 2021-06-14: Determine the closest item
CEntity *GetClosestItem(CPlayerBot *pen, FLOAT &fItemDist, SBotLogic &sbl) {
  // run towards the weapon
  CEntity *penItem = ClosestItemType(pen, CWeaponItem_DLLClass, fItemDist, sbl);

  // within range
  if (penItem != NULL && fItemDist < pen->m_fTargetDist && fItemDist < SBS.fWeaponDist) {
    return penItem;
  }
  
  // need health
  const FLOAT fBotHealth = pen->GetHealth();
  if (fBotHealth < SBS.fHealthSearch) {
    // run towards health
    penItem = ClosestItemType(pen, CHealthItem_DLLClass, fItemDist, sbl);
    
    // within range
    if (penItem != NULL && fItemDist < SBS.fHealthDist) {
      FLOAT fHealth = ((CItem *)penItem)->m_fValue;
      
      // only pick health if it's essential
      if (fHealth <= 10.0f && fBotHealth < 75.0f) {
        return penItem;

      } else if (fHealth <= 50.0f && fBotHealth < 85.0f) {
        return penItem;

      } else if (fHealth >= 100.0f && fBotHealth < 150.0f) {
        return penItem;
      }
    }
  }

  // run towards power ups
  penItem = ClosestItemType(pen, CPowerUpItem_DLLClass, fItemDist, sbl);
  
  // within range
  if (penItem != NULL && fItemDist < SBS.fWeaponDist) {
    return penItem;
  }

  // need armor
  if (pen->m_fArmor < 100.0f) {
    // run towards armor
    penItem = ClosestItemType(pen, CArmorItem_DLLClass, fItemDist, sbl);
  
    // within range
    if (penItem != NULL && fItemDist < SBS.fArmorDist) {
      FLOAT fArmor = ((CItem *)penItem)->m_fValue;

      // only pick health if it's essential
      if (fArmor <= 10.0f && pen->m_fArmor < 75.0f) {
        return penItem;

      } else if (fArmor <= 50.0f && pen->m_fArmor < 85.0f) {
        return penItem;

      } else if (fArmor >= 100.0f) {
        return penItem;
      }
    }
  }

  // run towards ammo
  penItem = ClosestItemType(pen, CAmmoPack_DLLClass, fItemDist, sbl);

  if (penItem == NULL) {
    // search for ammo if no ammo packs
    penItem = ClosestItemType(pen, CAmmoItem_DLLClass, fItemDist, sbl);
  }

  // within range
  if (penItem != NULL && fItemDist < SBS.fAmmoDist) {
    return penItem;
  }

  return NULL;
};

// [Cecil] 2018-10-11: Bot enemy searching
CEntity *ClosestEnemy(CPlayerBot *pen, FLOAT &fLast, SBotLogic &sbl) {
  CEntity *penReturn = NULL;

  // don't search for enemies
  if (!SBS.bTargetSearch) {
    return NULL;
  }

  // priorities
  FLOAT fLastHP = 1000.0f;
  BOOL bVisible = FALSE;
  fLast = -1.0f;

  // how many priorities have been fulfilled
  INDEX iLastPriority = 0;
  INDEX iPriority = 0;
  CEntity *penLastTarget = NULL;

  // for each entity in the world
  {FOREACHINDYNAMICCONTAINER(WORLD.wo_cenEntities, CEntity, iten) {
    CEntity *penCheck = iten;

    // if enemy (but not cannons - usually hard to reach)
    if (SBS.iTargetType >= 1 && IsDerivedFromDllClass(penCheck, CEnemyBase_DLLClass)
     /*&& !IsOfDllClass(penCheck, CCannonStatic_DLLClass) && !IsOfDllClass(penCheck, CCannonRotating_DLLClass)*/) {
      // if not alive
      CEnemyBase *penEnemy = (CEnemyBase *)penCheck;

      if (penEnemy->m_bTemplate || !(penEnemy->GetFlags() & ENF_ALIVE) || penEnemy->GetHealth() <= 0.0f) {
        continue;
      }

    // if player and it's not a coop or a singleplayer game
    } else if (SBS.iTargetType != 1 && pen->IsEnemyPlayer(penCheck)) {
      // if not alive
      CPlayer *penEnemy = (CPlayer *)penCheck;

      if (penEnemy == pen || !(penEnemy->GetFlags() & ENF_ALIVE) || penEnemy->GetHealth() <= 0.0f) {
        continue;
      }

    } else {
      // skip every other entity
      continue;
    }

    FLOAT3D vEnemy = penCheck->GetPlacement().pl_PositionVector;

    FLOAT fHealth = ((CMovableEntity *)penCheck)->GetHealth();
    FLOAT fDist = DistanceToPos(sbl.ViewPos(), vEnemy);
    BOOL bCurrentVisible = CastBotRay(pen, penCheck, sbl, TRUE); // [Cecil] TEMP: Test for physical polygons
    CEntity *penTargetEnemy = NULL;

    // target's target
    if (IsOfDllClass(penCheck, CPlayerBot_DLLClass)) {
      penTargetEnemy = ((CPlayerBot *)penCheck)->m_penTarget;

    } else if (IsDerivedFromDllClass(penCheck, CEnemyBase_DLLClass)) {
      penTargetEnemy = ((CEnemyBase *)penCheck)->m_penEnemy;
    }

    // priorities
    if (bCurrentVisible)                 iPriority++;
    if (fHealth < fLastHP)               iPriority++;
    if (fLast == -1.0f || fDist < fLast) iPriority++;
    if (penTargetEnemy == pen)           iPriority++;

    // if more priorities have been fulfilled
    if (iPriority >= iLastPriority) {
      // remember the target
      fLastHP = fHealth;
      fLast = fDist;
      bVisible = bCurrentVisible;

      penReturn = penCheck;
      penLastTarget = penTargetEnemy;

      iLastPriority = iPriority;
    }

    iPriority = 0;
  }}

  // target is too far
  if (fLast < 0.0f) {
    fLast = 1000.0f;
  }

  return penReturn;
};

CEntity *ClosestItemType(CPlayerBot *pen, const CDLLEntityClass &decClass, FLOAT &fDist, SBotLogic &sbl) {
  // don't search for items
  if (!SBS.bItemSearch) {
    return NULL;
  }

  // return selected item
  if (pen->m_tmLastItemSearch > _pTimer->CurrentTick()) {
    return NULL;
  }

  // reset last item
  pen->m_penLastItem = NULL;

  CEntity *penReturn = NULL;
  fDist = MAX_ITEM_DIST;

  // for each bot item
  {FOREACHINDYNAMICCONTAINER(WORLD.wo_cenEntities, CEntity, iten) {
    CEntity *penCheck = iten;

    // if not an item or already picked up
    if (!IsDerivedFromDllClass(penCheck, decClass)
     || !IsItemPickable(pen, (CItem *)penCheck)) {
      continue;
    }

    // if not visible
    if (SBS.bItemVisibility && !CastBotRay(pen, penCheck, sbl, TRUE)) { // [Cecil] TEMP: Test for physical polygons
      continue;
    }

    FLOAT3D vItem = penCheck->GetPlacement().pl_PositionVector;
    FLOAT fDistToItem = DistanceToPos(sbl.ViewPos(), vItem);

    if (fDistToItem < fDist) {
      fDist = fDistToItem;
      penReturn = penCheck;
    }
  }}

  return penReturn;
};

// [Cecil] 2019-05-30: Find closest real player
CEntity *ClosestRealPlayer(CPlayerBot *pen, FLOAT3D vCheckPos, FLOAT &fDist) {
  CEntity *penReturn = NULL;
  fDist = -1.0f;

  // for each entity in the world
  for (INDEX i = 0; i < CEntity::GetMaxPlayers(); i++) {
    CPlayer *penReal = (CPlayer *)CEntity::GetPlayerEntity(i);
      
    // skip unexistent and dead players
    if (!ASSERT_ENTITY(penReal) || !(penReal->GetFlags() & ENF_ALIVE)) {
      continue;
    }

    FLOAT3D vPlayer = penReal->GetPlacement().pl_PositionVector;

    if (fDist == -1.0f || DistanceToPos(vCheckPos, vPlayer) < fDist) {
      fDist = DistanceToPos(vCheckPos, vPlayer);
      penReturn = penReal;
    }
  }

  return penReturn;
};
