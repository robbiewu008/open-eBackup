#include "tools/agentcli/ShowStatusTest.h"

using namespace std;

static mp_int32 s_iCounter = 0;

static mp_bool stubIsStartted(mp_void)
{
    if (s_iCounter == 0)
    {
        return MP_FALSE;
    }
    else
    {
        return MP_TRUE;
    }
}

static mp_int32 stub_ReadFile(const mp_string& strFilePath, vector<mp_string>& vecOutput)
{
    static mp_int32 iCounter = 0;
    if ( iCounter++ == 0)
    {
        return MP_FAILED;
    }
    else
    {
        vecOutput.push_back("xx.xx.xx.xx");
        return MP_SUCCESS;
    }
}


TEST_F(CShowStatusTest, Handle)
{
    mp_int32 iRet = MP_FALSE;

    stub.set(&ShowStatus::IsStartted, stubIsStartted);

    ShowStatus showStatus;
    iRet = showStatus.Handle();
    EXPECT_EQ(iRet, MP_SUCCESS);

    s_iCounter = 1;
    iRet = showStatus.Handle();
    EXPECT_EQ(iRet, MP_SUCCESS);
    
    return;
}


TEST_F(CShowStatusTest, IsStartted)
{
    ShowStatus showStatus;
    
    showStatus.IsStartted(PROCESS_RDAGENT);
    showStatus.IsStartted(PROCESS_NGINX);
    showStatus.IsStartted(PROCESS_MONITOR);
}


TEST_F(CShowStatusTest, ShowSvn)
{
    ShowStatus showStatus;
    mp_int32 iRet = MP_FALSE;

    stub.set(&CMpFile::ReadFile, &stub_ReadFile);
    
    showStatus.ShowSvn();
    EXPECT_EQ(iRet, MP_SUCCESS);
    
    showStatus.ShowSvn();
    EXPECT_EQ(iRet, MP_SUCCESS);
}

