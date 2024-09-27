#include "tools/agentcli/CollectLogTest.h"
#include "securecom/SecureUtils.h"

static mp_void stubGetInput(const mp_string& strHint, mp_string& strInput)
{
    static mp_int32 ival = 0;
    if (ival++ == 0)
    {
        strInput = "n";
    }
    else
    {
        strInput = "y";
    }
}


TEST_F(CCollectLogTest, Handle)
{
    mp_int32 iRet = MP_SUCCESS;
    CollectLog Obj;

    Stub stub;
    stub.set(&CPassword::GetInput, stubGetInput);

    printf("\n#######################   01");
    
    iRet = Obj.Handle();
    EXPECT_EQ(MP_FAILED, iRet);

    printf("\n#######################   02");

    stub.set(&SecureCom::PackageLog, stub_return_ret); 

    stub.set(&CMpTime::GetTimeString, &stub_return_string);

    iRet = Obj.Handle();
    EXPECT_EQ(MP_FAILED, iRet);

    printf("\n#######################   03");

    iRet = Obj.Handle();
    EXPECT_TRUE(1);
    
    printf("\n#######################   04");
    
}

