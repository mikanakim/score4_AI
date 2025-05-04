#include <cmath>
#include <bitset>
#include <array>
#include <cstdint>
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
 * この関数はretu27.comさんのコードを参考にしました。
 * https://retu27.com/scorefour_cpu_nosupport.html
 * @param player player
 * @return double 得点
*/
double line_evaluation(int player){
    double result = 0.0;
    int renkoma, pattern;
    bool canset = false;

    const int opp = player ^ 1;

    const uint64_t ground = ground_detector();
    const uint64_t player_board = playerBoard[player];
    const uint64_t opp_board = playerBoard[opp];
    const uint64_t two_ren_player = two_ren_tate(player);
    const uint64_t two_ren_opp = two_ren_tate(opp);
    const uint64_t not_allMoves = ~allMoves;
    const int empty_count = __builtin_popcountll(not_allMoves);

    for (const uint64_t& mask : WIN_MASKS) {
        const uint64_t pmask = player_board & mask;
        const uint64_t omask = opp_board & mask;
        const uint64_t allmask = allMoves & mask;

        const int num0 = __builtin_popcountll(allmask);
        const int num1 = __builtin_popcountll(pmask);
        const int num2 = __builtin_popcountll(omask);

        if (num0 == 0) {
            pattern = 0;
            renkoma = 0;
        } else if (num0 == num1) {
            pattern = 1;
            renkoma = num1;
            if (renkoma == 3) {
                canset = (__builtin_popcountll(ground & (~pmask)) == 1);
            }
        } else if (num0 == num2) {
            pattern = 2;
            renkoma = num2;
            if (renkoma == 3) {
                canset = (__builtin_popcountll(ground & (~omask)) == 1);
            }
        } else {
            pattern = 3;
        }

        // 以下、評価値の加算（現状のロジックを簡略化せず維持）
        // 全て共通化できる条件をサブルーチンに切り出すとさらに綺麗にできる
        if (pattern == 0) continue;

        if (pattern == 1) {
            if (renkoma == 0) continue;
            else if (renkoma == 1) {
                if ((mask & LAYER_MASK[3]) == mask) {
                    result += (pmask & 0x0660000000000000) ? -15.0 :
                              (pmask & 0x6006000000000000) ? -5.0 : 1.0;
                } else {
                    result += 1.0;
                }

                if ((mask & LAYER_MASK[2]) == mask) {
                    result += 0.25;
                    if (two_ren_player & pmask) result += 0.75;
                } else if (mask & LAYER_MASK[2]) {
                    result += 0.2;
                    if ((two_ren_player >> 16) & pmask) {
                        result += 0.5;
                    }
                }
            } else if (renkoma == 2) {
                result += 2.5;
                bool isZ3 = (mask & LAYER_MASK[2]) == mask;
                if (player == 0 && ai_sente) {
                    if (isZ3) {
                        result += 3.0 + ((two_ren_player & pmask) ? 10.0 : 0.0);
                    } else if ((mask & LAYER_MASK[2]) && !(pmask & LAYER_MASK[2])) {
                        result += 3.0 + ((((two_ren_player >> 16) & pmask)) ? 10.0 : 0.0);
                    }
                } else {
                    if (isZ3) {
                        result += 1.5 + ((two_ren_player & pmask) ? 5.0 : 0.0);
                    } else if ((mask & LAYER_MASK[2]) && !(pmask & LAYER_MASK[2])) {
                        result += 1.5 + ((((two_ren_player >> 16) & pmask)) ? 5.0 : 0.0);
                    }
                }
            } else if (renkoma == 3) {
                if (__builtin_popcountll(reach_detector(player)) > 0)
                    result += 20.0;
            } else if (renkoma == 4) {
                return 10000.0 + empty_count;
            }
        } else if (pattern == 2) {
            if (renkoma == 0) continue;
            else if (renkoma == 1) {
                if ((mask & LAYER_MASK[3]) == mask) {
                    result += (omask & 0x0660000000000000) ? 15.0 :
                              (omask & 0x6006000000000000) ? 5.0 : -1.0;
                } else {
                    result += -1.0;
                }

                if ((mask & LAYER_MASK[2]) == mask) {
                    result += -2.5;
                    if (two_ren_opp & omask) result += -0.75;
                } else if (mask & LAYER_MASK[2]) {
                    result += -0.2;
                    if ((two_ren_opp >> 16) & omask) result += -0.5;
                }
            } else if (renkoma == 2) {
                result += -2.5;
                bool isZ3 = (mask & LAYER_MASK[2]) == mask;
                if (player == 0 && ai_sente) {
                    if (isZ3) {
                        result += -1.5 + ((two_ren_opp & omask) ? -5.0 : 0.0);
                    } else if ((mask & LAYER_MASK[2]) && !(omask & LAYER_MASK[2])) {
                        result += -1.5 + ((two_ren_opp >> 16 & omask) ? -5.0 : 0.0);
                    }
                } else {
                    if (isZ3) {
                        result += -0.75 + ((two_ren_opp & omask) ? -2.5 : 0.0);
                    } else if ((mask & LAYER_MASK[2]) && !(omask & LAYER_MASK[2])) {
                        result += -0.75 + ((two_ren_opp >> 16 & omask) ? -2.5 : 0.0);
                    }
                }
            }
        }
    }

    return result;
}

void get_surface(int player,
                 std::array<uint16_t, 10> &p_surface,
                 std::array<uint16_t, 10> &o_surface,
                 std::array<uint16_t, 10> &p_kst_lst,
                 std::array<uint16_t, 10> &o_kst_lst)
{
    const int opp = player ^ 1;
    const uint64_t player_board = playerBoard[player];
    const uint64_t opp_board = playerBoard[opp];
    const uint64_t p_kst = kst[player];
    const uint64_t o_kst = kst[opp];

    for (size_t s = 0; s < 10; ++s) {
        const uint64_t surface = SURFACE_MASK_WITHOUT_XY[s];

        uint16_t cp = 0;
        uint16_t co = 0;
        uint16_t kstp = 0;
        uint16_t ksto = 0;
        int cnt = 0;

        for (int i = 0; i < 64; ++i) {
            if ((surface >> i) & 1) {
                const uint16_t mask = 1u << cnt;
                if ((player_board >> i) & 1) cp |= mask;
                if ((opp_board >> i) & 1) co |= mask;
                if ((p_kst >> i) & 1) kstp |= mask;
                if ((o_kst >> i) & 1) ksto |= mask;
                ++cnt;
            }
        }

        p_surface[s] = cp;
        o_surface[s] = co;
        p_kst_lst[s] = kstp;
        o_kst_lst[s] = ksto;
    }
}

double surface_evaluation(int player) {
    // 固定長配列でstack領域利用（高速アクセス）
    std::array<uint16_t, 10> p_surface{}, o_surface{}, p_kst_lst{}, o_kst_lst{};
    get_surface(player, p_surface, o_surface, p_kst_lst, o_kst_lst); // ここは最適化対象外

    const uint16_t allMovesMask = ~allMoves;
    double value = 0.0;

    for (int i = 0; i < 10; ++i) {
        const uint16_t ps = p_surface[i];
        const uint16_t os = o_surface[i];
        const uint16_t pk = p_kst_lst[i];
        const uint16_t ok = o_kst_lst[i];

        for (size_t j = 0; j < PATTERN_RECOG1.size(); ++j) {
            const auto& p = PATTERN_RECOG1[j];
            if ((os & p[1]) != 0) continue;
            if ((ps & p[0]) != 0) continue;
            if ((allMovesMask & p[2]) != 0) continue;
            if ((pk & p[1]) != 0) continue;
            if (ok != 0) continue;

            // 重みをインライン化して条件削減
            value += (j < 6) ? 20.0 : (j < 20 ? 10.0 : 2.5);
            break; // 一致パターン1個でスコア加算、次のsurfaceへ
        }
    }

    return value;
}

// ゲームのロジックによる評価
/**
 * ゲームロジックに関わる勝利条件をベースにした評価関数
 * @param player player
 * @return double 得点
*/
inline int popcount(uint64_t x) { return __builtin_popcountll(x); }

double game_logic_evaluation(int player, int depth) {
    double value = 0;
    int opp = player ^ 1;
    uint64_t ground = ground_detector();
    uint64_t board_player = playerBoard[player];
    uint64_t board_opp = playerBoard[opp];
    uint64_t allMovesInv = ~allMoves;

    // z=2の連結部分（リーチ前提部分）
    uint64_t result_player = (board_player & (board_player << 16)) & LAYER_MASK[2];
    uint64_t result_opp = (board_opp & (board_opp << 16)) & LAYER_MASK[2];

    if (__builtin_popcountll(allMoves) == 0) return 0;

    uint64_t f_player = 0, f_opp = 0;

    // floating reach探索
    for (uint64_t mask : WIN_MASKS) {
        uint64_t overlap = mask & allMoves;
        if (popcount(overlap) != 3) continue;

        uint64_t tmp1 = mask & board_player;
        uint64_t tmp2 = mask & board_opp;

        if (popcount(tmp1) == 3) f_player |= mask ^ tmp1;
        if (popcount(tmp2) == 3) f_opp |= mask ^ tmp2;
    }

    uint64_t fp_tmp = f_player;
    uint64_t fo_tmp = f_opp;

    // 有効な決勝点（下に相手の球がないもの）
    f_player &= ~(fo_tmp << 16);
    f_opp &= ~(fp_tmp << 16);

    // ダブルリーチ（強制勝ち）検出
    int empty_count = popcount(allMovesInv);
    if ((f_player & (f_player >> 16)) != 0 || popcount(f_player & ground) >= 2)
        return 5000 + empty_count;

    if ((f_opp & (f_opp >> 16)) != 0 || popcount(f_opp & ground) >= 2)
        return -5000 - empty_count;

    int tmp_value = surface_evaluation(player) - surface_evaluation(opp);

    // cout << tmp_value << endl;

    value += tmp_value;

    // groundリーチ以外に限定
    f_player &= ~ground;
    f_opp &= ~ground;

    // --- ここからt点処理（省略せず残す） ---
    uint64_t gten_player = f_player & 0xFFFF0000FFFF0000;
    uint64_t gten_opp = f_opp & 0xFFFF0000FFFF0000;
    uint64_t ften_player = f_player & 0xFFFF000000000000;
    uint64_t ften_opp = f_opp & 0xFFFF000000000000;

    // 重複除去
    uint64_t tmp4 = gten_player & ~gten_opp;
    uint64_t tmp5 = gten_opp & ~gten_player;
    uint64_t tmp6 = (((tmp4 & LAYER_MASK[3]) >> 16) | ((tmp4 & LAYER_MASK[1]) << 16)) & LAYER_MASK[2];
    uint64_t tmp7 = (((tmp5 & LAYER_MASK[3]) >> 16) | ((tmp5 & LAYER_MASK[1]) << 16)) & LAYER_MASK[2];

    uint64_t tmp6_ften = ((ften_player & ~ften_opp) & LAYER_MASK[3]) >> 16 & LAYER_MASK[2];
    uint64_t tmp7_ften = ((ften_opp & ~ften_player) & LAYER_MASK[3]) >> 16 & LAYER_MASK[2];

    uint64_t two_ren_player = two_ren_tate(player);
    uint64_t two_ren_opp = two_ren_tate(opp);

    uint64_t broad1_t_player = 0, broad1_t_opp = 0;
    uint64_t broad2_t_player = 0, broad2_t_opp = 0;
    uint64_t broad3_t_player = 0, broad3_t_opp = 0;

    for (auto& tten_mask : TTEN) {
        int a = popcount(tmp6 & tten_mask);
        int b = popcount(two_ren_player & tten_mask);
        int c = popcount(allMoves & tten_mask);
        int d = popcount(playerBoard[player] & tten_mask);

        if (a == 2 && b == 1 && c == 1)
            broad1_t_player |= (~(tmp6 | two_ren_player)) & tten_mask;
        else if (a == 1 && b == 1 && c == 2 && d == 2)
            broad2_t_player |= (~(tmp6 | two_ren_player)) & tten_mask;

        a = popcount(tmp7 & tten_mask);
        b = popcount(two_ren_opp & tten_mask);
        c = popcount(allMoves & tten_mask);
        d = popcount(playerBoard[opp] & tten_mask);

        if (a == 2 && b == 1 && c == 1)
            broad1_t_opp |= (~(tmp7 | two_ren_opp)) & tten_mask;
        else if (a == 1 && b == 1 && c == 2 && d == 2)
            broad2_t_opp |= (~(tmp7 | two_ren_opp)) & tten_mask;

        // broad3
        if (popcount(tmp6_ften & tten_mask) == 3 && popcount(((~tmp6_ften) & tten_mask) & playerBoard[opp] & LAYER_MASK[2]) == 0)
            broad3_t_player |= (~tmp6_ften) & tten_mask;

        if (popcount(tmp7_ften & tten_mask) == 3 && popcount(((~tmp7_ften) & tten_mask) & playerBoard[player] & LAYER_MASK[2]) == 0)
            broad3_t_opp |= (~tmp7_ften) & tten_mask;
    }

    uint64_t broad_t_player = broad1_t_player | broad2_t_player | broad3_t_player;
    uint64_t broad_t_opp = broad1_t_opp | broad2_t_opp | broad3_t_opp;

    // 下に相手の決勝点または空きがあるかを確認
    uint64_t bottom_mask = (fo_tmp << 16) | ((~(allMoves & LAYER_MASK[1])) << 16);
    broad_t_player &= ~bottom_mask;

    bottom_mask = (fp_tmp << 16) | ((~(allMoves & LAYER_MASK[1])) << 16);
    broad_t_opp &= ~bottom_mask;

    // 重みを付加（以下は例、定数は要調整）
    value += popcount(broad_t_player) * 80;
    value -= popcount(broad_t_opp) * 80;

    return value;
}