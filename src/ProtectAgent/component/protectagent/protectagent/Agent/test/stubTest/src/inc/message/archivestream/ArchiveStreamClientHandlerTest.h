#ifndef __GET_FILE_CLIENT_2_ARCHIVE_H__
#define __GET_FILE_CLIENT_2_ARCHIVE_H__

#include <sstream>
#include <vector>
#include <map>
#include "common/Log.h"
#include "common/Utils.h"
#include "common/CMpTime.h"
#include "common/DB.h"
#include "common/Path.h"
#include "securecom/CryptAlg.h"
#include "common/ConfigXmlParse.h"
#include "message/archivestream/ArchiveStreamClientHandler.h"
#include "common/Uuid.h"
#include "gtest/gtest.h"
#include "stub.h"
#include <openssl/ssl.h>

class TestArchiveStreamClientHandler : public testing::Test {
public:
    void SetUp()
    {
    }

    void TearDown()
    {
    }

    static void SetUpTestCase()
    {
    }

    static void TearDownTestCase()
    {
    }
    static int times;
};

#endif