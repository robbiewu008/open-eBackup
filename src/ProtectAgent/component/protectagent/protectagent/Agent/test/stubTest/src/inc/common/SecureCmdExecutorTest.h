#ifndef _SECURECMDEXECUTOR_H_
#define _SECURECMDEXECUTOR_H_

#define private public

#ifndef WIN32
#include <signal.h>
#include <libgen.h>
#endif
#include "common/SecureCmdExecutor.h"
#include <sstream>
#include "securec.h"
#include "common/CSystemExec.h"
#include "common/ConfigXmlParse.h"
#include "common/Log.h"
#include "common/Utils.h"
#include "securecom/UniqueId.h"
#include "common/Path.h"
#include "common/ErrorCode.h"
#include "gtest/gtest.h"
#include "stub.h"

class SecureCmdExecutorTest: public testing::Test{
public:
    Stub stub;
};

#endif
