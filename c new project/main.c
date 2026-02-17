
// =============================================================
// Minesweeper (Raylib)
// Classic grid-based puzzle game
// Focus: clean structure and readable purpose-based comments
// =============================================================

#include "raylib.h"
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

// -------------------- Constants --------------------

// Game board size
#define ROWS 9
#define COLS 9

// Size of each tile in pixels
#define CELL_SIZE 60

// Total number of mines
#define TOTAL_MINES 10

// Frame rate limit
#define MAX_FPS 60

// -------------------- Data Structure --------------------

/*
Represents a single grid cell of the board.
Stores game state for each tile.
*/
typedef struct
{
    bool revealed;     // Whether the tile is opened
    bool hasMine;      // Whether the tile contains a mine
    bool flagged;      // Whether the player marked this tile
    int nearbyMines;   // Number shown when revealed
} Cell;

// -------------------- Global Audio & Texture --------------------

// Sound effects used in different game events
static Sound numberSound;
static Sound boomSound;
static Sound flagSound;
static Sound gameOverSound;
static Sound winSound;

// Explosion image for mines
static Texture2D boomTexture;

// Prevent repeated sound playback
static bool playedGameOver = false;
static bool playedWin = false;
static bool playedBoom = false;

// -------------------- Function Prototypes --------------------

// Board setup
void InitializeBoard(Cell board[ROWS][COLS]);
void PlaceMines(Cell board[ROWS][COLS]);
void CountNearbyMines(Cell board[ROWS][COLS]);

// Game actions
void RevealEmptyCells(Cell board[ROWS][COLS], int row, int col);
void RevealAllMines(Cell board[ROWS][COLS]);

// Player interaction
void HandleMouseInput(Cell board[ROWS][COLS], bool *gameOver);
bool CheckWin(Cell board[ROWS][COLS]);

// Rendering
void DrawGame(Cell board[ROWS][COLS], bool gameOver, bool win);

// =============================================================
//                         MAIN
// =============================================================

int main(void)
{
    // Create the game window
    InitWindow(COLS * CELL_SIZE, ROWS * CELL_SIZE + 50,
               "Minesweeper - Raylib Styled");

    // Set game speed
    SetTargetFPS(MAX_FPS);

    // Randomize mine placement
    srand((unsigned)time(NULL));

    // Initialize audio system
    InitAudioDevice();

    // Load game sounds
    numberSound = LoadSound("number.mp3");
    boomSound = LoadSound("boom.mp3");
    flagSound = LoadSound("flag.mp3");
    gameOverSound = LoadSound("over.mp3");
    winSound = LoadSound("win.mp3");

    // Load mine texture
    boomTexture = LoadTexture("boomm.png");

    // Create the game board
    Cell board[ROWS][COLS];

    // Prepare the board and mines
    InitializeBoard(board);
    PlaceMines(board);
    CountNearbyMines(board);

    bool gameOver = false;
    bool win = false;

    // Main game loop
    while (!WindowShouldClose())
    {
        // Allow input only during active game
        if (!gameOver && !win)
        {
            HandleMouseInput(board, &gameOver);

            // Check win condition
            if (CheckWin(board))
            {
                win = true;

                if (!playedWin)
                {
                    PlaySound(winSound);
                    playedWin = true;
                }
            }
        }

        // Render game
        DrawGame(board, gameOver, win);
    }

    // Release resources
    UnloadSound(numberSound);
    UnloadSound(boomSound);
    UnloadSound(flagSound);
    UnloadSound(gameOverSound);
    UnloadSound(winSound);

    UnloadTexture(boomTexture);

    CloseAudioDevice();
    CloseWindow();

    return 0;
}

// =============================================================
//                    BOARD INITIALIZATION
// =============================================================

/*
Resets the entire board for a new game.
*/
void InitializeBoard(Cell board[ROWS][COLS])
{
    for (int r = 0; r < ROWS; r++)
    {
        for (int c = 0; c < COLS; c++)
        {
            board[r][c].revealed = false;
            board[r][c].hasMine = false;
            board[r][c].flagged = false;
            board[r][c].nearbyMines = 0;
        }
    }
}

/*
Randomly distributes mines across the board.
*/
void PlaceMines(Cell board[ROWS][COLS])
{
    int placed = 0;

    while (placed < TOTAL_MINES)
    {
        int r = rand() % ROWS;
        int c = rand() % COLS;

        if (!board[r][c].hasMine)
        {
            board[r][c].hasMine = true;
            placed++;
        }
    }
}

/*
Calculates the number displayed on each safe tile.
*/
void CountNearbyMines(Cell board[ROWS][COLS])
{
    for (int r = 0; r < ROWS; r++)
    {
        for (int c = 0; c < COLS; c++)
        {
            if (board[r][c].hasMine)
                continue;

            int count = 0;

            for (int dr = -1; dr <= 1; dr++)
            {
                for (int dc = -1; dc <= 1; dc++)
                {
                    int nr = r + dr;
                    int nc = c + dc;

                    if (nr >= 0 && nr < ROWS && nc >= 0 && nc < COLS)
                    {
                        if (board[nr][nc].hasMine)
                            count++;
                    }
                }
            }

            board[r][c].nearbyMines = count;
        }
    }
}

// =============================================================
//                     GAME MECHANICS
// =============================================================

/*
Reveals connected empty tiles automatically.
*/
void RevealEmptyCells(Cell board[ROWS][COLS], int row, int col)
{
    for (int dr = -1; dr <= 1; dr++)
    {
        for (int dc = -1; dc <= 1; dc++)
        {
            int nr = row + dr;
            int nc = col + dc;

            if ((nr >= 0) && (nr < ROWS) && (nc >= 0) && (nc < COLS))
            {
                if (!board[nr][nc].revealed && !board[nr][nc].hasMine)
                {
                    board[nr][nc].revealed = true;

                    if (board[nr][nc].nearbyMines == 0)
                        RevealEmptyCells(board, nr, nc);
                }
            }
        }
    }
}

/*
Shows all mines after losing.
*/
void RevealAllMines(Cell board[ROWS][COLS])
{
    for (int r = 0; r < ROWS; r++)
    {
        for (int c = 0; c < COLS; c++)
        {
            if (board[r][c].hasMine)
                board[r][c].revealed = true;
        }
    }
}

// =============================================================
//                       INPUT HANDLING
// =============================================================

/*
Handles player clicks and updates game state.
*/
void HandleMouseInput(Cell board[ROWS][COLS], bool *gameOver)
{
    Vector2 mouse = GetMousePosition();

    int col = (int)(mouse.x / CELL_SIZE);
    int row = (int)(mouse.y / CELL_SIZE);

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        if ((row >= 0 && row < ROWS) && (col >= 0 && col < COLS))
        {
            if (!board[row][col].flagged && !board[row][col].revealed)
            {
                board[row][col].revealed = true;

                if (board[row][col].hasMine)
                {
                    if (!playedBoom)
                    {
                        PlaySound(boomSound);
                        playedBoom = true;
                    }

                    *gameOver = true;
                    RevealAllMines(board);

                    if (!playedGameOver)
                    {
                        PlaySound(gameOverSound);
                        playedGameOver = true;
                    }
                }
                else
                {
                    PlaySound(numberSound);

                    if (board[row][col].nearbyMines == 0)
                        RevealEmptyCells(board, row, col);
                }
            }
        }
    }

    if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
    {
        if ((row >= 0 && row < ROWS) && (col >= 0 && col < COLS))
        {
            if (!board[row][col].revealed)
            {
                board[row][col].flagged = !board[row][col].flagged;
                PlaySound(flagSound);
            }
        }
    }
}

/*
Checks whether all safe tiles are revealed.
*/
bool CheckWin(Cell board[ROWS][COLS])
{
    int revealedCount = 0;

    for (int r = 0; r < ROWS; r++)
    {
        for (int c = 0; c < COLS; c++)
        {
            if (board[r][c].revealed && !board[r][c].hasMine)
                revealedCount++;
        }
    }

    return (revealedCount == ROWS * COLS - TOTAL_MINES);
}

// =============================================================
//                        RENDERING
// =============================================================

/*
Draws the full game interface on the screen.
*/
void DrawGame(Cell board[ROWS][COLS], bool gameOver, bool win)
{
    BeginDrawing();

    ClearBackground((Color){48, 99, 47, 255});

    for (int r = 0; r < ROWS; r++)
    {
        for (int c = 0; c < COLS; c++)
        {
            Rectangle cell = {c * CELL_SIZE, r * CELL_SIZE,
                              CELL_SIZE, CELL_SIZE};

            Color hiddenColor = ((r + c) % 2 == 0)
                                    ? (Color){190, 224, 145, 255}
                                    : (Color){170, 214, 135, 255};

            Color revealedColor = ((r + c) % 2 == 0)
                                      ? (Color){240, 210, 170, 255}
                                      : (Color){225, 195, 150, 255};

            if (board[r][c].revealed)
                DrawRectangleRec(cell, revealedColor);
            else
            {
                DrawRectangleRec(cell, hiddenColor);
                DrawRectangleLinesEx(cell, 1, (Color){110, 110, 110, 255});
            }

            if (board[r][c].revealed)
            {
                if (board[r][c].hasMine)
                {
                    Rectangle src = {0, 0,
                                     (float)boomTexture.width,
                                     (float)boomTexture.height};

                    Rectangle dest = {cell.x, cell.y,
                                      CELL_SIZE, CELL_SIZE};

                    DrawTexturePro(boomTexture, src, dest,
                                   (Vector2){0, 0}, 0, WHITE);
                }
                else if (board[r][c].nearbyMines > 0)
                {
                    DrawText(TextFormat("%d", board[r][c].nearbyMines),
                             cell.x + CELL_SIZE / 2 - 8,
                             cell.y + CELL_SIZE / 2 - 12,
                             25, BLUE);
                }
            }
            else if (board[r][c].flagged)
            {
                DrawTriangle(
                    (Vector2){cell.x + CELL_SIZE / 2 - 8,
                              cell.y + CELL_SIZE / 2 + 8},

                    (Vector2){cell.x + CELL_SIZE / 2 - 8,
                              cell.y + CELL_SIZE / 2 - 12},

                    (Vector2){cell.x + CELL_SIZE / 2 + 8,
                              cell.y + CELL_SIZE / 2 - 2},

                    RED);
            }
        }
    }

    if (gameOver)
        DrawText("GAME OVER!", 10, ROWS * CELL_SIZE + 10, 30, RED);

    else if (win)
        DrawText("YOU WIN!", 10, ROWS * CELL_SIZE + 10, 30, GREEN);

    else
        DrawText("Left-click: Reveal | Right-click: Flag",
                 10, ROWS * CELL_SIZE + 15, 20, RAYWHITE);

    EndDrawing();
}


