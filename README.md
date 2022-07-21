# Seapong
A clone of pong written in C using the SDL2 library

This requires a TTF font file which
can be downloaded from [Pixel font converter!](https://yal.cc/r/20/pixelfont/) into the same directory as `main.c`.

The font file must be specifically named `PixelFont.ttf` in order for the game to run.

Two SDL2 libraries must be downloaded with the commands on Ubuntu:

`sudo apt-get install libsdl2-dev`

`sudo apt-get install libsdl2-ttf-dev`

To compile, simply type the command

`gcc -o [EXECUTABLE_NAME] main.c -lSDL2 -lSDL2_ttf`

Sorry, no makefile here. 
