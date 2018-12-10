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

#ifndef SmartMap_hpp
#define SmartMap_hpp

#include <stack>
#include <vector>
#include "../modules/libwolf/libwolf/libwolf.h"

//
// Flags
//
enum
{
    TF_DECO = 1,            // solid decoration, won't block attacks
    TF_WALL = 2,            // full wall
    TF_ENEMY = 4,           // enemy who counts kills
    TF_TREASURE = 8,        // treasure which counts
    TF_PUSHWALL = 0x10,     // pushwall (very important)
    TF_KEY1 = 0x20,         // keys (bosses will also provide them)
    TF_KEY2 = 0x40,
    TF_KEY3 = 0x80,
    TF_KEY4 = 0x100,
    TF_LOCK1 = 0x200,       // lock openable by given key
    TF_LOCK2 = 0x400,
    TF_LOCK3 = 0x800,
    TF_LOCK4 = 0x1000,
    TF_EXIT = 0x2000,       // regular exit
    TF_SECRETPAD = 0x4000,  // secret exit (more valuable)
    TF_DOOR = 0x8000,       // door (solid for pushing, not for playing)
    TF_CORPSE = 0x10000,    // corpse (like door but more permissive)
    TF_FINALE = 0x20000,    // quits the game completely (also bosses)
    TF_INVULNERABLE = 0x40000   // invulnerable enemy
};

//
// Game mode is relevant here
//
enum class GameMode
{
    wolf3d,
    spear
};

//
// How a map is ended
//
enum class FinishMode
{
    tally,  // use regular intermission tally
    bonus   // always give 15000 bonus
};

//
// Game tile
//
struct Tile
{
    unsigned flags; // various binary flags
    int score;      // score value
};

//
// Exit flags
//
enum
{
    EF_NORMAL = 1,
    EF_SECRET = 2,
    EF_FINALE = 4,
    EF_KEY1 = 8,
    EF_KEY2 = 0x10,
    EF_KEY3 = 0x20,
    EF_KEY4 = 0x40
};

//
// Coordinate
//
struct coord
{
    int x, y;
};

struct LocatedTile
{
    coord pos;
    Tile tile;
};

//
// State difference when doing something
//
struct UndoState
{
    int score;
    int kills;
    int items;
    int secret;
    unsigned access;
    coord playerpos;
    std::vector<LocatedTile> tiles;
};

//
// Analysis-ready map
//
class SmartMap
{
public:
    SmartMap(const uint16_t *tilemap, const uint16_t *actormap, int tedlevel, GameMode mode);
private:
    void collectPoints();

    std::stack<UndoState> mUndos;

    Tile mTiles[WOLF3D_MAPSIZE][WOLF3D_MAPSIZE];
    FinishMode mFinish;
    int mPlayerX;
    int mPlayerY;

    // Static data
    int mMaxKills;
    int mMaxItems;
    int mMaxSecret;

    // Current status
    int mScore = 0;
    int mKills = 0;
    int mItems = 0;
    int mSecret = 0;
    unsigned mAccess = 0;
};

#endif /* SmartMap_hpp */
