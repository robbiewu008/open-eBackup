#include "cunitpub/publicInc.h"
#include <vector>
namespace {
static  mp_int32  CUNIT_iCounter = 0;
static  mp_int32  CUNIT_bCounter = 0;
static  mp_int32  CUNIT_cCounter = 0;
static  mp_int32  CUNIT_sCounter = 0;
static  mp_int32  CUNIT_rCounter = 0;
static  mp_int32  CUNIT_nCounter = 0;
static  mp_int32  CUNIT_pCounter = 0;
static  mp_int32  CUNIT_SystemWithEchoCounter = 0;
static  mp_int32  CUNIT_SystemWithoutEchoCounter = 0;
static  mp_int32  CUNIT_ExecScriptCounter = 0;
static  mp_int32  CUNIT_CheckScriptSignCounter = 0;
}

static Stub *gp_stubLog = NULL;
static Stub *gp_CConfigXmlParser_0 = NULL;
static Stub *gp_CConfigXmlParser_1 = NULL;
static Stub *gp_GetJsonString = NULL;
static Stub *gp_GetJsonInt32 = NULL;
static Stub *gp_GetJsonArrayString  = NULL;
static Stub *gp_GetJsonInt64  = NULL;
static Stub *gp_GetJsonArrayJson_0  = NULL;
static Stub *gp_GetJsonArrayJson_1  = NULL;
static Stub *gp_ExecSystemWithEcho  = NULL;
static Stub *gp_ExecSystemWithoutEcho  = NULL;
static Stub *gp_ExecScript  = NULL;

/* 用户输入; */
static Stub *gp_InputUserPwd  = NULL;

mp_int32 stub_return_ret(mp_void)
{
    if (CUNIT_rCounter++ == 0)
    {
        return MP_FAILED;
    }
    else
    {
        return MP_SUCCESS;
    }
}

static mp_int32 StubGetJsonArrayJson(const Json::Value& jsValue, std::vector<Json::Value>& vecValue)
{
    return MP_SUCCESS;
}
mp_bool stub_return_bool(mp_void)
{
    static int CUNIT_bCounter = 0;
    if (CUNIT_bCounter++ == 0)
    {
        return MP_FALSE;
    }
    else
    {
        return MP_TRUE;
    }
}

mp_bool stub_return_success(mp_void)
{

    return MP_SUCCESS;

}
mp_bool stub_return_bool_true(void)
{
    return MP_TRUE;
}

mp_void reset_cunit_counter(mp_void)
{
    CUNIT_iCounter = 0;
    CUNIT_bCounter = 0;
    CUNIT_cCounter = 0;
    CUNIT_sCounter = 0;
    CUNIT_rCounter = 0;
    CUNIT_nCounter = 0;
    CUNIT_pCounter = 0;
    CUNIT_SystemWithEchoCounter = 0;
    CUNIT_SystemWithoutEchoCounter = 0;
    CUNIT_ExecScriptCounter = 0;
    CUNIT_CheckScriptSignCounter = 0;
}

mp_void stub_set_numberStr(mp_string &strInput)
{
    if (CUNIT_iCounter== 0)
    {
        strInput = "15";
    }
    strInput = toString<mp_int32>(CUNIT_iCounter++);
}

mp_string stub_return_string(mp_void)
{   
    mp_string strInput;
    stub_set_numberStr(strInput);

    return strInput;
}

static mp_int32 stubExecSystemWithoutEcho()
{
    if (CUNIT_SystemWithoutEchoCounter++ == 0)
    {
        return MP_FAILED;
    }
    else
    {
        return MP_SUCCESS;
    }
}

static mp_int32 stubSystemWithEcho(const mp_string& strCommand, std::vector<mp_string>& strEcho, mp_bool bNeedRedirect)
{
    if (CUNIT_SystemWithEchoCounter++ == 0)
    {
        return MP_FAILED;
    }
    else
    {
        strEcho.push_back("123");
        return MP_SUCCESS;
    }
}

mp_void stub_return_nothing(mp_void)
{
    return ;
}

mp_void stub_set_cpasswdString(const mp_string& strHint, mp_string& strInput)
{
    if (CUNIT_pCounter++ == 0)
    {
        strInput = "15";
    }
    else
    {
        strInput = toString<mp_int32>(CUNIT_pCounter);
    }
}

mp_int32 stub_return_number(mp_void)
{
    return CUNIT_nCounter++;
}

mp_void stub_set_cpasswdLongString(const mp_string& strHint, mp_string& strInput)
{
    if (CUNIT_pCounter++ == 0)
    {
        strInput = "Agent&12345679891234567891234567989123456789123456789123456789123456789";
    }
    else
    {
        strInput = "Agent&123";
    }
}


mp_void init_cunit_data(mp_void)
{
    gp_stubLog = new Stub;
    gp_stubLog->set(&CLogger::Log, &stub_return_nothing);
    gp_CConfigXmlParser_0 = new Stub;
    gp_CConfigXmlParser_0->set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), stub_return_success);
    gp_CConfigXmlParser_1 = new Stub;
    gp_CConfigXmlParser_1->set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), stub_return_success);
    gp_GetJsonString = new Stub;
    gp_GetJsonString->set(&CJsonUtils::GetJsonString, stub_return_ret);
    gp_GetJsonInt32 = new Stub;
    gp_GetJsonInt32->set(&CJsonUtils::GetJsonInt32, stub_return_ret);
    gp_GetJsonArrayString = new Stub;
    gp_GetJsonArrayString->set(&CJsonUtils::GetJsonArrayString, stub_return_ret);
    gp_GetJsonInt64 = new Stub;
    gp_GetJsonInt64->set(&CJsonUtils::GetJsonInt64, stub_return_ret);
    gp_ExecSystemWithEcho = new Stub;
    gp_ExecSystemWithEcho->set(&CSystemExec::ExecSystemWithEcho, stubSystemWithEcho);
    gp_ExecSystemWithoutEcho = new Stub;
    gp_ExecSystemWithoutEcho->set(&CSystemExec::ExecSystemWithoutEchoNoWin, stubExecSystemWithoutEcho);
    // gp_GetJsonArrayJson_0 = new Stub;
    // gp_GetJsonArrayJson_0->set((mp_int32(CJsonUtils::*)(const Json::Value&,std::vector<Json::Value>&))ADDR(CJsonUtils,GetJsonArrayJson), &StubGetJsonArrayJson);
    // gp_GetJsonArrayJson_1 = new Stub;
    // gp_GetJsonArrayJson_1->set((mp_int32(CJsonUtils::*)(const Json::Value&,mp_string,std::vector<Json::Value>&))ADDR(CJsonUtils,GetJsonArrayJson), stub_return_success);
    gp_InputUserPwd = new Stub;
    gp_InputUserPwd->set(&CPassword::InputUserPwd, stub_return_nothing);
}

mp_void destroy_cunit_data(mp_void)
{
    if (gp_stubLog) delete gp_stubLog;
    if (gp_CConfigXmlParser_0)    delete gp_CConfigXmlParser_0;
    if (gp_CConfigXmlParser_1)  delete gp_CConfigXmlParser_1;
    if (gp_GetJsonString) delete gp_GetJsonString;
    if (gp_GetJsonInt32) delete gp_GetJsonInt32;
    if (gp_GetJsonArrayString) delete gp_GetJsonArrayString;
    if (gp_GetJsonInt64) delete gp_GetJsonInt64;
    if (gp_ExecSystemWithEcho) delete gp_ExecSystemWithEcho;
    if (gp_ExecSystemWithoutEcho) delete gp_ExecSystemWithoutEcho;
    if (gp_ExecScript) delete gp_ExecScript;
    // if (gp_GetJsonArrayJson_0) delete gp_GetJsonArrayJson_0;
    // if (gp_GetJsonArrayJson_1) delete gp_GetJsonArrayJson_1;
    if (gp_InputUserPwd) delete gp_InputUserPwd;
}