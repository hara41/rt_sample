#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <lgpio.h>
#include <time.h>
#include <sched.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

/**
 * PREEMPT_RT対応 リアルタイムボタン入力検出プログラム
 * GPIO27のボタン入力を高精度で検出します
 * 実行: sudo ./button_input
 */

#define BUTTON_PIN 27  // GPIO27
#define BOUNCE_TIME_NS 20000000  // バウンス時間（20ms = 20,000,000ns）
#define POLL_INTERVAL_NS 1000000  // ポーリング間隔（1ms = 1,000,000ns）
#define RT_PRIORITY 70  // リアルタイム優先度

static volatile int running = 1;
static int handle = -1;

/**
 * シグナルハンドラー
 */
void signal_handler(int signum)
{
    (void)signum;
    running = 0;
}

/**
 * 絶対時刻までスリープ（時刻ドリフトを防ぐ）
 */
static inline void sleep_until(struct timespec *target_time)
{
    while (clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, target_time, NULL) != 0) {
        // EINTR の場合は再試行
    }
}

/**
 * timespecに指定ナノ秒を加算
 */
static inline void timespec_add_ns(struct timespec *ts, long long ns)
{
    ts->tv_nsec += ns;
    while (ts->tv_nsec >= 1000000000L) {
        ts->tv_nsec -= 1000000000L;
        ts->tv_sec++;
    }
}

/**
 * 相対時間スリープ（バウンス対策用）
 */
static inline void relative_sleep(long nanoseconds)
{
    struct timespec req, rem;
    req.tv_sec = nanoseconds / 1000000000L;
    req.tv_nsec = nanoseconds % 1000000000L;
    
    while (clock_nanosleep(CLOCK_MONOTONIC, 0, &req, &rem) != 0) {
        req = rem;
    }
}

/**
 * リアルタイムスレッドの設定
 * エラーが発生しても警告を出して続行する
 */
int setup_realtime(void)
{
    struct sched_param param;
    int rt_enabled = 0;
    int memlock_enabled = 0;
    
    if (mlockall(MCL_CURRENT | MCL_FUTURE) == 0)
    {
        memlock_enabled = 1;
        printf("✓ メモリロック有効\n");
    }
    else
    {
        fprintf(stderr, "⚠ メモリロック失敗: %s\n", strerror(errno));
        fprintf(stderr, "  (sudo権限で実行するとリアルタイム性能が向上します)\n");
    }
    
    param.sched_priority = RT_PRIORITY;
    if (sched_setscheduler(0, SCHED_FIFO, &param) == 0)
    {
        rt_enabled = 1;
        printf("✓ リアルタイムスケジューリング有効 (優先度: %d, SCHED_FIFO)\n", RT_PRIORITY);
    }
    else
    {
        fprintf(stderr, "⚠ リアルタイムスケジューリング失敗: %s\n", strerror(errno));
        fprintf(stderr, "  (sudo権限で実行するとリアルタイム性能が向上します)\n");
    }
    
    if (!rt_enabled && !memlock_enabled)
    {
        printf("\n標準モードで動作します\n");
        printf("高精度モードにするには: sudo ./button_input\n");
    }
    else if (rt_enabled && memlock_enabled)
    {
        printf("\n✓ フルリアルタイムモードで動作\n");
    }
    else
    {
        printf("\n部分的なリアルタイムモードで動作\n");
    }
    
    return 0;
}

int main(void)
{
    int button_state, prev_state = 1;
    struct timespec press_time, release_time;
    long long press_count = 0;
    
    printf("=== PREEMPT_RT対応 リアルタイムボタン入力検出プログラム ===\n");
    printf("ポーリング間隔: %.3f ms\n", POLL_INTERVAL_NS / 1000000.0);
    
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    setup_realtime();
    
    handle = lgGpiochipOpen(0);
    if (handle < 0)
    {
        fprintf(stderr, "GPIOチップのオープンに失敗: %d\n", handle);
        return 1;
    }
    
    if (lgGpioClaimInput(handle, 32, BUTTON_PIN) < 0)
    {
        fprintf(stderr, "GPIO27の設定に失敗\n");
        lgGpiochipClose(handle);
        return 1;
    }
    
    printf("GPIO27の設定完了\n");
    printf("ボタン入力を監視しています。Ctrl+Cで終了します\n\n");
    
    // 絶対時刻ベースのポーリング
    struct timespec next_poll_time;
    clock_gettime(CLOCK_MONOTONIC, &next_poll_time);
    timespec_add_ns(&next_poll_time, POLL_INTERVAL_NS);
    
    while (running)
    {
        button_state = lgGpioRead(handle, BUTTON_PIN);
        
        // ボタンが押された（HIGHからLOWへの変化）
        if (button_state == 0 && prev_state == 1)
        {
            clock_gettime(CLOCK_MONOTONIC, &press_time);
            press_count++;
            printf("[%lld] ボタンが押されました (時刻: %ld.%09ld)\n",
                   press_count, press_time.tv_sec, press_time.tv_nsec);
            
            relative_sleep(BOUNCE_TIME_NS);
        }
        
        // ボタンが離された（LOWからHIGHへの変化）
        if (button_state == 1 && prev_state == 0)
        {
            clock_gettime(CLOCK_MONOTONIC, &release_time);
            long long duration_ns = (release_time.tv_sec - press_time.tv_sec) * 1000000000LL +
                                    (release_time.tv_nsec - press_time.tv_nsec);
            
            printf("[%lld] ボタンが離されました (押下時間: %.3f ms)\n",
                   press_count, duration_ns / 1000000.0);
            
            relative_sleep(BOUNCE_TIME_NS);
        }
        
        prev_state = button_state;
        
        // 次のポーリング時刻まで絶対時刻でスリープ
        sleep_until(&next_poll_time);
        timespec_add_ns(&next_poll_time, POLL_INTERVAL_NS);
    }
    
    printf("\n終了処理を実行中...\n");
    printf("総押下回数: %lld\n", press_count);
    
    lgGpioFree(handle, BUTTON_PIN);
    lgGpiochipClose(handle);
    
    return 0;
}
