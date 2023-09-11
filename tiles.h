#ifndef TILES_H
#define TILES_H

#include "sprites.h"

void getCollisionTile(unsigned short x, unsigned short y, unsigned char *l0Tile);
void createTiles();
void drawMaze(Tank *tanks);

#endif