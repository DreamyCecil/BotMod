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

#pragma once

// [Cecil] 2021-06-11: Player bot
#include "EntitiesMP/Player.h"
#include "EntitiesMP/PlayerBot.h"

// [Cecil] 2021-06-18: Bot mod command prefixes
#define MODCOM_PREFIX "MOD_"
#define MODCOM_NAME(_Command) MODCOM_PREFIX _Command

#define BOTCOM_PREFIX "BOT_"
#define BOTCOM_NAME(_Command) BOTCOM_PREFIX _Command

// [Cecil] 2019-05-28: NavMesh commands
extern INDEX MOD_iRenderNavMesh;
extern INDEX MOD_iNavMeshPoint;
extern INDEX MOD_iNavMeshConnecting;

// [Cecil] 2021-06-11: List of bot entities
DECL_DLL extern CDynamicContainer<CPlayerBot> _cenPlayerBots;

// [Cecil] 2019-06-01: Initialize the bot mod
DECL_DLL void CECIL_InitBotMod(void);

// [Cecil] 2021-06-13: End the bot mod
DECL_DLL void CECIL_EndBotMod(void);

// [Cecil] 2021-06-13: Bot game start
DECL_DLL void CECIL_BotGameStart(CSessionProperties &sp);

// [Cecil] 2021-06-12: Bot game cleanup
DECL_DLL void CECIL_BotGameCleanup(void);

// [Cecil] Render extras on top of the world
void CECIL_WorldOverlayRender(CPlayer *penOwner, CEntity *penViewer, CAnyProjection3D &apr, CDrawPort *pdp);

// Receive and perform a sandbox action
void CECIL_SandboxAction(CPlayer *pen, const INDEX &iAction, const BOOL &bAdmin, CNetworkMessage &nmMessage);
