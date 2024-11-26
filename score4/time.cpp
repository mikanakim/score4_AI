#include "time.h"
#include "global.h"

// グローバルで時間を管理
steady_clock::time_point start_time;
int time_limit;  // ミリ秒単位で時間制限
bool time_over;

/**
 * 時間をチェックするヘルパー関数
 * @return bool 時間超過ならtrue
*/
bool is_time_over() {
    auto now = steady_clock::now();
    auto elapsed_time = duration_cast<milliseconds>(now - start_time).count();
    return elapsed_time > time_limit;
}