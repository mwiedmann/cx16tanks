#ifndef CONFIG_H
#define CONFIG_H

#define MAPBASE_TILE_WIDTH 128
#define MAPBASE_TILE_HEIGHT 128
#define MAPBASE_TILE_COUNT MAPBASE_TILE_WIDTH * MAPBASE_TILE_HEIGHT
#define MAPBASE_SIZE MAPBASE_TILE_COUNT*2

// Our default Tile and Map Base addresses
#define L0_MAPBASE_ADDR 0x00000L
#define L1_MAPBASE_ADDR L0_MAPBASE_ADDR + MAPBASE_SIZE 
#define TILEBASE_ADDR L1_MAPBASE_ADDR + MAPBASE_SIZE

#define TILESET_SIZE 256*10

#define SPRITE_GRAPHICS_ADDR TILEBASE_ADDR + TILESET_SIZE
#define TANK_A_SPRITE_GRAPHICS_ADDR SPRITE_GRAPHICS_ADDR
#define TANK_B_SPRITE_GRAPHICS_ADDR TANK_A_SPRITE_GRAPHICS_ADDR+1024
#define PALETTE_ADDR 0x1FA00L

void init();
void setZoom(unsigned char zoomMode);
void clearLayers();
void clearLayer0();
void clearLayer1();
void pickModes(unsigned char *zoomMode, unsigned char *gameMode, unsigned char *courseCount);
void waitForButtonPress();

#endif