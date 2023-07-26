CC=cl65

make:
	$(CC) -O -o TANKS.PRG -t cx16 main.c wait.c utils.c config.c sprites.c tiles.c

run:
	x16emur43/x16emu -prg TANKS.PRG -run

img:
	node gimp-convert.js tanka

zip:
	zip tanks.zip TANKS.PRG manifest.json
