// hash.cpp
void init_zobrist_table();

// game.cpp
bool move(int player, int idx);
void printBoard(int player);
int legal_actions();
bool is_win(int player);
void kifu_input(int idx);
void kifu_output();

// player.cpp
void BestPlayer(int player, int depth);
void HumanPlayer(int player);

// evaluation.cpp
double line_evaluation(int player);
double game_logic_evaluation(int player);