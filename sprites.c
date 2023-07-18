#include <cx16.h>
#include <joystick.h>

#include "sprites.h"
#include "config.h"


void spritesConfig() {
    // VRAM address for sprite 1 (this is fixed)
    unsigned long spriteGraphicAddress = TILEBASE_ADDR + (2 * 256);

    // Point to Sprite 1
    VERA.address = SPRITE1_ADDR;
    VERA.address_hi = SPRITE1_ADDR>>16;
    // Set the Increment Mode, turn on bit 4
    VERA.address_hi |= 0b10000;

    // Configure Sprite 1 (Ball)

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

    // Point to Sprite 2
    VERA.address = SPRITE1_ADDR+8;
    VERA.address_hi = (SPRITE1_ADDR+8)>>16;
    // Set the Increment Mode, turn on bit 4
    VERA.address_hi |= 0b10000;

    // Configure Sprite 2 (Tank)
    spriteGraphicAddress = SPRITE_GRAPHICS_ADDR;

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

void createSpriteGraphics() {
    
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
