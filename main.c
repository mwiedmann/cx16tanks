#include <cx16.h>
#include <cbm.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <joystick.h>

#include "config.h"
#include "utils.h"
#include "wait.h"
#include "sprites.h"

short scrollY = 0, previousScroll = 0, scrollX = 0;
unsigned char tileX, tileY, zoomMode = 0, gameMode = 1;
short ballX, ballY, tankX, tankY;
short moveX, moveY;

#define SECTION_COUNT 4
#define SECTION_SIZE 10
#define MAZE_SECTIONS 12
#define SCROLL_X_MIN 220
#define SCROLL_X_MAX 420
#define SCROLL_Y_MIN 140
#define SCROLL_Y_MAX 340
#define SCROLL_X_OVERALL_MAX (MAPBASE_TILE_WIDTH * 16)-640
#define SCROLL_Y_OVERALL_MAX (MAPBASE_TILE_HEIGHT * 16)-480

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

// unsigned char starter[] = {
// 1, 1, 1, 0, 1, 1, 1,
// 1, 1, 1, 1, 1, 1, 1,
// 1, 1, 1, 1, 1, 1, 1,
// 0, 1, 1, 1, 1, 1, 0,
// 1, 1, 1, 1, 1, 1, 1,
// 1, 1, 1, 1, 1, 1, 1,
// 1, 1, 1, 0, 1, 1, 1,
// };

// unsigned char t1[] = {
// 1, 1, 1, 0, 1, 1, 1,
// 1, 0, 0, 0, 1, 1, 1,
// 1, 0, 1, 0, 1, 1, 1,
// 0, 0, 1, 0, 0, 0, 0,
// 1, 1, 1, 1, 1, 0, 1,
// 1, 1, 1, 0, 0, 0, 1,
// 1, 1, 1, 0, 1, 1, 1,
// };

// unsigned char t2[] = {
// 1, 1, 1, 0, 1, 1, 1,
// 1, 1, 1, 0, 0, 0, 1,
// 1, 1, 1, 0, 1, 0, 1,
// 0, 0, 0, 0, 1, 0, 0,
// 1, 0, 1, 1, 1, 1, 1,
// 1, 0, 0, 0, 1, 1, 1,
// 1, 1, 1, 0, 1, 1, 1,
// };

// unsigned char t3[] = {
// 1, 1, 1, 0, 1, 1, 1,
// 1, 1, 1, 0, 0, 0, 1,
// 1, 1, 1, 1, 1, 0, 1,
// 0, 0, 0, 0, 1, 0, 0,
// 1, 1, 1, 0, 1, 0, 1,
// 1, 1, 1, 0, 0, 0, 1,
// 1, 1, 1, 0, 1, 1, 1,
// };

// unsigned char t4[] = {
// 1, 1, 1, 0, 1, 1, 1,
// 1, 0, 0, 0, 0, 1, 1,
// 1, 0, 1, 1, 0, 1, 1,
// 0, 0, 0, 1, 0, 0, 0,
// 1, 1, 0, 1, 1, 0, 1,
// 1, 1, 0, 0, 0, 0, 1,
// 1, 1, 1, 0, 1, 1, 1,
// };

// unsigned char t5[] = {
// 1, 1, 1, 0, 1, 1, 1,
// 1, 0, 0, 0, 1, 1, 1,
// 1, 0, 1, 0, 0, 0, 1,
// 0, 0, 1, 1, 1, 0, 0,
// 1, 0, 1, 1, 1, 0, 1,
// 1, 0, 0, 0, 0, 0, 1,
// 1, 1, 1, 0, 1, 1, 1,
// };

unsigned char* sections[] = {
    t1, t2, t3, t4
};

void getCollisionTile(unsigned short x, unsigned short y, unsigned char *l0Tile) {
    unsigned long tileAddr;

    tileX = x>>4;
    tileY = y>>4;

    // Get tile on L0 guy is "touching"
    tileAddr = L0_MAPBASE_ADDR + (((tileY * MAPBASE_TILE_WIDTH) + tileX) * 2);
    VERA.address = tileAddr;
    VERA.address_hi = tileAddr>>16;
    *l0Tile = VERA.data0;
}

void setScroll() {
    VERA.layer0.vscroll = scrollY;
    VERA.layer1.vscroll = scrollY;
    VERA.layer0.hscroll = scrollX;
    VERA.layer1.hscroll = scrollX;
}

void createTiles() {
    unsigned short i;
    
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

    // Ball tile
    for (i=0; i<256; i++) {
        if (i % 16 == 0 || i % 16 == 1 || i < 16 || i > 256-16) {
            VERA.data0 = 0;
        } else {
            VERA.data0 = 4;
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
            
            // sectionNum++;
            // if (sectionNum == SECTION_COUNT) {
            //     sectionNum = 0;
            // }
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
    
    // VERA.address = L0_MAPBASE_ADDR;
    // VERA.address_hi = L0_MAPBASE_ADDR>>16;
    // // Always set the Increment Mode, turn on bit 4
    // VERA.address_hi |= 0b10000;

    // for (i=0; i<MAPBASE_TILE_COUNT; i++) {
    //     if (i % 128 == 0 || i % 128 == 39 || i < 40 || i > (128 * 29)) {
    //         VERA.data0 = 1;
    //     } else if (i > (128*4) && i < (128 * 26) && i % 4 == 0) {
    //         VERA.data0 = 1;
    //     } else {
    //         VERA.data0 = 0;
    //     }

    //     VERA.data0 = 0; 
    // }
}

void main() {
    unsigned char l0Tile, joy;
    unsigned short ballTicks, ballActive;

    init();
    createTiles();
    spritesConfig();

    while(1) {
        // Set the zoom level
        clearLayers();
        drawMaze();

        ballX = 16*5;
        ballY = 16*5;
        tankX = 16*20;
        tankY = 16*4;
        moveX = 3;
        moveY = 3;

        ballTicks = 0;
        ballActive = 0;

        // Reset scrolling
        scrollY = tankY - 240;
        scrollX = tankX - 320;
        setScroll();

        toggle(SPRITE_NUM_TANK, 1);

        // while(1);
        // Main game loop
        while(1) {
            joy = joy_read(0);

            // Shoot ball
            if (JOY_BTN_1(joy)) {
                ballActive = 1;
                ballTicks = 0;
                ballX = tankX;
                ballY = tankY;
                toggle(SPRITE_NUM_BALL, 1);
            }
            
            if (ballActive) {
                // Get the tiles on each layer the guy is currently touching
                getCollisionTile(ballX+8+moveX, ballY+8, &l0Tile);

                if (l0Tile != 0) {
                    moveX*= -1;
                } else {
                    ballX+= moveX;
                }
                
                // Get the tiles on each layer the guy is currently touching
                getCollisionTile(ballX+8, ballY+8+moveY, &l0Tile);

                if (l0Tile != 0) {
                    moveY*= -1;
                } else {
                    ballY+= moveY;
                }

                move(SPRITE_NUM_BALL, ballX, ballY, scrollX, scrollY);

                ballTicks++;
                if (ballTicks > 180) {
                    ballActive = 0;
                    toggle(SPRITE_NUM_BALL, 0);
                }
            }

            if (JOY_LEFT(joy)) {
                getCollisionTile(tankX-2, tankY, &l0Tile);
                if (l0Tile == 0) {
                    getCollisionTile(tankX-2, tankY+16, &l0Tile);
                    if (l0Tile == 0) {
                        getCollisionTile(tankX-2, tankY+31, &l0Tile);
                        if (l0Tile == 0) {
                            tankX-= 2;
                        }
                    }
                    
                }
            } else if (JOY_RIGHT(joy)) {
                getCollisionTile(tankX+31+2, tankY, &l0Tile);
                if (l0Tile == 0) {
                    getCollisionTile(tankX+31+2, tankY+16, &l0Tile);
                    if (l0Tile == 0) {
                        getCollisionTile(tankX+31+2, tankY+31, &l0Tile);
                        if (l0Tile == 0) {
                            tankX+= 2;
                        }
                    }
                    
                }
            }

            if (JOY_UP(joy)) {
                getCollisionTile(tankX, tankY-2, &l0Tile);
                if (l0Tile == 0) {
                    getCollisionTile(tankX+16, tankY-2, &l0Tile);
                    if (l0Tile == 0) {
                        getCollisionTile(tankX+31, tankY-2, &l0Tile);
                        if (l0Tile == 0) {
                            tankY-= 2;
                        }
                    }
                }
            } else if (JOY_DOWN(joy)) {
                getCollisionTile(tankX, tankY+31+2, &l0Tile);
                if (l0Tile == 0) {
                    getCollisionTile(tankX+16, tankY+31+2, &l0Tile);
                    if (l0Tile == 0) {
                        getCollisionTile(tankX+31, tankY+31+2, &l0Tile);
                        if (l0Tile == 0) {
                            tankY+= 2;
                        }
                    }
                }
            }

            move(SPRITE_NUM_TANK, tankX, tankY, scrollX, scrollY);

            if (tankX - scrollX < SCROLL_X_MIN) {
                scrollX = tankX - SCROLL_X_MIN;
            } else if (tankX - scrollX > SCROLL_X_MAX) {
                scrollX = tankX - SCROLL_X_MAX;
            }

            if (tankY - scrollY < SCROLL_Y_MIN) {
                scrollY = tankY - SCROLL_Y_MIN;
            } else if (tankY - scrollY > SCROLL_Y_MAX) {
                scrollY = tankY - SCROLL_Y_MAX;
            }

            if (scrollX < 0) {
                scrollX = 0;
            } else if (scrollX > SCROLL_X_OVERALL_MAX) {
                scrollX = SCROLL_X_OVERALL_MAX;
            }

            if (scrollY < 0) {
                scrollY = 0;
            } else if (scrollY > SCROLL_Y_OVERALL_MAX) {
                scrollY = SCROLL_Y_OVERALL_MAX;
            }

            setScroll();
            wait();
        }

        // Reset scrolling
        scrollY = 0;
        scrollX = 0;
        setScroll();
        clearLayers();
    }
}