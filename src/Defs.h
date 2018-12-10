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

#ifndef Defs_h
#define Defs_h

enum class ObjType : int
{
    puddle = 23,
    barrelGreen,
    tableChairs,
    lampFloor,
    chandelier,
    hangedMan,
    foodDog,
    pillarRed,

    tree,
    skeletonFlat,
    sink,
    plantPotted,
    urn,
    tableBare,
    lightCeiling,
    kitchenStuffOrSolidGibs,

    armorSuit,
    hangingCage,
    hangingCageSkeleton,
    skeletonRelax,
    key1,
    key2,
    stuff1,
    stuff2,

    foodHuman,
    firstAid,
    clip,
    machineGun,
    gatlingGun,
    cross,
    chalice,
    chest,

    crown,
    extraLife,
    gibs,
    barrel,
    well,
    wellEmpty,
    gibs2,
    flag,

    apogeeCallOrRedLight,

    junk1,
    junk2,
    junk3,
    potsOrGibs,
    stove,
    spears,
    vines,

    clip2OrMarblePillar,

}

#endif /* Defs_h */
