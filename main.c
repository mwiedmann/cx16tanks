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
#include "tiles.h"

#define IRQ_HANDLER_STACK_SIZE 8
unsigned char irqHandlerStack[IRQ_HANDLER_STACK_SIZE];

// Global because these are accessed in the irq handler
short scrollY1 = 0, scrollX1 = 0, scrollY2 = 0, scrollX2 = 0;
unsigned char go, irqLineMode;
short tankAX, tankAY, tankBX, tankBY;

#define SCROLL_X_MIN 224
#define SCROLL_X_MAX 256
#define SCROLL_Y_MIN 104
#define SCROLL_Y_MAX 120
#define SCROLL_X_OVERALL_MAX ((MAPBASE_TILE_WIDTH+8) * 16)-640 // +8 for the UI overlay on the right
#define SCROLL_Y_OVERALL_MAX (MAPBASE_TILE_HEIGHT * 16)-240

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

    // return IRQ_HANDLED; 
    return IRQ_NOT_HANDLED;
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
    unsigned char l0Tile, joy, aiDir, tankATurret = 0, ticks = 0;
    short tankAIMoveX, tankAIMoveY;
    Ball balls[2] = {
        {0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0}
    };

    init();
    createTiles();
    loadSpriteGraphics();
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

        aiDir = 0;
        dirToXY(aiDir, &tankAIMoveX, &tankAIMoveY);

        toggle(SPRITE_NUM_TANK_A1, 1);
        toggle(SPRITE_NUM_TANK_A2, 1);
        toggle(SPRITE_NUM_TANK_B1, 1);
        toggle(SPRITE_NUM_TANK_B2, 1);
        
        while(1) {
            go = 0;
            joy = joy_read(0);

            // Shoot ball
            if (!balls[0].active && JOY_BTN_3(joy)) {
                balls[0].active = 1;
                balls[0].ticksRemaining = 180;
                balls[0].x = tankAX;
                balls[0].y = tankAY;
                balls[0].moveX = 3;
                balls[0].moveY = 3;
                balls[0].spriteNum = SPRITE_NUM_BALL_A1;
                toggle(SPRITE_NUM_BALL_A1, 1);
                toggle(SPRITE_NUM_BALL_A2, 1);
            }

            if (balls[0].active) {
                getCollisionTile(balls[0].x+8+balls[0].moveX, balls[0].y+8, &l0Tile);

                if (l0Tile != 0) {
                    balls[0].moveX*= -1;
                } else {
                    balls[0].x+= balls[0].moveX;
                }
                
                // Get the tiles on each layer the guy is currently touching
                getCollisionTile(balls[0].x+8, balls[0].y+8+balls[0].moveY, &l0Tile);

                if (l0Tile != 0) {
                    balls[0].moveY*= -1;
                } else {
                    balls[0].y+= balls[0].moveY;
                }

                move(SPRITE_NUM_BALL_A1, balls[0].x, balls[0].y, scrollX1, scrollY1, 0);
                move(SPRITE_NUM_BALL_A2, balls[0].x, balls[0].y+240, scrollX2, scrollY2, 1);

                balls[0].ticksRemaining--;
                if (balls[0].ticksRemaining == 0) {
                    balls[0].active = 0;
                    toggle(SPRITE_NUM_BALL_A1, 0);
                    toggle(SPRITE_NUM_BALL_A2, 0);
                }
            }

            moveTank(2, JOY_LEFT(joy), JOY_RIGHT(joy), JOY_UP(joy), JOY_DOWN(joy), &tankAX, &tankAY);
            
            if (ticks % 8 == 0 && (JOY_BTN_1(joy) || JOY_BTN_2(joy))) {
                // while(1);
                tankATurret+= JOY_BTN_1(joy) ? 1 : -1;
                if (tankATurret == 255) {
                    tankATurret = 15;
                } else if (tankATurret == 16) {
                    tankATurret = 0;
                }

                tankTurret(tankATurret, SPRITE_NUM_TANK_A1, 0);
                tankTurret(tankATurret, SPRITE_NUM_TANK_A2, 0);
            }
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

            // Waiting for VSYNC
            while(!go);
            ticks++;
        }
    }
}