#include "global.h"
#include "player.h"

/**
 * alpha beta探索を用いたplayer
 * @param player player
 * @param depth depth
 * @return void
*/
void BestPlayer(int player, int depth){
    int l_action = legal_actions();
    int idx;
    uint64_t reach_player = reach_detector(player);
    uint64_t reach_opp = reach_detector(player^1);

    if (reach_player != 0){
        cout << "reach!!!" << endl;
        idx = 63 - __builtin_clzll(reach_player);
    } else if (reach_opp != 0){
        cout << "reach block!!!" << endl;
        idx = 63 - __builtin_clzll(reach_opp);
    } else{
        idx = iterative_deepening(player, depth);
    }

    idx &= 0x0000000F;
    
    move(player, idx);
    kifu_input(idx);
    cout << "Eval: " << line_evaluation(player) << endl;
    cout << "stev: " << game_logic_evaluation(player) << endl;
    kifu_output();
    cout << "\nAI: " << idx << endl;
}

/**
 * Human用のplayer
 * @param player player
 * @return void
*/
void HumanPlayer(int player){
    int l_action = legal_actions();
    while (true){
        // legal actionを列挙
        cout << "l_action = [";
        for (int i = 0; i < 16; ++i){
            if (l_action & 0b00000000000000010000000000000000 << i){
                cout << i << ", ";
            }
        }
        cout << "]" << endl;

        cout << player << "座標を入力してください。100と打つと2手戻ります(待った機能)。" << endl;
        int idx;
        cin >> idx;

        int l_action = legal_actions();

        //100と打つと「待った」ができる
        if (idx == 100 && __builtin_popcountll(allMoves) >= 2){
            unmove(teban^1,kifu.back()&0x0000000F);
            kifu.pop_back();
            unmove(teban^1,kifu.back()&0x0000000F);
            kifu.pop_back();
            cout << "2手戻しました" << endl;
            printBoard(teban^1);
            continue;
        }

        if (idx < 0 || idx > 16) {
            cout << "Invalid move! Try again." << endl;
            continue;
        } else if (!check_inclusion(l_action, idx & 0x0000000F)){
            cout << "Not included in legal action! Try again.";
            continue;
        }

        move(player, idx);
        kifu_input(idx);
        cout << "Eval: " << line_evaluation(player) << endl;
        cout << "stev: " << game_logic_evaluation(player) << endl;
        kifu_output();
        break;

    }
}