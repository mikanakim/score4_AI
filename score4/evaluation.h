#include <iostream>

// game.cpp
uint64_t ground_detector();
uint64_t reach_detector(int player);
void diff_kst_detector(int player, int idx);
uint64_t get_valid_kst(int player, uint64_t kst_player, uint64_t kst_opp);
int reach_of_reach(int player);