#include <iostream>

// game.cpp
void printBoard(int player);
int legal_actions();
bool check_inclusion(int vec, int number);
uint64_t reach_detector(int player);
bool move(int player, int idx);
void unmove(int player, int idx);
void kifu_input(int idx);
void kifu_output();

// evaluation.cpp
double line_evaluation(int player);
double game_logic_evaluation(int player);


// alpha_beta.cpp
int iterative_deepening(int player, int max_depth);