#ifndef _AGENTCLI_SHOWSTATUS_H_
#define _AGENTCLI_SHOWSTATUS_H_

#include "common/Types.h"

static const mp_string RUNNING_TAG = "RUNNING";
static const mp_string SVN_CONF = "svn";

typedef enum { PROCESS_RDAGENT = 0, PROCESS_NGINX, PROCESS_MONITOR } PROCESS_TYPE;

class ShowStatus {
public:
    static mp_int32 Handle();

private:
    static mp_bool IsStartted(PROCESS_TYPE eType);
    static mp_void ShowSvn();
};

#endif
