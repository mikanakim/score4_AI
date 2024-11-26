#include <algorithm>
#include "masks.h"
#include "alpha_beta.h"
#include "time.h"
#include "global.h"
#include "hash.h"

using namespace std;

/**
 * alpha beta法 + 連鎖検出アルゴリズム
 * reach_aruとrensa_flagを用いて管理する
 * 通常探索する範囲(depth)までは探索。それを超えたときはrensa_flagがtrueのときのみ探索可能。
 * @param player player
 * @param idx idx
 * @param alpha alpha
 * @param beta beta
 * @param depth 探索深度
 * @param co_value pot_4_lineのスコア
 * @param rensa_flag 連鎖があるときtrue
 * @param reach_aru リーチがあるときtrue
 * @param max_depth 探索の最大深度
 * @param bestLine 最善手を打った場合のn手先までの着手予測
 * @return double: αβスコア
*/
double alpha_beta(int player, int idx, double alpha, double beta, int depth, int co_value, bool rensa_flag, bool reach_aru, int max_depth, vector<pair<int, int>>& bestLine){

    bestLine.clear();

    if (is_time_over()){
        time_over = true;
        return -10000000;  // 時間制限を超えた場合に探索を終了
    }
    
    int minus;// pot 4 lineのバグ予防
    int opp = player ^ 1;
    int l_action = legal_actions();
    double score;
    double original_alpha = alpha;
    idx &= 0x0000000F;

    //  リーチがあるとき
    if (reach_aru && rensa_start){
        uint64_t reach_player = reach_detector(player);
        uint64_t reach_opp = reach_detector(opp);

        // playerのリーチのときは勝ち
        if (reach_player != 0){
            return 10000 + __builtin_popcountll((~allMoves));//win
        }

        // 相手のリーチのときは防御
        else if (reach_opp != 0){
            int i = 63 - __builtin_clzll(reach_opp);
            i &= 0x0000000F;
            move(player, i);

            if (reach_count(player, i&0x0000000F)){
                reach_aru = true;
                rensa_flag = true;
            } 
            else{// リーチがない場合
                reach_aru = false;
                rensa_flag = true;
            }

            vector<pair<int, int>> line;
            score = -alpha_beta(opp, i, -beta, -alpha, depth-1, co_value, rensa_flag, reach_aru, max_depth, line);
            unmove(player, i);

            if(score > alpha){
                alpha = score;
                bestLine = line;  // 子ノードの最善手順を引き継ぐ
                bestLine.push_back({i, depth});
            }
            if(alpha >= beta){
                save_hash(alpha, beta, original_alpha, depth);
                return alpha;
            }
        }
        else{
            cout << "no reach" << endl;
        }

        // zobrist hashのために盤面保存
        save_hash(alpha, beta, original_alpha, depth);
        return alpha; 
    }

    else{
        // Zobrist hash に基づくキャッシュの確認
        if (transposition_table.find(current_hash) != transposition_table.end()) {
            TranspositionTableEntry entry = transposition_table[current_hash];

            // キャッシュが探索の深さ以下で、評価値が信頼できる場合は返す
            if (entry.depth <= depth) {
                if (entry.flag == EXACT) {
                    return entry.value;
                } else if (entry.flag == LOWER_BOUND && entry.value > alpha) {
                    alpha = entry.value;
                } else if (entry.flag == UPPER_BOUND && entry.value < beta) {
                    beta = entry.value;
                }
                if (alpha >= beta) {
                    return entry.value;
                }
            }
        }
        if (is_win(opp)) return -10000 - __builtin_popcountll((~allMoves));//lose
        if (is_end()) return 0;//draw

        // pot 4 lineのバグ対策
        if (is_ai2){
            if (player == 1) minus = -1;
            else minus = 1;
        }else{
            if (player == 0) minus = -1;
            else minus = 1;
        }

        // pot 4 lineで座標の勝ちを取得
        if (game_start) co_value += potential_4_line(opp, idx&0x0000000F) * minus;

        // 連鎖検出の時間短縮
        if (depth <= 0) l_action = reach_of_reach(opp);

        if (((depth <= 0) && (!rensa_flag)) || __builtin_popcountll(l_action) == 0){
            if (game_start) return -co_value;//最初数手は定石打ち

            return - line_evaluation(opp) - game_logic_evaluation(opp);
        }

        // move orderingの準備
        vector<pair<int, int>> moves; // (手のインデックス, スコア)
        for (int k = 0; k < 16; ++k) {
            if (l_action & (0b00000000000000010000000000000000 << k)) {
                move(player, k);
                moves.push_back({k, line_evaluation(player) + game_logic_evaluation(player)});
                unmove(player, k);
            }
        }

        // Move Ordering (スコア順にソート)
        sort(moves.begin(), moves.end(), [](const pair<int, int>& a, const pair<int, int>& b) {
            return a.second > b.second;
        });

        // リーチがないときのの探索
        for (auto& m : moves) {
            int i = m.first;
            i &= 0x0000000F;

            move(player, i);
            if (reach_count(player, i)){
                reach_aru = true;
                rensa_flag = true;
            } 
            else{
                reach_aru = false;
                rensa_flag = false;
            }

            vector<pair<int, int>> line;
            score = -alpha_beta(opp, i, -beta, -alpha, depth-1, co_value, rensa_flag, reach_aru, max_depth, line);
            unmove(player, i);

            if(score > alpha){
                alpha = score;
                bestLine = line;  // 子ノードの最善手順を引き継ぐ
                bestLine.push_back({i, depth});
            }
            if(alpha >= beta){
                save_hash(alpha, beta, original_alpha, depth);
                return alpha;
            }
        }
        save_hash(alpha, beta, original_alpha, depth);
        return alpha;
    } 
}

/**
 * For Research Purpose
 * 枝狩りの最適化をするための関数
 * リストの前n個を後ろにくっつける
 * @param moves リスト
 * @param n 何個動かすか
 * @return void
*/
void rotate_moves(vector<pair<int, int>>& moves, int n) {
    if (n <= 0 || n >= moves.size()) {
        cout << "Invalid value of n." << endl;
        return;
    }
    vector<pair<int, int>> temp(moves.begin(), moves.begin() + n);
    moves.erase(moves.begin(), moves.begin() + n);
    moves.insert(moves.end(), temp.begin(), temp.end());
}

/**
 * alpha beta法で探索した結果を用いて次の１手を出力する
 * @param player player
 * @param depth 探索の深さ
 * @param order_best move ordering用のscoreリストと最善手の組
 * @return Order_and_BestAction: move ordering用のscoreリストと最善手の組
*/
Order_and_BestAction alpha_beta_next_action(int player, int depth, Order_and_BestAction order_best){
    double alpha = -10000000.0;
    int best_action;
    int opp = player ^ 1;
    int l_action = legal_actions();
    vector<double> score_list = {};
    Order_and_BestAction result;

    vector<pair<int, int>> moves; // (手のインデックス, スコア)
    vector<pair<int, int>> bestLine;  // 理想的な着手順を保存するための変数
    
    // Move Ordering の準備
    if (order_best.order.size() == 0){
        for (int i = 0; i < 16; ++i) {
            if (l_action & (0b00000000000000010000000000000000 << i)) {
                move(player, i);
                int moveScore = line_evaluation(player) + game_logic_evaluation(player);
                unmove(player, i&0x0000000F);

                moves.push_back({i, moveScore});
            }
        }
    }else{
        moves = order_best.order;// iterative deepningの各深さでmove orderingする
    }

    // Move Ordering (スコア順にソート)
    sort(moves.begin(), moves.end(), [](const pair<int, int>& a, const pair<int, int>& b) {
        return a.second > b.second;
    });

    // 棋譜を出力
    kifu_output();


    for (auto& m : moves) {
        int i = m.first;
        i &= 0x0000000F;
        move(player, i);
        bool rensa_flag = false;
        bool reach_aru = false;

        if (reach_count(player, i)){
            reach_aru = true;
            rensa_flag = true;
        }
        else{
            reach_aru = false;
            rensa_flag = false;
        }

        double score = -alpha_beta(opp, i, -10000000, -alpha, depth, 0, rensa_flag, reach_aru, depth, bestLine);
        score_list.push_back(score);

        if (score > alpha){
            result.best = i;
            alpha = score;
            bestLine.push_back({i, depth+1});
        }

        unmove(player, i);

        if (is_time_over()) {
            time_over = true;
            break;  // 時間切れなのでループを抜ける
        }       

        // スコアを表示
        cout << i << ": " << score_list[score_list.size()-1];

        // 最善手を打った場合のn手先までの着手予測を表示
        if (bestLine.size() > 3){
            std::sort(bestLine.begin(), bestLine.end(), [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
                return a.second > b.second;
            });
            cout << " >> ";
            for (auto& move : bestLine) {
                cout << move.first << " ";
            }
            cout << endl;
        }
        else{
            cout << endl;
        }
       
        bestLine = {};
    }
    
    // 真の勝敗のときにのみ勝敗を表示させたい。
    // 不完全なので結構適当
    if ((alpha > 10000) && (__builtin_popcountll(allMoves) >= 36) && (depth > 10)){
        cout << "AIの勝ち" << endl;
    }
    else if ((alpha < -10000) && (__builtin_popcountll(allMoves) >= 36) && (depth > 10)){
        cout << "AIの負け" << endl;
    }
    cout <<  "Alpha = " << alpha << " " << endl;

    // 次回のmove ordering用に格納
    for (int i=0, n=score_list.size(); i<n; ++i){
        moves[i].second = score_list[i];
    }
    result.order = moves;

    return result;
}

/**
 * 反復深化
 * @param player player
 * @param max_depth 最大の探索深度
 * @return int: その深さでの最善手
*/
int iterative_deepening(int player, int max_depth) {
    int result;
    int best_action = -1;
    int l_action = legal_actions();
    vector<int> best_list = {};
    Order_and_BestAction ob = {0, {}};
    start_time = steady_clock::now();  // 探索開始時刻を記録

    if (__builtin_popcountll(allMoves) >= 0){// n手まではランダムに行動する。
        for (int depth = 0; depth < max_depth; depth+=2) {
            ob = alpha_beta_next_action(player, depth, ob);
            best_action = ob.best;

            // 時間切れで探索終了
            if (is_time_over() || time_over) {
                time_over = false;
                break;
            }
            best_list.push_back(best_action);
            cout << "Depth " << depth+1 << ": Best action = " << best_list[best_list.size()-1] << endl << endl;

        }
    }else{
        vector<int> la_list;
        for (int i=0; i<16; ++i){
            if ((l_action & (0x00010000 << i)) != 0){
                la_list.push_back(i);
            }
        }

        double ransu = dis(gen);
        int len_la = la_list.size();
        double tmp1 = 1.0/len_la;
        int randomIndex;

        // 0-1をlen_la等分し、ransuがどこに入るかで手を選択する。
        // こうすることで乱数の初期化による同一局面の繰り返しを回避する
        for (int i=0; i<len_la; ++i){
            if (ransu < tmp1*(i+1)){
                randomIndex = i;
                break;
            }
        }
        
        best_list.push_back(la_list[randomIndex]);
    }
    return best_list[best_list.size()-1];
}