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

#include <queue>
#include <string.h>
#include "SmartMap.hpp"
#include "TileClassification.h"

// visit mode
enum
{
    VF_SHOOT = 1,
    VF_WALK = 2
};

//
// Get tile from data
//
static Tile tileFromData(uint16_t tile, uint16_t actor, GameMode mode)
{
    Tile res = {};

    // Wall
    if(tile && tile <= 89)
        res.flags |= TF_WALL;
    else if(tile >= 90 && tile <= 101)
    {
        res.flags |= TF_DOOR;
        if(tile == 92 || tile == 93)
            res.flags |= TF_LOCK1;
        else if(tile == 94 || tile == 95)
            res.flags |= TF_LOCK2;
        else if(tile == 96 || tile == 97)
            res.flags |= TF_LOCK3;
        else if(tile == 98 || tile == 99)
            res.flags |= TF_LOCK4;
    }
    if(tile == 21)
        res.flags |= TF_EXIT;
    else if(tile == 107)
        res.flags |= TF_SECRETPAD;
    // TODO: handle the "deaf flag" clustering bug

    // Solid decoration
    if(!(res.flags & TF_WALL) && isActorSolidDecoration(actor, mode))
        res.flags |= TF_DECO;

    int score;
    if(isActorEnemy(actor, mode, score))
    {
        res.flags &= ~TF_WALL;  // enemies automatically clear walls
        res.flags |= TF_ENEMY;
        res.score = score;
        if(actor >= 224 && actor <= 227)
            res.flags |= TF_INVULNERABLE;
    }
    if(actor == 124)
    {
        res.flags &= ~TF_WALL;
        res.flags |= TF_CORPSE;
    }
    if(isActorFinale(actor, mode))
        res.flags |= TF_FINALE;
    if(isActorTreasure(actor, score))
    {
        res.flags |= TF_TREASURE;
        res.score = score;
    }
    if(actor == 98)
        res.flags |= TF_PUSHWALL;
    else if(actor == 43)
        res.flags |= TF_KEY1;
    else if(actor == 44)
        res.flags |= TF_KEY2;


    return res;
}

//
// Define a smart map
//
SmartMap::SmartMap(const uint16_t *tilemap, const uint16_t *actormap, int tedlevel, GameMode mode)
{
    // Setup defaults
    memset(mTiles, 0, sizeof(mTiles));
    mFinish = FinishMode::tally;
    mPlayerX = mPlayerY = 0;
    mMaxKills = mMaxItems = mMaxSecret = 0;

    for(int y = 0; y < WOLF3D_MAPSIZE; ++y)
    {
        for(int x = 0; x < WOLF3D_MAPSIZE; ++x)
        {
            int pos = y * WOLF3D_MAPSIZE + x;
            Tile &tile = mTiles[y][x];
            tile = tileFromData(tilemap[pos], actormap[pos], mode);
            if(actormap[pos] >= 19 && actormap[pos] <= 23)
            {
                mPlayerX = x;
                mPlayerY = y;
            }
            if(tile.flags & TF_ENEMY)
                mMaxKills++;
            if(tile.flags & TF_TREASURE)
                mMaxItems++;
            if(tile.flags & TF_PUSHWALL)
                mMaxSecret++;
            // TODO: check for guards walking into walls
        }
    }
}

//
// Collects all points
//
void SmartMap::collectPoints()
{
    static const coord delta[] = {
        { 1, 0 },
        { 0, -1 },
        { -1, 0},
        { 0, 1 }
    };
    unsigned visited[WOLF3D_MAPSIZE][WOLF3D_MAPSIZE] = {};

    std::queue<coord> coords;
    coords.push(coord{ mPlayerX, mPlayerY });
    visited[mPlayerY][mPlayerX] = VF_WALK | VF_SHOOT;

    coord pos;

    UndoState undo;
    undo.access = mAccess;
    undo.kills = mKills;
    undo.items = mItems;
    undo.secret = mSecret;
    undo.score = mScore;
    undo.playerpos.x = mPlayerX;
    undo.playerpos.y = mPlayerY;

    std::vector<coord> locks;

    while(!coords.empty())
    {
        pos = coords.front();
        coords.pop();

        Tile &tile = mTiles[pos.y][pos.x];
        if(tile.flags & TF_ENEMY && !(tile.flags & TF_INVULNERABLE))
        {
            mScore += tile.score;
            mKills++;
            undo.tiles.push_back({ pos, tile });
            tile.score = 0;
            tile.flags &= ~TF_ENEMY;
        }
        if(visited[pos.y][pos.x] & VF_WALK)
        {
            mPlayerX = pos.x;
            mPlayerY = pos.y;
            if(tile.flags & TF_TREASURE)
            {
                mScore += tile.score;
                mItems++;
                undo.tiles.push_back({ pos, tile });
                tile.score = 0;
                tile.flags &= ~TF_TREASURE;
            }
            if(tile.flags & TF_KEY1)
            {
                mAccess |= EF_KEY1;
                undo.tiles.push_back({ pos, tile });
                tile.flags &= ~TF_KEY1;
            }
            if(tile.flags & TF_KEY2)
            {
                mAccess |= EF_KEY2;
                undo.tiles.push_back({ pos, tile });
                tile.flags &= ~TF_KEY2;
            }
            if(tile.flags & TF_KEY3)
            {
                mAccess |= EF_KEY3;
                undo.tiles.push_back({ pos, tile });
                tile.flags &= ~TF_KEY3;
            }
            if(tile.flags & TF_KEY4)
            {
                mAccess |= EF_KEY4;
                undo.tiles.push_back({ pos, tile });
                tile.flags &= ~TF_KEY4;
            }
            if(tile.flags & TF_FINALE)
            {
                mAccess |= EF_FINALE;
                undo.tiles.push_back({ pos, tile });
                tile.flags &= ~TF_FINALE;
            }
        }

        for(int i = 0; i < 4; ++i)
        {
            int newx = pos.x + delta[i].x;
            int newy = pos.y + delta[i].y;
            if(newx >= 0 && newx < WOLF3D_MAPSIZE && newy >= 0 && newy < WOLF3D_MAPSIZE)
            {
                const Tile &tile = mTiles[newy][newx];
                if(!(tile.flags & TF_WALL))
                {
                    if(tile.flags & TF_DECO)
                    {
                        if(!(visited[newy][newx] & VF_SHOOT))
                        {
                            visited[newy][newx] |= VF_SHOOT;
                            coords.push(coord{ newx, newy });
                        }
                    }
                    else if((visited[newy][newx] & visited[pos.y][pos.x]) < visited[pos.y][pos.x])
                    {
                        bool allow = true;
                        if(tile.flags & TF_DOOR)
                        {
                            if((tile.flags & TF_LOCK1 && !(mAccess & EF_KEY1)) ||
                               (tile.flags & TF_LOCK2 && !(mAccess & EF_KEY2)) ||
                               (tile.flags & TF_LOCK3 && !(mAccess & EF_KEY3)) ||
                               (tile.flags & TF_LOCK4 && !(mAccess & EF_KEY4)))
                            {
                                allow = false;
                                if(visited[pos.y][pos.x] & VF_WALK)
                                    locks.push_back(coord{ newx, newy });
                            }
                        }
                        if(allow)
                        {
                            coords.push(coord{ newx, newy });
                            visited[newy][newx] = visited[pos.y][pos.x];
                        }
                    }
                }
                else if(tile.flags & TF_EXIT && delta[i].x)
                {
                    mAccess |= mTiles[pos.y][pos.x].flags & TF_SECRETPAD ? EF_SECRET : EF_NORMAL;
                }
            }
        }
    }
}
