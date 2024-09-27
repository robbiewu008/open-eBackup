#pragma

#include "header.h"

#define LOG_PRIORITY_ERROR  1
#define LOG_PRIORITY_WARN   2
#define LOG_PRIORITY_INFO   3
#define LOG_PRIORITY_DEBUG  4

VOID IomirroPrint(int level, int error, const wchar_t* lev, const wchar_t* file, int line, const wchar_t* func, const wchar_t* fmt, ...);
VOID SetLogPath(const wchar_t* wszPath);

#define IOMIRROR_ERROR(err, fmt, ...) IomirroPrint(LOG_PRIORITY_ERROR, err, L"ERROR", _T(__FILE__), __LINE__, _T(__FUNCTION__), fmt, ##__VA_ARGS__);
#define IOMIRROR_DEBUG(err, fmt, ...) IomirroPrint(LOG_PRIORITY_DEBUG, err, L"DEBUG", _T(__FILE__), __LINE__, _T(__FUNCTION__), fmt, ##__VA_ARGS__);
#define IOMIRROR_INFO(fmt, ...) IomirroPrint(LOG_PRIORITY_INFO, 0, L"INFO", _T(__FILE__), __LINE__, _T(__FUNCTION__), fmt, ##__VA_ARGS__);