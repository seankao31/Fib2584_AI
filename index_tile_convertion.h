#pragma once
#include <unordered_map>

const std::unordered_map<int, int>
    index2tile({{0, 0}, {1, 1}, {2, 2}, {3, 3}, {4, 5}, {5, 8},
                {6, 13}, {7, 21}, {8, 34}, {9, 55},
                {10, 89}, {11, 144}, {12, 233}, {13, 377},
                {14, 610}, {15, 987}, {16, 1597}, {17, 2584},
                {18, 4181}, {19, 6765}, {20, 10946},
                {21, 17711}, {22, 28657}, {23, 46368},
                {24, 75025}, {25, 121393}, {26, 196418},
                {27, 317811}, {28, 514229}, {29, 832040},
                {30, 1346269}, {31, 2178309}, {32, 3524578}}),
    tile2index({{0, 0}, {1, 1}, {2, 2}, {3, 3}, {5, 4}, {8, 5},
                {13, 6}, {21, 7}, {34, 8}, {55, 9},
                {89, 10}, {144, 11}, {233, 12}, {377, 13},
                {610, 14}, {987, 15}, {1597, 16}, {2584, 17},
                {4181, 18}, {6765, 19}, {10946, 20},
                {17711, 21}, {28657, 22}, {46368, 23},
                {75025, 24}, {121393, 25}, {196418, 26},
                {317811, 27}, {514229, 28}, {832040, 29},
                {1346269, 30}, {2178309, 31}, {3524578, 32}});

int i2t(int i) { return index2tile.at(i); }
int t2i(int t) { return tile2index.at(t); }