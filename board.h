#pragma once
#include <array>
#include <unordered_map>
#include <iostream>
#include <cstdio>
#include <algorithm>

/**
 * array-based board for 2048
 *
 * index (2-d form):
 * [0][0] [0][1] [0][2] [0][3]
 * [1][0] [1][1] [1][2] [1][3]
 * [2][0] [2][1] [2][2] [2][3]
 * [3][0] [3][1] [3][2] [3][3]
 *
 * index (1-d form):
 *  (0)  (1)  (2)  (3)
 *  (4)  (5)  (6)  (7)
 *  (8)  (9) (10) (11)
 * (12) (13) (14) (15)
 *
 */
class board {

public:
    board() : tile(),
              index2tile({{1, 1}, {2, 2}, {3, 3}, {4, 5}, {5, 8},
                          {6, 13}, {7, 21}, {8, 34}, {9, 55},
                          {10, 89}, {11, 144}, {12, 233}, {13, 377},
                          {14, 610}, {15, 987}, {16, 1597}, {17, 2584},
                          {18, 4181}, {19, 6765}, {20, 10946},
                          {21, 17711}, {22, 28657}, {23, 46368},
                          {24, 75025}, {25, 121393}, {26, 196418},
                          {27, 317811}, {28, 514229}, {29, 832040},
                          {30, 1346269}, {31, 2178309}, {32, 3524578}}),
              tile2index({{1, 1}, {2, 2}, {3, 3}, {5, 4}, {8, 5},
                          {13, 6}, {21, 7}, {34, 8}, {55, 9},
                          {89, 10}, {144, 11}, {233, 12}, {377, 13},
                          {610, 14}, {987, 15}, {1597, 16}, {2584, 17},
                          {4181, 18}, {6765, 19}, {10946, 20},
                          {17711, 21}, {28657, 22}, {46368, 23},
                          {75025, 24}, {121393, 25}, {196418, 26},
                          {317811, 27}, {514229, 28}, {832040, 29},
                          {1346269, 30}, {2178309, 31}, {3524578, 32}})
              {}
    board(const board& b) = default;
    board& operator =(const board& b) = default;

    std::array<int, 4>& operator [](const int& i) { return tile[i]; }
    const std::array<int, 4>& operator [](const int& i) const { return tile[i]; }
    int& operator ()(const int& i) { return tile[i / 4][i % 4]; }
    const int& operator ()(const int& i) const { return tile[i / 4][i % 4]; }

public:
    bool operator ==(const board& b) const { return tile == b.tile; }
    bool operator < (const board& b) const { return tile <  b.tile; }
    bool operator !=(const board& b) const { return !(*this == b); }
    bool operator > (const board& b) const { return b < *this; }
    bool operator <=(const board& b) const { return !(b < *this); }
    bool operator >=(const board& b) const { return !(*this < b); }

public:
    /**
     * apply an action to the board
     * return the reward gained by the action, or -1 if the action is illegal
     */
    int move(const int& opcode) {
        switch (opcode) {
        case 0: return move_up();
        case 1: return move_right();
        case 2: return move_down();
        case 3: return move_left();
        default: return -1;
        }
    }

    int move_left() {
        board prev = *this;
        int score = 0;
        for (int r = 0; r < 4; r++) {
            auto& row = tile[r];
            int top = 0, hold = 0;
            for (int c = 0; c < 4; c++) {
                int tile = row[c];
                if (tile == 0) continue;
                row[c] = 0;
                if (hold) {
                    if (mergeable(tile, hold)) {
                        int new_tile = std::max(tile, hold) + 1;
                        row[top++] = new_tile;
                        score += i2t(new_tile);
                        hold = 0;
                    } else {
                        row[top++] = hold;
                        hold = tile;
                    }
                } else {
                    hold = tile;
                }
            }
            if (hold) tile[r][top] = hold;
        }
        return (*this != prev) ? score : -1;
    }
    int move_right() {
        reflect_horizontal();
        int score = move_left();
        reflect_horizontal();
        return score;
    }
    int move_up() {
        rotate_right();
        int score = move_right();
        rotate_left();
        return score;
    }
    int move_down() {
        rotate_right();
        int score = move_left();
        rotate_left();
        return score;
    }

    void transpose() {
        for (int r = 0; r < 4; r++) {
            for (int c = r + 1; c < 4; c++) {
                std::swap(tile[r][c], tile[c][r]);
            }
        }
    }

    void reflect_horizontal() {
        for (int r = 0; r < 4; r++) {
            std::swap(tile[r][0], tile[r][3]);
            std::swap(tile[r][1], tile[r][2]);
        }
    }

    void reflect_vertical() {
        for (int c = 0; c < 4; c++) {
            std::swap(tile[0][c], tile[3][c]);
            std::swap(tile[1][c], tile[2][c]);
        }
    }

    /**
     * rotate the board clockwise by given times
     */
    void rotate(const int& r = 1) {
        switch (((r % 4) + 4) % 4) {
        default:
        case 0: break;
        case 1: rotate_right(); break;
        case 2: reverse(); break;
        case 3: rotate_left(); break;
        }
    }

    void rotate_right() { transpose(); reflect_horizontal(); } // clockwise
    void rotate_left() { transpose(); reflect_vertical(); } // counterclockwise
    void reverse() { reflect_horizontal(); reflect_vertical(); }

public:
    friend std::ostream& operator <<(std::ostream& out, const board& b) {
        char buff[32];
        out << "+------------------------+" << std::endl;
        for (int r = 0; r < 4; r++) {
            std::snprintf(buff, sizeof(buff), "|%6u%6u%6u%6u|",
                b.i2t(b[r][0]),
                b.i2t(b[r][1]),
                b.i2t(b[r][2]),
                b.i2t(b[r][3]));
            out << buff << std::endl;
        }
        out << "+------------------------+" << std::endl;
        return out;
    }

private:
    std::array<std::array<int, 4>, 4> tile;
    const std::unordered_map<int, int> index2tile, tile2index;
    int i2t(int i) const { return index2tile.at(i); }
    int t2i(int t) const { return tile2index.at(t); }
    bool mergeable(int i1, int i2) {
        return (i1 == 1 && i1 == i2) || i1 - i2 == 1 || i2 - i1 == -1;
    }
};
