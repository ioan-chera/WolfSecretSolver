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

// visit mode
enum
{
    VF_SHOOT = 1,
    VF_WALK = 2
};

//
// Determine if actor is a solid decoration
//
static bool isActorSolidDecoration(uint16_t actor, GameMode mode)
{
    switch(actor)
    {
        case 24:
        case 25:
        case 26:
        case 28:
        case 30:

        case 31:
        case 33:
        case 34:
        case 35:
        case 36:

        case 39:
        case 40:
        case 41:
        case 45:

        case 58:
        case 59:
        case 60:
        case 62:

        case 68:
        case 69:
            return true;

        case 38:
        case 67:
        case 71:
        case 73:
            return mode == GameMode::spear;

        case 63:
            return mode == GameMode::wolf3d;

        default:
            return false;
    }
}

//
// Determine if treasure
//
static bool isActorTreasure(uint16_t actor, int &score)
{
    score = 0;
    switch(actor)
    {
        case 52:
            score = 100;
            return true;
        case 53:
            score = 500;
            return true;
        case 54:
            score = 1000;
            return true;
        case 55:
            score = 5000;
            return true;
        case 56:
            return true;
        default:
            return false;
    }
}

//
// True if actor is an enemy.
// TODO: check skill level
//
static bool isActorEnemy(uint16_t actor, GameMode mode, int &score)
{
    score = 0;
    switch (actor)
    {
        case 108:
        case 109:
        case 110:
        case 111:
        case 112:
        case 113:
        case 114:
        case 115:
        case 144:
        case 145:
        case 146:
        case 147:
        case 148:
        case 149:
        case 150:
        case 151:
        case 180:
        case 181:
        case 182:
        case 183:
        case 184:
        case 185:
        case 186:
        case 187:
            score = 100;
            return true;

        case 116:
        case 117:
        case 118:
        case 119:
        case 120:
        case 121:
        case 122:
        case 123:
        case 152:
        case 153:
        case 154:
        case 155:
        case 156:
        case 157:
        case 158:
        case 159:
        case 188:
        case 189:
        case 190:
        case 191:
        case 192:
        case 193:
        case 194:
        case 195:
            score = 400;
            return true;

        case 126:
        case 127:
        case 128:
        case 129:
        case 130:
        case 131:
        case 132:
        case 133:
        case 162:
        case 163:
        case 164:
        case 165:
        case 166:
        case 167:
        case 168:
        case 169:
        case 198:
        case 199:
        case 200:
        case 201:
        case 202:
        case 203:
        case 204:
        case 205:
            score = 500;
            return true;

        case 134:
        case 135:
        case 136:
        case 137:
        case 138:
        case 139:
        case 140:
        case 141:
        case 170:
        case 171:
        case 172:
        case 173:
        case 174:
        case 175:
        case 176:
        case 177:
        case 206:
        case 207:
        case 208:
        case 209:
        case 210:
        case 211:
        case 212:
        case 213:
            score = 200;
            return true;

        case 216:
        case 217:
        case 218:
        case 219:
        case 220:
        case 221:
        case 222:
        case 223:
        case 234:
        case 235:
        case 236:
        case 237:
        case 238:
        case 239:
        case 240:
        case 241:
        case 252:
        case 253:
        case 254:
        case 255:
        case 256:
        case 257:
        case 258:
        case 259:
            score = 700;
            return true;

        case 179:
        case 196:
        case 197:
        case 214:
        case 215:
            if(mode == GameMode::wolf3d)
            {
                score = 5000;
                return true;
            }
            return false;

        case 160:
            if(mode == GameMode::wolf3d)
            {
                score = 2000;
                return true;
            }
            return false;

        case 178:
            if(mode == GameMode::wolf3d)
            {
                score = 10000;
                return true;
            }
            return false;

        case 106:
            if(mode == GameMode::spear)
            {
                score = 200;
                return true;
            }
            return false;

        case 107:
        case 125:
        case 142:
        case 143:
        case 161:
            if(mode == GameMode::spear)
            {
                score = 5000;
                return true;
            }
            return false;

        case 224:   // ghosts
        case 225:
        case 226:
        case 227:
            return true;
        default:
            return false;
    }
}

//
// True if actor is capable of ending the game suddenly
//
static bool isActorFinale(uint16_t actor, GameMode mode)
{
    switch(actor)
    {
        case 99:
            return true;
        case 178:
        case 179:
        case 196:
        case 215:
            return mode == GameMode::wolf3d;
        case 74:    // this is the spear actually
        case 107:
            return mode == GameMode::spear;
        default:
            return false;
    }
}

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
