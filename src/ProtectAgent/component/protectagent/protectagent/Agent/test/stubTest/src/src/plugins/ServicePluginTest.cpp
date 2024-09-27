#include "plugins/ServicePluginTest.h"
#include "plugins/ServicePlugin.h"
#include <vector>
namespace{
mp_void LogTest() {}
#define DoGetJsonStringTest() do { \
    stub11.set(ADDR(CLogger, Log), LogTest); \
} while (0)
}

mp_int32 InitFailedTest(std::vector<mp_uint32> &cmds)
{
    return MP_FAILED;
}
class abc
{
};
class PluginMTest : public IPluginManager {
public:
    virtual mp_void Initialize(IPluginCallback& pCallback) {}
    virtual mp_void Destroy() {}
    virtual mp_size GetSCN() {}
    virtual IPlugin* GetPlugin(mp_string pszPlg) {}
    virtual IPlugin* LoadPlugin(mp_string pszPlg) {}
    virtual mp_void UnloadPlugin(mp_string pszPlg) {}
    virtual mp_int32 Upgrade() {}
};

class CPlugx: public CServicePlugin
{
    public:
    virtual mp_int32 DoAction(CRequestMsg& req, CResponseMsg& rsp) { return MP_SUCCESS; }

    virtual mp_int32 DoAction(CDppMessage& req, CDppMessage& rsp) { return MP_SUCCESS; }
};

mp_int32 DoActionTest(CRequestMsg& req, CResponseMsg& rsp) { return MP_FAILED; }

TEST_F(CRestActionMapTest, Add)
{
    CRestActionMap<abc> restObj;
    restObj.Add("abc", "123", NULL);
}

TEST_F(CRestActionMapTest, GetAction)
{
    CRestActionMap<abc> restObj;
    CRestActionMap<abc>::rest_action_t act();
    //restObj.GetAction(mp_string("abc"), mp_string("123"), &act);
}

TEST_F(CRestActionMapTest, PrintMap)
{
    CRestActionMap<abc> restObj;
    restObj.PrintMap();
}

TEST_F(CRestActionMapTest, xfunc)
{
    DoGetJsonStringTest();
    Stub stub;
    CRequestMsg req;
    CResponseMsg rsp;
    CPlugx plugxObj;
    mp_string strTmp;
    IPlugin::DYN_CLASS dynClass;
    PluginMTest pluginMTest;
    std::vector<mp_uint32> cmds;
    plugxObj.GetTypeId();
    plugxObj.Initialize(cmds);
    // stub.set(ADDR(CPlugx, Init), InitFailedTest);
    // plugxObj.Initialize(cmds);
    plugxObj.Destroy();
    plugxObj.SetOption(strTmp, strTmp);
    plugxObj.GetOption(strTmp, strTmp);
    plugxObj.CreateObject(strTmp);
    plugxObj.GetClasses(dynClass, 1);
    plugxObj.GetName();
    plugxObj.GetVersion();
    plugxObj.GetSCN();
    plugxObj.Invoke(req, rsp);
    CDppMessage req1;
    CDppMessage rsp1;
    plugxObj.Invoke(req1, rsp1);
    // stub.set((mp_int32 (*)(CRequestMsg&, CResponseMsg&))ADDR(CPlugx, DoAction), DoActionTest);
    // plugxObj.Invoke(req, rsp);
    // plugxObj.Invoke(req1, rsp1);
}
