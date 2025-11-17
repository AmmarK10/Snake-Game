/*
  Controls:
    - Arrow keys or W/A/S/D to move
    - R to restart after Game Over
    - Esc or Q to quit
  Compile:
    gcc snake.c -o snake.exe -I "D:\SDL\x86_64-w64-mingw32\include" -L "D:\SDL\x86_64-w64-mingw32\lib" -lSDL3.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#define GRID_COLS   32
#define GRID_ROWS   20
#define CELL_SIZE   20
#define TOP_MARGIN  56
#define WINDOW_W    (GRID_COLS * CELL_SIZE)
#define WINDOW_H    (GRID_ROWS * CELL_SIZE + TOP_MARGIN)
#define MAX_SNAKE   (GRID_COLS * GRID_ROWS)

typedef struct { int x, y; } Cell;

/* ---------- Simple digit renderer (7-segment style) ---------- */
static void drawSegment(SDL_Renderer *ren, float x, float y, float w, float h) {
    SDL_FRect r = { x, y, w, h };
    SDL_RenderFillRect(ren, &r);
}

static void drawDigit(SDL_Renderer *ren, int digit, float x, float y, float scale) {
    static const int segs[10][7] = {
        {1,1,1,0,1,1,1}, //0
        {0,0,1,0,0,1,0}, //1
        {1,0,1,1,1,0,1}, //2
        {1,0,1,1,0,1,1}, //3
        {0,1,1,1,0,1,0}, //4
        {1,1,0,1,0,1,1}, //5
        {1,1,0,1,1,1,1}, //6
        {1,0,1,0,0,1,0}, //7
        {1,1,1,1,1,1,1}, //8
        {1,1,1,1,0,1,1}  //9
    };
    if (digit < 0 || digit > 9) return;

    float segLen = 18.0f * scale;
    float thick  = 4.0f * scale;

    const int *on = segs[digit];
    if (on[0]) drawSegment(ren, x, y, segLen, thick);                        // top
    if (on[1]) drawSegment(ren, x, y+thick, thick, segLen);                  // top-left
    if (on[2]) drawSegment(ren, x+segLen-thick, y+thick, thick, segLen);     // top-right
    if (on[3]) drawSegment(ren, x, y+segLen, segLen, thick);                 // middle
    if (on[4]) drawSegment(ren, x, y+segLen+thick, thick, segLen);           // bottom-left
    if (on[5]) drawSegment(ren, x+segLen-thick, y+segLen+thick, thick, segLen); // bottom-right
    if (on[6]) drawSegment(ren, x, y+2*segLen+thick, segLen, thick);         // bottom
}

static void drawChar(SDL_Renderer *ren, char c, float x, float y, float scale) {
    if (c >= '0' && c <= '9') {
        drawDigit(ren, c - '0', x, y, scale);
    } else if (c == '-') {
        SDL_RenderLine(ren, x, y + 12*scale, x + 18*scale, y + 12*scale);
    } else if (c == '.') {
        SDL_RenderPoint(ren, (int)(x + 9*scale), (int)(y + 36*scale));
    }
}

static void drawText(SDL_Renderer *ren, const char *s, float x, float y, float scale) {
    float spacing = 26.0f * scale;
    for (int i = 0; s[i]; ++i) {
        drawChar(ren, s[i], x + i * spacing, y, scale);
    }
}

/* ---------- Game helpers ---------- */
static void spawnFood(Cell *food, Cell snake[], int snake_len) {
    while (1) {
        int rx = rand() % GRID_COLS;
        int ry = rand() % GRID_ROWS;
        bool ok = true;
        for (int i = 0; i < snake_len; ++i) {
            if (snake[i].x == rx && snake[i].y == ry) { ok = false; break; }
        }
        if (ok) { food->x = rx; food->y = ry; return; }
    }
}

/* ---------- Main ---------- */
int main(int argc, char *argv[]) {
    (void)argc; (void)argv;
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    SDL_Window *win = SDL_CreateWindow("SDL3 Snake", WINDOW_W, WINDOW_H, 0);
    SDL_Renderer *ren = SDL_CreateRenderer(win, NULL);

    srand((unsigned)time(NULL));

    Cell snake[MAX_SNAKE];
    int snake_len = 4;
    int startx = GRID_COLS / 2;
    int starty = GRID_ROWS / 2;
    for (int i = 0; i < snake_len; ++i) { snake[i].x = startx - i; snake[i].y = starty; }
    int dirx = 1, diry = 0;

    Cell food;
    spawnFood(&food, snake, snake_len);

    int score = 0;
    bool running = true;
    bool gameOver = false;

    const Uint64 moveDelay = 100; // ms per move
    Uint64 lastMove = SDL_GetTicks();

    SDL_Event ev;
    while (running) {
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_EVENT_QUIT) running = false;
            else if (ev.type == SDL_EVENT_KEY_DOWN) {
                int k = ev.key.key;
                if (!gameOver) {
                    if ((k == SDLK_UP || k == SDLK_W) && !(dirx == 0 && diry == 1)) { dirx = 0; diry = -1; }
                    else if ((k == SDLK_DOWN || k == SDLK_S) && !(dirx == 0 && diry == -1)) { dirx = 0; diry = 1; }
                    else if ((k == SDLK_LEFT || k == SDLK_A) && !(dirx == 1 && diry == 0)) { dirx = -1; diry = 0; }
                    else if ((k == SDLK_RIGHT || k == SDLK_D) && !(dirx == -1 && diry == 0)) { dirx = 1; diry = 0; }
                }
                if (k == SDLK_ESCAPE || k == SDLK_Q) running = false;
                if (k == SDLK_R && gameOver) {
                    snake_len = 4;
                    for (int i = 0; i < snake_len; ++i) { snake[i].x = startx - i; snake[i].y = starty; }
                    dirx = 1; diry = 0;
                    spawnFood(&food, snake, snake_len);
                    score = 0;
                    gameOver = false;
                    lastMove = SDL_GetTicks();
                }
            }
        }

        Uint64 now = SDL_GetTicks();
        if (!gameOver && now - lastMove >= moveDelay) {
            lastMove = now;
            int nx = snake[0].x + dirx;
            int ny = snake[0].y + diry;

            if (nx < 0 || nx >= GRID_COLS || ny < 0 || ny >= GRID_ROWS) {
                gameOver = true;
            } else {
                bool self = false;
                for (int i = 0; i < snake_len; ++i)
                    if (snake[i].x == nx && snake[i].y == ny) { self = true; break; }
                if (self) gameOver = true;
                else {
                    if (nx == food.x && ny == food.y) {
                        for (int i = snake_len; i > 0; --i) snake[i] = snake[i-1];
                        snake[0].x = nx; snake[0].y = ny;
                        snake_len++;
                        score++;
                        spawnFood(&food, snake, snake_len);
                    } else {
                        for (int i = snake_len - 1; i > 0; --i) snake[i] = snake[i-1];
                        snake[0].x = nx; snake[0].y = ny;
                    }
                }
            }
        }

        /* -------- Rendering -------- */
        SDL_SetRenderDrawColor(ren, 16, 16, 16, 255);
        SDL_RenderClear(ren);

        SDL_SetRenderDrawColor(ren, 30, 30, 30, 255);
        SDL_FRect top = { 0.0f, 0.0f, (float)WINDOW_W, (float)TOP_MARGIN };
        SDL_RenderFillRect(ren, &top);

        char scoreStr[32];
        snprintf(scoreStr, sizeof(scoreStr), "%d", score);
        SDL_SetRenderDrawColor(ren, 0, 200, 0, 255);
        float textWidth = strlen(scoreStr) * 26.0f;
        float startX = (WINDOW_W - textWidth) / 2.0f;
        drawText(ren, scoreStr, startX, 8.0f, 1.0f);

        SDL_SetRenderDrawColor(ren, 220, 40, 40, 255);
        SDL_FRect frect = { food.x * CELL_SIZE + 2.0f, TOP_MARGIN + food.y * CELL_SIZE + 2.0f,
                            CELL_SIZE - 4.0f, CELL_SIZE - 4.0f };
        SDL_RenderFillRect(ren, &frect);

        for (int i = 0; i < snake_len; ++i) {
            if (i == 0) SDL_SetRenderDrawColor(ren, 60, 220, 60, 255);
            else SDL_SetRenderDrawColor(ren, 30, 160, 30, 255);

            SDL_FRect r = { snake[i].x * CELL_SIZE + 1.0f,
                            TOP_MARGIN + snake[i].y * CELL_SIZE + 1.0f,
                            CELL_SIZE - 2.0f, CELL_SIZE - 2.0f };
            SDL_RenderFillRect(ren, &r);
        }

        if (gameOver) {
            SDL_SetRenderDrawColor(ren, 0, 0, 0, 160);
            SDL_FRect over = { 0, (float)TOP_MARGIN, (float)WINDOW_W, (float)(WINDOW_H - TOP_MARGIN) };
            SDL_RenderFillRect(ren, &over);

            char onlyScore[32];
            snprintf(onlyScore, sizeof(onlyScore), "%d", score);
            SDL_SetRenderDrawColor(ren, 255, 100, 100, 255);
            drawText(ren, onlyScore, (WINDOW_W - strlen(onlyScore) * 26.0f) / 2.0f,
                     TOP_MARGIN + (WINDOW_H - TOP_MARGIN) / 2.0f - 20.0f, 1.2f);
        }

        SDL_RenderPresent(ren);
        SDL_Delay(8);
    }

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
