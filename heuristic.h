#pragma once
#include <algorithm>
#include "board.h"
#include "action.h"
#include "index_tile_convertion.h"

#include <iostream>

class heuristic {
public:
    heuristic(board& b, const action& act, const std::array<int, 7>& coef = {64, 32, 16, 8, 4, 2, 1}) :
                                             b(b), opcode(act), coef(coef) {}
    heuristic(board& b, const int& op = -1, const std::array<int, 7>& coef = {64, 32, 16, 8, 4, 2, 1}) :
                                              b(b), opcode(op), coef(coef) {}
    operator int() const { return value; }

    int estimate() {
        score = b.move(opcode);
        empty = 0;
        value = 0;
        int max_value = 0;
        /**
         *
         * coef{0, 1, 2, 3, 4, 5, 6}
         *  (0)  (1)  (2)  (3)
         *  (1)  (2)  (3)  (4)
         *  (2)  (3)  (4)  (5)
         *  (3)  (4)  (5)  (6)
         *
         */
        // this block consider one additional move
        // simply assume player can move 2 times in a row
        for (int op = 0; op < 4; op++) {
            board b_move = b;
            int score2 = b_move.move(op);
            for (int rt = 0; rt < 4; rt++) {
                value = 0;
                board b_rotate = b_move;
                b_rotate.rotate(rt);
                for (int r = 0; r < 4; r++) {
                    for (int c = 0; c < 4; c++) {
                        value += coef[r+c] * b_rotate[r][c];
                        if(b_rotate[r][c] == 0)
                            empty++;
                    }
                }
                value += 10 * score2;
                value += 15 * empty;
                max_value = std::max(max_value, value);
            }
        }
        value = max_value;

        // --- this block consider only this next move
        // for (int rt = 0; rt < 4; rt++) {
        //     value = 0;
        //     board b_rotate = b;
        //     b_rotate.rotate(rt);
        //     for (int r = 0; r < 4; r++) {
        //         for (int c = 0; c < 4; c++) {
        //             value += coef[r+c] * b_rotate[r][c];
        //         }
        //     }
        //     max_value = std::max(max_value, value);
        // }
        // for (int i = 0; i < 16; i++) {
        //     if (b(i) == 0)
        //         empty++;
        // }
        // value = max_value;
        // value += 10 * score;
        // value += 15 * empty;
        // ---

        value = (score == -1)? score: value;
        // std::cout << value << std::endl;
        // std::cout << std::endl << value << std::endl << b;
        return value;
    }

private:
    board b;
    const int opcode;
    std::array<int, 7> coef;
    int value; // final heuristic value
    int empty; // number of empty tiles
    int score; // score gained by this move
};
