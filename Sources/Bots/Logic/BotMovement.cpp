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

// [Cecil] 2021-06-14: This file is for functions primarily used by PlayerBot class
#include "StdH.h"

#include "BotFunctions.h"
#include "BotMovement.h"

// Shortcuts
#define SETTINGS (pb.props.m_sbsBot)
#define WEAPON   (pb.GetWeapons())

// [Cecil] 2021-06-25: Too long since the last position change
BOOL NoPosChange(CPlayerBotController &pb) {
  if (_pTimer->CurrentTick() - pb.props.m_tmPosChange > 1.0f) {
    pb.props.m_tmPosChange = _pTimer->CurrentTick();
    return TRUE;
  }
  return FALSE;
};

// [Cecil] 2021-06-14: Try to find some path
void BotPathFinding(CPlayerBotController &pb, SBotLogic &sbl) {
  if (_pNavmesh->bnm_cbppPoints.Count() <= 0) {
    return;
  }

  const FLOAT3D &vBotPos = pb.pen->GetPlacement().pl_PositionVector;

  CEntity *penTarget = NULL;
  BOOL bSeeTarget = FALSE;

  // Go to the player
  if (sbl.Following()) {
    // If can't see them
    if (!sbl.SeePlayer()) {
      penTarget = pb.props.m_penFollow;
      bSeeTarget = TRUE;
    }

  // Go to the item
  } else if (sbl.ItemExists()) {
    penTarget = pb.props.m_penFollow;
    bSeeTarget = TRUE;

  // Go to the enemy
  } else {
    penTarget = pb.props.m_penTarget;
    bSeeTarget = sbl.SeeEnemy();
  }

  if (!sbl.Following()) {
    // Select important points sometimes
    if (!pb.props.m_bImportantPoint && pb.props.m_tmPickImportant <= _pTimer->CurrentTick()) {
      // Compare chance
      if (SETTINGS.fImportantChance > 0.0f && pb.pen->FRnd() <= SETTINGS.fImportantChance) {
        CBotPathPoint *pbppImportant = _pNavmesh->FindImportantPoint(pb, -1);

        if (pbppImportant != NULL) {
          pb.props.m_pbppTarget = pbppImportant;
          pb.props.m_bImportantPoint = TRUE;

          pb.props.Thought("New important point: ^caf3f3f%d", pbppImportant->bpp_iIndex);
        }
      }

      pb.props.m_tmPickImportant = _pTimer->CurrentTick() + 5.0f;
    }
  }

  BOOL bReachedImportantPoint = FALSE;

  // Only change the important point if reached it
  if (pb.props.m_bImportantPoint) {
    if (pb.props.m_pbppTarget != NULL) {
      // Position difference
      FLOAT3D vPointDiff = (pb.props.m_pbppTarget->bpp_vPos - vBotPos);
      
      // Close to the point
      bReachedImportantPoint = (vPointDiff.Length() < pb.props.m_pbppTarget->bpp_fRange);

    // Lost target point
    } else {
      pb.props.m_bImportantPoint = FALSE;
    }

    // If reached the important point
    if (bReachedImportantPoint) {
      CEntity *penImportant = pb.props.m_pbppTarget->bpp_penImportant;

      // Reset important point
      if (pb.props.m_pbppTarget->bpp_pbppNext == NULL) {
        pb.props.m_bImportantPoint = FALSE;
        pb.props.Thought("^caf3f3fReached important point");

        // Reset the path if no target
        if (penTarget == NULL) {
          pb.props.m_pbppCurrent = NULL;
          pb.props.m_pbppTarget = NULL;
          pb.props.m_ulPointFlags = 0;
        }

      // Proceed to the next important point
      } else {
        pb.props.m_pbppTarget = pb.props.m_pbppTarget->bpp_pbppNext;
        pb.props.Thought("^c3f3fafNext important point");
      }

      // Use important entity
      UseImportantEntity(pb, penImportant);
    }
  }

  // Need to find path to the target or the important point
  BOOL bReasonForNewPoint = pb.props.m_pbppCurrent == NULL && (penTarget != NULL || pb.props.m_bImportantPoint);

  // Able to select new target point
  BOOL bChangeTargetPoint = (bReasonForNewPoint || pb.props.m_tmChangePath <= _pTimer->CurrentTick() || NoPosChange(pb));
  CBotPathPoint *pbppReached = NULL;

  // If timer is up and there's a point
  if (!bChangeTargetPoint && pb.props.m_pbppCurrent != NULL) {
    // Position difference
    FLOAT3D vPointDiff = (pb.props.m_pbppCurrent->bpp_vPos - vBotPos);

    // Close to the point
    FLOAT &fRange = pb.props.m_pbppCurrent->bpp_fRange;
    bChangeTargetPoint = (vPointDiff.Length() < fRange);

    // A bit higher up
    vPointDiff = (pb.props.m_pbppCurrent->bpp_vPos - vBotPos) - pb.pen->en_vGravityDir * (fRange * 0.5f);
    bChangeTargetPoint |= (vPointDiff.Length() < fRange);

    // Reached this point
    if (bChangeTargetPoint) {
      pbppReached = pb.props.m_pbppCurrent;
    }
  }

  if (bChangeTargetPoint) { 
    // Find first point to go to
    CBotPathPoint *pbppClosest = NearestNavMeshPointBot(pb, FALSE);

    // Can see the enemy or don't have any point yet
    BOOL bSelectTarget = (bSeeTarget || pb.props.m_pbppCurrent == NULL);

    // If not following the important point, select new one if possible
    if (penTarget != NULL && !pb.props.m_bImportantPoint && bSelectTarget) {
      pb.props.m_pbppTarget = NearestNavMeshPointPos(penTarget, penTarget->GetPlacement().pl_PositionVector);
    }

    // [Cecil] 2022-05-11: Construct a path as long as there's a target point
    if (pb.props.m_pbppTarget != NULL) {
      CTString strThought;

      // [Cecil] 2021-06-21: Just go to the first point if haven't reached it yet
      if (pbppReached != pbppClosest) {
        pb.props.m_pbppCurrent = pbppClosest;
        pb.props.m_ulPointFlags = pbppClosest->bpp_ulFlags;
      
        FLOAT3D vToPoint = (pbppClosest->bpp_vPos - vBotPos).SafeNormalize();
        ANGLE3D aToPoint; DirectionVectorToAngles(vToPoint, aToPoint);
        strThought.PrintF("Closest point ^c00ff00%d ^c00af00[%.1f, %.1f]",
                          pbppClosest->bpp_iIndex, aToPoint(1), aToPoint(2));

      // Pick the next point on the path
      } else {
        CBotPathPoint *pbppNext = _pNavmesh->FindNextPoint(pbppClosest, pb.props.m_pbppTarget);

        // Remember the point if found
        if (pbppNext != NULL) {
          // [Cecil] 2021-09-09: Point is locked
          BOOL bStay = pbppNext->IsLocked();

          // Stay on point if it's locked
          if (bStay) {
            sbl.ulFlags |= BLF_STAYONPOINT;
          }

          // [Cecil] 2021-06-16: Target point is unreachable, stay on it if it's not important
          if (!pb.props.m_bImportantPoint) {
            bStay |= (pbppNext->bpp_ulFlags & PPF_UNREACHABLE);
          }

          // Select new path point
          pb.props.m_pbppCurrent = (bStay ? pbppClosest : pbppNext);

          // Get flags of the closest point or override them
          pb.props.m_ulPointFlags = (pbppNext->bpp_ulFlags & PPF_OVERRIDE) ? pbppNext->bpp_ulFlags : pbppClosest->bpp_ulFlags;

          FLOAT3D vToPoint = (pbppNext->bpp_vPos - vBotPos).SafeNormalize();
          ANGLE3D aToPoint; DirectionVectorToAngles(vToPoint, aToPoint);
          strThought.PrintF("%s ^c00ff00%d ^c00af00[%.1f, %.1f]", (bStay ? "Staying at" : "Next point"),
                            pbppNext->bpp_iIndex, aToPoint(1), aToPoint(2));

        // No next point
        } else {
          pb.props.m_pbppCurrent = NULL;
          pb.props.m_ulPointFlags = 0;
        }
      }

      if (pb.props.m_pbppCurrent != NULL) {
        pb.props.Thought(strThought);
      }

    // No target point
    } else {
      pb.props.m_pbppCurrent = NULL;
      pb.props.m_ulPointFlags = 0;
    }

    pb.props.m_tmChangePath = _pTimer->CurrentTick() + 5.0f;
  }
};

// [Cecil] 2021-06-15: Set bot aim
void BotAim(CPlayerBotController &pb, CPlayerAction &pa, SBotLogic &sbl) {
  // [Cecil] 2021-06-16: Aim in the walking direction if haven't seen the enemy in a while
  if (_pTimer->CurrentTick() - pb.props.m_tmLastSawTarget > 2.0f)
  {
    // Running speed
    FLOAT3D vRunDir = HorizontalDiff(pb.pen->en_vCurrentTranslationAbsolute, pb.pen->en_vGravityDir);
    
    // Relative position
    CPlacement3D plToDir(pb.pen->en_vCurrentTranslationAbsolute, sbl.ViewAng());

    // Angle towards the target (negate pitch)
    FLOAT2D vToTarget = FLOAT2D(GetRelH(plToDir), -sbl.ViewAng()(2));
    FLOAT fPitch = GetRelP(plToDir);

    // [Cecil] TODO: Try to find 'm_fFallTime' from the property list
    CPlayer *penPlayer = (CPlayer *)pb.pen;

    // Look down after some time
    if (penPlayer->m_fFallTime > 1.0f && fPitch < 0.0f) {
      vToTarget(2) = fPitch;
    }

    // Set rotation speed
    if (vRunDir.Length() >= 2.0f) {
      sbl.aAim(1) = Clamp(vToTarget(1) * 0.3f, -30.0f, 30.0f) / _pTimer->TickQuantum;
    }
    sbl.aAim(2) = Clamp(vToTarget(2) * 0.3f, -30.0f, 30.0f) / _pTimer->TickQuantum;

    return;
  }

  // No enemy
  if (!sbl.EnemyExists()) {
    return;
  }

  EntityInfo *peiTarget = (EntityInfo *)pb.props.m_penTarget->GetEntityInfo();
  FLOAT3D vEnemy;

  // Get target center position
  if (peiTarget != NULL) {
    GetEntityInfoPosition(pb.props.m_penTarget, peiTarget->vTargetCenter, vEnemy);

  // Just enemy position if no entity info
  } else {
    vEnemy = pb.props.m_penTarget->GetPlacement().pl_PositionVector + FLOAT3D(0.0f, 1.0f, 0.0f) * pb.props.m_penTarget->GetRotationMatrix();
  }

  // Current weapon
  const SBotWeaponConfig &bw = sbl.aWeapons[pb.props.m_iBotWeapon];

  // Next position prediction
  vEnemy += ((CMovableEntity*)&*pb.props.m_penTarget)->en_vCurrentTranslationAbsolute
          * (SETTINGS.fPrediction + pb.pen->FRnd() * SETTINGS.fPredictRnd) // Default: *= 0.2f
          * (1.0f - bw.bw_fAccuracy); // [Cecil] 2021-06-20: Prediction based on accuracy

  // Look a bit higher if it's a player
  if (IS_PLAYER(pb.props.m_penTarget)) {
    vEnemy += FLOAT3D(0, 0.25f, 0) * pb.props.m_penTarget->GetRotationMatrix();
  }

  // Relative position
  CPlacement3D plToTarget = sbl.plBotView;
  plToTarget.pl_PositionVector = vEnemy - plToTarget.pl_PositionVector;

  // Angle towards the target
  FLOAT2D vToTarget = FLOAT2D(GetRelH(plToTarget), GetRelP(plToTarget));

  // Update accuracy angles
  if (SETTINGS.fAccuracyAngle > 0.0f) {
    FLOAT tmNow = _pTimer->CurrentTick();
        
    // Randomize every half a second
    if (pb.props.m_tmBotAccuracy <= tmNow) {
      // More accurate with the sniper
      FLOAT fAccuracyMul = (UsingScope(pb) ? 0.2f : 1.0f) * SETTINGS.fAccuracyAngle;

      // Invisible targets are hard to aim at
      if (pb.props.m_penTarget->GetFlags() & ENF_INVISIBLE) {
        fAccuracyMul *= 3.0f;
      }

      pb.props.m_vAccuracy = FLOAT3D(pb.pen->FRnd() - 0.5f, pb.pen->FRnd() - 0.5f, 0.0f) * fAccuracyMul;
      pb.props.m_tmBotAccuracy = tmNow + 0.5f;
    }

    vToTarget(1) += pb.props.m_vAccuracy(1);
    vToTarget(2) += pb.props.m_vAccuracy(2);
  }
      
  // Limit to one tick, otherwise aim will go too far and miss
  const FLOAT fDistRotSpeed = ClampDn(SETTINGS.fRotSpeedDist, 0.05f);      // 400
  const FLOAT fMinRotSpeed = ClampDn(SETTINGS.fRotSpeedMin, 0.05f);        // 0.05
  const FLOAT fMaxRotSpeed = ClampDn(SETTINGS.fRotSpeedMax, fMinRotSpeed); // 0.2
  const FLOAT fSpeedLimit = SETTINGS.fRotSpeedLimit;                       // 30

  // Clamp the speed
  FLOAT fRotationSpeed = Clamp(pb.props.m_fTargetDist / fDistRotSpeed, fMinRotSpeed, fMaxRotSpeed);

  // Max speed
  if (fSpeedLimit >= 0.0f) {
    vToTarget(1) = Clamp(vToTarget(1), -fSpeedLimit, fSpeedLimit);
    vToTarget(2) = Clamp(vToTarget(2), -fSpeedLimit, fSpeedLimit);
  }

  // Set rotation speed
  sbl.aAim(1) = vToTarget(1) / fRotationSpeed;
  sbl.aAim(2) = vToTarget(2) / fRotationSpeed;

  // Try to shoot
  if (Abs(vToTarget(1)) < SETTINGS.fShootAngle && Abs(vToTarget(2)) < SETTINGS.fShootAngle) {
    // Shoot if the enemy is visible or the crosshair is on them
    BOOL bTargetingEnemy = WEAPON->m_penRayHit == pb.props.m_penTarget;

    if (sbl.SeeEnemy() || bTargetingEnemy) {
      sbl.ulFlags |= BLF_CANSHOOT;
    }
  }
};

// [Cecil] 2021-06-14: Set bot movement
void BotMovement(CPlayerBotController &pb, CPlayerAction &pa, SBotLogic &sbl) {
  // No need to set any moving speed if nowhere to go
  if (!sbl.EnemyExists() && !sbl.ItemExists()
   && pb.props.m_penFollow == NULL && pb.props.m_pbppCurrent == NULL) {
    return;
  }

  const FLOAT3D &vBotPos = pb.pen->GetPlacement().pl_PositionVector;

  // Randomize strafe direction every once in a while
  if (pb.props.m_tmChangeBotDir <= _pTimer->CurrentTick()) {
    pb.props.m_fSideDir = (pb.pen->IRnd() % 2 == 0) ? -1.0f : 1.0f;
    pb.props.m_tmChangeBotDir = _pTimer->CurrentTick() + (pb.pen->FRnd() * 2.0f) + 2.0f; // 2 to 4 seconds
  }

  FLOAT3D vBotMovement = FLOAT3D(0.0f, 0.0f, 0.0f); // In which direction bot needs to go
  FLOAT fVerticalMove = 0.0f; // Jumping or crouching

  const SBotWeaponConfig &bwWeapon = sbl.aWeapons[pb.props.m_iBotWeapon]; // Current weapon config

  // Strafe further if lower health
  FLOAT fHealthRatio = Clamp(100.0f - pb.pen->GetHealth(), 0.0f, 100.0f) / 100.0f;
  const FLOAT fStrafeRange = (bwWeapon.bw_fMaxDistance - bwWeapon.bw_fMinDistance) * bwWeapon.bw_fStrafe;

  // Avoid the target in front of the bot
  FLOAT fStrafe = Clamp(fStrafeRange * fHealthRatio, 3.0f, 16.0f);
  
  // [Cecil] TODO: Try to find 'm_pstState' from the property list
  CPlayer *penPlayer = (CPlayer *)pb.pen;

  BOOL bInLiquid = (penPlayer->m_pstState == PST_SWIM || penPlayer->m_pstState == PST_DIVE);

  if (SETTINGS.bStrafe && pb.props.m_fTargetDist < (bwWeapon.bw_fMinDistance + fStrafe)
   && (pb.props.m_penFollow == NULL || pb.props.m_penFollow == pb.props.m_penTarget || sbl.Following())) {
    // Run around the enemy
    vBotMovement = FLOAT3D(pb.props.m_fSideDir, 0.0f, 0.0f);

  } else {
    CPlacement3D plDelta = pb.pen->GetPlacement();
    BOOL bShouldFollow = FALSE;
    BOOL bShouldJump = FALSE;
    BOOL bShouldCrouch = FALSE;
    
    // Run towards the following target
    if (pb.props.m_penFollow != NULL) {
      // Back off if needed
      if (sbl.BackOff()) {
        plDelta.pl_PositionVector = (vBotPos - pb.props.m_penFollow->GetPlacement().pl_PositionVector);
      } else {
        plDelta.pl_PositionVector = (pb.props.m_penFollow->GetPlacement().pl_PositionVector - vBotPos);
      }

      // Check vertical difference
      FLOAT3D vVertical = VerticalDiff(plDelta.pl_PositionVector, pb.pen->en_vGravityDir);

      // Jump if it's higher
      bShouldJump = (vVertical.Length() > 1.0f);

      // Jump if going for an item and not the player
      if (sbl.ItemExists()) {
        bShouldJump &= !sbl.Following();
      }

      bShouldFollow = TRUE;
    }
    
    // Run towards the point
    if (pb.props.m_pbppCurrent != NULL) {
      // Ignore the point if there's an item (if it exists and current point is target point)
      if (!sbl.ItemExists() || pb.props.m_pbppCurrent != pb.props.m_pbppTarget) {
        plDelta.pl_PositionVector = (pb.props.m_pbppCurrent->bpp_vPos - vBotPos);
        bShouldJump = pb.props.m_ulPointFlags & PPF_JUMP;
        bShouldCrouch = pb.props.m_ulPointFlags & PPF_CROUCH;

        bShouldFollow = TRUE;
      }
    }

    // Set the speed if there's a place to go
    if (bShouldFollow && plDelta.pl_PositionVector.Length() > 0.0f) {
      FLOAT aDeltaHeading = GetRelH(plDelta);

      // Compare directional difference
      FLOAT3D vHor = HorizontalDiff(plDelta.pl_PositionVector, pb.pen->en_vGravityDir);
      FLOAT3D vVer = VerticalDiff(plDelta.pl_PositionVector, pb.pen->en_vGravityDir);

      // Stay in place if it's too high up
      if (vHor.Length() < 1.0f && vVer.Length() > 4.0f) {
        sbl.ulFlags |= BLF_STAYONPOINT;
      }

      // Move towards the point if in liquid
      if (bInLiquid) {
        vBotMovement = plDelta.pl_PositionVector.Normalize();

      } else {
        // Stop moving if needed
        if (sbl.StayOnPoint()) {
          vBotMovement = FLOAT3D(0.0f, 0.0f, 0.0f);

        } else {
          FLOAT3D vRunDir;
          AnglesToDirectionVector(ANGLE3D(aDeltaHeading, 0.0f, 0.0f), vRunDir);
          vBotMovement = vRunDir;

          // Crouch if needed instead of jumping
          if (bShouldCrouch) {
            fVerticalMove = -1.0f;

          // Jump if allowed
          } else if (bShouldJump && SETTINGS.bJump) {
            fVerticalMove = 1.0f;

          } else {
            fVerticalMove = 0.0f;
          }
        }
      }
    }
  }

  // Try to avoid obstacles
  BOOL bNowhereToGo = (pb.props.m_pbppCurrent == NULL || NoPosChange(pb));

  if (bNowhereToGo && WEAPON->m_fRayHitDistance < 4.0f
   && !(WEAPON->m_penRayHit->GetPhysicsFlags() & EPF_MOVABLE)) {
    // Only jump if following players
    if (sbl.SeePlayer()) {
      fVerticalMove = 1.0f;

    } else {
      vBotMovement = FLOAT3D(pb.props.m_fSideDir, 0.0f, 1.0f);
    }
  }

  // Apply vertical movement if not in liquid
  if (!bInLiquid) {
    // Vertical movement (holding crouch or spamming jump)
    if (fVerticalMove < -0.1f || (fVerticalMove > 0.1f && pb.ButtonAction())) {
      vBotMovement(2) = fVerticalMove;
    } else {
      vBotMovement(2) = 0.0f;
    }
  }

  // Move around
  const FLOAT3D vMoveSpeed = FLOAT3D(plr_fSpeedSide, plr_fSpeedUp, (vBotMovement(3) > 0.0f) ? plr_fSpeedForward : plr_fSpeedBackward);

  // Use direction instead of pressing buttons (it's like bot is using a controller)
  pa.pa_vTranslation(1) += vBotMovement(1) * vMoveSpeed(1);
  pa.pa_vTranslation(2) += vBotMovement(2) * vMoveSpeed(2);
  pa.pa_vTranslation(3) += vBotMovement(3) * vMoveSpeed(3);

  // Walk if needed
  if (pb.props.m_ulPointFlags & PPF_WALK) {
    pa.pa_vTranslation(1) /= 2.0f;
    pa.pa_vTranslation(3) /= 2.0f;
  }
};
