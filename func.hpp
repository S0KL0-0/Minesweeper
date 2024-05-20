#include <vector>
#include <chrono>
#include <random>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <windows.h>

#define COLOR_UNOPENED "\e[1;30m"
#define COLOR_NUMBERED "\e[0;96m"
#define COLOR_MARKED "\e[0;95m"
#define COLOR_MOUSE "\033[1;31m"

void initializeMenu(int options, std::string* menu) {
    for (int i = 0; i < options; i++) {
        if (i == 0) {
            std::cout << "\033[1;31m" << menu[i] << std::endl;
        } else {
            std::cout << "\033[0m" << menu[i] << std::endl;
        }
    }
}

void updateMenu(HANDLE hConsole , int option, int options, std::string* menu) {
    for (int i = 0; i < options; i++) {
        COORD currentCoord = { 0, static_cast<SHORT>(i) };
        SetConsoleCursorPosition(hConsole, currentCoord);
        if (i == option) {
            std::cout << "\033[1;31m" << menu[i] << std::endl;
        } else {
            std::cout << "\033[0m" << menu[i] << std::endl;
        }
    }
}

struct state {
    char type;
    bool visible;
    bool marked;
};

bool inbounds(int y, int x, int cols, int rows) {
    return x >= 0 && x < rows && y >= 0 && y < cols;
}

void initializeNumber(int pos1, int pos2, state** arr, int cols, int rows) {

    if (inbounds(pos1, pos2, cols, rows)) {
        if (arr[pos1][pos2].type == 32) {
            arr[pos1][pos2].type = '1';
        } else if (arr[pos1][pos2].type >= '1' && arr[pos1][pos2].type <= '7') {
            arr[pos1][pos2].type++;
        }
    }

}

void initializeBoard(state**& arr, int cols, int rows, double mineProb, int& minesGenerated, int minesToGenerate) {
    struct mine {
        int x;
        int y;
    };

    std::vector<mine> mines;
    arr = new state*[cols];

    std::mt19937 generator(std::chrono::system_clock::now().time_since_epoch().count());
    std::uniform_real_distribution<float> distribution(0.0f, 1.0f);

    for (int i = 0; i < cols; i++) {
        arr[i] = new state[rows];
        for (int j = 0; j < rows; j++) {
            float r_float = distribution(generator);
            if (r_float <= mineProb && minesGenerated < minesToGenerate) {
                arr[i][j] = state{33, false, false}; 
                mines.push_back(mine{i, j}); 
                minesGenerated++;
            } else {
                arr[i][j] = state{32, false, false}; 
            }
        }
    }

    // [-1, -1] [-1, 0] [-1, 1]
    // [0, -1]  [0, 0]  [0, 1]
    // [1, -1]  [1, 0]  [1, 1]

    for (size_t i = 0; i < mines.size(); i++) {
        initializeNumber(mines[i].x-1, mines[i].y-1, arr, cols, rows);
        initializeNumber(mines[i].x-1, mines[i].y, arr, cols, rows);
        initializeNumber(mines[i].x-1, mines[i].y+1, arr, cols, rows);
        initializeNumber(mines[i].x, mines[i].y-1, arr, cols, rows);
        initializeNumber(mines[i].x, mines[i].y+1, arr, cols, rows);
        initializeNumber(mines[i].x+1, mines[i].y-1, arr, cols, rows);
        initializeNumber(mines[i].x+1, mines[i].y, arr, cols, rows);
        initializeNumber(mines[i].x+1, mines[i].y+1, arr, cols, rows);
    }


}

char displayChar(state** arr, int pos1, int pos2, bool isCurrentPos = false) {
    state c = arr[pos1][pos2];

    if (c.marked) {
        return 'X';
    } else if ( ! c.visible) {
        return '#';
    } else if (c.type == 32) {
        return (isCurrentPos ? 'O' : ' ');
    } else {
        return c.type;
    }  
}

void initialDisplay(int cols, int rows) {
    for (int i = 0; i < cols; i++) {
        for (int j = 0; j < rows; j++) { 
            std::cout << COLOR_UNOPENED << '#';
        }
        std::cout << std::endl;
    }
}

void update(HANDLE hConsole, state** arr, int* currentPos, int* previousPos) {

    // Calculate console coordinates
    COORD currentCoord = { static_cast<SHORT>(currentPos[1]), static_cast<SHORT>(currentPos[0]) };
    COORD previousCoord = { static_cast<SHORT>(previousPos[1]), static_cast<SHORT>(previousPos[0]) };

    // Reset console color at the previous position
    SetConsoleCursorPosition(hConsole, previousCoord);

    char c = displayChar(arr, previousPos[0], previousPos[1], false);

    if ( ! arr[previousPos[0]][previousPos[1]].visible) {
        std::cout << COLOR_UNOPENED;
    } else if (c >= '1' && c <= '8') {
        std::cout << COLOR_NUMBERED;
    } else if (c == 'X') {
        std::cout << COLOR_MARKED;
    }

    std::cout << displayChar(arr, previousPos[0], previousPos[1], false);

    // Set console color and write character at the current position
    SetConsoleCursorPosition(hConsole, currentCoord);


    std::cout << COLOR_MOUSE << displayChar(arr, currentPos[0], currentPos[1], true);

}

bool isNumberValid(state** arr, int pos1, int pos2) {
    return arr[pos1][pos2].type >= '1' && arr[pos1][pos2].type <= '8' && ! arr[pos1][pos2].visible;
}

void updateOne(HANDLE hConsole, state** arr, int pos1, int pos2) {
    COORD currentCoord = { static_cast<SHORT>(pos2), static_cast<SHORT>(pos1) };
    SetConsoleCursorPosition(hConsole, currentCoord);
    
    char c = displayChar(arr, pos1, pos2, false);

    if ( ! arr[pos1][pos2].visible) {
        std::cout << COLOR_UNOPENED;
    } else if (c >= '1' && c <= '8') {
        std::cout << COLOR_NUMBERED;
    } else if (c == 'X') {
        std::cout << COLOR_MARKED;
    }

    std::cout << c;

}

void Flud_Fill(state** arr, int cols, int rows, int pos1, int pos2, HANDLE hConsole) {
    if (pos1 < 0 || pos2 >= rows || pos2 < 0 || pos1 >= cols || arr[pos1][pos2].visible) {
        return;
    }

    if (arr[pos1][pos2].type == 32) { // Unopened Empty

        arr[pos1][pos2].visible = true;

        updateOne(hConsole, arr, pos1, pos2);

        Flud_Fill(arr, cols, rows, pos1-1, pos2, hConsole);
        Flud_Fill(arr, cols, rows, pos1+1, pos2, hConsole);
        Flud_Fill(arr, cols, rows, pos1, pos2-1, hConsole);
        Flud_Fill(arr, cols, rows, pos1, pos2+1, hConsole);
        Flud_Fill(arr, cols, rows, pos1-1, pos2-1, hConsole);
        Flud_Fill(arr, cols, rows, pos1-1, pos2+1, hConsole);
        Flud_Fill(arr, cols, rows, pos1+1, pos2-1, hConsole);
        Flud_Fill(arr, cols, rows, pos1+1, pos2+1, hConsole);

    } else if (isNumberValid(arr, pos1, pos2)) { // Unopened Number
        arr[pos1][pos2].visible = true;
        updateOne(hConsole, arr, pos1, pos2);
    }
}

void freeMemory(state**& arr, int cols) {
    for (int i = 0; i < cols; i++) {
        delete[] arr[i];
    }
    delete[] arr;
}

