// minesweeper_raylib_with_sfx_fixed.c
// Full Minesweeper code with sounds and boom image integration (Raylib)
// Place number.mp3, boom.mp3, flag.mp3, over.mp3, win.mp3 and boomm.jpg
// in the same folder as the executable (or adjust paths in LoadSound / LoadTexture).

#include "raylib.h"
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define ROWS 9
#define COLS 9
#define CELL_SIZE 60
#define MINES 10

typedef struct {
    bool revealed;
    bool hasMine;
    bool flagged;
    int nearbyMines;
} Cell;

// --- Globals for audio & texture ---
static Sound sNumber;
static Sound sBoom;
static Sound sFlag;
static Sound sOver;
static Sound sWin;
static Texture2D texBoom;

// play-once trackers so we don't re-play repeatedly
static bool playedGameOverSound = false;
static bool playedWinSound = false;
static bool playedBoomSound = false;

// ---------------- Function prototypes ----------------
void InitializeBoard(Cell board[ROWS][COLS]);
void PlaceMines(Cell board[ROWS][COLS]);
void CountNearbyMines(Cell board[ROWS][COLS]);
void RevealEmpty(Cell board[ROWS][COLS], int row, int col);
void RevealAllMines(Cell board[ROWS][COLS]);

int main(void) {
    InitWindow(COLS * CELL_SIZE, ROWS * CELL_SIZE + 50, "Minesweeper - Raylib Styled");
    SetTargetFPS(60);
    srand((unsigned)time(NULL));

    // Initialize audio device and load sounds
    InitAudioDevice();
    sNumber = LoadSound("number.mp3");
    sBoom   = LoadSound("boom.mp3");
    sFlag   = LoadSound("flag.mp3");
    sOver   = LoadSound("over.mp3");
    sWin    = LoadSound("win.mp3");
    texBoom = LoadTexture("boomm.png");

    // NOTE: Removed SetTextureFilter call for compatibility with different raylib versions

    Cell board[ROWS][COLS];
    InitializeBoard(board);
    PlaceMines(board);
    CountNearbyMines(board);

    bool gameOver = false;
    bool win = false;

    while (!WindowShouldClose()) {
        if (!gameOver && !win) {
            Vector2 mouse = GetMousePosition();
            int col = (int)(mouse.x / CELL_SIZE);
            int row = (int)(mouse.y / CELL_SIZE);

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (row >= 0 && row < ROWS && col >= 0 && col < COLS) {
                    if (!board[row][col].flagged && !board[row][col].revealed) {
                        board[row][col].revealed = true;

                        if (board[row][col].hasMine) {
                            // stepped on a mine
                            if (!playedBoomSound) {
                                PlaySound(sBoom);
                                playedBoomSound = true;
                            }
                            gameOver = true;
                            RevealAllMines(board);
                            // play over sound after showing boom (play once)
                            if (!playedGameOverSound) {
                                PlaySound(sOver);
                                playedGameOverSound = true;
                            }
                        } else {
                            // revealed a non-mine cell (Option B: play number sound for every reveal)
                            PlaySound(sNumber);

                            if (board[row][col].nearbyMines == 0) {
                                // reveal adjacent empty cells (RevealEmpty will also play number sound
                                // each time it reveals a cell so Option B is respected)
                                RevealEmpty(board, row, col);
                            }
                        }
                    }
                }
            }

            if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
                if (row >= 0 && row < ROWS && col >= 0 && col < COLS && !board[row][col].revealed) {
                    board[row][col].flagged = !board[row][col].flagged;
                    PlaySound(sFlag);
                }
            }

            // Win check
            int revealedCount = 0;
            for (int r = 0; r < ROWS; r++) {
                for (int c = 0; c < COLS; c++) {
                    if (board[r][c].revealed && !board[r][c].hasMine)
                        revealedCount++;
                }
            }

            if (revealedCount == ROWS * COLS - MINES) {
                win = true;
                if (!playedWinSound) {
                    PlaySound(sWin);
                    playedWinSound = true;
                }
            }
        } else {
            // If gameOver true but we haven't played the over sound (e.g., mines revealed by other means),
            // ensure it's played once.
            if (gameOver && !playedGameOverSound) {
                PlaySound(sOver);
                playedGameOverSound = true;
            }
            if (win && !playedWinSound) {
                PlaySound(sWin);
                playedWinSound = true;
            }
        }

        // --- Draw ---
        BeginDrawing();
        ClearBackground((Color){ 48, 99, 47, 255 }); // dark green background

        for (int r = 0; r < ROWS; r++) {
            for (int c = 0; c < COLS; c++) {
                Rectangle cell = { c * CELL_SIZE, r * CELL_SIZE, CELL_SIZE, CELL_SIZE };

                // checkerboard background color
                Color baseColor = ((r + c) % 2 == 0)
                    ? (Color){ 190, 224, 145, 255 }   // light green
                    : (Color){ 170, 214, 135, 255 };  // darker green

                Color revealedColor = ((r + c) % 2 == 0)
                    ? (Color){ 240, 210, 170, 255 }   // beige 1
                    : (Color){ 225, 195, 150, 255 };  // beige 2

                // Draw background based on state
                if (board[r][c].revealed)
                    DrawRectangleRec(cell, revealedColor);
                else
                    DrawRectangleRec(cell, baseColor);

                DrawRectangleLinesEx(cell, 1, (Color){ 110, 110, 110, 255 });

                // Draw cell content
                if (board[r][c].revealed) {
                    if (board[r][c].hasMine) {
                        // Draw boom image scaled to cell size
                        Rectangle src = { 0.0f, 0.0f, (float)texBoom.width, (float)texBoom.height };
                        Rectangle dest = { cell.x, cell.y, CELL_SIZE, CELL_SIZE };
                        Vector2 origin = { 0.0f, 0.0f };
                        DrawTexturePro(texBoom, src, dest, origin, 0.0f, WHITE);
                    } else if (board[r][c].nearbyMines > 0) {
                        DrawText(TextFormat("%d", board[r][c].nearbyMines),
                                 cell.x + CELL_SIZE / 2 - 8, cell.y + CELL_SIZE / 2 - 12, 25, BLUE);
                    }
                } else if (board[r][c].flagged) {
                    DrawTriangle(
                        (Vector2){ cell.x + CELL_SIZE / 2 - 8, cell.y + CELL_SIZE / 2 + 8 },
                        (Vector2){ cell.x + CELL_SIZE / 2 - 8, cell.y + CELL_SIZE / 2 - 12 },
                        (Vector2){ cell.x + CELL_SIZE / 2 + 8, cell.y + CELL_SIZE / 2 - 2 },
                        RED
                    );
                }
            }
        }

        // bottom text
        if (gameOver)
            DrawText("💥 GAME OVER!", 10, ROWS * CELL_SIZE + 10, 30, RED);
        else if (win)
            DrawText("🎉 YOU WIN!", 10, ROWS * CELL_SIZE + 10, 30, GREEN);
        else
            DrawText("Left-click: Reveal | Right-click: Flag", 10, ROWS * CELL_SIZE + 15, 20, RAYWHITE);

        EndDrawing();
    }

    // Unload resources and close audio
    UnloadSound(sNumber);
    UnloadSound(sBoom);
    UnloadSound(sFlag);
    UnloadSound(sOver);
    UnloadSound(sWin);
    UnloadTexture(texBoom);
    CloseAudioDevice();

    CloseWindow();
    return 0;
}

// ------------------- Functions -------------------

void InitializeBoard(Cell board[ROWS][COLS]) {
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++) {
            board[r][c].revealed = false;
            board[r][c].hasMine = false;
            board[r][c].flagged = false;
            board[r][c].nearbyMines = 0;
        }
}

void PlaceMines(Cell board[ROWS][COLS]) {
    int placed = 0;
    while (placed < MINES) {
        int r = rand() % ROWS;
        int c = rand() % COLS;
        if (!board[r][c].hasMine) {
            board[r][c].hasMine = true;
            placed++;
        }
    }
}

void CountNearbyMines(Cell board[ROWS][COLS]) {
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            if (board[r][c].hasMine) continue;
            int count = 0;
            for (int dr = -1; dr <= 1; dr++)
                for (int dc = -1; dc <= 1; dc++) {
                    int nr = r + dr, nc = c + dc;
                    if (nr >= 0 && nr < ROWS && nc >= 0 && nc < COLS && board[nr][nc].hasMine)
                        count++;
                }
            board[r][c].nearbyMines = count;
        }
    }
}

// RevealEmpty: recursively reveal adjacent non-mine cells.
// Option B selected: play number sound for every cell revealed here.
void RevealEmpty(Cell board[ROWS][COLS], int row, int col) {
    for (int dr = -1; dr <= 1; dr++) {
        for (int dc = -1; dc <= 1; dc++) {
            int nr = row + dr, nc = col + dc;
            if (nr >= 0 && nr < ROWS && nc >= 0 && nc < COLS) {
                if (!board[nr][nc].revealed && !board[nr][nc].hasMine) {
                    board[nr][nc].revealed = true;
                    // Play number sound for every reveal (Option B)
                    PlaySound(sNumber);
                    if (board[nr][nc].nearbyMines == 0) {
                        RevealEmpty(board, nr, nc);
                    }
                }
            }
        }
    }
}

void RevealAllMines(Cell board[ROWS][COLS]) {
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            if (board[r][c].hasMine)
                board[r][c].revealed = true;
}
