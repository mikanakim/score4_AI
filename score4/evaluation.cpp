#include <cmath>
#include<bitset>
#include "masks.h"
#include "evaluation.h"
#include "global.h"

using namespace std;

// 静的評価関数
// idx: 1桁
// line: 1-2桁
// 決勝点: 3桁
// game logic: 4桁
// 勝敗: 5桁

/**
 * idxのpot 4 lineを出力する
 * @param player player
 * @param idx idx
 * @return int idxがもつpotential lineの数
*/
int potential_4_line(int player, int idx){
    int result = 0;

    // idxのzはいくつなのか調べて適切なidxに変換
    for (int i = 0; i < 4; ++i){
        int irev = 3 - i;
        if ((allMoves & (1ULL << (16*irev + idx))) != 0) {
            idx = idx + 16*irev;
            break;
        }
    }

    // 自分のlineのみを探索するように変更する。
    uint64_t player_board = playerBoard[player];
    uint64_t opp_board = playerBoard[player^1];

    for (auto& pot_mask: POT_4_MASK[idx]){
        uint64_t player_mask = player_board & pot_mask;
        uint64_t opp_mask = opp_board & pot_mask;
        if(((player_mask | opp_mask) == player_mask) || ((player_mask | opp_mask) == opp_mask)){
            result += 1;
        }
    }
    return result;
}

/**
 * z=2, 3の縦両取りをしているところを検出
 * @param player player
 * @return uint64_t z=3に縦2連がある場所の64bit mask
*/
uint64_t two_ren_tate(int player){
    uint64_t board = playerBoard[player];
    uint64_t result = (board & (board << 16));
    result &= LAYER_MASK[2];
    return result;
}

/**
 * lineごとに評価する
 * この関数はretsu27.comさんのコードを参考にしました。
 * https://retu27.com/scorefour_cpu_nosupport.html
 * @param player player
 * @return double 得点
*/
double line_evaluation(int player){
    double result = 0;
    int pattern;
    int renkoma;
    bool canset;

    uint64_t ground = ground_detector();
    uint64_t player_board = playerBoard[player];
    uint64_t opp_board = playerBoard[player^1];
    uint64_t two_ren_player = two_ren_tate(player);
    uint64_t two_ren_opp = two_ren_tate(player^1);

    for (auto& mask: WIN_MASKS){
        uint64_t player_mask = player_board & mask;
        uint64_t opp_mask = opp_board & mask;

        int num0 = __builtin_popcountll(allMoves & mask);
        int num1 = __builtin_popcountll(player_mask);
        int num2 = __builtin_popcountll(opp_mask);

        // pattern、renkoma、cansetを設定。
        if(num0 == 0){// lineにコマが0
            pattern = 0;
            renkoma = 0;
        }else{
            if (num0 == num1){// lineのコマがplayerのみ
                pattern = 1;
                renkoma = num1;
                if (num1 == 3){
                    canset = __builtin_popcountll(ground & (~player_mask)) == 1;
                }
            }else if (num0 == num2){// lineのコマがoppのみ
                pattern = 2;
                renkoma = num2;
                if (num2 == 3){
                    canset = __builtin_popcountll(ground & (~opp_mask)) == 1;
                }
            }
            else{
                pattern = 3;
            }
        }

        // スコアリング
        if (pattern == 0){// lineに球がない
            result += 0.0;
        }
        else if(pattern == 1){// lineの球がplayerのみ
            if (renkoma == 0) result += 0.0;

            // max 2.0
            else if (renkoma == 1) {
                    if (((mask & LAYER_MASK[3]) == mask) && ((player_mask & 0x0660000000000000) != 0)){// z=4かつ0x0660000000000000にあるとき、得点を少なくする。
                        result += 0.1;
                    }else {
                        result += 1.0;// default
                    }
                    if ((mask & LAYER_MASK[2]) == mask){// z=3にlineがある場合は加点。method1のt点を想定
                        result += 0.25;
                        if ((two_ren_player & player_mask) != 0){// 縦2連の3段目なら強いので加点
                            result += 0.75;
                        }
                    } else if ((mask & LAYER_MASK[2]) != 0){// method2。lineがz=3と一致せず、z=3を含む→斜めを検出してる
                        result += 0.2;
                        if (((two_ren_player >> 16) & player_mask) != 0){// method2によるt点は縦2連のz=2が強い
                            for (auto& l_mask: LAYER_MASK){
                                if (l_mask != mask){
                                    result += 0.5;
                                    break;
                                }
                            }
                        }
                    }
                }

            // max 10.5
            // AIが先手の場合: 先手のt点は強いので点数高めに設定
            else if (renkoma == 2) {
                result += 2.5;
                //オリジナル評価値
                if ((player == 0) && (ai_sente)){//
                    if ((mask & LAYER_MASK[2]) == mask){// lineがz=3にある場合加点
                        result += 3.0;
                        if ((two_ren_player & player_mask) != 0){// method1によるt点は縦2連のz=3があると強いのでpattern3のリーチ作りよりも優先される
                            result += 5.0;
                        }
                    }
                    else if ((mask & LAYER_MASK[2]) != 0){// z=3を含む斜めに2つあれば加点
                        if ((player_mask & LAYER_MASK[2]) == 0){
                            result += 2.0;
                            if (((two_ren_player >> 16) & player_mask) != 0){// method2によるt点は縦2連のz=2が強い
                                for (auto& l_mask: LAYER_MASK){
                                    if (l_mask != mask){
                                    result += 6.0;// method2の弱点がなくなったので合計でmethod1と同じ強さになるようにした
                                    }
                                }
                            }
                        }
                    }
                }
                else{// AIが後手の場合
                    if ((mask & LAYER_MASK[2]) == mask){
                        result += 1.5;
                        if ((two_ren_player & player_mask) != 0){// method1によるt点は縦2連のz=3があると強いので1.5倍の点数
                            result += 2.5;// 同じ比率で圧縮
                        }
                    }
                    else if ((mask & LAYER_MASK[2]) != 0){// z=3を含む斜めに2つあれば加点。
                        if ((player_mask & LAYER_MASK[2]) == 0){
                            result += 1.0;
                            if (((two_ren_player >> 16) & player_mask) != 0){// method2によるt点は縦2連のz=2が強い
                                for (auto& l_mask: LAYER_MASK){
                                    if (l_mask != mask){
                                        result += 3.0;// method2の弱点がなくなったので合計でmethod1と同じ強さになるようにした
                                    }
                                }
                            }
                        }
                    }
                }
            } 
            else if (renkoma == 3) {
                // 決勝点の評価は別関数で行うので一旦default値のみ加点
                result += 5.0;
            }
            else if (renkoma == 4) return 10000.0 + __builtin_popcountll((~allMoves));
        }
        else if (pattern == 2){// lineの球がoppのみ。
            if (renkoma == 0) result += 0.0;
            else if (renkoma == 1){
                if (((mask & LAYER_MASK[3]) == mask) && ((opp_mask & 0x0660000000000000) != 0)){// z=4かつ0x0660000000000000にあるとき、得点を半分にする
                    result += -0.1;
                }else {
                    result += -1.0;
                }
                if ((mask & LAYER_MASK[2]) == mask){// z=3にlineがある場合
                    result += -2.5;
                    if ((two_ren_opp & opp_mask) != 0){// method1によるt点は縦2連のz=3があると強い
                        result += -0.75;
                    }
                } else if ((mask & LAYER_MASK[2]) != 0){// z=3を含む斜めに2つあれば加点。
                    result += -0.2;
                    if (((two_ren_opp >> 16) & opp_mask) != 0){// method2によるt点は縦2連のz=2があると強い
                        for (auto& l_mask: LAYER_MASK){
                            if (l_mask != mask){
                                result += -0.5;
                                break;
                            }
                        }
                    }
                }
            }
            else if (renkoma == 2) {
                result += -2.5;
                if ((player == 0) && (ai_sente)){//AIが先手の場合: 先手のt点は強いので後手より点数高めに設定
                    if ((mask & LAYER_MASK[2]) == mask){
                        result += -1.5;
                        if ((two_ren_opp & opp_mask) != 0){// method1によるt点は縦2連のz=3があると強いので1.5倍の点数
                            result += -2.5;// 同じ比率で圧縮
                        }
                    }
                    else if ((mask & LAYER_MASK[2]) != 0){// z=3を含む斜めに2つあれば加点。
                        if ((opp_mask & LAYER_MASK[2]) == 0){
                            result += -1.0;
                            if (((two_ren_opp >> 16) & opp_mask) != 0){// method2によるt点は縦2連のz=2が強い
                                for (auto& l_mask: LAYER_MASK){
                                    if (l_mask != mask){
                                        result += -3.0;// method2の弱点がなくなったので合計でmethod1と同じ強さになるようにした
                                    }
                                }
                            }
                        }
                    }
                }
                else{// AIが後手の場合
                    if ((mask & LAYER_MASK[2]) == mask){
                        result += -3.0;
                        if ((two_ren_opp & opp_mask) != 0){// method1によるt点は縦2連のz=3があると強いので1.5倍の点数
                            result += -5.0;
                        }
                    }
                    else if ((mask & LAYER_MASK[2]) != 0){// z=3を含む斜めに2つあれば加点。
                        if ((opp_mask & LAYER_MASK[2]) == 0){
                            result += -2.0;
                            if (((two_ren_opp >> 16) & opp_mask) != 0){// method2によるt点は縦2連のz=2が強い
                                for (auto& l_mask: LAYER_MASK){
                                    if (l_mask != mask){
                                        result += -6.0;// method2の弱点がなくなったので合計でmethod1と同じ強さになるようにした
                                    }
                                }
                            }
                        }
                    }
                }
                
            }
            else if (renkoma == 3) {
                if (canset) result += -200;
                else {
                    result += -7.5;
                }
            }
            else if (renkoma == 4) return -10000.0 - __builtin_popcountll((~allMoves));
        }
        else if (pattern == 3){
            result += 0;
        }
    }

    return result;
}

// ゲームのロジックによる評価
/**
 * ゲームロジックに関わる勝利条件をベースにした評価関数
 * @param player player
 * @return double 得点
*/
double game_logic_evaluation(int player){
    double value = 0;
    int opp = player ^1;
    uint64_t ground = ground_detector();
    uint64_t board_player = playerBoard[player];
    uint64_t board_opp = playerBoard[opp];
    uint64_t result_player = (board_player & (board_player << 16));
    uint64_t result_opp = (board_opp & (board_opp << 16));

    result_player &= LAYER_MASK[2];
    result_opp &= LAYER_MASK[2];

    // 各種決勝点の数を数える
    if (__builtin_popcount(allMoves) > 0){
        uint64_t f_player = 0; 
        uint64_t fp_tmp = 0; 
        uint64_t f_opp = 0;
        uint64_t fo_tmp = 0; 
        int ts = 0;
        int tg = 0;
        int gg = 0;
        int nn = 0;//重複t点

        // floating reach(決勝点)探し
        for (uint64_t mask: WIN_MASKS){
            if ((__builtin_popcountll(mask & allMoves)) == 3){// allMovesと3つ被っているwin_maskを抽出
                uint64_t tmp1 = mask & board_player;
                uint64_t tmp2 = mask & board_opp;
                if ((__builtin_popcountll(tmp1)) == 3){// その3つがplayerのみから構成されることを確認
                    f_player |= ((~tmp1) & mask);// 決勝点(仮)が存在する場所にbitを立てる。
                }
                if ((__builtin_popcountll(tmp2)) == 3){
                    f_opp |= ((~tmp2) & mask);
                }
            }
        }
        fp_tmp = f_player;
        fo_tmp = f_opp;

        // 下に相手の決勝点があるような無効な決勝点は除く。
        f_player &= (~(fo_tmp << 16));
        f_opp &= (~(fp_tmp << 16));

        // ダブルリーチは勝敗と同等の点数を与える。
        // playerのz方向のダブルリーチ
        // 上下にplayerの決勝点が隣接して存在し、かつその直下にはoppの決勝点が存在しない
        if (((f_player & (f_player >> 16)) & (~f_opp)) != 0){
            value += 10000;
            return value;
        }

        // playerのground reachが2個以上あるようなダブルリーチ
        if (__builtin_popcountll(f_player & ground) >= 2){
            value += 10000;
            return value;
        }

        // oppのz方向のダブルリーチ
        if (((f_opp & (f_opp >> 16)) & (~f_player)) != 0){
            value += -10000;
            return value;
        }

        // oppのground reachが2個以上あるようなダブルリーチ
        if (__builtin_popcountll(f_opp & ground) >= 2){
            value += -10000;
            return value;
        }

        // groundリーチを除き、floating reachのみにする。
        f_player &= (~ground);
        f_opp &= (~ground);

        // t点の価値を決める
        // 先手と後手でt点の強さが変わるのでそれを考慮
        if ((player == 0) && (ai_sente)){
            value += ((__builtin_popcountll(f_player & LAYER_MASK[2])) - (__builtin_popcountll(f_opp & LAYER_MASK[2]))) * 100;
        }else{
            value += ((__builtin_popcountll(f_player & LAYER_MASK[2])) - (__builtin_popcountll(f_opp & LAYER_MASK[2]))*2) * 100;
        }
        
        // 重複t点が存在するか
        if (__builtin_popcountll((f_player & LAYER_MASK[2]) & (f_opp & LAYER_MASK[2])) != 0){
            nn = __builtin_popcountll((f_player & LAYER_MASK[2]) & (f_opp & LAYER_MASK[2]));
        }

        // ts, tg, ggの数
        if (ai_sente && (player == 0)){
            ts = __builtin_popcountll(f_player & LAYER_MASK[2]);
            tg = __builtin_popcountll(f_opp & LAYER_MASK[2]);
            gg = __builtin_popcountll(f_opp & LAYER_MASK[1]) + __builtin_popcountll(f_opp & LAYER_MASK[3]);
        } else{
            ts = __builtin_popcountll(f_opp & LAYER_MASK[2]);
            tg = __builtin_popcountll(f_player & LAYER_MASK[2]);
            gg = __builtin_popcountll(f_player & LAYER_MASK[1]) + __builtin_popcountll(f_player & LAYER_MASK[3]);
        }   

        double tmp_v = 0;
        // ゲームがある程度進行したら以下の評価も加算
        if ((ts > 0) || (tg > 0) || (gg > 0) || (__builtin_popcountll(allMoves) > 24)){
            // ロジスティック曲線。30手で約500、64手で約2000になるように調整。
            tmp_v = (2000/(1+999*pow(2.718, (-1.0/5.0)*__builtin_popcountll(allMoves))));
        }

        if (!(ai_sente & (player == 0))){// AIが先手のとき
            tmp_v *= -1;
        }

        // 立体四目並べの勝利条件
        // 局面Yからの評価
        if (nn <= 0){
            if (ts > 0){
                if (tg-ts > 1){
                    value += -tmp_v;
                }else if ((tg-ts == 0) || (tg-ts == 1)){
                    if (gg > 0){
                        value += -tmp_v;
                    }
                } else if (tg-ts < 0){
                    value += tmp_v;
                }
            }
            else if (ts == 0){
                if ((tg > 2) || (gg > 0)){
                    value += -tmp_v;
                }
            }
        }
        else if (nn > 0){
            if (tg-ts > 0){
                value += -tmp_v;
            }
            else if (tg-ts == 0){
                if ((nn & 0x00000001) == 0){
                    value += -tmp_v;
                }
                else{
                    value += tmp_v;
                }
            }
            else if (tg-ts < 0){
                value += tmp_v;
            }
        }
    }

    return value;
}