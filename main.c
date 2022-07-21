#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <unistd.h>
#include <time.h>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

#define PADDLE_WIDTH 20
#define PADDLE_HEIGHT 60

#define BALL_WIDTH 20
#define BALL_HEIGHT 20

#define MAX_SCORE 7

enum Scene
{
    MAIN_MENU,
    MAIN_LOOP,
    GAME_OVER
};

enum Paddle
{
    PLAYER,
    ENEMY,
    NONE
};

static enum Scene currScene = MAIN_MENU;
static enum Paddle paddleWon = NONE;
static SDL_bool quitGame = SDL_FALSE;

void initialize(SDL_Window **window, SDL_Renderer **renderer)
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

void drawMiddleLine(SDL_Renderer *renderer)
{
    SDL_Rect stripe =
    {
        .x = 310,
        .y = 0,
        .w = 20,
        .h = WINDOW_HEIGHT
    };
    SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
    SDL_RenderFillRect(renderer, &stripe);

}

void mainMenuScene(SDL_Renderer *renderer, TTF_Font *font)
{
    SDL_bool exitMainMenu = SDL_FALSE;

    TTF_SetFontSize(font, 24);
    SDL_Color white = {255, 255, 255};

    SDL_Rect logoRect =
    {
        .x = 70,
        .y = 50,
        .w = 500,
        .h = 200
    };

    SDL_Rect continueRect =
    {
        .x = 70,
        .y = 300,
        .w = 490,
        .h = 50
    };

    SDL_Surface *mainLogoSurf = TTF_RenderText_Solid(font, "Seapong", white);
    SDL_Texture *mainLogoTexture = 
        SDL_CreateTextureFromSurface(renderer, mainLogoSurf);

    SDL_Surface *continueSurf = TTF_RenderText_Solid(font, "Press Enter to continue", white);
    SDL_Texture *continueTexture = SDL_CreateTextureFromSurface(renderer, continueSurf);

    while(!quitGame && !exitMainMenu)
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
                    if(event.key.keysym.sym == SDLK_RETURN)
                    {
                        currScene = MAIN_LOOP;
                        exitMainMenu = SDL_TRUE;
                    }
                    else if(event.key.keysym.sym == SDLK_ESCAPE)
                    {
                        quitGame = SDL_TRUE;
                    }
                default:
                    break;
            }
        }
        SDL_RenderClear(renderer);

        SDL_RenderCopy(renderer, continueTexture, NULL, &continueRect);
        SDL_RenderCopy(renderer, mainLogoTexture, NULL, &logoRect);

        SDL_RenderPresent(renderer);

        usleep(1000);
    }

    SDL_FreeSurface(mainLogoSurf);
    SDL_DestroyTexture(mainLogoTexture);

    SDL_FreeSurface(continueSurf);
    SDL_DestroyTexture(continueTexture);
}

void movePaddleY(SDL_Rect *paddle, int dy)
{
    if(dy > 0)
    {
        if((WINDOW_HEIGHT - (paddle->y + paddle->h)) <= dy)
        {
            paddle->y = WINDOW_HEIGHT - paddle->h;
            return;
        }
    }
    else if(dy < 0)
    {
        if(paddle->y <= abs(dy))
        {
            paddle->y = 0;
            return;
        }
    }
    paddle->y += dy;
}

SDL_bool ballHitPaddle(SDL_Rect *ball, SDL_Rect *paddle)
{
    if((ball->x <= (paddle->x + PADDLE_WIDTH)) &&
       ((ball->x + BALL_WIDTH) >= paddle->x) &&
       (ball->y <= (paddle->y + PADDLE_HEIGHT)) &&
       ((ball->y + BALL_HEIGHT) >= paddle->y))
    {
        return SDL_TRUE;
    }
    else
    {
        return SDL_FALSE;
    }
}

SDL_bool ballAtLeftEdge(SDL_Rect *ball)
{
    if(ball->x <= 0)
    {
        return SDL_TRUE;
    }
    else
    {
        return SDL_FALSE;
    }
}

SDL_bool ballAtRightEdge(SDL_Rect *ball)
{
    if((ball->x + BALL_WIDTH) >= WINDOW_WIDTH)
    {
        return SDL_TRUE;
    }
    else
    {
        return SDL_FALSE;
    }
}

SDL_bool ballAtTop(SDL_Rect *ball)
{
    if(ball->y <= 0)
    {
        return SDL_TRUE;
    }
    else
    {
        return SDL_FALSE;
    }
}

SDL_bool ballAtBottom(SDL_Rect *ball)
{
    if((ball->y + BALL_HEIGHT) >= WINDOW_HEIGHT)
    {
        return SDL_TRUE;
    }
    else
    {
        return SDL_FALSE;
    }
}

void respawnBall(SDL_Rect *ball)
{
    ball->x = 200 + rand() % 50;
    ball->y = 200 + rand() % 50;
}

void mainLoopScene(SDL_Renderer *renderer, TTF_Font *font)
{
    int playerScore = 0;
    int enemyScore = 0;

    int ballVelX = -1;
    int ballVelY = 1;

    char buf[200];

    SDL_Rect playerPaddle =
    {
        .x = 0,
        .y = 0,
        .w = PADDLE_WIDTH,
        .h = PADDLE_HEIGHT
    };

    SDL_Rect enemyPaddle =
    {
        .x = (WINDOW_WIDTH - PADDLE_WIDTH),
        .y = 0,
        .w = PADDLE_WIDTH,
        .h = PADDLE_HEIGHT
    };

    SDL_Rect ball =
    {
        .x = 200,
        .y = 200,
        .w = BALL_WIDTH,
        .h = BALL_HEIGHT
    };

    SDL_Rect playerScoreRect =
    {
        .x = 100,
        .y = 100,
        .w = 100,
        .h = 100
    };

    SDL_Rect enemyScoreRect =
    {
        .x = 450,
        .y = 100,
        .w = 100,
        .h = 100
    };

    TTF_SetFontSize(font, 24);
    SDL_Color white = {255, 255, 255};

    SDL_Surface *playerScoreSurf = TTF_RenderText_Solid(font, "0", white);
    SDL_Texture *playerScoreTexture = SDL_CreateTextureFromSurface(renderer, playerScoreSurf);

    SDL_Surface *enemyScoreSurf = TTF_RenderText_Solid(font, "0", white);
    SDL_Texture *enemyScoreTexture = SDL_CreateTextureFromSurface(renderer, enemyScoreSurf);

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
                    movePaddleY(&playerPaddle, -10);
                }
                else if(event.key.keysym.sym == SDLK_DOWN)
                {
                    movePaddleY(&playerPaddle, 10);
                }
                else if(event.key.keysym.sym == SDLK_ESCAPE)
                {
                    quitGame = SDL_TRUE;
                }
                break;
            default:
                break;
            }
        }

        if(ballAtLeftEdge(&ball))
        {
            enemyScore++;
            memset(buf, '\0', sizeof(buf));
            sprintf(buf, "%d", enemyScore);
            enemyScoreSurf = TTF_RenderText_Solid(font, buf, white);
            enemyScoreTexture = SDL_CreateTextureFromSurface(renderer, enemyScoreSurf);
            if(enemyScore == MAX_SCORE)
            {
                paddleWon = ENEMY;
                currScene = GAME_OVER;
                return;
            }
            else
            {
                respawnBall(&ball);
                sleep(1);
                continue;
            }
        }

        if(ballAtRightEdge(&ball))
        {
            playerScore++;
            memset(buf, '\0', sizeof(buf));
            sprintf(buf, "%d", playerScore);
            playerScoreSurf = TTF_RenderText_Solid(font, buf, white);
            playerScoreTexture = SDL_CreateTextureFromSurface(renderer, playerScoreSurf);
            if(playerScore == MAX_SCORE)
            {
                paddleWon = PLAYER;
                currScene = GAME_OVER;
                return;
            }
            else
            {
                respawnBall(&ball);
                sleep(1);
                continue;
            }
        }

        if(ballHitPaddle(&ball, &playerPaddle))
        {
            ball.x += 5;
            ball.y += 5;
            ballVelX = -ballVelX;
            ballVelY = -ballVelY;
        }

        if(ballHitPaddle(&ball, &enemyPaddle))
        {
            ball.x -= 5;
            ball.y += 5;
            ballVelX = -ballVelX;
            ballVelY = -ballVelY;
        }

        if(ballAtTop(&ball))
        {
            ball.y += 5;
            ballVelY = -ballVelY;
        }

        if(ballAtBottom(&ball))
        {
            ball.y -= 5;
            ballVelY = -ballVelY;
        }

        int cpuPaddleY = ball.y + (int)(PADDLE_HEIGHT / 3);
        if(cpuPaddleY > (WINDOW_WIDTH - PADDLE_HEIGHT))
        {
            cpuPaddleY = WINDOW_WIDTH - PADDLE_HEIGHT;
        }
        else if(cpuPaddleY < 0)
        {
            cpuPaddleY = 0;
        }
        enemyPaddle.y  = cpuPaddleY;

        ball.y += ballVelY;
        ball.x += ballVelX;

        SDL_RenderClear(renderer);

        SDL_RenderCopy(renderer, playerScoreTexture, NULL, &playerScoreRect);
        SDL_RenderCopy(renderer, enemyScoreTexture, NULL, &enemyScoreRect);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &playerPaddle);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &enemyPaddle);

        drawMiddleLine(renderer);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &ball);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderPresent(renderer);

        usleep(2000);
    }

    SDL_FreeSurface(playerScoreSurf);
    SDL_DestroyTexture(playerScoreTexture);

    SDL_FreeSurface(enemyScoreSurf);
    SDL_DestroyTexture(enemyScoreTexture);
}

void gameOverScene(SDL_Renderer *renderer, TTF_Font *font)
{
    SDL_bool exitWinScreen = SDL_FALSE;

    TTF_SetFontSize(font, 24);
    SDL_Color white = {255, 255, 255};

    char buf[200];

    memset(buf, '\0', sizeof(buf));
    if(paddleWon == PLAYER)
    {
        sprintf(buf, "%s", "Player wins!!!");
    }
    else if(paddleWon == ENEMY)
    {
        sprintf(buf, "%s", "Enemy wins!!!");
    }

    SDL_Surface *enemyWinSurf = TTF_RenderText_Solid(font, buf, white);
    SDL_Texture *enemyWinTexture = SDL_CreateTextureFromSurface(renderer, enemyWinSurf);

    SDL_Surface *direction1Surf = TTF_RenderText_Solid(font, "Press n to play again", white);
    SDL_Texture *direction1Texture = SDL_CreateTextureFromSurface(renderer, direction1Surf);

    SDL_Surface *direction2Surf = TTF_RenderText_Solid(font, "Press Enter to go to main menu", white);
    SDL_Texture *direction2Texture = SDL_CreateTextureFromSurface(renderer, direction2Surf);

    SDL_Rect enemyRect =
    {
        .x = 70,
        .y = 50,
        .w = 500,
        .h = 200
    };

    SDL_Rect direction1Rect =
    {
        .x = 70,
        .y = 300,
        .w = 500,
        .h = 50
    };

    SDL_Rect direction2Rect =
    {
        .x = 70,
        .y = 400,
        .w = 500,
        .h = 50
    };

    SDL_RenderClear(renderer);

    SDL_RenderCopy(renderer, enemyWinTexture, NULL, &enemyRect);
    SDL_RenderCopy(renderer, direction1Texture, NULL, &direction1Rect);
    SDL_RenderCopy(renderer, direction2Texture, NULL, &direction2Rect);

    SDL_RenderPresent(renderer);

    while(!quitGame && !exitWinScreen)
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
                if(event.key.keysym.sym == SDLK_RETURN)
                {
                    currScene = MAIN_MENU;
                    exitWinScreen = SDL_TRUE;
                }
                else if(event.key.keysym.sym == SDLK_n)
                {
                    currScene = MAIN_LOOP;
                    exitWinScreen = SDL_TRUE;
                }
                else if(event.key.keysym.sym == SDLK_ESCAPE)
                {
                    quitGame = SDL_TRUE;
                }
                break;
            default:
                break;
            }
        }
    }

    SDL_FreeSurface(enemyWinSurf);
    SDL_DestroyTexture(enemyWinTexture);
    SDL_FreeSurface(direction1Surf);
    SDL_DestroyTexture(direction1Texture);
    SDL_FreeSurface(direction2Surf);
    SDL_DestroyTexture(direction2Texture);
}

int main(int argc, char *argv[])
{
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;

    currScene = MAIN_MENU;

    initialize(&window, &renderer);

    srand(time(0));

    TTF_Font *pixelFont = TTF_OpenFont("PixelFont.ttf", 24);
    if(pixelFont == NULL)
    {
        printf("Unable to open ttf file with error: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }

    while(!quitGame)
    {
        switch(currScene)
        {
        case MAIN_MENU:
            mainMenuScene(renderer, pixelFont);
            break;
        case MAIN_LOOP:
            mainLoopScene(renderer, pixelFont);
            break;
        case GAME_OVER:
            gameOverScene(renderer, pixelFont);
        default:
            break;
        }
    }

    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();
}
