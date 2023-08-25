#include <cx16.h>
#include <joystick.h>

#include "sprites.h"
#include "config.h"
#include "tiles.h"
#include "utils.h"

#define BARREL_COLOR 3
#define TANK_A_COLOR 9
#define TANK_B_COLOR 10

void loadSpriteGraphics() {
    loadFileToVRAM("tanka.bin", TANK_A_SPRITE_GRAPHICS_ADDR);
    loadFileToVRAM("tankb.bin", TANK_B_SPRITE_GRAPHICS_ADDR);
}

void spritesConfig() {
    unsigned char i;

    // VRAM address for sprite 1 graphics...reusing a tile for now
    unsigned long spriteGraphicAddress = TILEBASE_ADDR + (4 * 256);

    // Create 4 ball sprites
    for (i=0; i<4; i++) {
        VERA.address = SPRITE1_ADDR+(SPRITE_NUM_BALL_A1*8)+(i*8);
        VERA.address_hi = SPRITE1_ADDR+(SPRITE_NUM_BALL_A1*8)+(i*8)>>16;
        // Set the Increment Mode, turn on bit 4
        VERA.address_hi |= 0b10000;

        // Graphic address bits 12:5
        VERA.data0 = spriteGraphicAddress>>5;
        // 256 color mode, and graphic address bits 16:13
        VERA.data0 = 0b10000000 | spriteGraphicAddress>>13;
        VERA.data0 = 100;
        VERA.data0 = 100;
        VERA.data0 = 100;
        VERA.data0 = 100;
        VERA.data0 = 0; // 0b00001000; // Z-Depth=2 (or 0 to hide)
        VERA.data0 = 0b00000000; // 8x8 pixel image
    }
   
    // Create 4 tank sprites
    for (i=0; i<4; i++) {
        // Point to Sprite 2
        VERA.address = SPRITE1_ADDR+(SPRITE_NUM_TANK_A*8)+(i*8);
        VERA.address_hi = SPRITE1_ADDR+(SPRITE_NUM_TANK_A*8)+(i*8)>>16;
        // Set the Increment Mode, turn on bit 4
        VERA.address_hi |= 0b10000;

        // Configure Sprite 2 (Tank)
        spriteGraphicAddress = i<2 ? TANK_A_SPRITE_GRAPHICS_ADDR : TANK_B_SPRITE_GRAPHICS_ADDR;

        // Graphic address bits 12:5
        VERA.data0 = spriteGraphicAddress>>5;
        // 256 color mode, and graphic address bits 16:13
        VERA.data0 = 0b10000000 | spriteGraphicAddress>>13;
        VERA.data0 = 100;
        VERA.data0 = 100;
        VERA.data0 = 100;
        VERA.data0 = 100;
        VERA.data0 = 0; // 0b00001000; // Z-Depth=2 (or 0 to hide)
        VERA.data0 = 0b10100000; // 32x32 pixel image
    }
}

void toggle(unsigned char spriteNum, unsigned char show) {
    // Update Sprite 1 X/Y Position
    // Point to Sprite 1 byte 2
    VERA.address = SPRITE1_ADDR+(spriteNum*8)+6;
    VERA.address_hi = (SPRITE1_ADDR+(spriteNum*8))>>16;

    VERA.data0 = show ? 0b00001000 : 0; // Z-Depth=2 (or 0 to hide)
}

void move(unsigned char spriteNum, short x, short y, short scrollX, short scrollY, unsigned char zone) {
    short finalX, finalY;

    finalY = y-scrollY;
    if ((zone == 0 && finalY >= 224) ||
        (zone == 1 && finalY<=224)) {
        finalY = 480;
    }

    // Set the X and Y values
    finalX = x-scrollX;
    if (finalX<0 || finalX >= 640) {
        finalX = 640;
    }

    // Update Sprite 1 X/Y Position
    // Point to Sprite 1 byte 2
    VERA.address = SPRITE1_ADDR+(spriteNum*8)+2;
    VERA.address_hi = (SPRITE1_ADDR+(spriteNum*8))>>16;
    // Set the Increment Mode, turn on bit 4
    VERA.address_hi |= 0b10000;

    if (finalY >= 480 || finalY < 0 || finalX == 640) {
        VERA.data0 = 640;
        VERA.data0 = 640>>8;
        VERA.data0 = 480;
        VERA.data0 = 480>>8;
    } else {
        VERA.data0 = finalX;
        VERA.data0 = finalX>>8;
        VERA.data0 = finalY;
        VERA.data0 = finalY>>8;
    }
}

void tankTurret(unsigned char turret, unsigned char spriteNum, unsigned char tankGraphic) {
    unsigned long spriteGraphicAddress;
    unsigned long spriteAddr = SPRITE1_ADDR+(spriteNum*8);
    unsigned long graphicsAdjust = TANK_FRAME_SIZE*turret;
    unsigned short vFlip = 0, hFlip = 0;

    spriteGraphicAddress = tankGraphic == 0 ? TANK_A_SPRITE_GRAPHICS_ADDR : TANK_B_SPRITE_GRAPHICS_ADDR;

    switch (turret) {
        case 5: graphicsAdjust = TANK_FRAME_SIZE*3; vFlip = 1; break;
        case 6: graphicsAdjust = TANK_FRAME_SIZE*2; vFlip = 1; break;
        case 7: graphicsAdjust = TANK_FRAME_SIZE*1; vFlip = 1; break;
        case 8: graphicsAdjust = 0; vFlip = 1; break;
        case 9: graphicsAdjust = TANK_FRAME_SIZE*1; vFlip = 1; hFlip = 1; break;
        case 10: graphicsAdjust = TANK_FRAME_SIZE*2; vFlip = 1; hFlip = 1; break;
        case 11: graphicsAdjust = TANK_FRAME_SIZE*3; vFlip = 1; hFlip = 1; break;
        case 12: graphicsAdjust = TANK_FRAME_SIZE*4; hFlip = 1; break;
        case 13: graphicsAdjust = TANK_FRAME_SIZE*3; hFlip = 1; break;
        case 14: graphicsAdjust = TANK_FRAME_SIZE*2; hFlip = 1; break;
        case 15: graphicsAdjust = TANK_FRAME_SIZE*1; hFlip = 1; break;
    }

    spriteGraphicAddress+= graphicsAdjust;

    VERA.address = spriteAddr;
    VERA.address_hi = spriteAddr>>16;
    // Set the Increment Mode, turn on bit 4
    VERA.address_hi |= 0b10000;

    // Graphic address bits 12:5
    VERA.data0 = spriteGraphicAddress>>5;
    // 256 color mode, and graphic address bits 16:13
    VERA.data0 = 0b10000000 | spriteGraphicAddress>>13;

    VERA.address = spriteAddr+6;
    VERA.address_hi = spriteAddr+6>>16;
    // Set the Increment Mode, turn on bit 4
    VERA.address_hi |= 0b10000;

    VERA.data0 = 0b1000 | vFlip<<1 | hFlip;
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
