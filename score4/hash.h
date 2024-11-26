#include <unordered_map>

using namespace std;

#ifndef HASH_H  // インクルードガードの開始
#define HASH_H

// Zobrist Hash
#define BOARD_SIZE 64  // 盤面サイズ
#define NUM_PLAYERS 2  // 2プレイヤー

extern uint64_t zobrist_table[BOARD_SIZE][NUM_PLAYERS];  // Zobrist ハッシュテーブルを2次元配列として宣言
extern uint64_t current_hash;  // 現在のボードのハッシュ

enum Flag {
    EXACT,        // 正確な値 (α < スコア < β)
    LOWER_BOUND,  // スコアが β より大きい
    UPPER_BOUND   // スコアが α より小さい
};

struct TranspositionTableEntry {
    int value;
    int depth;
    int flag;  // EXACT, LOWER_BOUND, UPPER_BOUND
};

#endif

