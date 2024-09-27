#ifndef _KMCCALLBACK_H_
#define _KMCCALLBACK_H_

#define private public

#include "securecom/KmcCallback.h"
#include "common/Utils.h"
#include "securecom/UniqueId.h"
#include "common/Log.h"
#include "common/File.h"
#include "common/Path.h"
#include "common/ErrorCode.h"
#include "common/CSystemExec.h"
#include "common/ConfigXmlParse.h"
#include "securec.h"
#include "gtest/gtest.h"
#include "stub.h"
#include <vector>


class KmcCallbackTest: public testing::Test{
public:
    Stub stub;
};

#endif
