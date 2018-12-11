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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "../modules/libwolf/libwolf/libwolf.hpp"
#include "SmartMap.hpp"

//
// Entry point
//
int main(int argc, const char * argv[])
{
    if(argc <= 4)
    {
        puts("Usage: WolfSecretSolver <maphead path> <gamemaps path> <tedlevel> <wolf3d|spear>");
        return EXIT_FAILURE;
    }
    const char *mapheadpath = argv[1];
    const char *gamemapspath = argv[2];
    int tedlevel = atoi(argv[3]);
    GameMode mode = tolower(argv[4][0]) == 's' ? GameMode::spear : GameMode::wolf3d;

    printf("Using %s mode\n", mode == GameMode::spear ? "Spear of Destiny" : "Wolfenstein 3-D");

    wolf3d::LevelSet set;
    wolf3d_LoadFileResult result = set.openFile(mapheadpath, gamemapspath);
    if(result != wolf3d_LoadFileOk)
    {
        fprintf(stderr, "Failed loading %s and %s\n", mapheadpath, gamemapspath);
        return EXIT_FAILURE;
    }
    result = set.loadMap(tedlevel);
    if (result != wolf3d_LoadFileOk)
    {
        fprintf(stderr, "Failed loading level %d\n", tedlevel);
        return EXIT_FAILURE;
    }

    const uint16_t *tiles = set.getMap(tedlevel, 0);
    const uint16_t *actors = set.getMap(tedlevel, 1);
    // assume non-NULL

    SmartMap map(tiles, actors, tedlevel, mode);
    

    return 0;
}
