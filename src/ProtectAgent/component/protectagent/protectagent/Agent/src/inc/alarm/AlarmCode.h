#ifndef _ALARMCODE_H_
#define _ALARMCODE_H_
#include "common/Types.h"
const mp_string RUN_EVENT_JOB_LONG_TIME_FULL_LOAD = "0x106403400002";
const mp_string ALARM_THRIFT_SERVER_START_FAILED = "0x106403400001";
const mp_string ALARM_ORACLE_ARCHIVE_AREA = "0x6403400001";
#ifdef WIN32
const mp_string ALARM_START_PLUGIN = "0x6403400007";
#else
const mp_string ALARM_START_PLUGIN = "0x6403400002";
#endif

#endif
