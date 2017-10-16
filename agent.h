#pragma once
#include <string>
#include <random>
#include <sstream>
#include <map>
#include <cmath>
#include <utility>
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
        else {
            weights.push_back(weight(SIZE));
            weights.push_back(weight(SIZE));
        }
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

            std::array<std::pair<size_t, size_t>, 8> ielist = get_idx_entry_list(episode[i].after);
            for (std::pair<size_t, size_t> ie : ielist) {
                if (ie.second >= SIZE) {
                    std::cout << "index out of bound (maybe achieved unexpected larger tile)" << std::endl;
                    continue;
                }

                weights[ie.first][ie.second] += delta;
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
        std::array<std::pair<size_t, size_t>, 8> ielist = get_idx_entry_list(b);
        for (std::pair<size_t, size_t> ie : ielist) {
            if (ie.second >= SIZE)
                continue;
            value += weights[ie.first][ie.second];
        }
        return value;
    }

    std::array<std::pair<size_t, size_t>, 8> get_idx_entry_list(const board& b) {
        std::array<std::pair<size_t, size_t>, 8> ielist;
        board r = b;
        ielist[0] = std::make_pair(0, get_entry(r[0]));
        ielist[1] = std::make_pair(1, get_entry(r[1]));
        ielist[2] = std::make_pair(1, get_entry(r[2]));
        ielist[3] = std::make_pair(0, get_entry(r[3]));
        r.rotate_right();
        ielist[4] = std::make_pair(0, get_entry(r[0]));
        ielist[5] = std::make_pair(1, get_entry(r[1]));
        ielist[6] = std::make_pair(1, get_entry(r[2]));
        ielist[7] = std::make_pair(0, get_entry(r[3]));
        return ielist;
    }

    /*
     *outer outer outer outer
     *inner inner inner inner
     *inner inner inner inner
     *outer outer outer outer
     */
    size_t get_entry(const std::array<int, 4>& row) {
        size_t entry = 0;
        if (row[0] > row[3] || (row[0] == row[3] && row[1] > row[2])) {
            for (int i = 3; i >= 0; i--) {
                entry *= TILENUMBER;
                entry += row[i];
            }
        }
        else {
            for (int i = 0; i < 4; i++) {
                entry *= TILENUMBER;
                entry += row[i];
            }
        }
        if (entry >= SIZE) {
            std::cout << "index out of bound" << std::endl;
            std::cout << "the row: " << row[0] << ' ' << row[1] << ' ' << row[2] << ' ' << row[3] << std::endl;
        }

        return entry;
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
