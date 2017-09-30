#pragma once
#include <algorithm>
#include <set>
#include "board.h"
#include "action.h"
#include "index_tile_convertion.h"

#include <iostream>

class heuristic {
public:
    heuristic(board& b, const action& act) : b(b), opcode(act),
                                             coef({64, 32, 16, 8, 4, 2, 1}) {}
    heuristic(board& b, const int& op = -1) : b(b), opcode(op),
                                              coef({64, 32, 16, 8, 4, 2, 1}) {}
    operator int() const { return value; }

    int estimate() {
        score = b.move(opcode);
        empty = 0;
        value = 0;
        max_index = 0;
        std::set<int> max_pos;
        /**
         *
         * pos: means times to rotate to align with coef
         *  (0)  (0)  (3)  (3)
         *  (0)  (0)  (3)  (3)
         *  (1)  (1)  (2)  (2)
         *  (1)  (1)  (2)  (2)
         *
         */
        // for (int i = 0; i < 16; i++) {
        //     if (b(i) > max_index) {
        //         max_index = b(i);
        //         max_pos.clear();
        //         max_pos.insert(position(i));
        //     }
        //     else if (b(i) == max_index) {
        //         max_pos.insert(position(i));
        //     }
        //     else continue;
        // }
        int max_value = 0;
        // for (int pos : max_pos) {
        for (int pos = 0; pos < 4; pos++) {
            value = 0;
            board b_rotate = b;
            b_rotate.rotate(pos);
            for (int r = 0; r < 4; r++) {
                for (int c = 0; c < 4; c++) {
                    value += coef[r+c] * b_rotate[r][c];
                }
            }
            max_value = std::max(max_value, value);
        }
        value = max_value;
        // for (int i = 0; i < 16; i++) {
        //     max_index = std::max(max_index, b(i));
        //     if (b(i) == 0)
        //         empty++;
        // }
        max_tile = i2t(max_index);
        // value += score;
        value += 10 * score;
        value = (score == -1)? score: value;
        // std::cout << value << std::endl;
        // std::cout << std::endl << value << std::endl << b;
        return value;
    }

private:
    board b;
    const int opcode;
    int value; // final heuristic value
    int empty; // number of empty tiles
    int score; // score gained by this move
    int max_index;
    int max_tile;
    std::array<int, 7> coef;
    const int position (const int& i) const {
        switch (i) {
        default:
        case 0: case 1: case 4: case 5: return 0;
        case 8: case 9: case 12: case 13: return 1;
        case 10: case 11: case 14: case 15: return 2;
        case 2: case 3: case 6: case 7: return 3;
        }
    }
};
