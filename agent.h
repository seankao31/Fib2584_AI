#pragma once
#include <string>
#include <random>
#include <sstream>
#include <map>
#include <cmath>
#include <type_traits>
#include <algorithm>
#include "board.h"
#include "action.h"
#include "weight.h"

#define TILENUMBER 24

class agent {
public:
    agent(const std::string& args = "") {
        std::stringstream ss(args);
        for (std::string pair; ss >> pair; ) {
            std::string key = pair.substr(0, pair.find('='));
            std::string value = pair.substr(pair.find('=') + 1);
            property[key] = { value };
        }
    }
    virtual ~agent() {}
    virtual void open_episode(const std::string& flag = "") {}
    virtual void close_episode(const std::string& flag = "") {}
    virtual action take_action(const board& b) { return action(); }
    virtual bool check_for_win(const board& b) { return false; }

public:
    virtual std::string name() const {
        auto it = property.find("name");
        return it != property.end() ? std::string(it->second) : "unknown";
    }
protected:
    typedef std::string key;
    struct value {
        std::string value;
        operator std::string() const { return value; }
        template<typename numeric, typename = typename std::enable_if<std::is_arithmetic<numeric>::value, numeric>::type>
        operator numeric() const { return numeric(std::stod(value)); }
    };
    std::map<key, value> property;
};

/**
 * evil (environment agent)
 * add a new random tile on board, or do nothing if the board is full
 * 2-tile: 90%
 * 4-tile: 10%
 */
class rndenv : public agent {
public:
    rndenv(const std::string& args = "") : agent("name=rndenv " + args) {
        if (property.find("seed") != property.end())
            engine.seed(int(property["seed"]));
    }

    virtual action take_action(const board& after) {
        int space[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
        std::shuffle(space, space + 16, engine);
        for (int pos : space) {
            if (after(pos) != 0) continue;
            std::uniform_int_distribution<int> popup(0, 9);
            int tile = popup(engine) ? 1 : 2;
            return action::place(tile, pos);
        }
        return action();
    }

private:
    std::default_random_engine engine;
};

class player : public agent {
public:
    player(const std::string& args = "") : agent("name=player " + args), alpha(0.0025f) {
        episode.reserve(32768);
        if (property.find("seed") != property.end())
            engine.seed(int(property["seed"]));
        if (property.find("alpha") != property.end())
            alpha = float(property["alpha"]);

        if (property.find("load") != property.end())
            load_weights(property["load"]);
        else
            weights.push_back(weight(2 * SIZE));
    }
    ~player() {
        if (property.find("save") != property.end())
            save_weights(property["save"]);
    }

    virtual void open_episode(const std::string& flag = "") {
        episode.clear();
        episode.reserve(32768);
    }

    virtual void close_episode(const std::string& flag = "") {
        // TODO: train the n-tuple network by TD(0)
        //for (state s: episode) {
            //std::cout << s.reward << std::endl;
            //std::cout << s.after << std::endl << std::endl;
        //}

        //std::cout << "reward: " << episode.back().reward << std::endl;
        //std::cout << "ignore final board" << std::endl;
        //std::cout << "====================" << std::endl;

        for (int i = episode.size()-2; i >= 0; i--) {

            //std::cout << "reward: " << episode[i].reward << std::endl;
            //std::cout << episode[i].after << std::endl << std::endl;

            float delta = alpha * (episode[i+1].reward + episode[i+1].value - episode[i].value);

            //std::cout << delta << " = " << alpha << " * (" << episode[i+1].reward << " + " << episode[i+1].value << " - " <<  episode[i].value << ")" << std::endl;
            //std::cout << "apply to the following:" << std::endl;

            std::array<size_t, 8> lidx = get_idx_list(episode[i].after);
            for (size_t idx : lidx) {
                if (idx >= 2*SIZE) {
                    std::cout << "index out of bound (maybe achieved unexpected larger tile)" << std::endl;
                    continue;
                }

                // print debug begin
                //size_t pidx = idx;
                //std::cout << pidx << std::endl;
                //if (pidx >= SIZE) {
                    //std::cout << "inner ";
                    //pidx -= SIZE;
                //}
                //else
                    //std::cout << "outer ";
                //for (int i = 0; i < 4; ++i) {
                    //std::cout << pidx % TILENUMBER << ",";
                    //pidx /= TILENUMBER;
                //}
                //std::cout << std::endl;
                // print debug end

                weights.back()[idx] += delta;
                episode[i].value += delta;
            }
        }
    }

    virtual action take_action(const board& before) {
        action best;
        state s;
        s.reward = 0;
        s.value = 0;

        float highest = - INFINITY;
        int opcode[] = {0, 1, 2, 3};
        for (int op : opcode) {
            board b = before;
            int score = b.move(op);
            if (score != -1) {
                float value = get_value(b);

                if (value + score > highest) {
                    // TODO == comparison for float is not precise
                    highest = value + score;
                    best = action::move(op);
                    s.value = value;
                    s.after = b;
                    s.reward = score;
                    //s.op = op;
                }
            }
        }
        // if can't move, best remain nothing

        episode.push_back(s);
        return best;
    }

public:
    virtual void load_weights(const std::string& path) {
        std::ifstream in;
        in.open(path.c_str(), std::ios::in | std::ios::binary);
        if (!in.is_open()) std::exit(-1);
        size_t size;
        in.read(reinterpret_cast<char*>(&size), sizeof(size));
        weights.resize(size);
        for (weight& w : weights)
            in >> w;
        in.close();
    }

    virtual void save_weights(const std::string& path) {
        std::ofstream out;
        out.open(path.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
        if (!out.is_open()) std::exit(-1);
        size_t size = weights.size();
        out.write(reinterpret_cast<char*>(&size), sizeof(size));
        for (weight& w : weights)
            out << w;
        out.flush();
        out.close();
    }

private:
    float get_value(const board& b) {
        float value = 0;
        std::array<size_t, 8> lidx = get_idx_list(b);
        for (size_t idx : lidx) {
            if (idx >= 2*SIZE)
                continue;
            value += weights.back()[idx];
        }
        return value;
    }

    std::array<size_t, 8> get_idx_list(const board& b) {
        std::array<size_t, 8> lidx;
        board r = b;
        lidx[0] = get_idx(r[0], 0);
        lidx[1] = get_idx(r[1], 1);
        lidx[2] = get_idx(r[2], 1);
        lidx[3] = get_idx(r[3], 0);
        r.rotate_right();
        lidx[4] = get_idx(r[0], 0);
        lidx[5] = get_idx(r[1], 1);
        lidx[6] = get_idx(r[2], 1);
        lidx[7] = get_idx(r[3], 0);
        return lidx;
    }

    /*
     *outer outer outer outer
     *inner inner inner inner
     *inner inner inner inner
     *outer outer outer outer
     */
    size_t get_idx(const std::array<int, 4>& row, bool inner) {
        size_t idx = 0;
        if (row[0] > row[3] || (row[0] == row[3] && row[1] > row[2])) {
            for (int i = 3; i >= 0; i--) {
                idx *= TILENUMBER;
                idx += row[i];
            }
        }
        else {
            for (int i = 0; i < 4; i++) {
                idx *= TILENUMBER;
                idx += row[i];
            }
        }
        if (idx >= SIZE) // should be out of bound
            idx += SIZE;

        if (inner)
            idx += SIZE;

        if (idx >= 2*SIZE) {
            std::cout << "index out of bound" << std::endl;
            std::cout << "the row: " << row[0] << ' ' << row[1] << ' ' << row[2] << ' ' << row[3] << std::endl;
        }

        return idx;
    }

private:
    std::vector<weight> weights;

    struct state {
        board after;
        float value;
        int reward;
    };

    std::vector<state> episode;
    float alpha;

private:
    std::default_random_engine engine;
    unsigned int SIZE = pow(TILENUMBER, 4);
};
