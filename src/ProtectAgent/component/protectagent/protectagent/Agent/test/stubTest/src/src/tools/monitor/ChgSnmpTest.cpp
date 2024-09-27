#include "tools/agentcli/ChgSnmpTest.h"

using namespace std;

static mp_bool stubCheckAdminOldPwd(mp_void)
{
    static mp_int32 iCounter = 0;
    if (iCounter++ <= MAX_FAILED_COUNT)
    {
        return MP_FALSE;
    }
    else
    {
        return MP_TRUE;
    }
}

TEST_F(CChgSnmpTest, HandleInner)
{
    mp_int32 iRet;
    ChgSnmp snmp;

    stub.set(&ChgSnmp::GetChoice, stub_return_number);

    stub.set(&ChgSnmp::CheckSNMPPwd, stub_return_bool);
    
    //stub.set((mp_int32 (CPassword::*)(PASSWOD_TYPE,string &))ADDR(CPassword,ChgPwd)), stub_return_ret);

    stub.set(&ChgSnmp::ChgAuthProtocol, stub_return_ret);
    stub.set(&ChgSnmp::ChgPrivateProtocol, stub_return_ret);
    stub.set(&ChgSnmp::ChgSecurityName, stub_return_ret);
    stub.set(&ChgSnmp::ChgSecurityLevel, stub_return_ret);
    stub.set(&ChgSnmp::ChgSecurityModel, stub_return_ret);
    stub.set(&ChgSnmp::ChgContextEngineID, stub_return_ret);
    stub.set(&ChgSnmp::ChgContextName, stub_return_ret);

    mp_int32 ret_table[SNMP_CHOOSE_SET_BUTT + 1] = 
    {
        MP_SUCCESS,
        MP_SUCCESS,
        MP_FAILED, 
        MP_FAILED, 
        MP_FAILED, 
        MP_SUCCESS, 
        MP_SUCCESS, 
        MP_SUCCESS, 
        MP_SUCCESS, 
        MP_SUCCESS, 
        MP_SUCCESS
    };
    
    for (mp_int32 i = 1; i <= SNMP_CHOOSE_SET_BUTT; i++)
    {
            iRet = snmp.HandleInner();
            printf("%d____%d_____%d\n", i, ret_table[i], iRet);
            EXPECT_EQ(ret_table[i], iRet);
    }
}

TEST_F(CChgSnmpTest, GetChoice)
{
    mp_int32 iRet;
    ChgSnmp snmp;
    stub.set(&CPassword::GetInput, stub_set_cpasswdString);

    iRet = snmp.GetChoice();
    EXPECT_EQ(SNMP_CHOOSE_SET_BUTT, iRet);

    iRet = snmp.GetChoice();
    EXPECT_EQ(SNMP_CHOOSE_SET_PRI_PASSWD, iRet);
}


TEST_F(CChgSnmpTest, ChgAuthProtocol)
{
    mp_int32 iRet;
    ChgSnmp snmp;
    stub.set(&CPassword::GetInput, stub_set_cpasswdString);

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,const mp_string&))ADDR(CConfigXmlParser,SetValue), &stub_return_ret);

    iRet = snmp.ChgAuthProtocol();
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = snmp.ChgAuthProtocol();
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = snmp.ChgAuthProtocol();
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = snmp.ChgAuthProtocol();
    EXPECT_EQ(MP_FAILED, iRet);
}


TEST_F(CChgSnmpTest, ChgPrivateProtocol)
{
    mp_int32 iRet;
    ChgSnmp snmp;
    stub.set(&CPassword::GetInput, stub_set_cpasswdString);    

    iRet = snmp.ChgPrivateProtocol();
    EXPECT_EQ(MP_FAILED, iRet);
}


TEST_F(CChgSnmpTest, ChgSecurityName)
{
    mp_int32 iRet;
    ChgSnmp snmp;
    stub.set(&CPassword::GetInput, stub_set_cpasswdLongString);

    iRet = snmp.ChgSecurityName();
    EXPECT_EQ(MP_FAILED, iRet);
}


TEST_F(CChgSnmpTest, ChgSecurityLevel)
{
    mp_int32 iRet;
    ChgSnmp snmp;
    stub.set(&CPassword::GetInput, stub_set_cpasswdString);
    
    iRet = snmp.ChgSecurityLevel();
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = snmp.ChgSecurityLevel();
    EXPECT_EQ(MP_FAILED, iRet);
}


TEST_F(CChgSnmpTest, ChgSecurityModel)
{
    mp_int32 iRet;
    ChgSnmp snmp;
    stub.set(&CPassword::GetInput, stub_set_cpasswdString);

    iRet = snmp.ChgSecurityModel();
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = snmp.ChgSecurityModel();
    EXPECT_EQ(MP_FAILED, iRet);
}


TEST_F(CChgSnmpTest, ChgContextEngineID)
{
    mp_int32 iRet;
    ChgSnmp snmp;
    stub.set(&CPassword::GetInput, stub_set_cpasswdString);

    iRet = snmp.ChgContextEngineID();
    EXPECT_EQ(MP_FAILED, iRet);
}


TEST_F(CChgSnmpTest, ChgContextName)
{
    mp_int32 iRet;
    ChgSnmp snmp;
    stub.set(&CPassword::GetInput, stub_set_cpasswdLongString);

    iRet = snmp.ChgContextName();
    EXPECT_EQ(MP_FAILED, iRet);
}



