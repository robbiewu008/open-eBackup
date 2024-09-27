#define private public
#include "pluginfx/PluginCfgParseTest.h"

using namespace std;
using namespace tinyxml2;

namespace {
mp_bool LoadCfgFalse(const mp_string& pszFileName)
{
    return MP_FALSE;
}

mp_bool LoadCfgTrue(const mp_string& pszFileName)
{
    return MP_TRUE;
}

plugin_def_t* GetPluginNULL(const mp_string& pszPluginName)
{
    return NULL;
}

plugin_def_t* GetPluginNoNULL(const mp_string& pszPluginName)
{
    plugin_def_t *p = new plugin_def_t;
    p->version = "1.1.1";
    return p;
}

mp_int32 GetlLastModifyTimeFailed(const mp_char* pszFilePath, mp_time& tLastModifyTime)
{
    return MP_FAILED;
}

mp_int32 GetlLastModifyTimeSucc(const mp_char* pszFilePath, mp_time& tLastModifyTime)
{
    return MP_SUCCESS;
}

}

static mp_void StubCLoggerLog(mp_void){
    return;
}

TEST_F(CMpPluginCfgParseTest, PluginCfgParseTest)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubCMpPluginCfgTestLogVoid);
    try
    {

        /********Begin PluginCfgParse.cpp test********/

        // write config information to temp file
        vector<mp_string> vecTmpCfgFile;
        vecTmpCfgFile.push_back("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
        vecTmpCfgFile.push_back("<Config>");
        vecTmpCfgFile.push_back("    <PluginList>");
        vecTmpCfgFile.push_back("        <Plugin name=\"liboracle\" version=\"V100R005C00\" service=\"oracle\">");
        vecTmpCfgFile.push_back("        </Plugin>");
        vecTmpCfgFile.push_back("        <Plugin name=\"libdb2\" version=\"V100R005C00\" service=\"db2\">");
        vecTmpCfgFile.push_back("        </Plugin>");
        vecTmpCfgFile.push_back("        <Plugin name=\"libhost\" version=\"V100R005C00\" service=\"host\">");
        vecTmpCfgFile.push_back("        </Plugin>");
        vecTmpCfgFile.push_back("        <Plugin name=\"libdevice\" version=\"V100R005C00\" service=\"device\">");
        vecTmpCfgFile.push_back("        </Plugin>");
        vecTmpCfgFile.push_back("        <Plugin name=\"libexchange\" version=\"V100R005C00\" service=\"exchange\">");
        vecTmpCfgFile.push_back("        </Plugin>");
        vecTmpCfgFile.push_back("        <Plugin name=\"libcluster\" version=\"V100R005C00\" service=\"cluster\">");
        vecTmpCfgFile.push_back("        </Plugin>");
        vecTmpCfgFile.push_back("        <Plugin name=\"libsqlserver\" version=\"V100R005C00\" service=\"sqlserver\">");
        vecTmpCfgFile.push_back("        </Plugin>");
        vecTmpCfgFile.push_back("    </PluginList>");
        vecTmpCfgFile.push_back("    <ServiceList>");
        vecTmpCfgFile.push_back("        <Service name=\"oracle\" plugin=\"liboracle\">");
        vecTmpCfgFile.push_back("        </Service>");
        vecTmpCfgFile.push_back("        <Service name=\"db2\" plugin=\"libdb2\">");
        vecTmpCfgFile.push_back("        </Service>");
        vecTmpCfgFile.push_back("        <Service name=\"exchange\" plugin=\"libexchange\">");
        vecTmpCfgFile.push_back("        </Service>");
        vecTmpCfgFile.push_back("    </ServiceList>");
        vecTmpCfgFile.push_back("</Config>");
        mp_string tmpCfgFile("gtestcfg.xml");
        
        // delete file
        CMpFile::DelFile(tmpCfgFile.c_str());

        PluginCfgParse pluginParse;
        // NULL file
        mp_int32 iRet = pluginParse.Init("");
        EXPECT_EQ(iRet, ERROR_COMMON_READ_CONFIG_FAILED);

        // not exist file
        iRet = pluginParse.Init((mp_char *)tmpCfgFile.c_str());
        EXPECT_EQ(iRet, ERROR_COMMON_READ_CONFIG_FAILED);

        // exist file
        // CIPCFile::WriteFile(tmpCfgFile, vecTmpCfgFile);
        iRet = pluginParse.Init(tmpCfgFile);
        EXPECT_EQ(iRet, ERROR_COMMON_READ_CONFIG_FAILED);

        // get not exists services
        plugin_def_t pluginInfo;
        iRet = pluginParse.GetPluginByService(mp_string("sqlserver1111"), pluginInfo);
        EXPECT_EQ(iRet, MP_FAILED);

        // check exists services
        iRet = pluginParse.GetPluginByService(mp_string("oracle"), pluginInfo);
        EXPECT_EQ(iRet, MP_FAILED);
        // EXPECT_EQ("oracle", pluginInfo.service);
        // EXPECT_EQ("liboracle", pluginInfo.name);
        // EXPECT_EQ("V100R005C00", pluginInfo.version);

        mp_string pluginVersion;
        // get no exists plugin
        iRet = pluginParse.GetPluginVersion("sqlserver", pluginVersion);
        EXPECT_EQ(iRet, ERROR_COMMON_READ_CONFIG_FAILED);

        // get exists plugin
        iRet = pluginParse.GetPluginVersion("liboracle", pluginVersion);
        EXPECT_EQ(iRet, ERROR_COMMON_READ_CONFIG_FAILED);
        // EXPECT_EQ("V100R005C00", pluginVersion);
        

        CMpFile::DelFile(tmpCfgFile.c_str());
        iRet = pluginParse.GetPluginVersion("", pluginVersion);
        /********End PluginCfgParse.cpp test********/
    }
    catch(...)
    {
        printf("Error on %s file %d line.\n", __FILE__, __LINE__);
        exit(0);
    }
}

TEST_F(CMpPluginCfgParseTest, InitTest)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger,Log), StubCMpPluginCfgTestLogVoid);
    PluginCfgParse pLuginCfgParse;
    mp_string fileName;

    mp_int32 iRet = pLuginCfgParse.Init(fileName);
    EXPECT_EQ(iRet, ERROR_COMMON_READ_CONFIG_FAILED);

    fileName = "Agent";
    stub.set(ADDR(PluginCfgParse, LoadCfg), LoadCfgFalse);
    iRet = pLuginCfgParse.Init(fileName);
    EXPECT_EQ(iRet, ERROR_COMMON_READ_CONFIG_FAILED);

    stub.set(ADDR(PluginCfgParse, LoadCfg), LoadCfgTrue);
    iRet = pLuginCfgParse.Init(fileName);
    EXPECT_EQ(iRet, MP_SUCCESS);
}

TEST_F(CMpPluginCfgParseTest, GetPluginByServiceTest)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger,Log), StubCMpPluginCfgTestLogVoid);
    PluginCfgParse pLuginCfgParse;
    mp_string strServiceName = "Agent";
    plugin_def_t plugin;
    plugin.service = strServiceName;

    mp_int32 iRet = pLuginCfgParse.GetPluginByService(strServiceName, plugin);
    EXPECT_EQ(iRet, MP_FAILED);

    pLuginCfgParse.m_vecPlgDefs.push_back(plugin);
    iRet = pLuginCfgParse.GetPluginByService(strServiceName, plugin);
    EXPECT_EQ(iRet, MP_SUCCESS);
}

TEST_F(CMpPluginCfgParseTest, GetPreLoadPlugins1Test)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger,Log), StubCMpPluginCfgTestLogVoid);
    PluginCfgParse pLuginCfgParse;
    plugin_def_t plugin;
    std::vector<plugin_def_t> plugins;

    plugin.lazyload = MP_FALSE;
    plugin.service = "Agent";
    pLuginCfgParse.m_vecPlgDefs.push_back(plugin);
    pLuginCfgParse.GetPreLoadPlugins(plugins);
    EXPECT_TRUE(plugin.service == plugins.back().service);
}

TEST_F(CMpPluginCfgParseTest, GetPreLoadPlugins2Test)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger,Log), StubCMpPluginCfgTestLogVoid);
    PluginCfgParse pLuginCfgParse;
    plugin_def_t plugin;
    mp_string name = "Agent";
    
    plugin.lazyload = MP_FALSE;
    plugin.name = name;
    pLuginCfgParse.m_vecPlgDefs.push_back(plugin);
    vector<mp_string> vecPlgNames;
    pLuginCfgParse.GetPreLoadPlugins(vecPlgNames);
    EXPECT_TRUE(name == vecPlgNames.back());
}

TEST_F(CMpPluginCfgTest, GetPluginVersionTest)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger,Log), StubCMpPluginCfgTestLogVoid);
    PluginCfgParse pLuginCfgParse;
    mp_string plgName;
    mp_string strVersion;
    
    mp_int32 iRet = pLuginCfgParse.GetPluginVersion(plgName, strVersion);
    EXPECT_EQ(iRet, ERROR_COMMON_READ_CONFIG_FAILED);
    
    plgName = "Agent";
    stub.set(ADDR(PluginCfgParse, GetPlugin), GetPluginNULL);
    iRet = pLuginCfgParse.GetPluginVersion(plgName, strVersion);
    EXPECT_EQ(iRet, ERROR_COMMON_READ_CONFIG_FAILED);

    stub.set(ADDR(PluginCfgParse, GetPlugin), GetPluginNoNULL);
    iRet = pLuginCfgParse.GetPluginVersion(plgName, strVersion);
    EXPECT_EQ(iRet, MP_SUCCESS);
}

TEST_F(CMpPluginCfgTest, LoadCfgTest)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger,Log), StubCMpPluginCfgTestLogVoid);
    PluginCfgParse pLuginCfgParse;
    mp_string fileName;
    
    mp_bool iRet = pLuginCfgParse.LoadCfg(fileName);
    EXPECT_FALSE(iRet);

    fileName = "Agent";
    stub.set(ADDR(CMpFile, GetlLastModifyTime), GetlLastModifyTimeFailed);
    iRet = pLuginCfgParse.LoadCfg(fileName);
    EXPECT_FALSE(iRet);

    stub.set(ADDR(CMpFile, GetlLastModifyTime), GetlLastModifyTimeSucc);
    iRet = pLuginCfgParse.LoadCfg(fileName);
    EXPECT_FALSE(iRet);
}

TEST_F(CMpPluginCfgTest, LoadPluginDefsTest)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger,Log), StubCMpPluginCfgTestLogVoid);
    PluginCfgParse pLuginCfgParse;
    XMLDocument doc;
    XMLElement* root = doc.NewElement("test");

    mp_bool iRet = pLuginCfgParse.LoadPluginDefs(*root);
    EXPECT_TRUE(iRet);
}

TEST_F(CMpPluginCfgTest, LoadPluginDefTest)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger,Log), StubCMpPluginCfgTestLogVoid);
    PluginCfgParse pLuginCfgParse;
    XMLDocument doc;
    XMLElement* root = doc.NewElement("test");

    mp_bool iRet = pLuginCfgParse.LoadPluginDef(*root);
    EXPECT_FALSE(iRet);
}

TEST_F(CMpPluginCfgParseTest, AddPluginDefTest)
{
    PluginCfgParse pLuginCfgParse;
    plugin_def_t plgDef;
    plgDef.name = "Agent";
    pLuginCfgParse.AddPluginDef(plgDef);
}

TEST_F(CMpPluginCfgParseTest, GetPluginTest)
{
    PluginCfgParse pLuginCfgParse;
    mp_string pluginName = "Agent";

    plugin_def_t* ret = pLuginCfgParse.GetPlugin(pluginName);
    EXPECT_TRUE(ret == NULL);

    plugin_def_t plugin;
    plugin.name = pluginName;
    pLuginCfgParse.m_vecPlgDefs.push_back(plugin);
    ret = pLuginCfgParse.GetPlugin(pluginName);
    EXPECT_TRUE(ret->name == pluginName);
}
