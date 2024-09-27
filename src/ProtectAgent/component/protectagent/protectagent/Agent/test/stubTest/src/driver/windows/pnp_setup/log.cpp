/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
#include "stdafx.h"
#include <time.h>
#include "log.h"


#define  LOG_LENGTH 1024  * sizeof(wchar_t)

//#define LOG_PATH "C:\\iomirror\\log\\server.data"

static wchar_t LOG_PATH[256];


const wchar_t* g_pformat = L"[%4u-%02u-%02u %s %02u:%02u:%02u] [%5s][%08d][%s:%4d] %s: %s\r\n"; //日志输出的格式,年月日，星期，时分秒，日志级别， 模块名，函数，行号,内容
wchar_t *wday[] = { L"Sun", L"Mon", L"Tue", L"Wed", L"Thu", L"Fri", L"Sat"};


VOID SetLogPath(const wchar_t* wszPath)
{
	wcscpy_s(LOG_PATH, wszPath);
}

VOID IomirroPrint(int level, int error, const wchar_t* lev, const wchar_t* file, int line, const wchar_t* func, const wchar_t* fmt, ...)
{
	wchar_t *s = NULL;
	s = (wchar_t *)malloc(LOG_LENGTH);
	if (NULL == s)
	{
		return;
	}
	memset(s, 0, LOG_LENGTH);
	va_list args;
	va_start(args, fmt);
	(void)_vsnwprintf(s, LOG_LENGTH, fmt, args);
	va_end(args);

	wchar_t *msg = (wchar_t *)malloc(wcslen(g_pformat) * sizeof(wchar_t) + LOG_LENGTH);
	if (NULL == msg)
	{
		free(s);
		return;
	}
	memset(msg, 0, wcslen(g_pformat) * sizeof(wchar_t) + LOG_LENGTH);

	time_t timep;
	struct tm *p;
	time(&timep);
	p = localtime(&timep);

	if (-1 >= _snwprintf(msg, wcslen(g_pformat) + LOG_LENGTH / sizeof(wchar_t), g_pformat, 1900+p->tm_year, 1+p->tm_mon, p->tm_mday,
		wday[p->tm_wday], p->tm_hour, p->tm_min, p->tm_sec, lev, error, file, line, func, s))
	{
		free(msg);
		free(s);
		return;
	}

	FILE *f = _wfopen(LOG_PATH, L"ab+");
	if (NULL == f)
	{
		return;
	}
	fwrite(msg, wcslen(msg) * sizeof(wchar_t), 1, f);
	fclose(f);
	free(msg);
	free(s);
}