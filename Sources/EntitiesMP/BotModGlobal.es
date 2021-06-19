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

// [Cecil] 2019-11-12: Global Bot Mod Entity
2001
%{
#include "StdH.h"

// [Cecil] TEMP: Last processed point in the NavMesh generation
extern INDEX _iLastPoint = 0;
%}

class CBotModGlobal : CEntity {
name      "BotModGlobal";
thumbnail "";

properties:

components:

functions:
  // Constructor
  void CBotModGlobal(void) {
    _iLastPoint = 0;
  };

  void Write_t(CTStream *ostr) {
    // write navmesh
    _pNavmesh->Write(ostr);
    *ostr << _iLastPoint; // [Cecil] TEMP

    CEntity::Write_t(ostr);
  };

  void Read_t(CTStream *istr) {
    // read navmesh
    _pNavmesh->Read(istr);
    _pNavmesh->bnm_pwoWorld = GetWorld();

    *istr >> _iLastPoint; // [Cecil] TEMP

    CEntity::Read_t(istr);
  };

procedures:
  Main() {
    InitAsVoid();
    SetFlags(GetFlags() | ENF_CROSSESLEVELS);

    return;
  };
};