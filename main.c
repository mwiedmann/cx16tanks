#include <cx16.h>
#include <cbm.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <joystick.h>
#include <6502.h>

#include "config.h"
#include "utils.h"
#include "wait.h"
#include "sprites.h"

#define IRQ_HANDLER_STACK_SIZE 8
unsigned char irqHandlerStack[IRQ_HANDLER_STACK_SIZE];

short scrollY1 = 0, scrollX1 = 0, scrollY2 = 0, scrollX2 = 0;
unsigned char tileX, tileY, zoomMode = 0, gameMode = 1, go, irqLineMode;
short ballAX, ballAY, tankAX, tankAY;
short ballBX, ballBY, tankBX, tankBY;
short moveX, moveY, tankAIMoveX, tankAIMoveY;

#define SECTION_COUNT 4
#define SECTION_SIZE 10
#define MAZE_SECTIONS 12
#define SCROLL_X_MIN 192
#define SCROLL_X_MAX 320
#define SCROLL_Y_MIN 80
#define SCROLL_Y_MAX 128
#define SCROLL_X_OVERALL_MAX ((MAPBASE_TILE_WIDTH+8) * 16)-640 // +8 for the UI overlay on the right
#define SCROLL_Y_OVERALL_MAX (MAPBASE_TILE_HEIGHT * 16)-240

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


unsigned char irqHandler() {
    if (VERA.irq_flags & 0b10) {
       if (irqLineMode == 0) {
            VERA.irq_raster = 240;
            VERA.irq_enable = 0b00000011; 
            irqLineMode = 1;

            if (tankAX - scrollX1 < SCROLL_X_MIN) {
                scrollX1 = tankAX - SCROLL_X_MIN;
            } else if (tankAX - scrollX1 > SCROLL_X_MAX) {
                scrollX1 = tankAX - SCROLL_X_MAX;
            }

            if (tankAY - scrollY1 < SCROLL_Y_MIN) {
                scrollY1 = tankAY - SCROLL_Y_MIN;
            } else if (tankAY - scrollY1 > SCROLL_Y_MAX) {
                scrollY1 = tankAY - SCROLL_Y_MAX;
            }

            if (scrollX1 < 0) {
                scrollX1 = 0;
            } else if (scrollX1 > SCROLL_X_OVERALL_MAX) {
                scrollX1 = SCROLL_X_OVERALL_MAX;
            }

            if (scrollY1 < 0) {
                scrollY1 = 0;
            } else if (scrollY1 > SCROLL_Y_OVERALL_MAX) {
                scrollY1 = SCROLL_Y_OVERALL_MAX;
            }

            VERA.layer0.hscroll = scrollX1;
            VERA.layer0.vscroll = scrollY1;
        } else {
            VERA.irq_raster = 0;
            VERA.irq_enable = 0b00000011; 
            irqLineMode = 0;

            if (tankBX - scrollX2 < SCROLL_X_MIN) {
                scrollX2 = tankBX - SCROLL_X_MIN;
            } else if (tankBX - scrollX2 > SCROLL_X_MAX) {
                scrollX2 = tankBX - SCROLL_X_MAX;
            }

            if (tankBY - scrollY2 < SCROLL_Y_MIN) {
                scrollY2 = tankBY - SCROLL_Y_MIN;
            } else if (tankBY - scrollY2 > SCROLL_Y_MAX) {
                scrollY2 = tankBY - SCROLL_Y_MAX;
            }

            if (scrollX2 < 0) {
                scrollX2 = 0;
            } else if (scrollX2 > SCROLL_X_OVERALL_MAX) {
                scrollX2 = SCROLL_X_OVERALL_MAX;
            }

            if (scrollY2 < 0) {
                scrollY2 = 0;
            } else if (scrollY2 > SCROLL_Y_OVERALL_MAX) {
                scrollY2 = SCROLL_Y_OVERALL_MAX;
            }

            VERA.layer0.hscroll = scrollX2;
            VERA.layer0.vscroll = scrollY2-240;
        }
        
        VERA.irq_flags = 0b10;

        return IRQ_HANDLED; 
    } else {
        go = 1;
        return IRQ_HANDLED;
    }

    return IRQ_HANDLED; 
    // return IRQ_NOT_HANDLED;
}

void test() {
    clearLayers();
    drawMaze();
    scrollX1 = 0;
    scrollY1 = 0;

    // Setup the IRQ handler for sprite collisions
    set_irq(&irqHandler, irqHandlerStack, IRQ_HANDLER_STACK_SIZE);

    VERA.irq_raster = 0;
    VERA.irq_enable |= 0b00000010; // 0b00000100;

    while(1) {
        scrollX1++;
        // scrollY=0;

        wait();
        VERA.layer0.hscroll = scrollX1;
        
    }
}

unsigned char moveTank(unsigned char speed, unsigned char moveLeft, unsigned char moveRight, unsigned char moveUp, unsigned char moveDown, short *x, short *y) {
    unsigned char l0Tile;
    unsigned char moved = 0;

    if (moveLeft) {
        getCollisionTile((*x)-speed, (*y), &l0Tile);
        if (l0Tile == 0) {
            getCollisionTile((*x)-speed, (*y)+16, &l0Tile);
            if (l0Tile == 0) {
                getCollisionTile((*x)-speed, (*y)+31, &l0Tile);
                if (l0Tile == 0) {
                    (*x)-= speed;
                    moved=1;
                }
            }
        }
    } else if (moveRight) {
        getCollisionTile((*x)+31+speed, (*y), &l0Tile);
        if (l0Tile == 0) {
            getCollisionTile((*x)+31+speed, (*y)+16, &l0Tile);
            if (l0Tile == 0) {
                getCollisionTile((*x)+31+speed, (*y)+31, &l0Tile);
                if (l0Tile == 0) {
                    (*x)+= speed;
                    moved=1;
                }
            }
        }
    }

    if (moveUp) {
        getCollisionTile((*x), (*y)-speed, &l0Tile);
        if (l0Tile == 0) {
            getCollisionTile((*x)+16, (*y)-speed, &l0Tile);
            if (l0Tile == 0) {
                getCollisionTile((*x)+31, (*y)-speed, &l0Tile);
                if (l0Tile == 0) {
                    (*y)-= speed;
                    moved=1;
                }
            }
        }
    } else if (moveDown) {
        getCollisionTile((*x), (*y)+31+speed, &l0Tile);
        if (l0Tile == 0) {
            getCollisionTile((*x)+16, (*y)+31+speed, &l0Tile);
            if (l0Tile == 0) {
                getCollisionTile((*x)+31, (*y)+31+speed, &l0Tile);
                if (l0Tile == 0) {
                    (*y)+= speed;
                    moved=1;
                }
            }
        }
    }

    return moved;
}

void dirToXY(unsigned char dir, short *x, short *y) {
    switch (dir) {
        case 0 : *x=0; *y=-1; return;
        case 1 : *x=1; *y=-1; return;
        case 2 : *x=1; *y=0; return;
        case 3 : *x=1; *y=1; return;
        case 4 : *x=0; *y=1; return;
        case 5 : *x=-1; *y=1; return;
        case 6 : *x=-1; *y=0; return;
        case 7 : *x=-1; *y=-1; return;
    }
}

void main() {
    unsigned char l0Tile, joy, aiDir;
    unsigned short ballTicks, ballActive;

    init();
    createTiles();
    createSpriteGraphics();
    spritesConfig();

    // test();

    // Setup the IRQ handler for sprite collisions
    set_irq(&irqHandler, irqHandlerStack, IRQ_HANDLER_STACK_SIZE);

    irqLineMode = 0;
    VERA.irq_raster = 0;
    VERA.irq_enable = 0b00000011;

    while(1) {
        // Set the zoom level
        clearLayers();
        drawMaze();

        tankAX = 32*10;
        tankAY = 32*2;
        tankBX = 32*10;
        tankBY = 32*7;
        ballAX = tankAX;
        ballAY = tankAY;
        moveX = 3;
        moveY = 3;

        aiDir = 0;
        dirToXY(aiDir, &tankAIMoveX, &tankAIMoveY);

        ballTicks = 0;
        ballActive = 0;

        toggle(SPRITE_NUM_TANK_A1, 1);
        toggle(SPRITE_NUM_TANK_A2, 1);
        toggle(SPRITE_NUM_TANK_B1, 1);
        toggle(SPRITE_NUM_TANK_B2, 1);
        
        while(1) {
            go = 0;
            joy = joy_read(0);

            // Shoot ball
            if (JOY_BTN_1(joy)) {
                ballActive = 1;
                ballTicks = 0;
                ballAX = tankAX;
                ballAY = tankAY;
                toggle(SPRITE_NUM_BALL_A1, 1);
            }

            if (ballActive) {
                // Get the tiles on each layer the guy is currently touching
                getCollisionTile(ballAX+8+moveX, ballAY+8, &l0Tile);

                if (l0Tile != 0) {
                    moveX*= -1;
                } else {
                    ballAX+= moveX;
                }
                
                // Get the tiles on each layer the guy is currently touching
                getCollisionTile(ballAX+8, ballAY+8+moveY, &l0Tile);

                if (l0Tile != 0) {
                    moveY*= -1;
                } else {
                    ballAY+= moveY;
                }

                move(SPRITE_NUM_BALL_A1, ballAX, ballAY+240, scrollX2, scrollY2, 0);

                ballTicks++;
                if (ballTicks > 180) {
                    ballActive = 0;
                    toggle(SPRITE_NUM_BALL_A1, 0);
                }
            }

            moveTank(2, JOY_LEFT(joy), JOY_RIGHT(joy), JOY_UP(joy), JOY_DOWN(joy), &tankAX, &tankAY);
            
            // Manual move for Tank B
            // moveTank(1, JOY_LEFT(joy), JOY_RIGHT(joy), JOY_UP(joy), JOY_DOWN(joy), &tankBX, &tankBY);

            // AI For Tank B
            while (!moveTank(1, tankAIMoveX == -1, tankAIMoveX == 1, tankAIMoveY == -1, tankAIMoveX == 1, &tankBX, &tankBY)) {
                aiDir = rand();
                aiDir>>=5;
                dirToXY(aiDir, &tankAIMoveX, &tankAIMoveY);
            }

            // Tank A on top screen
            move(SPRITE_NUM_TANK_A1, tankAX, tankAY, scrollX1, scrollY1, 0);
            // Tank A "shadow" on 2nd screen
            move(SPRITE_NUM_TANK_A2, tankAX, tankAY+240, scrollX2, scrollY2, 1);

            // Tank B "shadow" on top screen
            move(SPRITE_NUM_TANK_B1, tankBX, tankBY, scrollX1, scrollY1, 0);
            // Tank A "shadow" on 2nd screen
            move(SPRITE_NUM_TANK_B2, tankBX, tankBY+240, scrollX2, scrollY2, 1);

            while(!go);
            
            //wait();
            
            // setScroll();
        }
    }
}