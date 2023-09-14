#ifndef SPRITES_H
#define SPRITES_H

#define SPRITE1_ADDR 0x1FC08

#define SPRITE_NUM_BALL_A 0
#define SPRITE_NUM_BALL_B SPRITE_NUM_BALL_A+2
#define SPRITE_NUM_BALL_C SPRITE_NUM_BALL_B+2

#define SPRITE_NUM_TANK_A 16
#define SPRITE_NUM_TANK_B SPRITE_NUM_TANK_A+2
#define SPRITE_NUM_TANK_C SPRITE_NUM_TANK_B+2

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
    unsigned char wallId;
    unsigned char isAI;
    short startX;
    short startY;
    short x;
    short y;
    short moveX;
    short moveY;
    unsigned char speed;
    unsigned char turret;
    unsigned char ballNum;
    unsigned char nextShot;
    unsigned char hit;
} Tank;

void loadSpriteGraphics();
void spritesConfig();
void move(unsigned char spriteNum, short x, short y, short scrollX, short scrollY, unsigned char zone);
void moveHide(unsigned char spriteNum);
void toggle(unsigned char spriteNum, unsigned char show);
unsigned char moveTank(unsigned char moveLeft, unsigned char moveRight, unsigned char moveUp, unsigned char moveDown, Tank *tank);
void tankTurret(unsigned char turret, unsigned char spriteNum, unsigned char tankGraphic);

#endif