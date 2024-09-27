#ifndef _AGENTCLI_UNIXTIMESTAMP_H_
#define _AGENTCLI_UNIXTIMESTAMP_H_

#include "common/Types.h"

class UnixTimeStamp {
public:
    static mp_int32 Handle(const mp_string& timeString, const mp_string& transMode);

private:
    static mp_int32 UnixStampTranforDate(const mp_string& timeString);
    static mp_int32 DateTransforUnixStamp(const mp_string& timeString);
};
#endif