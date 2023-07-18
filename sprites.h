#ifndef SPRITES_H
#define SPRITES_H

#define SPRITE1_ADDR 0x1FC08
#define SPRITE_TEXT_ADDR 0x1FC10

#define SPRITE_NUM_BALL 0
#define SPRITE_NUM_TANK 1

void spritesConfig();
void move(unsigned char spriteNum, short x, short y, short scrollX, short scrollY);
void toggle(unsigned char spriteNum, unsigned char show);

#endif