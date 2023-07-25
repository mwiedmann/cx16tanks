#ifndef SPRITES_H
#define SPRITES_H

#define SPRITE1_ADDR 0x1FC08

#define SPRITE_NUM_BALL_A1 0
#define SPRITE_NUM_BALL_A2 SPRITE_NUM_BALL_A1+1
#define SPRITE_NUM_BALL_B1 SPRITE_NUM_BALL_A2+1
#define SPRITE_NUM_BALL_B2 SPRITE_NUM_BALL_B1+1

#define SPRITE_NUM_TANK_A1 16
#define SPRITE_NUM_TANK_A2 SPRITE_NUM_TANK_A1+1
#define SPRITE_NUM_TANK_B1 SPRITE_NUM_TANK_A2+1
#define SPRITE_NUM_TANK_B2 SPRITE_NUM_TANK_B1+1

void createSpriteGraphics();
void spritesConfig();
void move(unsigned char spriteNum, short x, short y, short scrollX, short scrollY);
void toggle(unsigned char spriteNum, unsigned char show);

#endif