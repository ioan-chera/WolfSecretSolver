#include "TileClassification.h"

//
// Determine if actor is a solid decoration
//
bool isActorSolidDecoration(uint16_t actor, GameMode mode)
{
   switch (actor)
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
bool isActorTreasure(uint16_t actor, int &score)
{
   score = 0;
   switch (actor)
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
bool isActorEnemy(uint16_t actor, GameMode mode, int &score)
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
      if (mode == GameMode::wolf3d)
      {
         score = 5000;
         return true;
      }
      return false;

   case 160:
      if (mode == GameMode::wolf3d)
      {
         score = 2000;
         return true;
      }
      return false;

   case 178:
      if (mode == GameMode::wolf3d)
      {
         score = 10000;
         return true;
      }
      return false;

   case 106:
      if (mode == GameMode::spear)
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
      if (mode == GameMode::spear)
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
bool isActorFinale(uint16_t actor, GameMode mode)
{
   switch (actor)
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
