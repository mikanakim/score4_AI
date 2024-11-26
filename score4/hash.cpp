#include <vector>
#include <random>
#include <string>
#include "global.h"
#include "hash.h"

using namespace std;

// Zobrist Hash
uint64_t zobrist_table[BOARD_SIZE][NUM_PLAYERS];  // Zobrist ハッシュテーブル
uint64_t current_hash = 0;  // 現在のボードのハッシュ

/**
 * ランダムなハッシュ値の生成・初期化関数
 * @return void
*/
void init_zobrist_table() {
    random_device rd;
    mt19937_64 gen(rd());
    uniform_int_distribution<uint64_t> dis(0, UINT64_MAX);
    
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < NUM_PLAYERS; ++j) {
            zobrist_table[i][j] = dis(gen);
        }
    }
}

/**
 * playerがidxに置いたとき、盤面のhashを再計算
 * @param idx idx
 * @param player player
 * @return void
*/
void update_zobrist_hash(int idx, int player) {
    for (int i = 0; i < 4; ++i){
        int irev = 3 - i;
        if ((allMoves & (1ULL << (16*irev + idx))) != 0) {
            idx = idx + 16*irev;
            break;
        }
    }
    current_hash ^= zobrist_table[idx][player];  // 石を置く・取り除く
}

/**
 * 盤面をhash化して保存する
 * @param alpha alpha
 * @param beta beta
 * @param original_alpha もともとのalpha
 * @param depth 探索深度
 * @return void
*/
void save_hash(double alpha, double beta, double original_alpha, int depth){
    TranspositionTableEntry new_entry;
    new_entry.value = alpha;  // 最終的な評価値
    new_entry.depth = depth;
    if (alpha <= original_alpha) {
        new_entry.flag = UPPER_BOUND;
    } else if (alpha >= beta) {
        new_entry.flag = LOWER_BOUND;
    } else {
        new_entry.flag = EXACT;
    }
    transposition_table[current_hash] = new_entry;
}