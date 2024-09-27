#include <sstream>
#include <ctime>
#include "common/CMpTime.h"
#include "common/ErrorCode.h"
#include "tools/agentcli/UnixTimeStamp.h"


mp_int32 UnixTimeStamp::Handle(const mp_string& timeString, const mp_string& transMode)
{
    if (transMode == "Date2Unix") {
        return DateTransforUnixStamp(timeString);
    } else if (transMode == "Unix2Date") {
        return UnixStampTranforDate(timeString);
    } else {
        printf("TransMode is incorrect,please input Data2Unix or Unix2Date! transMode is %s\n", transMode.c_str());
        return MP_FAILED;
    }
}

mp_int32 UnixTimeStamp::DateTransforUnixStamp(const mp_string& timeString)
{
#ifdef WIN32
    printf("%s\n", "undefine func");
    return ERROR_COMMON_FUNC_UNIMPLEMENT;
#else
    tm timep;
    timep.tm_isdst = -1;
    if (timeString.empty()) {
        printf("Data time Param is empty!");
        return MP_FAILED;
    }

    char* result = strptime(timeString.c_str(), "%Y-%m-%d %H:%M:%S", &timep);
    if (result == NULL) {
        printf("Transform date to timestamp failed!\n");
        return MP_FAILED;
    }

    time_t unixStamp = mktime(&timep);
    if (unixStamp == -1) {
        printf("Date time mktime failed!\n");
        return MP_FAILED;
    }

    printf("%lld\n", (mp_int64)unixStamp);
    return MP_SUCCESS;
#endif
}

mp_int32 UnixTimeStamp::UnixStampTranforDate(const mp_string& timeString)
{
    if (timeString.empty()) {
        printf("Unix time stamp para is empty!");
        return MP_FAILED;
    }

    if (timeString.find_first_not_of("0123456789") != std::string::npos) {
        printf("The input unix stamp is not a digit. timestamp:%s!\n", timeString.c_str());
        return MP_FAILED;
    }

    std::istringstream timeStreasm(timeString);
    time_t timeStamp;
    timeStreasm >> timeStamp;
    mp_tm timep;
    mp_tm* ret = CMpTime::LocalTimeR(timeStamp, timep);
    if (ret == NULL) {
        printf("Unix time stamp local time failed!\n");
        return MP_FAILED;
    }

    char sDestTime[NOW_TIME_LENGTH] = { 0 };
    size_t result = strftime(sDestTime, sizeof(sDestTime) - 1, "%Y-%m-%d %H:%M:%S", &timep);
    if (result == 0) {
        printf("Unix time stamp transfor Date failed!\n");
        return MP_FAILED;
    }
    printf("%s\n", sDestTime);
    return MP_SUCCESS;
}
