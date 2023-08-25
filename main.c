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

#define SCROLL_X_MIN 224
#define SCROLL_X_MAX 256
#define SCROLL_Y_MIN 104
#define SCROLL_Y_MAX 120
#define SCROLL_X_OVERALL_MAX ((MAPBASE_TILE_WIDTH+8) * 16)-640 // +8 for the UI overlay on the right
#define SCROLL_Y_OVERALL_MAX (MAPBASE_TILE_HEIGHT * 16)-240

#define TANKS_COUNT 2
#define BALLS_COUNT 2

Tank tanks[TANKS_COUNT] = {
    { SPRITE_NUM_TANK_A, 0, 0, 32*10, 32*2, 1, 1, 2, 0 },
    { SPRITE_NUM_TANK_B, 1, 1, 32*10, 32*7, 1, 1, 1, 2 }
};

unsigned char irqHandler() {
    if (VERA.irq_flags & 0b10) {
       if (irqLineMode == 0) {
            VERA.irq_raster = 240;
            VERA.irq_enable = 0b00000011; 
            irqLineMode = 1;

            if (tanks[0].x - scrollX1 < SCROLL_X_MIN) {
                scrollX1 = tanks[0].x - SCROLL_X_MIN;
            } else if (tanks[0].x - scrollX1 > SCROLL_X_MAX) {
                scrollX1 = tanks[0].x - SCROLL_X_MAX;
            }

            if (tanks[0].y - scrollY1 < SCROLL_Y_MIN) {
                scrollY1 = tanks[0].y - SCROLL_Y_MIN;
            } else if (tanks[0].y - scrollY1 > SCROLL_Y_MAX) {
                scrollY1 = tanks[0].y - SCROLL_Y_MAX;
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

            if (tanks[1].x - scrollX2 < SCROLL_X_MIN) {
                scrollX2 = tanks[1].x - SCROLL_X_MIN;
            } else if (tanks[1].x - scrollX2 > SCROLL_X_MAX) {
                scrollX2 = tanks[1].x - SCROLL_X_MAX;
            }

            if (tanks[1].y - scrollY2 < SCROLL_Y_MIN) {
                scrollY2 = tanks[1].y - SCROLL_Y_MIN;
            } else if (tanks[1].y - scrollY2 > SCROLL_Y_MAX) {
                scrollY2 = tanks[1].y - SCROLL_Y_MAX;
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

void turretToXY(unsigned char turret, short *x, short *y) {
    *x = 0;
    *y = 0;

    switch(turret) {
        case 0: *y=-4; break;
        case 1: *y=-3; *x=2; break;
        case 2: *y=-3; *x=3; break;
        case 3: *y=-2; *x=3; break;
        case 4: *x=4; break;
        case 5: *y=2; *x=3; break;
        case 6: *y=3; *x=3; break;
        case 7: *y=3; *x=2; break;
        case 8: *y=4; break;
        case 9: *y=3; *x=-2; break;
        case 10: *y=3; *x=-3; break;
        case 11: *y=2; *x=-3; break;
        case 12: *x=-4; break;
        case 13: *y=-2; *x=-3; break;
        case 14: *y=-3; *x=-3; break;
        case 15: *y=-3; *x=-2; break;
    }
}

void main() {
    unsigned char l0Tile, joy, aiDir, ticks = 0, i, firePressed;
    short ballX, ballY;
    Ball balls[BALLS_COUNT] = {
        {SPRITE_NUM_BALL_A, 0,0,0,0,0,0,0},
        {SPRITE_NUM_BALL_B, 1,0,0,0,0,0,0}
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

        toggle(SPRITE_NUM_TANK_A, 1);
        toggle(SPRITE_NUM_TANK_A+1, 1);
        toggle(SPRITE_NUM_TANK_B, 1);
        toggle(SPRITE_NUM_TANK_B+1, 1);
        
        while(1) {
            go = 0;
            joy = joy_read(0);

            // Shoot ball
            for (i=0; i<TANKS_COUNT; i++) {
                firePressed = i==0 ? JOY_BTN_3(joy) : 1;

                if (!balls[i].active && firePressed) {
                    // Shoot in the direction of the turret
                    turretToXY(tanks[i].turret, &ballX, &ballY);

                    balls[i].active = 1;
                    balls[i].ticksRemaining = 90;

                    // Middle of tank adjusted for ball size
                    balls[i].x = (tanks[i].x+16)-4;
                    balls[i].y = (tanks[i].y+16)-4;

                    balls[i].moveX = ballX;
                    balls[i].moveY = ballY;
                    // balls[i].spriteNum = SPRITE_NUM_BALL_A;

                    toggle(SPRITE_NUM_BALL_A+(i*2), 1);
                    toggle(SPRITE_NUM_BALL_A+(i*2)+1, 1);
                }
            }
            for (i=0; i<BALLS_COUNT; i++) {
                if (balls[i].active) {
                    getCollisionTile(balls[i].x+4+balls[i].moveX, balls[i].y+4, &l0Tile);

                    if (l0Tile != 0) {
                        balls[i].moveX*= -1;
                    } else {
                        balls[i].x+= balls[i].moveX;
                    }
                    
                    getCollisionTile(balls[i].x+4, balls[i].y+4+balls[i].moveY, &l0Tile);

                    if (l0Tile != 0) {
                        balls[i].moveY*= -1;
                    } else {
                        balls[i].y+= balls[i].moveY;
                    }

                    move(balls[i].spriteNum, balls[i].x, balls[i].y, scrollX1, scrollY1, 0);
                    move(balls[i].spriteNum+1, balls[i].x, balls[i].y+240, scrollX2, scrollY2, 1);
                    
                    balls[i].ticksRemaining--;
                    if (balls[i].ticksRemaining == 0) {
                        balls[i].active = 0;
                        toggle(balls[i].spriteNum, 0);
                        toggle(balls[i].spriteNum+1, 0);
                    }
                }
            }

            for (i=0; i<TANKS_COUNT; i++) {
                if (!tanks[i].isAI) {
                    moveTank(tanks[i].speed, JOY_LEFT(joy), JOY_RIGHT(joy), JOY_UP(joy), JOY_DOWN(joy), &tanks[i].x, &tanks[i].y);
                } else {
                    // AI for Tank
                    while (!moveTank(tanks[i].speed, tanks[i].moveX == -1, tanks[i].moveX == 1, tanks[i].moveY == -1, tanks[i].moveY == 1, &tanks[i].x, &tanks[i].y)) {
                        aiDir = rand();
                        aiDir>>=5;
                        dirToXY(aiDir, &tanks[i].moveX, &tanks[i].moveY);
                    }
                }

                // Rotate the turret
                if (ticks % 8 == 0 && (
                        (JOY_BTN_1(joy) || JOY_BTN_2(joy))
                        || tanks[i].isAI
                    )
                ) {
                    tanks[i].turret+= JOY_BTN_1(joy) ? 1 : -1;
                    if (tanks[i].turret == 255) {
                        tanks[i].turret = 15;
                    } else if (tanks[i].turret == 16) {
                        tanks[i].turret = 0;
                    }

                    tankTurret(tanks[i].turret, tanks[i].spriteNum, tanks[i].side);
                    tankTurret(tanks[i].turret, tanks[i].spriteNum+1, tanks[i].side);
                }

                // Tank on top screen
                move(tanks[i].spriteNum, tanks[i].x, tanks[i].y, scrollX1, scrollY1, 0);
                // Tank "shadow" on 2nd screen
                move(tanks[i].spriteNum+1, tanks[i].x, tanks[i].y+240, scrollX2, scrollY2, 1);
            }

            

            // Waiting for VSYNC
            while(!go);
            ticks++;
        }
    }
}