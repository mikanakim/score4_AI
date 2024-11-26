#include <iostream>
#include <vector>
#include <random>
#include <bitset>
#include "global.h"
#include "masks.h"
#include "game.h"

using namespace std;

/**
 * ビットボードの表示
 * @param player player
 * @return void
*/
void printBoard(int player) {
    for (int i = 0; i < 64; ++i) {
        int tmpi;
        if (0 <= i && i < 16){
            tmpi = 48+(i&0x0000000F);
        }else if (16 <= i && i < 32){
            tmpi = 32+(i&0x0000000F);
        }else if (32 <= i && i < 48){
            tmpi = 16+(i&0x0000000F);
        }else{
            tmpi = (i&0x0000000F);
        }
        
        if (playerBoard[player] & (1ULL << tmpi)) {
            cout << "O ";
        } else if (playerBoard[player^1] & (1ULL << tmpi)) {
            cout << "X ";
        } else {
            cout << ". ";
        }
        if (i % 4 == 3) cout << endl;
        if (i % 16 == 15) cout << endl;
    }

    cout << "-- guide --" << endl;
    cout << " 0  1  2  3" << endl;
    cout << " 4  5  6  7" << endl;
    cout << " 8  9 10 11" << endl;
    cout << "12 13 14 15" << endl << endl;
}

/**
 * 勝利条件
 * @param player player
 * @return bool playerが勝利したらtrue
*/
bool is_win(int player) {
    uint64_t board = playerBoard[player];
    for (uint64_t mask : WIN_MASKS) {
        if ((board & mask) == mask) {
            return true;
        }
    }
    return false;
}

/**
 * 引き分け判定
 * @return bool 引き分けならtrue
*/
bool is_end(){
    if (allMoves == 0xFFFFFFFFFFFFFFFF){
        return true;
    }
    return false;
}

/**
 * playerがidxに球を置く
 * @param player player
 * @param idx idx
 * @return 球を実際に置けたらtrue
*/
bool move(int player, int idx){
    update_zobrist_hash(idx, player);
    for (int i = 0; i < 4; ++i){
        if ((allMoves & (1ULL << (16*i + idx))) == 0) {
            playerBoard[player] |= (1ULL << (16*i + idx));
            allMoves |= (1ULL << (16*i + idx));
            teban ^= 1;
            return true;
        }
    }
    cout << "重複しています。選択し直してください。" << endl;
    return false;
}

/**
 * playerのidxをもとに戻す
 * @param player player
 * @param idx idx
 * @return void
*/
void unmove(int player, int idx){
    update_zobrist_hash(idx, player);
    for (int i = 0; i < 4; ++i){
        int irev = 3 - i;
        if ((allMoves & (1ULL << (16*irev + idx))) != 0){
            playerBoard[player] ^= (1ULL << (16*irev + idx));
            allMoves ^= (1ULL << (16*irev + idx));
            teban ^= 1;
            return;
        }
    }    
}

/**
 * numberがvecに含まれているかどうか
 * @param vec 検索されるベクトル
 * @param number vector内で検索する数字
 * @return bool 含まれていればtrue
*/
bool check_inclusion(int vec, int number){
    if (vec & (1UL << number) == 0){
            return false;
        }
    return true;
}

/**
 * 合法手を探す
 * @return int 置ける場所を32bit maskで返す。allMovesをbit反転して、z=4についてmaskで検出し、32bit右シフト。
*/
int legal_actions(){
    return ((~allMoves) & 0xFFFF000000000000) >> 32;
}

/**
 * 合法手の正確な場所検出。実際に球をおける座標を抽出する。
 * @return uint64_t 64bitでgroundの場所のmaskを返す
*/
uint64_t ground_detector(){
    uint64_t ground = (((allMoves << 16) | 0x000000000000FFFF) ^ allMoves);
    return ground;
}

/**
 * main関数で使用するgroundリーチ検出関数。複数リーチがあっても1つのリーチの座標のみを検出する。
 * @param player player
 * @return uint64_t ground reachの場所1つを64bit maskで返す
*/
uint64_t reach_detector(int player){
    uint64_t ground = ground_detector();
    for (uint64_t mask : WIN_MASKS){
        if (__builtin_popcountll(mask & ground) == 1){
            if (__builtin_popcountll((playerBoard[player] & mask)) == 3){
                return mask & ground;
            }
        }
    }
    return 0;
}

/**
 * 敵味方関係なく、リーチがあるかどうかだけを高速で判定する関数。
 * @param player player
 * @param idx idx
 * @return bool リーチが存在すればtrue
*/
bool reach_count(int player, int idx){
    uint64_t ground = ground_detector();

    for (int i = 0; i < 4; ++i){
        int irev = 3 - i;
        if ((allMoves & (1ULL << (16*irev + idx))) != 0) {
            idx = idx + 16*irev;
            break;
        }
    }

    // idxにおいたとき、リーチができうる場所はidxをlineに含む場所 + idx+16をlineに含む場所
    for (auto& id_mask: POT_4_MASK[idx]){
        if (__builtin_popcountll(id_mask & ground) == 1){
            if (__builtin_popcountll((playerBoard[player] & id_mask)) == 3){
                return true;
            }
        }
    }

    if (idx < 48){
        for (auto& id_mask: POT_4_MASK[idx+16]){
            if (__builtin_popcountll(id_mask & ground) == 1){
                if (__builtin_popcountll((playerBoard[player] & id_mask)) == 3){
                    return true;
                } else if (__builtin_popcountll((playerBoard[player^1] & id_mask)) == 3){
                    return true;
                }
            }
        }
    }
    
    return false;
}

/**
 * alpha beta用の高速reach detector。
 * 最後においたidxを利用して、それに関連するリーチのみ検出するようにする。
 * @param player player
 * @param idx idx
 * @return vector<uint64_t> playerとoppのリーチを64bit maskとして返す
*/
vector<uint64_t> reach_detector_alpha_beta(int player, int idx){ // playerがidxに置いた。
    uint64_t ground = ground_detector();
    uint64_t player_reach = 0;
    uint64_t opp_reach = 0;

    for (int i = 0; i < 4; ++i){
        int irev = 3 - i;
        if ((allMoves & (1ULL << (16*irev + idx))) != 0) {
            idx = idx + 16*irev;
            break;
        }
    }

    // idxにおいたとき、リーチができうる場所はidxをlineに含む場所 + idx+16をlineに含む場所
    for (auto& id_mask: POT_4_MASK[idx]){
        if (__builtin_popcountll(id_mask & ground) == 1){
            if (__builtin_popcountll((playerBoard[player] & id_mask)) == 3){
                player_reach = id_mask & ground;
                break;
            }
        }
    }
    if (idx < 48){
        for (auto& id_mask: POT_4_MASK[idx+16]){
            if (__builtin_popcountll(id_mask & ground) == 1){
                if ((player_reach != 0) && (opp_reach != 0)){
                    break;
                    if ((__builtin_popcountll((playerBoard[player] & id_mask)) == 3)){
                        player_reach = id_mask & ground;
                    }
                    if ((__builtin_popcountll((playerBoard[player^1] & id_mask)) == 3)){
                        opp_reach = id_mask & ground;
                    }
                }
            }
        }
        
    }
    return {player_reach, opp_reach};
}

/**
 * 連鎖検出のために、リーチになりそうなlineを見つける関数。
 * @param player player
 * @return int 32bit maskで場所を返す
*/
int reach_of_reach(int player){
    uint64_t result = 0;
    uint64_t ground = ground_detector();

    // z方向以外: groundに2つあり空欄2つ、playerの球2つ
    for (auto& mask: WIN_MASKS){
        if (__builtin_popcountll(mask & ground) == 2){
            if (__builtin_popcountll((~allMoves & mask)) == 2){// 空きマスが2つある
                if (__builtin_popcountll((playerBoard[player] & mask)) == 2){// playerの玉が２つ
                    result |= mask & ground;
                }
            }

        // z方向：groundに1つあり、空欄に2つ、playerの玉2つ
        } else if (__builtin_popcountll(mask & ground) == 1){
            for (auto& id_mask: IDX_MASK){
                if ((mask & id_mask) == mask){// z方向のlineを検出
                    if (__builtin_popcountll((~allMoves & mask)) == 2){
                        if (__builtin_popcountll((playerBoard[player] & mask)) == 2){
                            result |= mask & ground;
                        }
                    }
                }
            }
        }
    }
    // 下位16bitをFの場所に集める。
    result |= (result >> 16);
    result |= (result >> 16);
    result |= (result << 16);

    // 32bitで返す
    return 0xFFFF0000 & result;
}

/**
 * 棋譜を作成する
 * @param idx idx
 * @return void
*/
void kifu_input(int idx){
    for (int i = 0; i < 4; ++i){
        int irev = 3 - i;
        if ((allMoves & (1ULL << (16*irev + idx))) != 0){
            kifu.push_back(16*irev + idx);
            return;
        }
    } 
}

/**
 * 棋譜を出力する
 * @return void
*/
void kifu_output(){
    cout << "棋譜: ";
    for (int i=0, n=kifu.size(); i<n; ++i){
        if (i == n-1)
            cout << kifu[i] << endl;
        else cout << kifu[i] << " ";
    }
    return;
}