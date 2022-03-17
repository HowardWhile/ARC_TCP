#ifndef arc_console_h__
#define arc_console_h__

#include <time.h>
#include <sys/time.h>

#include <stdio.h>
#include <limits.h>

#define ENABLE_ARC_CONSOLE (1)
#define PRINT_FUNCTION printf

#ifdef _SYS_TIME_H // linux #include <sys/time.h>
static uint64_t clock_ms()
{
    // https://stackoverflow.com/questions/3756323/how-to-get-the-current-time-in-milliseconds-from-c-in-linux
    struct timeval te;
    // get current time
    gettimeofday(&te, NULL);
    // calculate milliseconds
    uint64_t milliseconds = te.tv_sec * 1000LL + te.tv_usec / 1000;
    // printf("milliseconds: %lld\n", milliseconds);
    return milliseconds;
}
#define CLOCK_MS_FUNCTION clock_ms

#else

#define CLOCK_MS_FUNCTION clock
#endif

/* ----------------------------- */
// 範例1: 一次性關閉所有debug print
/*//#define DBG_PRINT(...)  // 關閉輸出
#define DBG_PRINT(...) console(__VA_ARGS__)
{
	DBG_PRINT("HelloWorld");
}*/

// 範例2: debug 打印包含標籤
/*#define DBG_PRINT(...) console_tag("DEBUG", __VA_ARGS__)
{
	DBG_PRINT("HelloWorld"); 
}*/
/* ----------------------------- */
// 名稱: EXEC_INTERVAL
// 功能: 與EXEC_INTERVAL_END成對週期執行
// 範例:
/*	EXEC_INTERVAL(1000)
*	{
*		printf("clock() = %d \r\n", clock());
*	}
*	EXEC_INTERVAL_END;
*/
/* ----------------------------- */
#define EXEC_INTERVAL(period)                           \
    {                                                   \
        static uint64_t check_time = 0;                 \
        if (CLOCK_MS_FUNCTION() - check_time >= period) \
        {

#define EXEC_INTERVAL_END             \
    check_time = CLOCK_MS_FUNCTION(); \
    }                                 \
    }

/* ----------------------------- */

#if ENABLE_ARC_CONSOLE == 0

#define console(...)
#define console_throttle(period, ...)
#define console_fps(tag_name)
#define console_tag(tag_name, ...)

#else
/* ----------------------------- */
// 名稱: console
// 功能: 等同於printf
/* ----------------------------- */
#define console(...)                                       \
    {                                                      \
        PRINT_FUNCTION("[%lld ms] ", CLOCK_MS_FUNCTION()); \
        PRINT_FUNCTION(__VA_ARGS__);                       \
        PRINT_FUNCTION("\r\n");                            \
    }
/* ----------------------------- */
// 名稱: console_tag
// 功能: 多加入tag欄位
/* ----------------------------- */
#define console_tag(tag_name, ...)                                       \
    {                                                                    \
        PRINT_FUNCTION("[%lld ms][%s] ", CLOCK_MS_FUNCTION(), tag_name); \
        PRINT_FUNCTION(__VA_ARGS__);                                     \
        PRINT_FUNCTION("\r\n");                                          \
    }
/* ----------------------------- */
// 名稱: console_throttle
// 功能: 週期輸出
/* ----------------------------- */
#define console_throttle(period, ...) \
    {                                 \
        EXEC_INTERVAL(period)         \
        {                             \
            console(__VA_ARGS__);     \
        }                             \
        EXEC_INTERVAL_END;            \
    }
/* ----------------------------- */
// 名稱: console_fps
// 功能: 輸出執行處的FPS
// 格式: [TimeStamp][Tag] FPS \r\n
/* ----------------------------- */
#define console_fps(tag_name)       \
    {                               \
        static int fps_count = 0;   \
        EXEC_INTERVAL(1000)         \
        {                           \
            console("[%s] fps: %d", \
                    tag_name,       \
                    fps_count);     \
            fps_count = 0;          \
        }                           \
        EXEC_INTERVAL_END;          \
        fps_count++;                \
    }

#define TIME_FORMATE_STR "%d-%02d-%02d %02d:%02d:%02d.%03d"
#define TIME_FORMATE_SYSTEMTIME(systime) \
    (systime).wYear,                     \
        (systime).wMonth,                \
        (systime).wDay,                  \
        (systime).wHour,                 \
        (systime).wMinute,               \
        (systime).wSecond,               \
        (systime).wMilliseconds

#endif // ENABLE_ARC_CONSOLE

#endif // arc_console_h__