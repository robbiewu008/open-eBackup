/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2020. All rights reserved.
 * Description: time processing function
 * Author: z00118096
 * Create: 2014-07-11
 * History: 2018-10-06 Zhang Jie (employee ID: 00316590) Rectification by UK
 */

#include "wsecv2_datetime.h"
#include "securec.h"
#include "wsecv2_callbacks.h"
#include "wsecv2_order.h"

#define MIN_MONTH 1
#define MAX_MONTH 12
#define HOURS_PER_DAY 24
#define MINUTES_PER_HOUR 60
#define SECONDS_PER_MINUTE 60
#define WSEC_WEEK_DAYS 7

#define TIME_TM_BASE_YEAR 1900

#define KMCIS_LEAP_YEAR(y) (((!((y) % 4)) && ((y) % 100)) || (!((y) % 400)))

#define WSEC_MONTH_DAYS(y, m) \
    ((((m) == 4) || ((m) == 6) || ((m) == 9) || ((m) == 11)) ? 30 : (((m) == 2) ? (KMCIS_LEAP_YEAR(y) ? 29 : 28) : 31))

/*
 * Checks whether the specified parameter is valid.
 * The system only checks whether the values of year, month, day, hour, minute, and second are valid.
 * The system cannot determine whether the week value matches the year, month, and day.
 */
WsecBool WsecIsDateTime(const WsecSysTime *timeValue)
{
    WsecBool dateTimeValid = WSEC_FALSE;

    if ((timeValue != NULL) &&
        (timeValue->kmcYear > 0) &&
        WSEC_IN_SCOPE(timeValue->kmcMonth, MIN_MONTH, MAX_MONTH) && /* Month: 1-12 */
        (timeValue->kmcHour < HOURS_PER_DAY) && /* 24 hours: 0-23 */
        (timeValue->kmcMinute < MINUTES_PER_HOUR) && /* 60 points: 0 to 59 */
        (timeValue->kmcSecond <= SECONDS_PER_MINUTE) && /* 61s: 0-60 (leap second) */
        (WSEC_IN_SCOPE(timeValue->kmcDate, 1, WSEC_MONTH_DAYS(timeValue->kmcYear, timeValue->kmcMonth)))) {
        dateTimeValid = WSEC_TRUE;
    }

    return dateTimeValid;
}

/* Converts the tm data structure (tmTime) in the C language to WsecSysTime. */
static WsecVoid TmToWsecSystemTime(const struct tm *tmTime, WsecSysTime *wsecTime)
{
    WSEC_ASSERT(tmTime != NULL);
    WSEC_ASSERT(wsecTime != NULL);

    wsecTime->kmcYear = (WsecUint16)(tmTime->tm_year + TIME_TM_BASE_YEAR);
    wsecTime->kmcMonth = (unsigned char)(tmTime->tm_mon + 1);
    wsecTime->kmcDate = (unsigned char)tmTime->tm_mday;
    wsecTime->kmcHour = (unsigned char)tmTime->tm_hour;
    wsecTime->kmcMinute = (unsigned char)tmTime->tm_min;
    wsecTime->kmcSecond = (unsigned char)tmTime->tm_sec;
    wsecTime->kmcWeek = (unsigned char)tmTime->tm_wday;

    if (wsecTime->kmcWeek == 0) {
        wsecTime->kmcWeek = KMC_TIME_SUNDAY;
    }
}

/* To query the TM time of the current system */
static WsecBool OsGmTime(const time_t *nowTime, struct tm *gmTm)
{
    WSEC_ASSERT(gmTm != NULL);
    WSEC_ASSERT(nowTime != NULL);
    return WsecGmTime(nowTime, gmTm);
}

/* Add nAdd time units (days) to the specified date and time (baseTime). */
WsecBool WsecDateTimeAddDay(const WsecSysTime *baseTime, WsecUint32 day, WsecSysTime *newTime)
{
    WsecUint32 monthDays;
    WsecUint32 addDays;
    WsecBool ret = WsecIsDateTime(baseTime);
    if (ret == WSEC_FALSE || newTime == NULL) {
        return WSEC_FALSE;
    }
    ret = WsecDateTimeCopy(baseTime, newTime);
    if (ret != WSEC_TRUE) {
        return WSEC_FALSE;
    }
    addDays = newTime->kmcDate + day;
    newTime->kmcDate = 0;
    do {
        monthDays = (WsecUint32)WSEC_MONTH_DAYS(newTime->kmcYear, newTime->kmcMonth);
        if (addDays > monthDays) {
            if (newTime->kmcMonth == 12) { /* kmcMonth: 1 to 12 */
                newTime->kmcYear = (WsecUint16)(newTime->kmcYear + 1);
                newTime->kmcMonth = 1;
            } else {
                newTime->kmcMonth = (unsigned char)(newTime->kmcMonth + 1);
            }
            addDays = addDays - monthDays;
        } else {
            newTime->kmcDate = (unsigned char)addDays;
            addDays = 0;
        }
    } while (addDays > 0);
    newTime->kmcWeek = (unsigned char)(((day + newTime->kmcWeek - 1) % WSEC_WEEK_DAYS) + 1);    /* soter 554 */

    return WSEC_TRUE;
}

/*
 * Compares dates. If A is greater than B, 1 is returned. If A is equal to B, 0 is returned.
 * If A is less than B, -1 is returned. The external system has verified timeA and timeB.
 * Therefore, no verification is required.
 */
static int WsecDateTimeCompare(const WsecSysTime *timeA, const WsecSysTime *timeB)
{
    /* Year of comparison */
    if (timeA->kmcYear > timeB->kmcYear) {
        return 1;
    }
    if (timeA->kmcYear < timeB->kmcYear) {
        return -1;
    }

    /* Comparison Month */
    if (timeA->kmcMonth > timeB->kmcMonth) {
        return 1;
    }
    if (timeA->kmcMonth < timeB->kmcMonth) {
        return -1;
    }

    /* Comparison day */
    if (timeA->kmcDate > timeB->kmcDate) {
        return 1;
    }
    if (timeA->kmcDate < timeB->kmcDate) {
        return -1;
    }

    return 0;
}

/* Number of extra days between endTime and startTime. The unit is day. */
WsecBool WsecDateTimeDiffDay(const WsecSysTime *startTime, const WsecSysTime *endTime, int *day)
{
    int resultDay = 0;
    int monthDays;
    const WsecSysTime *smaller = NULL;
    const WsecSysTime *bigger = NULL;
    WsecSysTime baseTime;
    WsecBool startSmaller = WSEC_FALSE;
    if (WsecIsDateTime(startTime) == WSEC_FALSE || WsecIsDateTime(endTime) == WSEC_FALSE || day == NULL) {
        return WSEC_FALSE;
    }

    if (WsecDateTimeCompare(startTime, endTime) < 0) {
        smaller = startTime;
        bigger = endTime;
        startSmaller = WSEC_TRUE;
    } else {
        smaller = endTime;
        bigger = startTime;
    }
    if (WsecDateTimeCopy(smaller, &baseTime) == WSEC_FALSE) {
        return WSEC_FALSE;
    }

    /* Calculate the number of days in a month. */
    while ((baseTime.kmcYear < bigger->kmcYear) ||
        (baseTime.kmcYear == bigger->kmcYear && baseTime.kmcMonth < bigger->kmcMonth)) {
        monthDays = WSEC_MONTH_DAYS(baseTime.kmcYear, baseTime.kmcMonth);
        resultDay = resultDay + monthDays;
        if (baseTime.kmcMonth == 12) { /* kmcMonth: 1 to 12 */
            baseTime.kmcMonth = 1;
            baseTime.kmcYear = (WsecUint16)(baseTime.kmcYear + 1);
        } else {
            baseTime.kmcMonth = (unsigned char)(baseTime.kmcMonth + 1);
        }
    }
    /* Accumulate the difference of days. */
    resultDay = resultDay + (int)bigger->kmcDate - (int)smaller->kmcDate;   /* soter 554 */
    if (startSmaller == WSEC_FALSE) {
        resultDay *= -1;
    }
    *day = resultDay;
    return WSEC_TRUE;
}

/* Copy the DateTime structure. */
WsecBool WsecDateTimeCopy(const WsecSysTime *srcTime, WsecSysTime *destTime)
{
    WsecBool isDateTime = WSEC_FALSE;

    WSEC_ASSERT(destTime != NULL);
    WSEC_ASSERT(srcTime != NULL);
    isDateTime = WsecIsDateTime(srcTime);
    (void)memcpy_s(destTime, sizeof(WsecSysTime), srcTime, sizeof(WsecSysTime));
    return isDateTime;
}

/* Convert the byte order of date and time data. */
WsecVoid WsecCvtByteOrderForDateTime(WsecSysTime *dateTime, WsecUint32 direction)
{
    WSEC_ASSERT(WSEC_IS2(direction, WBCHOST2NETWORK, WBCNETWORK2HOST));

    dateTime->kmcYear = WSEC_BYTE_ORDER_CVT_S(direction, dateTime->kmcYear);
}

/* Obtaining the Current Universal Time Coordinated (UTC) Date and Time */
WsecBool WsecGetUtcDateTime(WsecSysTime *nowUtc)
{
    WsecBool ret = WSEC_FALSE;
    time_t nowTime;
    struct tm tmTime;
    (void)memset_s(&tmTime, sizeof(tmTime), 0, sizeof(tmTime));
    WSEC_ASSERT(nowUtc != NULL);

    nowTime = time(NULL);

    ret = OsGmTime(&nowTime, &tmTime);
    TmToWsecSystemTime(&tmTime, nowUtc);

    return ret;
}
