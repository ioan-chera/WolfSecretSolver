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

#include <bitset>
#include <queue>
#include <string.h>
#include "SmartMap.hpp"
#include "TileClassification.h"

// visit mode
enum class VisitLevel : uint8_t
{
    shoot = 1,  // visit by shooting (against enemies)
    walk = 2    // visit by moving (against everything else)
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
    else if(actor == 43 || actorDropsKey(actor, mode))
        res.flags |= TF_KEY1;
    else if(actor == 44)
        res.flags |= TF_KEY2;


    return res;
}

//
// Collects all items. Necessary to call after setting the start state
//
void PushState::collectItems()
{
    static const Position delta[] =
    {
        { 1, 0 },
        { 0, -1 },
        { -1, 0 },
        { 0, 1 }
    };
    static const unsigned keyTileFlags[4] = { TF_KEY1, TF_KEY2, TF_KEY3, TF_KEY4 };
    static const unsigned keyInventoryFlags[4] = { IF_KEY1, IF_KEY2, IF_KEY3, IF_KEY4 };
    static const unsigned lockTileFlags[4] = { TF_LOCK1, TF_LOCK2, TF_LOCK3, TF_LOCK4 };

    std::queue<Position> tiles;
    std::vector<Position> lockedDoors;
    tiles.push(playerPos);

    VisitLevel visited[WOLF3D_MAPSIZE][WOLF3D_MAPSIZE] = {};
    visited[playerPos.y][playerPos.x] = VisitLevel::walk;
    
    do
    {
        for (auto it = lockedDoors.begin(); it != lockedDoors.end(); ++it)
        {
            const Tile &tile = get(*it);
            bool getout = false;
            for (int i = 0; i < 4; ++i)
            {
                if (tile.flags & lockTileFlags[i] && inventory & keyInventoryFlags[i])
                {
                    printf("Will open locked door %d at %d %d\n", i, it->x, it->y);
                    tiles.push(*it);
                    visited[it->y][it->x] = VisitLevel::walk;
                    lockedDoors.erase(it);
                    getout = true;
                    break;
                }
            }
            if (getout)
                break;
        }
        if (tiles.empty())
            break;
        while (!tiles.empty())
        {
            Position pos = tiles.front();
            tiles.pop();

            Tile &tile = get(pos);
            if (tile.flags & TF_WALL)
                continue;
            VisitLevel &visit = visited[pos.y][pos.x];
            if (tile.flags & TF_DOOR)
            {
                if (visit == VisitLevel::shoot)
                    continue;   // FIXME: can't support shooting past doors even if enemies hear
                bool skip = false;
                for (int i = 0; i < 4; ++i)
                    if (tile.flags & lockTileFlags[i] && !(inventory & keyInventoryFlags[i]))
                    {
                        printf("Found locked door %d at %d %d, will go there later\n", i, pos.x, pos.y);
                        lockedDoors.push_back(pos);
                        skip = true;
                        break;
                    }
                if (skip)
                    continue;
            }
            if (tile.flags & TF_DECO)
                visit = VisitLevel::shoot;
            if (tile.flags & TF_ENEMY && !(tile.flags & TF_INVULNERABLE))
            {
                tile.flags &= ~TF_ENEMY; // kill it
                score += tile.score;
                ++kills;
                printf("Kill nazi at %d %d score %d\n", pos.x, pos.y, tile.score);
                if (visit == VisitLevel::walk)
                    playerPos = pos;
            }
            if (visit == VisitLevel::walk)
            {
                if (tile.flags & TF_TREASURE)
                {
                    tile.flags &= ~TF_TREASURE;
                    score += tile.score;
                    ++items;
                    playerPos = pos;
                    printf("Pick treasure at %d %d score %d\n", pos.x, pos.y, tile.score);
                }

                for (int i = 0; i < 4; ++i)
                {
                    if (tile.flags & keyTileFlags[i])
                    {
                        tile.flags &= ~keyTileFlags[i];
                        inventory |= keyInventoryFlags[i];
                        playerPos = pos;
                        printf("Found key %d at %d %d\n", i, pos.x, pos.y);
                    }
                }
            }
            if (tile.flags & TF_FINALE)
            {
                if (tile.flags & TF_ENEMY || visit == VisitLevel::walk) // boss or victory tile
                {
                    access |= AF_FINALE;
                    printf("Found finale at %d %d\n", pos.x, pos.y);
                }
            }
            for (int i = 0; i < 4; ++i)
            {
                Position neigh = pos + delta[i];
                if (neigh.x < 0 || neigh.x >= WOLF3D_MAPSIZE || neigh.y < 0 || neigh.y >= WOLF3D_MAPSIZE)
                    continue;
                if (visited[neigh.y][neigh.x] >= visit)
                    continue;
                // Check for exit
                if (visit == VisitLevel::walk && get(neigh).flags & TF_EXIT && delta[i].x)
                {
                    if (tile.flags & TF_SECRETPAD)
                    {
                        access |= AF_SECRET;
                        printf("Found secret exit at %d %d\n", neigh.x, neigh.y);
                    }
                    else
                    {
                        access |= AF_NORMAL;
                        printf("Found exit at %d %d\n", neigh.x, neigh.y);
                    }
                }

                tiles.push(neigh);
                visited[neigh.y][neigh.x] = visit;
            }
        }
    } while (!lockedDoors.empty());
}

//
// Define a smart map
//
SmartMap::SmartMap(const uint16_t *tilemap, const uint16_t *actormap, int tedlevel, GameMode mode)
{
    PushState state = {};
    // Setup defaults
    mFinish = FinishMode::tally;
    mMaxKills = mMaxItems = mMaxSecret = 0;

    for(int y = 0; y < WOLF3D_MAPSIZE; ++y)
    {
        for(int x = 0; x < WOLF3D_MAPSIZE; ++x)
        {
            int pos = y * WOLF3D_MAPSIZE + x;
            Tile &tile = state.tiles[y][x];
            tile = tileFromData(tilemap[pos], actormap[pos], mode);
            if(actormap[pos] >= 19 && actormap[pos] <= 23)
                state.playerPos = { x, y };
            if(tile.flags & TF_ENEMY)
                mMaxKills++;
            if(tile.flags & TF_TREASURE)
                mMaxItems++;
            if(tile.flags & TF_PUSHWALL)
                mMaxSecret++;
            // TODO: check for guards walking into walls
        }
    }

    state.collectItems();
    mStack.push(state);
}
