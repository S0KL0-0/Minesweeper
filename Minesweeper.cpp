#include <iostream>
#include <windows.h>
#include <conio.h>
#include <random>
#include "func.hpp"

using namespace std;

// Uses Flud Fill algorithm to reveal all empty cells
// Used DFS to search adjacent cells

#define KEY_UP 72
#define KEY_DOWN 80
#define KEY_LEFT 75
#define KEY_RIGHT 77
#define KEY_ENTER 13
#define KEY_SPACEBAR 32
#define KEY_M 109

// 32 = Empty
// 33 = Mine

// '1', '2', '3', '4', '5', '6', '7', '8' = Opened Numbered

void settingsMenu(HANDLE hConsole, int& cols, int& rows, double& mineProb) {
    std::string menu[] = {"Rows: " + std::to_string(rows), "Cols: " + std::to_string(cols), "Mine Probability: " + std::to_string(int(mineProb*100)), "Back"};
    int option = 0;
    int options = sizeof(menu) / sizeof(menu[0]);

    updateMenu(hConsole, option, options, menu);

    while(1) {
        int key = _getch();

        if (key == KEY_UP) {
            if (option > 0) {
                option--;
                updateMenu(hConsole, option, options, menu);
            }

        } else if (key == KEY_DOWN) {
            if (option < options - 1) {
                option++;
                updateMenu(hConsole, option, options, menu);
            }

        } else if (key == KEY_RIGHT) {
            if (option == 0) {
                rows++;
                menu[0] = "Rows: " + std::to_string(rows);
            } else if (option == 1) {
                cols++;
                menu[1] = "Cols: " + std::to_string(cols);
            } else if (option == 2) {
                if (mineProb < 0.3) {mineProb += 0.01;}
                menu[2] = "Mine Probability: " + std::to_string(int(mineProb*100));
            }
            updateMenu(hConsole, option, options, menu);

        } else if (key == KEY_LEFT) {
            if (option == 0) {
                rows--;
                menu[0] = "Rows: " + std::to_string(rows);
            } else if (option == 1) {
                cols--;
                menu[1] = "Cols: " + std::to_string(cols);
            } else if (option == 2) {
                if (mineProb > 0.11) {mineProb -= 0.01;}
                menu[2] = "Mine Probability: " + std::to_string(int(mineProb*100));
            }
            updateMenu(hConsole, option, options, menu);

        } else if (key == KEY_ENTER && option == 3) {
            system("cls"); // Clear the screen
            break;
        }
    }
}

int main() {

    int rows = 30;
    int cols = 15;
    double mineProb = 0.1;

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    if (hConsole == NULL || hConsole == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to obtain console handle." << std::endl;
        system("pause");
        return 1;
    }

    SMALL_RECT windowSize = {0, 0, static_cast<SHORT>(rows), static_cast<SHORT>(cols)};
    SetConsoleWindowInfo(hConsole, TRUE, &windowSize);

    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = false; // Hide the cursor
    SetConsoleCursorInfo(hConsole, &cursorInfo);

    // Menu

    int option = 0;
    int options = 3;

    string menuText[] = {
        "Start Game",
        "Settings",
        "Exit"
    };

    while(1) {

        initializeMenu(options, menuText);
        system("cls");
        initializeMenu(options, menuText);

        while(1) {
            
            int key = _getch();

            if (key == KEY_UP) {
                if (option > 0) {
                    option--;
                }
            } else if (key == KEY_DOWN) {
                if (option < options - 1) {
                    option++;
                }
            } else if (key == KEY_ENTER) {
                if (option == 0) {
                    break;
                } else if (option == 1) {
                    system("cls"); // Clear the screen
                    settingsMenu(hConsole, cols, rows, mineProb);
                } else if (option == 2) {
                    return 0;
                }
            }

            updateMenu(hConsole, option, options, menuText);

        }

        system("cls"); // Clear the screen

        // Game

        state** arr = nullptr;

        int minesGenerated = 0;
        int minesToGenerate = static_cast<int>(std::ceil((rows * cols) * mineProb));;

        initializeBoard(arr, cols, rows, mineProb, minesGenerated, minesToGenerate);

        int currentPos[2] = {0, 0};
        int previousPos[2] = {0, 0};

        initialDisplay(cols, rows);

        update(hConsole, arr, currentPos, previousPos);

        while(1) {

            int key = _getch();

            if (key == KEY_UP) {
                if (currentPos[0] > 0) {
                    previousPos[0] = currentPos[0];
                    previousPos[1] = currentPos[1];
                    currentPos[0]--;
                    update(hConsole, arr, currentPos, previousPos);
                }

            } else if (key == KEY_DOWN) {
                if (currentPos[0] < cols - 1) {
                    previousPos[0] = currentPos[0];
                    previousPos[1] = currentPos[1];
                    currentPos[0]++;
                    update(hConsole, arr, currentPos, previousPos);
                }

            } else if (key == KEY_LEFT) {
                if (currentPos[1] > 0) {
                    previousPos[0] = currentPos[0];
                    previousPos[1] = currentPos[1];
                    currentPos[1]--;
                    update(hConsole, arr, currentPos, previousPos);
                }

            } else if (key == KEY_RIGHT) {
                if (currentPos[1] < rows - 1) {
                    previousPos[0] = currentPos[0];
                    previousPos[1] = currentPos[1];
                    currentPos[1]++;
                    update(hConsole, arr, currentPos, previousPos);
                }

            } else if (key == KEY_ENTER) {
                if (arr[currentPos[0]][currentPos[1]].type == 33) {
                    COORD currentCoord = { static_cast<SHORT>(0), static_cast<SHORT>(cols)};
                    SetConsoleCursorPosition(hConsole, currentCoord);
                    cout << "\033[1;31m" << "Game Over!" << endl;
                    system("pause");

                    update(hConsole, arr, currentPos, previousPos);

                    break;
                } else {
                    Flud_Fill(arr, cols, rows, currentPos[0], currentPos[1], hConsole);
                    update(hConsole, arr, currentPos, previousPos);
                }

            } else if (key == KEY_M) {
                if (arr[currentPos[0]][currentPos[1]].marked == false) {
                    arr[currentPos[0]][currentPos[1]].marked = true;
                    updateOne(hConsole, arr, currentPos[0], currentPos[1]);
                } else {
                    arr[currentPos[0]][currentPos[1]].marked = false;
                    updateOne(hConsole, arr, currentPos[0], currentPos[1]);
                }
            }

            int temp = minesGenerated;
            for (int i = 0; i < cols; i++) {
                for (int j = 0; j < rows; j++) {
                    if (arr[i][j].visible == false) {
                        temp--;
                    }
                }
            }
            if (temp == 0) {
                COORD currentCoord = { static_cast<SHORT>(0), static_cast<SHORT>(cols)};
                SetConsoleCursorPosition(hConsole, currentCoord);
                cout << "You Win!" << endl;
                system("pause");
                break;
            }

        }

        system("cls"); // Clear the screen

        freeMemory(arr, cols);

    }

    return 0;
}