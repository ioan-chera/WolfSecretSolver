/*
 WolfSecretSolver: offline solver of Wolf3D secret puzzles
 Copyright (C) 2018  Ioan Chera

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include "SmartMap.hpp"

bool isActorSolidDecoration(uint16_t actor, GameMode mode);
bool isActorTreasure(uint16_t actor, int &score);
bool isActorEnemy(uint16_t actor, GameMode mode, int &score);
bool actorDropsKey(uint16_t actor, GameMode mode);
bool isActorFinale(uint16_t actor, GameMode mode);