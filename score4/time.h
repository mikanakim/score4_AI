#include <chrono>

using namespace std::chrono;

#ifndef TIME_H
#define TIME_H

// グローバルで時間を管理
extern steady_clock::time_point start_time;
extern int time_limit;  // ミリ秒単位で時間制限
extern bool time_over;

#endif