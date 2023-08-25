#include <cx16.h>
#include <stdlib.h>

#include "config.h"
#include "utils.h"

#define SECTION_COUNT 4
#define SECTION_SIZE 10
#define MAZE_SECTIONS 12

unsigned char starter[] = {
1, 1, 1, 1, 0, 0, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
0, 1, 1, 1, 1, 1, 1, 1, 1, 0,
0, 1, 1, 1, 1, 1, 1, 1, 1, 0,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 0, 0, 1, 1, 1, 1,
};

unsigned char t1[] = {
1, 1, 1, 1, 0, 0, 1, 1, 1, 1,
1, 1, 1, 1, 0, 0, 0, 0, 0, 1,
1, 1, 1, 1, 0, 0, 0, 0, 0, 1,
1, 1, 1, 1, 0, 0, 1, 0, 0, 1,
0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
1, 1, 0, 0, 1, 1, 1, 1, 1, 1,
1, 1, 0, 0, 0, 0, 1, 1, 1, 1,
1, 1, 0, 0, 0, 0, 1, 1, 1, 1,
1, 1, 1, 1, 0, 0, 1, 1, 1, 1,
};

unsigned char t2[] = {
1, 1, 1, 1, 0, 0, 1, 1, 1, 1,
1, 1, 1, 1, 0, 0, 0, 0, 1, 1,
1, 1, 1, 1, 0, 0, 0, 0, 1, 1,
1, 1, 1, 1, 1, 1, 0, 0, 1, 1,
0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
1, 0, 0, 1, 0, 0, 1, 1, 1, 1,
1, 0, 0, 0, 0, 0, 1, 1, 1, 1,
1, 0, 0, 0, 0, 0, 1, 1, 1, 1,
1, 1, 1, 1, 0, 0, 1, 1, 1, 1,
};

unsigned char t3[] = {
1, 1, 1, 1, 0, 0, 1, 1, 1, 1,
1, 1, 1, 1, 0, 0, 0, 0, 0, 1,
1, 1, 1, 1, 0, 0, 0, 0, 0, 1,
1, 1, 1, 1, 0, 0, 1, 0, 0, 1,
0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
1, 1, 1, 1, 1, 1, 1, 0, 0, 1,
1, 1, 1, 1, 0, 0, 0, 0, 0, 1,
1, 1, 1, 1, 0, 0, 0, 0, 0, 1,
1, 1, 1, 1, 0, 0, 1, 1, 1, 1,
};

unsigned char t4[] = {
1, 1, 1, 1, 0, 0, 1, 1, 1, 1,
1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
1, 0, 0, 1, 1, 1, 1, 0, 0, 1,
0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
1, 1, 1, 1, 0, 0, 1, 1, 1, 1,
1, 1, 1, 1, 0, 0, 1, 1, 1, 1,
1, 1, 1, 1, 0, 0, 1, 1, 1, 1,
1, 1, 1, 1, 0, 0, 1, 1, 1, 1,
};

unsigned char* sections[] = {
    t1, t2, t3, t4
};

void getCollisionTile(unsigned short x, unsigned short y, unsigned char *l0Tile) {
    unsigned long tileAddr;
    unsigned char tileX, tileY;

    tileX = x>>4;
    tileY = y>>4;

    // Get tile on L0 guy is "touching"
    tileAddr = L0_MAPBASE_ADDR + (((tileY * MAPBASE_TILE_WIDTH) + tileX) * 2);
    VERA.address = tileAddr;
    VERA.address_hi = tileAddr>>16;
    *l0Tile = VERA.data0;
}

void createTiles() {
    unsigned short i,x,y;
    
    loadFileToVRAM("font.bin", TILEBASE_FONT_START);
    
    // Clear layer 0
    VERA.address = TILEBASE_ADDR;
    VERA.address_hi = TILEBASE_ADDR>>16;
    // Always set the Increment Mode, turn on bit 4
    VERA.address_hi |= 0b10000;

    // Empty tile
    for (i=0; i<256; i++) {
        VERA.data0 = 0;
    }

    // Solid tile
    for (i=0; i<256; i++) {
        VERA.data0 = 15;
    }

    // Solid tile2
    for (i=0; i<256; i++) {
        VERA.data0 = 8;
    }

    // Divider tile
    for (y=0; y<16; y++) {
        for (x=0; x<16; x++) {
            VERA.data0 = y>7 ? 8 : 0;
        }
    }

    // Ball tile 1
    for (i=0; i<256; i++) {
        if (i % 16 == 0 || i % 16 == 1 || i < 16 || i > 256-16) {
            VERA.data0 = 0;
        } else {
            VERA.data0 = 4;
        }
    }

    // Ball tile 2
    for (i=0; i<256; i++) {
        if (i % 16 == 0 || i % 16 == 1 || i < 16 || i > 256-16) {
            VERA.data0 = 0;
        } else {
            VERA.data0 = 7;
        }
    }
}

unsigned long mapBaseFromTileXY(unsigned short x, unsigned short y) {
    return L0_MAPBASE_ADDR + (y * MAPBASE_TILE_WIDTH * 2) + (x * 2);
}

void drawMaze() {
    // Note we need a `short` here because there are more than 255 tiles
    unsigned char sectionX, sectionY, sectionNum, lastSectionNum, x, y, xCount, yCount;
    unsigned long addr;

    for (sectionY=0; sectionY<MAZE_SECTIONS; sectionY++) {
        for (sectionX=0; sectionX<MAZE_SECTIONS; sectionX++) {
            lastSectionNum = sectionNum;
            do {
                sectionNum = rand();
                sectionNum>>=6;
            } while(sectionNum == lastSectionNum || sectionNum>=SECTION_COUNT);
            
            for (y = sectionY * SECTION_SIZE, yCount=0; yCount < SECTION_SIZE; y++, yCount++){
                for (x = sectionX * SECTION_SIZE, xCount=0; xCount < SECTION_SIZE; x++, xCount++) {
                    addr = mapBaseFromTileXY(x,y);

                    VERA.address = addr;
                    VERA.address_hi = addr>>16;
                    // Always set the Increment Mode, turn on bit 4
                    VERA.address_hi |= 0b10000;

                    VERA.data0 = sections[sectionNum][(yCount*SECTION_SIZE)+xCount];
                    VERA.data0 = 0;
                }
            }
        }
    }

    for (y=0; y<MAPBASE_TILE_HEIGHT; y++) {
        for (x=0; x<MAPBASE_TILE_WIDTH; x++) {
            if (y == 0 || y == (MAPBASE_TILE_HEIGHT - 1) || x == 0 || x == (MAPBASE_TILE_WIDTH - 1)) {
                addr = mapBaseFromTileXY(x,y);

                VERA.address = addr;
                VERA.address_hi = addr>>16;
                // Always set the Increment Mode, turn on bit 4
                VERA.address_hi |= 0b10000;

                VERA.data0 = 1;
                VERA.data0 = 0;
            }
        }
    }
}
