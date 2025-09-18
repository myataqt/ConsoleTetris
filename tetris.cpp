#include <ncurses.h>
#include <unistd.h>
#include <vector>
#include <cstdlib>
#include <ctime>

using namespace std;

const int FIELD_WIDTH = 10;
const int FIELD_HEIGHT = 20;
const int INFO_WIDTH = 15;
const int BORDER_WIDTH = 2;

const vector<vector<vector<int>>> TETROMINOS = {
    // I-form
    {{1, 1, 1, 1}},
    // O-form
    {{1, 1}, {1, 1}},
    // T-form
    {{0, 1, 0}, {1, 1, 1}},
    // L-form
    {{1, 0}, {1, 0}, {1, 1}},
    // J-form
    {{0, 1}, {0, 1}, {1, 1}},
    // S-form
    {{0, 1, 1}, {1, 1, 0}},
    // Z-form
    {{1, 1, 0}, {0, 1, 1}}
};

class Tetris {
private:
    vector<vector<int>> field;
    vector<vector<int>> currentPiece;
    vector<vector<int>> nextPiece;
    int pieceX, pieceY;
    int ghostY;
    int score;

    void calculateGhost() {
        ghostY = pieceY;
        while (true) {
            ghostY++;
            if (checkCollision(pieceX, ghostY, currentPiece)) {
                ghostY--;
                break;
            }
        }
    }

    bool checkCollision(int x, int y, const vector<vector<int>>& piece) {
        for (int py = 0; py < piece.size(); py++) {
            for (int px = 0; px < piece[py].size(); px++) {
                if (piece[py][px] &&
                    (y + py >= FIELD_HEIGHT ||
                     x + px < 0 ||
                     x + px >= FIELD_WIDTH ||
                     (y + py >= 0 && field[y + py][x + px]))) {
                    return true;
                }
            }
        }
        return false;
    }

    void drawBorder() {
        for (int y = 0; y < FIELD_HEIGHT + 2; y++) {
            mvprintw(y, 0, "|");
            mvprintw(y, FIELD_WIDTH * 2 + 1, "|");
        }
        for (int x = 0; x < FIELD_WIDTH * 2 + 2; x++) {
            mvprintw(0, x, "-");
            mvprintw(FIELD_HEIGHT + 1, x, "-");
        }

        for (int y = 0; y < 8; y++) {
            mvprintw(y, FIELD_WIDTH * 2 + 3, "|");
            mvprintw(y, FIELD_WIDTH * 2 + 3 + INFO_WIDTH, "|");
        }
        for (int x = 0; x < INFO_WIDTH + 1; x++) {
            mvprintw(0, FIELD_WIDTH * 2 + 3 + x, "-");
            mvprintw(8, FIELD_WIDTH * 2 + 3 + x, "-");
        }

        mvprintw(1, FIELD_WIDTH * 2 + 5, "NEXT");
        mvprintw(4, FIELD_WIDTH * 2 + 5, "SCORE");
        mvprintw(5, FIELD_WIDTH * 2 + 7, "%d", score);

        mvprintw(FIELD_HEIGHT + 3, 0, "Controls:");
        mvprintw(FIELD_HEIGHT + 4, 0, "A/D - Left/Right");
        mvprintw(FIELD_HEIGHT + 5, 0, "W - Rotate");
        mvprintw(FIELD_HEIGHT + 6, 0, "S - Soft Drop");
        mvprintw(FIELD_HEIGHT + 7, 0, "Space - Hard Drop");
        mvprintw(FIELD_HEIGHT + 8, 0, "Q - Quit");
    }

public:
    Tetris() : field(FIELD_HEIGHT, vector<int>(FIELD_WIDTH, 0)), score(0) {
        srand(time(0));
        generateNextPiece();
        newPiece();
    }

    void generateNextPiece() {
        int index = rand() % TETROMINOS.size();
        nextPiece = TETROMINOS[index];
    }

    void newPiece() {
        currentPiece = nextPiece;
        generateNextPiece();
        pieceX = FIELD_WIDTH / 2 - currentPiece[0].size() / 2;
        pieceY = 0;
        calculateGhost();

        if (checkCollision(pieceX, pieceY, currentPiece)) {
            endwin();
            printf("Game Over! Final Score: %d\n", score);
            exit(0);
        }
    }

    bool isCollision() {
        return checkCollision(pieceX, pieceY, currentPiece);
    }

    void rotate() {
        vector<vector<int>> rotated(currentPiece[0].size(), vector<int>(currentPiece.size()));
        for (int y = 0; y < currentPiece.size(); y++) {
            for (int x = 0; x < currentPiece[y].size(); x++) {
                rotated[x][currentPiece.size() - 1 - y] = currentPiece[y][x];
            }
        }

        vector<vector<int>> oldPiece = currentPiece;
        currentPiece = rotated;

        if (isCollision()) {
            currentPiece = oldPiece;
        } else {
            calculateGhost();
        }
    }

    void hardDrop() {
        pieceY = ghostY;
        mergePiece();
        newPiece();
    }

    void draw() {
        clear();
        drawBorder();

        for (int y = 0; y < FIELD_HEIGHT; y++) {
            for (int x = 0; x < FIELD_WIDTH; x++) {
                if (field[y][x]) {
                    mvprintw(y + 1, x * 2 + 1, "[]");
                }
            }
        }

        for (int y = 0; y < currentPiece.size(); y++) {
            for (int x = 0; x < currentPiece[y].size(); x++) {
                if (currentPiece[y][x]) {
                    mvprintw(ghostY + y + 1, (pieceX + x) * 2 + 1, "..");
                }
            }
        }

        for (int y = 0; y < currentPiece.size(); y++) {
            for (int x = 0; x < currentPiece[y].size(); x++) {
                if (currentPiece[y][x]) {
                    mvprintw(pieceY + y + 1, (pieceX + x) * 2 + 1, "[]");
                }
            }
        }

        for (int y = 0; y < nextPiece.size(); y++) {
            for (int x = 0; x < nextPiece[y].size(); x++) {
                if (nextPiece[y][x]) {
                    mvprintw(y + 2, FIELD_WIDTH * 2 + 6 + x * 2, "[]");
                }
            }
        }

        mvprintw(5, FIELD_WIDTH * 2 + 7, "%d", score);

        refresh();
    }

    void run() {
        int ch;
        int speed = 0;
        while (true) {
            draw();
            usleep(50000);

            if (speed++ % 10 == 0) {
                pieceY++;
                if (isCollision()) {
                    pieceY--;
                    mergePiece();
                    newPiece();
                } else {
                    calculateGhost();
                }
            }

            if ((ch = getch()) != ERR) {
                switch(ch) {
                    case 'a':
                        pieceX--;
                        if (isCollision()) pieceX++;
                        else calculateGhost();
                        break;
                    case 'd':
                        pieceX++;
                        if (isCollision()) pieceX--;
                        else calculateGhost();
                        break;
                    case 's':
                        pieceY++;
                        if (isCollision()) pieceY--;
                        else calculateGhost();
                        break;
                    case 'w':
                        rotate();
                        break;
                    case ' ':
                        hardDrop();
                        break;
                    case 'q':
                        return;
                }
            }
        }
    }

    void mergePiece() {
        for (int y = 0; y < currentPiece.size(); y++) {
            for (int x = 0; x < currentPiece[y].size(); x++) {
                if (currentPiece[y][x]) {
                    field[pieceY + y][pieceX + x] = 1;
                }
            }
        }
        clearLines();
    }


    void clearLines() {
        int linesCleared = 0;
        for (int y = 0; y < FIELD_HEIGHT; y++) {
            bool full = true;
            for (int x = 0; x < FIELD_WIDTH; x++) {
                if (!field[y][x]) {
                    full = false;
                    break;
                }
            }
            if (full) {
                field.erase(field.begin() + y);
                field.insert(field.begin(), vector<int>(FIELD_WIDTH, 0));
                linesCleared++;
            }
        }

        if (linesCleared > 0) {
            score += linesCleared * linesCleared * 100;
        }
    }
};

int main() {
    initscr();
    timeout(0);
    noecho();
    keypad(stdscr, TRUE);

    Tetris game;
    game.run();

    endwin();
    return 0;
}
