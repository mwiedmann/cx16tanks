#include <cx16.h>
#include <joystick.h>

#include "sprites.h"
#include "config.h"

void createSpriteGraphics() {
    unsigned short i;
    
    // Clear layer 0
    VERA.address = SPRITE_GRAPHICS_ADDR;
    VERA.address_hi = SPRITE_GRAPHICS_ADDR>>16;
    // Always set the Increment Mode, turn on bit 4
    VERA.address_hi |= 0b10000;

    // Tank A 32x32
    for (i=0; i<32*32; i++) {
        VERA.data0 = 9;
    }

    // Tank B 32x32
    for (i=0; i<32*32; i++) {
        VERA.data0 = 10;
    }
}

void spritesConfig() {
    unsigned char i;

    // VRAM address for sprite 1 graphics...reusing a tile for now
    unsigned long spriteGraphicAddress = TILEBASE_ADDR + (2 * 256);

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
        VERA.data0 = 0b01010000; // 16x16 pixel image
    }
   
    // Create 4 tank sprites
    for (i=0; i<4; i++) {
        // Point to Sprite 2
        VERA.address = SPRITE1_ADDR+(SPRITE_NUM_TANK_A1*8)+(i*8);
        VERA.address_hi = SPRITE1_ADDR+(SPRITE_NUM_TANK_A1*8)+(i*8)>>16;
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

void move(unsigned char spriteNum, short x, short y, short scrollX, short scrollY) {
    short finalX, finalY;

    // Update Sprite 1 X/Y Position
    // Point to Sprite 1 byte 2
    VERA.address = SPRITE1_ADDR+(spriteNum*8)+2;
    VERA.address_hi = (SPRITE1_ADDR+(spriteNum*8))>>16;
    // Set the Increment Mode, turn on bit 4
    VERA.address_hi |= 0b10000;
    
    // Set the X and Y values
    finalX = x-scrollX;
    if (finalX<0 || finalX >= 640) {
        finalX = 640;
    }

    finalY = y-scrollY;
    if (finalY<0 || finalY >= 480) {
        finalY = 480;
    }

    VERA.data0 = finalX;
    VERA.data0 = finalX>>8;
    VERA.data0 = finalY;
    VERA.data0 = finalY>>8;
}
