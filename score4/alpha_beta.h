#include <iostream>
#include <vector>
#include <cstdint>

using namespace std;
#ifndef alphabeta_H  // インクルードガードの開始
#define alphabeta_H

struct Order_and_BestAction {
    int best;
    vector<pair<int, int>> order;
};

int result_time_over;

// time.cpp
bool is_time_over();

// game.cpp
int legal_actions();
uint64_t reach_detector(int player);
bool move(int player, int idx);
void unmove(int player, int idx);
bool reach_count(int player, int idx);
bool is_win(int player);
bool is_end();
int reach_of_reach(int player);
void kifu_output();

// evaluation.cpp
int potential_4_line(int player, int idx);
double line_evaluation(int player);
double game_logic_evaluation(int player, int depth);

// hash.cpp
void save_hash(double alpha, double beta, double original_alpha, int depth);

#endif