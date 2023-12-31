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
    loadFileToVRAM("tankc.bin", TANK_C_SPRITE_GRAPHICS_ADDR);
}

void spritesConfig() {
    unsigned char i;
    unsigned long spriteGraphicAddress;

    // Create 6 ball sprites
    for (i=0; i<BALLS_COUNT*2; i++) {
        VERA.address = SPRITE1_ADDR+(SPRITE_NUM_BALL_A*8)+(i*8);
        VERA.address_hi = SPRITE1_ADDR+(SPRITE_NUM_BALL_A*8)+(i*8)>>16;
        // Set the Increment Mode, turn on bit 4
        VERA.address_hi |= 0b10000;

        spriteGraphicAddress = i<2 ? BALL_A_SPRITE_GRAPHICS_ADDR : BALL_B_SPRITE_GRAPHICS_ADDR;

        // Graphic address bits 12:5
        VERA.data0 = spriteGraphicAddress>>5;
        // 256 color mode, and graphic address bits 16:13
        VERA.data0 = 0b10000000 | spriteGraphicAddress>>13;
        VERA.data0 = 100;
        VERA.data0 = 100;
        VERA.data0 = 100;
        VERA.data0 = 100;
        VERA.data0 = i<2 ? 0b01100000 : i<4 ? 0b01010000 : 0b00110000;
        VERA.data0 = 0b00000000; // 8x8 pixel image
    }
   
    // Create tank sprites
    for (i=0; i<TANKS_COUNT*2; i++) {
        // Point to Sprite 2
        VERA.address = SPRITE1_ADDR+(SPRITE_NUM_TANK_A*8)+(i*8);
        VERA.address_hi = SPRITE1_ADDR+(SPRITE_NUM_TANK_A*8)+(i*8)>>16;
        // Set the Increment Mode, turn on bit 4
        VERA.address_hi |= 0b10000;

        // Configure Sprite 2 (Tank)
        spriteGraphicAddress = i<2 ? TANK_A_SPRITE_GRAPHICS_ADDR : i<4 ? TANK_B_SPRITE_GRAPHICS_ADDR : TANK_C_SPRITE_GRAPHICS_ADDR;

        // Graphic address bits 12:5
        VERA.data0 = spriteGraphicAddress>>5;
        // 256 color mode, and graphic address bits 16:13
        VERA.data0 = 0b10000000 | spriteGraphicAddress>>13;
        VERA.data0 = 100;
        VERA.data0 = 100;
        VERA.data0 = 100;
        VERA.data0 = 100;
        VERA.data0 = i<2 ? 0b10010000 : i<4 ? 0b10100000 : 0b11000000;
        VERA.data0 = 0b10100000; // 32x32 pixel image
    }
}

void toggle(unsigned char spriteNum, unsigned char show) {
    // Update Sprite 1 X/Y Position
    // Point to Sprite 1 byte 2
    VERA.address = SPRITE1_ADDR+(spriteNum*8)+6;
    VERA.address_hi = (SPRITE1_ADDR+(spriteNum*8))>>16;

    VERA.data0 = show ? VERA.data0 | 0b1000 : VERA.data0 & 0b11110111; // Z-Depth=2 (or 0 to hide)
}

void move(unsigned char spriteNum, short x, short y, short scrollX, short scrollY, unsigned char zone) {
    short finalX, finalY;

    finalY = y-scrollY;
    if ((zone == 0 && finalY >= 212) ||
        (zone == 1 && finalY<=240)) {
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
        VERA.data0 = 0b10000000 | 640>>8;
        VERA.data0 = 480;
        VERA.data0 = 480>>8;
    } else {
        VERA.data0 = finalX;
        VERA.data0 = 0b10000000 | finalX>>8;
        VERA.data0 = finalY;
        VERA.data0 = finalY>>8;
    }
}

void moveHide(unsigned char spriteNum) {
    unsigned short finalY;

    // Update Sprite 1 X/Y Position
    // Point to Sprite 1 byte 2
    VERA.address = SPRITE1_ADDR+(spriteNum*8)+2;
    VERA.address_hi = (SPRITE1_ADDR+(spriteNum*8))>>16;
    // Set the Increment Mode, turn on bit 4
    VERA.address_hi |= 0b10000;

    finalY = 640 + (32*spriteNum);
    VERA.data0 = 0;
    VERA.data0 = 0b10000000;
    VERA.data0 = finalY;
    VERA.data0 = finalY>>8;
}

void tankTurret(unsigned char turret, unsigned char spriteNum, unsigned char tankGraphic) {
    unsigned long spriteGraphicAddress;
    unsigned long spriteAddr = SPRITE1_ADDR+(spriteNum*8);
    unsigned long graphicsAdjust = TANK_FRAME_SIZE*turret;
    unsigned char vFlip = 0, hFlip = 0;

    spriteGraphicAddress = tankGraphic == 0
        ? TANK_A_SPRITE_GRAPHICS_ADDR
        : tankGraphic == 1
            ? TANK_B_SPRITE_GRAPHICS_ADDR
            : TANK_C_SPRITE_GRAPHICS_ADDR;

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

    VERA.data0 = VERA.data0 & 0b11111100; // Clear flip bits
    VERA.data0 = VERA.data0 | vFlip<<1 | hFlip;
}

unsigned char moveTank(unsigned char moveLeft, unsigned char moveRight, unsigned char moveUp, unsigned char moveDown, Tank *tank) {
    unsigned char l0Tile;
    unsigned char moved = 0;

    if (moveLeft) {
        getCollisionTile(tank->x-tank->speed, tank->y, &l0Tile);
        if (l0Tile == 0 || l0Tile == tank->wallId) {
            getCollisionTile(tank->x-tank->speed, tank->y+16, &l0Tile);
            if (l0Tile == 0 || l0Tile == tank->wallId) {
                getCollisionTile(tank->x-tank->speed, tank->y+31, &l0Tile);
                if (l0Tile == 0 || l0Tile == tank->wallId) {
                    tank->x-= tank->speed;
                    moved=1;
                }
            }
        }
    } else if (moveRight) {
        getCollisionTile(tank->x+31+tank->speed, tank->y, &l0Tile);
        if (l0Tile == 0 || l0Tile == tank->wallId) {
            getCollisionTile(tank->x+31+tank->speed, tank->y+16, &l0Tile);
            if (l0Tile == 0 || l0Tile == tank->wallId) {
                getCollisionTile(tank->x+31+tank->speed, tank->y+31, &l0Tile);
                if (l0Tile == 0 || l0Tile == tank->wallId) {
                    tank->x+= tank->speed;
                    moved=1;
                }
            }
        }
    }

    if (moveUp) {
        getCollisionTile(tank->x, tank->y-tank->speed, &l0Tile);
        if (l0Tile == 0 || l0Tile == tank->wallId) {
            getCollisionTile(tank->x+16, tank->y-tank->speed, &l0Tile);
            if (l0Tile == 0 || l0Tile == tank->wallId) {
                getCollisionTile(tank->x+31, tank->y-tank->speed, &l0Tile);
                if (l0Tile == 0 || l0Tile == tank->wallId) {
                    tank->y-= tank->speed;
                    moved=1;
                }
            }
        }
    } else if (moveDown) {
        getCollisionTile(tank->x, tank->y+31+tank->speed, &l0Tile);
        if (l0Tile == 0 || l0Tile == tank->wallId) {
            getCollisionTile(tank->x+16, tank->y+31+tank->speed, &l0Tile);
            if (l0Tile == 0 || l0Tile == tank->wallId) {
                getCollisionTile(tank->x+31, tank->y+31+tank->speed, &l0Tile);
                if (l0Tile == 0 || l0Tile == tank->wallId) {
                    tank->y+= tank->speed;
                    moved=1;
                }
            }
        }
    }

    return moved;
}
