#ifndef SPRITES_H
#define SPRITES_H

#define SPRITE1_ADDR 0x1FC08

#define SPRITE_NUM_BALL_A1 0
#define SPRITE_NUM_BALL_A2 SPRITE_NUM_BALL_A1+1
#define SPRITE_NUM_BALL_B1 SPRITE_NUM_BALL_A2+1
#define SPRITE_NUM_BALL_B2 SPRITE_NUM_BALL_B1+1

#define SPRITE_NUM_TANK_A 16
#define SPRITE_NUM_TANK_B SPRITE_NUM_TANK_A+2

typedef struct Ball {
    unsigned char spriteNum;
    unsigned char side;
    unsigned char active;
    short x;
    short y;
    short moveX;
    short moveY;
    unsigned short ticksRemaining;
} Ball;

typedef struct Tank {
    unsigned char spriteNum;
    unsigned char side;
    unsigned char isAI;
    short x;
    short y;
    short moveX;
    short moveY;
    unsigned char speed;
    unsigned char turret;
} Tank;

void loadSpriteGraphics();
void spritesConfig();
void move(unsigned char spriteNum, short x, short y, short scrollX, short scrollY, unsigned char zone);
void toggle(unsigned char spriteNum, unsigned char show);
unsigned char moveTank(unsigned char speed, unsigned char moveLeft, unsigned char moveRight, unsigned char moveUp, unsigned char moveDown, short *x, short *y);
void tankTurret(unsigned char turret, unsigned char spriteNum, unsigned char tankGraphic);

#endif