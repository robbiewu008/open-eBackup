#define private public
#include "pluginfx/PluginManagerTest.h"
    
using namespace std;

namespace {
mp_void StubCMpPluginCfgTestLogVoid(mp_void* pthis)
{
    return;
}

IPlugin* LoadPluginNull(mp_string pszPlg)
{
    return NULL;
}

IPlugin* GetPluginNull()
{
    return NULL;
}

mp_void UnloadOldModulesReturn()
{
    return;
}

mp_bool NeedReloadTrue(CModule& module)
{
    return MP_TRUE;
}

}

static mp_void StubCLoggerLog(mp_void)
{
    return;
}

CPluginManagerCBTest::CPluginManagerCBTest()
{
}

CPluginManagerCBTest::~CPluginManagerCBTest()
{
}

mp_bool CPluginManagerCBTest::CanUnload(IPlugin& pOldPlg)
{
    return MP_TRUE;
}

mp_void CPluginManagerCBTest::OnUpgraded(IPlugin& pOldPlg, IPlugin& pNewPlg)
{
}

mp_void CPluginManagerCBTest::SetOptions(IPlugin& plg)
{
}

mp_string CPluginManagerCBTest::GetReleaseVersion(const mp_string& pszLib, mp_string& pszVer)
{
    return "";
}


TEST_F(CMpPluginManagerTest, PluginManagerTest)
{
    try
    {
        /********Begin PluginManager.cpp test********/
        CPluginManagerCBTest pluginMCBTset;
        CPluginManager pluginManager;

        Stub mp_stub;
        mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubCMpPluginManagerTestLogVoid);

        // initialize
        mp_int32 iRet = pluginManager.Initialize(pluginMCBTset);
        EXPECT_EQ(iRet, MP_SUCCESS);
        
        pluginManager.SetPluginPath("./");

        // get no exists plugin
        IPlugin* pPlg = pluginManager.GetPlugin("libtest");
        EXPECT_EQ(pPlg, (IPlugin *)NULL);

        // loal plugin
        pPlg = pluginManager.LoadPlugin("libtest");
        //EXPECT_NE(pPlg, (IPlugin *)NULL);

        if (pPlg != NULL)
        {
            // check plugin
            pPlg = pluginManager.GetPlugin("libtest");
            EXPECT_NE(pPlg, (IPlugin *)NULL);

            // unload plugin
            pluginManager.UnloadPlugin("libtest");
        
            // check plugin
            pPlg = pluginManager.GetPlugin("libtest");
            EXPECT_EQ(pPlg, (IPlugin *)NULL);
        }
        
        // check upgrade
        //iRet = pluginManager.Upgrade();
        //EXPECT_EQ(iRet, MP_SUCCESS);
        /********End PluginManager.cpp test********/
    }
    catch(...)
    {
        printf("Error on %s file %d line.\n", __FILE__, __LINE__);
        exit(0);
    }
}

TEST_F(CMpPluginManagerTest, GetPluginTest)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger,Log), StubCMpPluginCfgTestLogVoid);
    CPluginManager cPluginManager;
    mp_uint32 manageCmd;

    stub.set(((CModule* (CPluginManager::*)(const mp_string&))ADDR(CPluginManager, GetModule)), StubGetModule0);
    IPlugin* plugin = cPluginManager.GetPlugin(manageCmd);
    EXPECT_TRUE(plugin == NULL);
}

TEST_F(CMpPluginManagerTest, LoadPreLoadPluginsTest)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger,Log), StubCMpPluginCfgTestLogVoid);
    CPluginManager cPluginManager;
    vector<mp_string> vecPlgs;

    cPluginManager.LoadPreLoadPlugins(vecPlgs);

    vecPlgs.push_back("Agent");
    stub.set(ADDR(CPluginManager, LoadPlugin), LoadPluginNull);
    cPluginManager.LoadPreLoadPlugins(vecPlgs);
}

TEST_F(CMpPluginManagerTest, UpgradeTest)
{
    CPluginManager cPluginManager;
    stub.set(ADDR(CPluginManager, UnloadOldModules), UnloadOldModulesReturn);
    stub.set(ADDR(CPluginManager, NeedReload), NeedReloadTrue);
    //mp_int32 iRet = cPluginManager.Upgrade();
    //EXPECT_EQ(iRet, MP_SUCCESS);
}

TEST_F(CMpPluginManagerTest, InitModuleTest)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger,Log), StubCMpPluginCfgTestLogVoid);
    CPluginManager cPluginManager;
    CFileSearcher pSearcher;
    mp_string pszName;
    mp_string pszVersion;
    CModule module(pSearcher, pszName, pszVersion);

    stub.set(ADDR(CModule, GetPlugin), GetPluginNull);
    mp_bool iRet = cPluginManager.InitModule(module);
    EXPECT_FALSE(iRet);
}

TEST_F(CMpPluginManagerTest, PrintCmdModulesTest)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger,Log), StubCMpPluginCfgTestLogVoid);
    CPluginManager cPluginManager;
    cPluginManager.PrintCmdModules();
}


TEST_F(CMpPluginManagerTest,LoadPlugin){
    CPluginManager m_CPluginManager;
    mp_string pszPlg;
    
    {
        stub.set(((CModule* (CPluginManager::*)(const mp_string&))ADDR(CPluginManager,GetModule)), StubGetModule);
        m_CPluginManager.LoadPlugin(pszPlg);
    }
    
    {
        stub.set(((CModule* (CPluginManager::*)(const mp_string&))ADDR(CPluginManager,GetModule)), StubGetModule0);
        stub.set(((CModule* (CPluginManager::*)(const mp_string&))ADDR(CPluginManager,Load)), StubLoad);
        m_CPluginManager.LoadPlugin(pszPlg);
    }
}

TEST_F(CMpPluginManagerTest,GetSCN){
    CPluginManager m_CPluginManager;
    
    m_CPluginManager.GetSCN();
}

TEST_F(CMpPluginManagerTest,UnloadPlugin){
    CPluginManager m_CPluginManager;
    CPluginManagerCBTest m_CPluginManagerCBTest;
    m_CPluginManager.m_Callback = &m_CPluginManagerCBTest;
    mp_char pszPlg;
    
    {
        m_CPluginManager.UnloadPlugin(&pszPlg);
    }
    
    {
        stub.set(((CModule* (CPluginManager::*)(const mp_string&))ADDR(CPluginManager,GetModule)), StubGetModule);
        m_CPluginManager.UnloadPlugin(&pszPlg);
    }
}

TEST_F(CMpPluginManagerTest,Load){
    CFileSearcher m_FileSearcher;
    mp_char pszName;
    mp_char pszVersion;
    CModule* m_CModule = new CModule(m_FileSearcher,&pszName,&pszVersion);
    
    {
        stub.set(((mp_bool (CFileSearcher::*)(const mp_string&, vector<mp_string>&))ADDR(CFileSearcher,Search)), StubSearch0);
        stub.set(&DlibOpenEx, StubDlibOpenEx0);
        m_CModule->Load();
    }
    
    {
        stub.set(((mp_bool (CFileSearcher::*)(const mp_string&, vector<mp_string>&))ADDR(CFileSearcher,Search)), StubSearch0);
        stub.set(&DlibOpenEx, StubDlibOpenEx);
        stub.set(&DlibDlsym, StubDlibDlsym0);
        stub.set(&DlibClose, StubDlibClose);
        m_CModule->Load();
    }
}

TEST_F(CMpPluginManagerTest,Reload){
    CPluginManager m_CPluginManager;
    CPluginManagerCBTest m_CPluginManagerCBTest;
    m_CPluginManager.m_Callback = &m_CPluginManagerCBTest;
    CModule* pModule;
    CFileSearcher m_FileSearcher;
    mp_string pszName;
    mp_string pszVersion;
    pModule = new CModule(m_FileSearcher,pszName,pszVersion);
    
    {
        stub.set(((CModule* (CPluginManager::*)(const mp_string&))ADDR(CPluginManager,Load)), StubLoad0);
        m_CPluginManager.Reload(*pModule);
    }

    {
        stub.set(((CModule* (CPluginManager::*)(const mp_string&))ADDR(CPluginManager,Load)), StubLoad);
        m_CPluginManager.Reload(*pModule);
    }
}



TEST_F(CMpPluginManagerTest,NeedReload){
    CPluginManager m_CPluginManager;
    CPluginManagerCBTest m_CPluginManagerCBTest;
    m_CPluginManager.m_Callback = &m_CPluginManagerCBTest;
    CModule* pModule;
    CFileSearcher m_FileSearcher;
    mp_string pszName;
    mp_string pszVersion;
    pModule = new CModule(m_FileSearcher,pszName,pszVersion);
    
    m_CPluginManager.NeedReload(*pModule);
}

TEST_F(CMpPluginManagerTest,UnloadOldModules){
    CPluginManager m_CPluginManager;
    CPluginManagerCBTest m_CPluginManagerCBTest;
    m_CPluginManager.m_Callback = &m_CPluginManagerCBTest;
    CFileSearcher m_FileSearcher;
    mp_string pszName;
    mp_string pszVersion;
    
    m_CPluginManager.m_OldModules.insert(pair<mp_string,CModule*>("test",new CModule(m_FileSearcher,pszName,pszVersion)));
    
    m_CPluginManager.UnloadOldModules();
}
