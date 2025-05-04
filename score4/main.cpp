#include <iostream>
#include <sstream>
#include <fstream>
#include <bitset>
#include <random>
#include <algorithm>
#include <cstdlib>
#include "main.h"
#include "global.h"
#include "hash.h"
#include "time.h"

using namespace std;

string save_filename = "./kifu/kifu.csv";
string opening_book_filename = "./kifu/opening_book_with_human_made_book.csv";

// pythonコードを動かす
void runPythonScript(const std::string& script, const std::string& args) {
    std::string command = "python3 " + script + " --slist " + args;
    std::system(command.c_str());
}

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
    std::ostringstream python_args_kifu;
    for (int i = 0, n = kifu.size(); i < n; ++i) {
        current_kifu << kifu[i] << ",";
        python_args_kifu << kifu[i] << " ";
    }
    if (winner == 0) {
        current_kifu << "w";
        python_args_kifu << "w";
    } else if (winner == 1) {
        current_kifu << "l";
        python_args_kifu << "l";
    } else if (winner == 2){
        current_kifu << "d";
        python_args_kifu << "d";
    }

    cout << python_args_kifu.str() << endl;

    string save_or_not = "n";
    cout << "棋譜を保存する: y/n" << endl;
    cin >> save_or_not;
    // save_or_not = "y";
    if (save_or_not == "y"){
        runPythonScript("./kifu/auto_analysis_kifu.py", python_args_kifu.str());
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
        kifu_input(pre_kifu[i]&0x0000000F);
        kifu_output();
    }
    cout << "Eval: " << line_evaluation(teban) << endl;
    cout << "stev: " << game_logic_evaluation(teban, 0) << endl;
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
        if (teban == 0){
            ai_sente = true;
        }else{
            ai_sente = false;
        }
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

    // 定石をロード
    loadOpeningBook(opening_book_filename);

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
            kifu_for_book.clear();
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
            if (__builtin_popcountll(allMoves) >= 8){// 普通は8
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
                    time_limit = 90000;
                } else if (30  < tesu && tesu <= 64-comp_search){
                    depth = depth_input+4;
                    time_limit = 90000;
                } else if (64-comp_search < tesu){
                    time_limit = 90000;
                    if (tesu%2 == 0) depth = 64-tesu;
                    else if (tesu%2 == 1) depth = 64-(tesu-1);
                }
                BestPlayer(teban, depth, true);
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
                        time_limit = 60000;
                    } else if (24  < tesu && tesu < 64-comp_search){
                        depth = depth_input+4;
                        time_limit = 60000;
                    } else if (64-comp_search <= tesu){
                        time_limit = 60000;
                        if (tesu%2 == 0) depth = 64-tesu;
                        else if (tesu%2 == 1) depth = 64-(tesu-1);
                    } 
                    BestPlayer(teban, depth, true);
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
                    BestPlayer(teban, depth, false);
                }
            }

            // 勝敗判定
            if (teban ^ 1 == 0) {
                if (is_win(teban^1)) {
                    printBoard(0);
                    cout << "Player " << (teban^1) << " wins!" << endl;
                    if (!ai_sente){
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
                    if (ai_sente){
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
    transposition_table.clear();
    kifu_for_book.clear();
    return 0;
}


