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

enum
{
    PUSH_DISTANCE = 2,  // TODO: also support distance 3
};

// visit mode
enum class VisitLevel : uint8_t
{
    shoot = 1,  // visit by shooting (against enemies)
    walk = 2    // visit by moving (against everything else)
};

static const Position DIR_DELTA[] =
{
    { 1, 0 },
    { 0, -1 },
    { -1, 0 },
    { 0, 1 }
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
                Position neigh = pos + DIR_DELTA[i];
                if (!neigh.valid())
                    continue;
                if (visit == VisitLevel::walk)
                {
                    if (get(neigh).flags & TF_PUSHWALL)
                    {
                        PushPosition pp = { pos, neigh };
                        if (pushable(pp))
                        {
                            pushPositions.push_back(pp);
                            printf("Found pushable from %d %d to %d %d\n", pp.player.x, pp.player.y, pp.wall.x, pp.wall.y);
                        }
                    }

                    // Check for exit
                    if (get(neigh).flags & TF_EXIT && DIR_DELTA[i].x)
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
                }

                if (visited[neigh.y][neigh.x] >= visit)
                    continue;

                tiles.push(neigh);
                visited[neigh.y][neigh.x] = visit;
            }
        }
    } while (!lockedDoors.empty());
}

//
// True if a given position is pushable
//
bool PushState::pushable(const PushPosition &pp) const
{
    if (!pp.valid() || get(pp.player).flags & (TF_WALL | TF_DECO) || !(get(pp.wall).flags & TF_WALL) || 
        !(get(pp.wall).flags & TF_PUSHWALL) || (pp.wall - pp.player).manhattan() != 1)
    {
        return false;
    }
    Position nextPos = 2 * pp.wall - pp.player;
    if (!nextPos.valid())
        return false;
    return !(get(nextPos).flags & (TF_WALL | TF_DECO | TF_CORPSE | TF_DOOR));
}

//
// True if wall is trivially pushable from just one point
//
bool PushState::isTrivialWall(const PushPosition &pp) const
{
    auto mutated = const_cast<Tile(*)[WOLF3D_MAPSIZE]>(tiles);
    std::vector<Position> restore;

    for (int y = 0; y < WOLF3D_MAPSIZE; ++y)
    {
        for (int x = 0; x < WOLF3D_MAPSIZE; ++x)
        {
            if ((y == pp.wall.y && x == pp.wall.x) || !(mutated[y][x].flags & TF_WALL) || !(mutated[y][x].flags & TF_PUSHWALL))
                continue;
            restore.push_back({ x, y });
            mutated[y][x].flags &= ~TF_WALL;
        }
    }

    auto restoreMutated = [mutated, &restore]()
    {
        for (Position pos : restore)
        {
            mutated[pos.y][pos.x].flags |= TF_WALL;
        }
    };

    // Explore from player's position ignoring all other pushwalls and doors besides this
    std::queue<Position> tiles;
    tiles.push(playerPos);

    std::bitset<WOLF3D_MAPAREA> visited;
    visited.set(playerPos.index());

    PushPosition opp;
    opp.wall = pp.wall;

    while (!tiles.empty())
    {
        Position pos = tiles.front();
        tiles.pop();

        opp.player = pos;
        if (opp != pp && pushable(opp))
        {
            restoreMutated();
            return false;
        }

        for (int i = 0; i < 4; ++i)
        {
            Position neigh = pos + DIR_DELTA[i];
            if (!neigh.valid() || visited[neigh.index()] || get(neigh).flags & (TF_WALL | TF_DECO))
                continue;

            tiles.push(neigh);
            visited.set(neigh.index());
        }
    }

    restoreMutated();
    return true;
}

//
// Pushes a wall inline, without expecting to add a new layer
//
void PushState::pushInline(PushPosition pp)
{
    if (!pushable(pp))
        return;

    printf("Pushing trivial wall %d %d to %d %d\n", pp.player.x, pp.player.y, pp.wall.x, pp.wall.y);
    ++secret;

    Position dest;
    for (int i = 0; i < PUSH_DISTANCE; ++i)
    {
        if (!pushable(pp))
        {
            get(pp.wall).flags &= ~TF_PUSHWALL;
            return;
        }
        Position delta = pp.wall - pp.player;
        dest = pp.wall + delta;
        get(dest).flags |= TF_WALL | TF_PUSHWALL;
        get(pp.wall).flags &= ~(TF_WALL | TF_PUSHWALL);
        pp += delta;
    }
    get(pp.wall).flags &= ~TF_PUSHWALL;
}

//
// Push all walls which are guaranteed not to have other destinations
//
int PushState::pushTrivialWalls()
{
    std::vector<PushPosition> keep;
    keep.reserve(pushPositions.size());
    for (auto it = pushPositions.begin(); it != pushPositions.end(); ++it)
    {
        if (!isTrivialWall(*it))
        {
            keep.push_back(*it);
            continue;
        }
        pushInline(*it);
    }
    int pushed = static_cast<int>(pushPositions.size() - keep.size());
    pushPositions = keep;
    return pushed;
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
            if(actormap[pos] >= 19 && actormap[pos] < 23)
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

    int pushed;
    do
    {
        state.pushPositions.clear();    // push positions must be cleared prior to collecting items
        state.collectItems();
        pushed = state.pushTrivialWalls();
    } while (pushed);

    printf("Kills left: %d\n", mMaxKills - state.kills);
    printf("Items left: %d\n", mMaxItems - state.items);
    printf("Secret left: %d\n", mMaxSecret - state.secret);
    printf("Nontrivial pushwalls accessible: %d\n", (int)state.pushPositions.size());

    mStack.push(state);
}
