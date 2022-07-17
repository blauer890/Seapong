#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <unistd.h>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

#define PADDLE_WIDTH 20
#define PADDLE_HEIGHT 60

#define BALL_WIDTH 20
#define BALL_HEIGHT 20

#define MAX_SCORE 7

enum GameState
{
    PLAYING,
    RESPAWNING,
    ENDED
};

struct Point
{
    int x;
    int y;
};

struct BoundingBox
{
    struct Point topLeft;
    struct Point topRight;
    struct Point bottomLeft;
    struct Point bottomRight;
};

void SetScreenBoundingBox(struct BoundingBox *screenBB)
{
    screenBB->topLeft.x = 0;
    screenBB->topLeft.y = 0;

    screenBB->topRight.x = WINDOW_WIDTH;
    screenBB->topRight.y = 0;

    screenBB->bottomLeft.x = 0;
    screenBB->bottomLeft.y = WINDOW_HEIGHT;

    screenBB->bottomRight.x = WINDOW_WIDTH;
    screenBB->bottomRight.y = WINDOW_HEIGHT;
}

void UpdateBoundingBox(struct BoundingBox *bb, SDL_Rect *rect)
{
    bb->topLeft.x = rect->x;
    bb->topLeft.y = rect->y;

    bb->topRight.x = rect->x + rect->w;
    bb->topRight.y = rect->y;

    bb->bottomLeft.x = rect->x;
    bb->bottomLeft.y = rect->y + rect->h;

    bb->bottomRight.x = rect->x + rect->w;
    bb->bottomRight.y = rect->y + rect->h;
}

void Initialize(SDL_Window **window, SDL_Renderer **renderer)
{
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("Error initializing video with error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    if(TTF_Init() == -1)
    {
        printf("Error initializing TTF with error: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }

    *window = SDL_CreateWindow("Seapong", SDL_WINDOWPOS_CENTERED, 
        SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if(*window == NULL)
    {
        printf("Window creation failed with error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    *renderer = SDL_CreateRenderer(*window, -1, 0);
    if(*renderer == NULL)
    {
        printf("Renderer creation failed with error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
}

void DrawMiddleLine(SDL_Renderer *renderer)
{
    for(size_t i = 0; i < (size_t)(WINDOW_HEIGHT / 10); i++)
    {
        SDL_Rect stripe =
        {
            .x = 310,
            .w = 20,
            .h = 10
        };
        if(i % 2 == 0)
        {
            stripe.y = 10 * i;
        }
        SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
        SDL_RenderFillRect(renderer, &stripe);
    }
}

int main(int argc, char *argv[])
{
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_bool quitGame = 0;

    SDL_Rect playerPaddle;
    SDL_Rect enemyPaddle;
    SDL_Rect ball;

    struct BoundingBox screenBB;
    SetScreenBoundingBox(&screenBB);

    struct BoundingBox playerPaddleBB;
    struct BoundingBox enemyPaddleBB;
    struct BoundingBox ballBB;

    playerPaddle.x = 0;
    playerPaddle.w = PADDLE_WIDTH;
    playerPaddle.h = PADDLE_HEIGHT;

    ball.x = 200;
    ball.y = 200;
    ball.w = BALL_WIDTH;
    ball.h = BALL_HEIGHT;

    int ballVelX = 1;
    int ballVelY = 1;

    int playerScore = 0;
    int cpuScore = 0;

    enemyPaddle.x = WINDOW_WIDTH - 20;
    enemyPaddle.y = 80;
    enemyPaddle.w = PADDLE_WIDTH;
    enemyPaddle.h = PADDLE_HEIGHT;

    Initialize(&window, &renderer);

    TTF_Font *pixelFont = TTF_OpenFont("PixelFont.ttf", 24);
    if(pixelFont == NULL)
    {
        printf("Unable to open ttf file with error: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_Color white = {255, 255, 255};

    SDL_Surface* mainLogoSurf;
    SDL_Texture* mainLogoTexture;

    SDL_Surface* onePlayerSurf;
    SDL_Texture* onePlayerTexture;

    SDL_Surface* twoPlayerSurf;
    SDL_Texture* twoPlayerTexture;

    SDL_Surface* yourScoreSurf;
    SDL_Texture* yourScoreTexture;

    SDL_Surface* cpuScoreSurf;
    SDL_Surface* cpuScoreTexture;

    SDL_Surface* gameOverSurf;
    SDL_Texture* gameOverTexture;

    while(!quitGame)
    {
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_QUIT:
                    quitGame = SDL_TRUE;
                    break;
                case SDL_KEYDOWN:
                    if(event.key.keysym.sym == SDLK_UP)
                    {
                        playerPaddle.y -= 10;
                    }
                    else if(event.key.keysym.sym == SDLK_DOWN)
                    {
                        playerPaddle.y += 10;
                    }
                    break;
                default:
                    break;
            }
        }
        SDL_RenderClear(renderer);

        // We need to move the object's position
        // back to within the screen frame
        if(playerPaddleBB.bottomLeft.y > screenBB.bottomLeft.y)
        {
            playerPaddle.y = screenBB.bottomLeft.y - PADDLE_HEIGHT;
        }

        if(playerPaddleBB.topLeft.y < screenBB.topLeft.y)
        {
            playerPaddle.y = screenBB.topLeft.y;
        }


        ball.x += ballVelX;
        ball.y += ballVelY;


        if(ballBB.topLeft.x <= screenBB.topLeft.x)
        {
            ball.x += 5;
            ballVelX = -ballVelX;
        }

        if(ballBB.topLeft.y <= screenBB.topLeft.y)
        {
            ball.y += 5;
            ballVelY = -ballVelY;
        }

        if(ballBB.bottomRight.x >= screenBB.bottomRight.x)
        {
            ball.x -= 5; // Get it away from the edge
            ballVelX = -ballVelX;
        }

        if(ballBB.bottomRight.y >= screenBB.bottomRight.y)
        {
            // TODO: Add some randomization to the get away value
            ball.y -= 5; // Get it away from the edge
            ballVelY = -ballVelY;
        }

        if((ballBB.topLeft.x <= playerPaddleBB.topRight.x) &&
           (ballBB.topRight.x >= playerPaddleBB.bottomLeft.x) &&
           (ballBB.topRight.y <= playerPaddleBB.bottomLeft.y) &&
           (ballBB.bottomLeft.y >= playerPaddleBB.topRight.y))
        {
            ball.x += 5;
            ball.y += 5;
            ballVelX = -ballVelX;
            ballVelY = -ballVelY;
        }


        if((ballBB.topLeft.x <= enemyPaddleBB.topRight.x) &&
           (ballBB.topRight.x >= enemyPaddleBB.bottomLeft.x) &&
           (ballBB.topRight.y <= enemyPaddleBB.bottomLeft.y) &&
           (ballBB.bottomLeft.y >= enemyPaddleBB.topRight.y))
        {
            ball.x -= 5;
            ball.y -= 5;
            ballVelX = -ballVelX;
            ballVelY = -ballVelY;
        }

        int cpuPaddleY = ball.y + (int)(PADDLE_HEIGHT / 3);
        if(cpuPaddleY > (screenBB.bottomLeft.y - PADDLE_HEIGHT))
        {
            cpuPaddleY = screenBB.bottomLeft.y - PADDLE_HEIGHT;
        }
        else if(cpuPaddleY < (screenBB.topLeft.y))
        {
            cpuPaddleY = screenBB.topLeft.y;
        }
        enemyPaddle.y = cpuPaddleY;

        UpdateBoundingBox(&playerPaddleBB, &playerPaddle);
        UpdateBoundingBox(&enemyPaddleBB, &enemyPaddle);
        UpdateBoundingBox(&ballBB, &ball);

        yourScoreSurf = TTF_RenderText_Solid(pixelFont, "13", white);
        yourScoreTexture = SDL_CreateTextureFromSurface(renderer, yourScoreSurf);

        SDL_Rect yourScoreRect;
        yourScoreRect.x = 100;
        yourScoreRect.y = 100;
        yourScoreRect.w = 100;
        yourScoreRect.h = 100;

        SDL_Rect cpuScoreRect;
        cpuScoreRect.x = 450;
        cpuScoreRect.y = 100;
        cpuScoreRect.w = 100;
        cpuScoreRect.h = 100;

        SDL_RenderCopy(renderer, yourScoreTexture, NULL, &yourScoreRect);
        SDL_RenderCopy(renderer, yourScoreTexture, NULL, &cpuScoreRect);

        DrawMiddleLine(renderer);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &playerPaddle);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &enemyPaddle);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &ball);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderPresent(renderer);

        usleep(1000);
    }

    SDL_FreeSurface(yourScoreSurf);
    SDL_DestroyTexture(yourScoreTexture);
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();
    return 0;
}
