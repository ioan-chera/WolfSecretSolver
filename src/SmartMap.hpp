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
// Access flags
//
enum
{
    AF_NORMAL = 1,
    AF_SECRET = 2,
    AF_FINALE = 4
};

enum
{
    IF_KEY1 = 1,
    IF_KEY2 = 2,
    IF_KEY3 = 4,
    IF_KEY4 = 8,
};

//
// Coordinate
//
struct Position
{
    int x, y;

    Position operator+(Position other) const
    {
        return { x + other.x, y + other.y };
    }
};

struct LocatedTile
{
    Position pos;
    Tile tile;
};

//
// State after pushing a wall
//
struct PushState
{
    Tile tiles[WOLF3D_MAPSIZE][WOLF3D_MAPSIZE]; // current tile setup (after pushing and picking up everything)
    Position playerPos;                            // player position (after pushing and picking up everything)
    int score;          // score (accumulated)
    int kills;          // kills (accumulated)
    int items;          // items (accumulated)
    int secret;         // secret (accumulated, by one per each step)
    unsigned inventory; // inventory of important items (accumulated)
    unsigned access;    // current access (NOT accumulated)

    void collectItems();
    Tile &get(Position pos)
    {
        return tiles[pos.y][pos.x];
    }
};

//
// Analysis-ready map
//
class SmartMap
{
public:
    SmartMap(const uint16_t *tilemap, const uint16_t *actormap, int tedlevel, GameMode mode);
private:
    std::stack<PushState> mStack;
    FinishMode mFinish;

    int mMaxKills;
    int mMaxItems;
    int mMaxSecret;
};

#endif /* SmartMap_hpp */
