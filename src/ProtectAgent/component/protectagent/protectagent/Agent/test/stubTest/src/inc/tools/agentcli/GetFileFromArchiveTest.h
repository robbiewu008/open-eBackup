#ifndef _GET_FILE_FROM_ARCHIVE_TEST_H_
#define _GET_FILE_FROM_ARCHIVE_TEST_H_

#include "cunitpub/publicInc.h"
#include "tools/agentcli/GetFileFromArchive.h"

class CGetFileFromArchiveTest: public testing::Test
{
public:
    static void SetUpTestCase(void)
    {
        init_cunit_data();
    }

    static void TearDownTestCase(void)
    { 
        destroy_cunit_data();
    }
    Stub stub;
protected:
    virtual void SetUp()
    { 
        reset_cunit_counter();
    }
    virtual void TearDown()
    {
        ;
    }
};


mp_int32 SignTestStubReadFile(mp_string& strFilePath, std::vector<mp_string>& vecOutput){
    vecOutput.push_back("test");
    return 0;
}


#endif /* _AGENT_CLI_TEST_H_; */

