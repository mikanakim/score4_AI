#include <random>
#include "global.h"

// ゲームの管理
int teban = 0;              //現在の手番
uint64_t allMoves = 0;      // 全盤面情報
vector<int> kifu = {};      // 棋譜
vector<int> kifu_for_book(64, 0);
vector<uint64_t> playerBoard = {0, 0};  // 64bitで盤面を表現するbitboard
vector<uint64_t> kst = {0, 0};  // 決勝点の絶対座標を管理するbit
vector<vector<uint64_t>> kst_history = {};
map<vector<int>, OpeningMove> openingBook;

// alpha beta探索
int comp_search = 26;      // 終盤に何手完全読みするか
double threshold = 1.0;    // 通常の探索を行う確率。
bool game_start;            // trueのときに、pot4lineを使用する(初手の定石打ち)
bool rensa_start = false;   // trueで連鎖解析を行う
bool is_ai2 = false;        // AI vs AIでpot4lineがバグらないようにする
bool ai_sente;              // aiが先手ならtrue(AI vs Humanのとき)

// hash関数
unordered_map<uint64_t, TranspositionTableEntry> transposition_table;

// 乱数生成
std::mt19937 gen;   
std::uniform_real_distribution<> dis(0.0, 1.0);
void initialize_random_generator() {
    std::random_device rd;  // シード値として使う
    gen.seed(rd());         // 乱数生成器にシード値を設定
}