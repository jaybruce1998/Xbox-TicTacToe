#include <hal/debug.h>
#include <hal/video.h>
#include <SDL.h>
#include <windows.h>
#include <stdbool.h>

#define GRID_SIZE 3
#define EMPTY 0
#define PLAYER_X 1
#define PLAYER_O 2

#define GREEN  0xFF00FF00
#define BLUE   0xFF00A5FF
#define ORANGE 0xFFFFA500
#define RED    0xFFFF0000


int grid[GRID_SIZE][GRID_SIZE];
int turn = PLAYER_X;
int cursorX = 0, cursorY = 0;
SDL_GameController *pad = NULL;
char winner, cur;
#include <pbkit/pbkit.h>

void drawO(int32_t centreX, int32_t centreY, int32_t radius) {
    const int32_t diameter = (radius * 2);
    int32_t x = (radius - 1);
    int32_t y = 0;
    int32_t tx = 1;
    int32_t ty = 1;
    int32_t error = (tx - diameter);

    while (x >= y) {
        // Draw points in all octants with BLUE color
        pb_fill(centreX + x, centreY - y, 1, 1, BLUE); // Top right
        pb_fill(centreX + x, centreY + y, 1, 1, BLUE); // Bottom right
        pb_fill(centreX - x, centreY - y, 1, 1, BLUE); // Top left
        pb_fill(centreX - x, centreY + y, 1, 1, BLUE); // Bottom left
        pb_fill(centreX + y, centreY - x, 1, 1, BLUE); // Top right
        pb_fill(centreX + y, centreY + x, 1, 1, BLUE); // Bottom right
        pb_fill(centreX - y, centreY - x, 1, 1, BLUE); // Top left
        pb_fill(centreX - y, centreY + x, 1, 1, BLUE); // Bottom left

        if (error <= 0) {
            ++y;
            error += ty;
            ty += 2;
        }

        if (error > 0) {
            --x;
            tx += 2;
            error += (tx - diameter);
        }
    }
}

void drawW(int x, int y) {
    int width = 20; // Set the width of the W
    int height = 30; // Set the height of the W

    for (int i = 0; i < height; i++) {
        pb_fill(x + i * (width / 4) / height, y + i, 1, 1, RED); // Left diagonal to the peak
    }
    for (int i = 0; i < height; i++) {
        pb_fill(x + (width / 4) + (i * (width / 4) / height), y + height - i, 1, 1, RED); // Right diagonal going down
    }
    // Draw the right V (offset to the right)
    for (int i = 0; i < height; i++) {
        pb_fill(x + (width / 2) + i * (width / 4) / height, y + i, 1, 1, RED); // Left diagonal to the peak of second V
    }
    for (int i = 0; i < height; i++) {
        pb_fill(x + (3 * width / 4) + (i * (width / 4) / height), y + height - i, 1, 1, RED); // Right diagonal going down of second V
    }
}

void drawN(int x, int y) {
    int height = 30; // Set height for the N
    int width = 20;  // Set width for the N

    pb_fill(x, y, 1, height, RED); // Left vertical line
    pb_fill(x + width, y, 1, height, RED); // Right vertical line
    // Draw diagonal line from left to right
    for (int i = 0; i <= height; i++) {
        pb_fill(x + i * width / height, y + i, 1, 1, RED); // Diagonal line
    }
}


void drawT(int x, int y) {
    int width = 30;  // Increased width for larger 'T'
    int height = 45; // Increased height for larger 'T'

    pb_fill(x, y, width, 1, RED); // Top horizontal line
    pb_fill(x + width / 2, y, 1, height, RED); // Vertical line
}

void drawI(int x, int y) {
    drawT(x, y);
    pb_fill(x, y + 45, 30, 1, RED); // Draw bottom horizontal line of I
}

void drawE(int x, int y) {
    int width = 30;  // Increased width for larger 'E'
    int height = 45; // Increased height for larger 'E'

    pb_fill(x, y, 1, height, RED); // Vertical line
    pb_fill(x, y, width, 1, RED); // Top horizontal line
    pb_fill(x, y + height / 2, width, 1, RED); // Middle horizontal line
    pb_fill(x, y + height, width, 1, RED); // Bottom horizontal line
}

void drawX(int x, int y, int size) {
    for (int i = 0; i <= size; i++) {
        pb_fill(x + i, y + i, 1, 1, ORANGE); // Diagonal top-left to bottom-right
        pb_fill(x + size - i, y + i, 1, 1, ORANGE); // Diagonal top-right to bottom-left
    }
}

void draw_cursor() {
    int xPos = cursorX * (640 / GRID_SIZE);
    int yPos = cursorY * (480 / GRID_SIZE);
    int cellWidth = 640 / GRID_SIZE;
    int cellHeight = 480 / GRID_SIZE;
    int lineWidth = 2; // Thickness of the border

    pb_fill(xPos, yPos, cellWidth, lineWidth, GREEN); // Top border
    pb_fill(xPos, yPos + cellHeight - lineWidth, cellWidth, lineWidth, GREEN); // Bottom border
    pb_fill(xPos, yPos, lineWidth, cellHeight, GREEN); // Left border
    pb_fill(xPos + cellWidth - lineWidth, yPos, lineWidth, cellHeight, GREEN); // Right border
}

void draw_grid() {
    pb_fill(0, 0, 640, 480, 0); // Clear screen with black

    for (int i = 1; i < GRID_SIZE; i++) {
        int linePosX = 640 / GRID_SIZE * i;
        int linePosY = 480 / GRID_SIZE * i;
        pb_fill(linePosX, 0, 2, 480, 0xFFFFFFFF); // Vertical lines
        pb_fill(0, linePosY, 640, 2, 0xFFFFFFFF); // Horizontal lines
    }

    // Draw X's and O's on the grid
    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            int cellX = x * (640 / GRID_SIZE);
            int cellY = y * (480 / GRID_SIZE);
            int centreX = cellX + (640 / GRID_SIZE) / 2;
            int centreY = cellY + (480 / GRID_SIZE) / 2;
            int radius = 80;

            if (grid[y][x] == PLAYER_X) {
                drawX(cellX + 5, cellY + 5, 150); // Draw 'X' with padding
            } else if (grid[y][x] == PLAYER_O) {
                drawO(centreX, centreY, radius); // Draw larger 'O' in the center of the cell
            }
        }
    }

    draw_cursor();

    if(winner)
    {
        if(winner=='T') {
            drawT(250, 100);
            drawI(290, 100);
            drawE(330, 100);
        }
        else {
            if(winner==PLAYER_X)
                drawX(250, 100, 30);
            else
                drawO(270, 115, 15);
            drawW(300, 100);
            drawO(340, 115, 15);
            drawN(360, 100);
        }
    }
}

void reset_game() {
    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            grid[y][x] = EMPTY;
        }
    }
    turn = PLAYER_X; // Reset to Player X
    winner = 0;
    cur = 1;
}

bool movedBefore = false;
void handle_input(SDL_GameController *pad) {
    bool moved = false;

    if (SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_DPAD_UP)) {
        if(!movedBefore)
          cursorY=cursorY==0?2:cursorY-1;
        moved = true;
    } else if (SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_DPAD_DOWN)) {
        if(!movedBefore)
          cursorY=cursorY==2?0:cursorY+1;
        moved = true;
    } else if (SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_DPAD_LEFT)) {
        if(!movedBefore)
          cursorX=cursorX==0?2:cursorX-1;
        moved = true;
    } else if (SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_DPAD_RIGHT)) {
        if(!movedBefore)
          cursorX=cursorX==2?0:cursorX+1;
        moved = true;
    }
    movedBefore=moved;

    if (SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_A)) {
        if (winner == 0 && grid[cursorY][cursorX] == EMPTY) {
            grid[cursorY][cursorX] = turn;
            if((grid[0][cursorX]==turn&&grid[1][cursorX]==turn&&grid[2][cursorX]==turn)||(grid[cursorY][0]==turn&&grid[cursorY][1]==turn&&grid[cursorY][2]==turn)
                ||(grid[1][1]==turn&&((grid[0][0]==turn&&grid[2][2]==turn)||(grid[0][2]==turn&&grid[2][0]==turn))))
                winner=turn;
            else if(cur++==9)
                winner='T';
            else
                turn=turn==PLAYER_X?PLAYER_O:PLAYER_X;
        }
    }

    if (SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_START)) {
        if (winner) {
          reset_game();
        }
    }
}

int main(void) {
    static SDL_Event e;
    bool pbk_init = false, sdl_init = false;

    XVideoSetMode(640, 480, 32, REFRESH_DEFAULT);

    sdl_init = SDL_Init(SDL_INIT_GAMECONTROLLER) == 0;
    if (!sdl_init) {
        debugPrint("SDL_Init failed: %s\n", SDL_GetError());
        goto wait_then_cleanup;
    }

    pbk_init = pb_init() == 0;
    if (!pbk_init) {
        debugPrint("pbkit init failed\n");
        goto wait_then_cleanup;
    }
    reset_game();
    pb_show_front_screen();

    while (1) {
        pb_wait_for_vbl();
        pb_target_back_buffer();
        pb_reset();
        pb_erase_text_screen();

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_CONTROLLERDEVICEADDED) {
                SDL_GameController *new_pad = SDL_GameControllerOpen(e.cdevice.which);
                if (pad == NULL) {
                    pad = new_pad;
                }
            } else if (e.type == SDL_CONTROLLERDEVICEREMOVED) {
                if (pad == SDL_GameControllerFromInstanceID(e.cdevice.which)) {
                    pad = NULL;
                }
                SDL_GameControllerClose(SDL_GameControllerFromInstanceID(e.cdevice.which));
            }
        }

        SDL_GameControllerUpdate();
        if (pad != NULL) {
            handle_input(pad);
        }

        draw_grid();
        pb_draw_text_screen();
        while (pb_busy());
        while (pb_finished());
    }

wait_then_cleanup:
    Sleep(5000);

cleanup:
    if (pbk_init) {
        pb_kill();
    }
    if (pad != NULL) {
        SDL_GameControllerClose(pad);
    }
    if (sdl_init) {
        SDL_Quit();
    }

    return 0;
}