#include <iostream>
#include <sstream>
#include <fstream>
#include <bitset>
#include <random>
#include <algorithm>
#include "main.h"
#include "global.h"
#include "hash.h"
#include "time.h"

using namespace std;

string save_filename = "./kifu/kifu.csv";

void save_kifu(int winner) {
    // ファイルを読み込むためのストリーム
    std::ifstream infile(save_filename);
    std::vector<std::string> existing_kifu = {};

    if (infile.is_open()) {
        std::string line;
        // ファイルの内容を1行ずつ読み込む
        while (std::getline(infile, line)) {
            existing_kifu.push_back(line);
        }
        infile.close();
    } else {
        std::cerr << "既存のファイルを開けませんでした: " << save_filename << std::endl;
    }

    // 現在の棋譜を1つの文字列として作成
    std::ostringstream current_kifu;
    for (int i = 0, n = kifu.size(); i < n; ++i) {
        current_kifu << kifu[i] << ",";
    }
    if (winner == 0) {
        current_kifu << "w";
    } else if (winner == 1) {
        current_kifu << "l";
    } else if (winner == 2){
        current_kifu << "d";
    }

    // 現在の棋譜が既存の棋譜リストに含まれるかを確認
    std::string current_kifu_str = current_kifu.str();
    if (std::find(existing_kifu.begin(), existing_kifu.end(), current_kifu_str) != existing_kifu.end()) {
        std::cout << "同じ棋譜が既に存在します。保存をスキップします。" << std::endl;
        return;
    }

    // 新しい棋譜を追記
    std::ofstream outfile(save_filename, std::ios::app);  // ファイルを追記モードで開く
    if (outfile.is_open()) {
        outfile << current_kifu_str << "\n";
        outfile.close();
    } else {
        std::cerr << "ファイルを開けませんでした: " << save_filename << std::endl;
    }
}


/**
 * 既存の棋譜を入力する
 * 入力形式は、半角数字スペース区切り↓
 * 「0 15 3 12 1 2」のような文字列
 * @return void
*/
void input_prekifu(){
    int num;
    vector<int> pre_kifu;
    string cin_line;
    cin.ignore();
    cout << "棋譜を入力してください(しない場合はreturn): " << endl;
    std::getline(cin, cin_line);
    std::istringstream iss(cin_line);
    while (iss >> num) {
        pre_kifu.push_back(num);
    }
    for (int i=0, n=pre_kifu.size(); i<n; ++i){
        move(teban, pre_kifu[i]&0x0000000F);
        kifu_input(pre_kifu[i]);
        kifu_output();
    }
    cout << "Eval: " << line_evaluation(teban) << endl;
    cout << "stev: " << game_logic_evaluation(teban) << endl;
}

int main(){
    init_zobrist_table();// hashの初期化
    initialize_random_generator();

    int vs;
    int depth = 6;// defaultの深さ
    int depth_input;
    int game_count = 1;// playするゲーム数
    int total_time = 300000;

    time_limit = 60000;

    // 対戦形式の入力
    cout << "AI vs AI: 0, AI vs Human: 1, research: 2" << endl;
    while (true){
        cin >> vs;
        if (vs == 0 || vs == 1 || vs == 2){
            break;
        }else{
            cout << "only accept 0, 1, 2" << endl;
        }
    }

    // 先手がどちらかを決める(AI vs AIのときは指定しない)
    if (vs == 0){
        teban = 0;
    }else{
        cout << "AI先手: 0, Human先手: 1" << endl;
        while (true){
            cin >> teban;
            if (teban == 0 || teban == 1){
                break;
            } else{
                cout << "Only accept 0 or 1" << endl;
            }
        }
    }
    
    // 探索の深さを指定する
    if (vs == 2){
        depth_input = depth;
    }else{
        cout << "depthを入力。defaultは7" << endl;
        cin >> depth_input;
        depth = depth_input;
        if (teban == 0){
            ai_sente = true;
        }else{
            ai_sente = false;
        }
    }

    // AI vs AIのときは試合数を入力
    if (vs == 0){
        cout << "game数: ";
        cin >> game_count;
    }

    // 事前に棋譜がある場合は入力
    input_prekifu();

    // gameスタート
    int count = 0;
    while (count < game_count){
        if (count > 0){
            teban = 0;
            allMoves = 0; 
            playerBoard = {0, 0};
            is_ai2 = false;
            ai_sente = true;
            rensa_start = false;
            kifu = {};
            depth = depth_input;
            transposition_table.clear();
        }
        while (true){
            printBoard(0);
            cout << "盤面情報" << endl;
            cout << std::showbase;
            cout << "AI: " << std::hex << playerBoard[0] << endl;
            cout << "opp: " << std::hex << playerBoard[1] << endl;
            cout << std::noshowbase << std::dec; /* 解除 */
            cout << __builtin_popcountll(allMoves)+1 << "手目" << endl;
            int l_action = legal_actions();

            // n手目以降で連鎖検出をする。バグ対策でもあるので変更しない。
            if (__builtin_popcountll(allMoves) >= 8){
                rensa_start = true;
            }

            // 6手目までは、pot4lineで簡易的に定石手順を打つ
            if (__builtin_popcountll(allMoves) <= 6){
                game_start = true;
            } else{
                game_start= false;
            }

            // AI vs AI
            if (vs == 0){
                int tesu = __builtin_popcountll(allMoves);
                int remain = __builtin_popcountll(legal_actions());

                // potential 4 line関数がバグらないようにする処置
                if (teban == 1){
                    is_ai2 = true;
                }
                else{
                    is_ai2 = false;
                }

                // 探索の深さと上限時間
                if (6 < tesu && tesu <= 30){
                    depth = depth_input+2;
                    time_limit = 120000;
                } else if (30  < tesu && tesu <= 64-comp_search){
                    depth = depth_input+4;
                    time_limit = 180000;
                } else if (64-comp_search < tesu){
                    time_limit = 180000;
                    if (tesu%2 == 0) depth = 64-tesu;
                    else if (tesu%2 == 1) depth = 64-(tesu-1);
                }
                BestPlayer(teban, depth);
            }

            // AI vs Human
            else if (vs == 1){
                if (teban == 1){
                    HumanPlayer(teban);
                }else{
                    int tesu = __builtin_popcountll(allMoves);
                    int remain = __builtin_popcountll(legal_actions());

                    // 探索の深さと上限時間
                    if (6 < tesu && tesu <= 24){
                        depth = depth_input+2;
                        time_limit = 120000;
                    } else if (24  < tesu && tesu < 64-comp_search){
                        depth = depth_input+4;
                        time_limit = 90000;
                    } else if (64-comp_search <= tesu){
                        time_limit = 90000;
                        if (tesu%2 == 0) depth = 64-tesu;
                        else if (tesu%2 == 1) depth = 64-(tesu-1);
                    } 
                    BestPlayer(teban, depth);
                }
            }

            // For Research Purpose
            else if (vs == 2){
                time_limit = 10000000;// 時間制限なしで深読みさせる
                if (teban == 1){
                    HumanPlayer(teban);
                }else{
                    cout << "探索の深さ: ";
                    cin >> depth;
                    int tesu = __builtin_popcountll(allMoves);
                    int remain = __builtin_popcountll(legal_actions());
                    BestPlayer(teban, depth);
                }
            }

            // 勝敗判定
            if (teban ^ 1 == 0) {
                if (is_win(teban^1)) {
                    printBoard(0);
                    cout << "Player " << (teban^1) << " wins!" << endl;
                    if (!ai_sente){// AIが先手のとき
                    // if (__builtin_popcountll(allMoves) % 2 == 0){//後手番で終了
                        save_kifu(1);
                    }else{
                        save_kifu(0);
                    }
                    break;
                }
            } else {
                if (is_win(teban^1)) {
                    printBoard(0);
                    cout << "Player " << (teban^1) << " wins!" << endl;
                    // if (__builtin_popcountll(allMoves) % 2 == 0){//後手番で終了
                    if (ai_sente){// AIが先手のとき
                        save_kifu(1);
                    }else{
                        save_kifu(0);
                    }
                    break;
                }
            }

            // 引き分け判定
            if (allMoves == 0xFFFFFFFFFFFFFFFF) { // 勝敗無しで全てのマスが埋まったら引き分け
                printBoard(0);
                cout << "It's a draw!" << endl;
                save_kifu(2);
                break;
            }
        }
        count += 1;
    }
    return 0;
}


