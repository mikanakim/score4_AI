#include <random>
#include <iostream>
#include <vector>
#include <unordered_map>
#include "hash.h"

#ifndef GLOBAL_H
#define GLOBAL_H

using namespace std;

extern int teban;
extern int comp_search;
extern bool  ai_sente;
extern bool game_start;
extern bool rensa_start;
extern bool is_ai2;
extern double threshold;
extern uint64_t allMoves; 
extern vector<int> kifu;
extern vector<uint64_t> playerBoard;
extern unordered_map<uint64_t, TranspositionTableEntry> transposition_table;

extern std::mt19937 gen;  // 乱数生成器を宣言（外部から使用可能にする）
extern std::uniform_real_distribution<> dis;
void initialize_random_generator();  // 乱数生成器の初期化関数

#endif