#include <map>
#include "common/Log.h"
#include "common/Path.h"
#include "common/ConfigXmlParse.h"
#include "common/Utils.h"
#include "securecom/SecureUtils.h"
#include "securecom/CryptAlg.h"
#include "pluginfx/ExternalPluginSDK.h"

AGENT_EXPORT enum SecurityItem {
    CRETIFICATE_ROOT_PATH,
    KEY_FILE_NAME,
    TRUSTE_CRETIFICATE_FILE_NAME,
    USE_CRETIFICATE_FILE_NAME,
    PASSWORD,
    ALGORITEHM_SUITE,
    HOST_NAME
};

const std::string DEFAULT_CAINFO = "agentca.pem";
const std::string DEFAULT_SSLCERT = "server.pem";
const std::string DEFAULT_SSLKEY = "server.key";
using FilePair = std::pair<std::string, std::string>;
static std::map<SecurityItem, FilePair> g_securityFileName = {
    {SecurityItem::TRUSTE_CRETIFICATE_FILE_NAME, {CFG_AGENT_CA_INFO, DEFAULT_CAINFO}},
    {SecurityItem::USE_CRETIFICATE_FILE_NAME, {CFG_SSL_CERT, DEFAULT_SSLCERT}},
    {SecurityItem::KEY_FILE_NAME, {CFG_SSL_KEY, DEFAULT_SSLKEY}}
};

namespace {
    bool  g_initKmc = false;
}


/*
 * The initialization function is used to allocate global configuration.
 * The parameter rootPath is protectagent running root directory.
 */
AGENT_EXPORT uint32_t OpaInitialize(const std::string& rootPath, bool isInitKmc)
{
    if (rootPath.empty()) {
        CPath::GetInstance().SetRootPath("/opt/DataBackup/ProtectClient/ProtectClient-E");
    } else {
        mp_string strParam = rootPath;
        if (CheckCmdDelimiter(strParam) != MP_SUCCESS) {
            return OPA_FAILED;
        }
        CPath::GetInstance().SetRootPath(rootPath);
    }

    static const std::string FRAMEWORK_LOG_NAME = "rdagentsdk.log";
    static const std::string FRAMEWORK_XML_CONF = "agent_cfg.xml";
    CLogger::GetInstance().Init(FRAMEWORK_LOG_NAME.c_str(), CPath::GetInstance().GetLogPath());

    auto iRet = CConfigXmlParser::GetInstance().Init(CPath::GetInstance().GetConfFilePath(FRAMEWORK_XML_CONF));
    if (iRet != MP_SUCCESS) {
        ERRLOG("Init conf file %s failed.", FRAMEWORK_XML_CONF.c_str());
        return OPA_FAILED;
    }
    g_initKmc = isInitKmc;
    if (g_initKmc) {
        iRet = InitCrypt(KMC_ROLE_TYPE_MASTER);
        if (iRet != MP_SUCCESS) {
            ERRLOG("Init KMC failed.");
            return OPA_FAILED;
        }
    }
    INFOLOG("OpaInitialize return success");
    return OPA_SUCCESS;
}

/*
 * The uninitialization function is used to free global configuration.
 */
AGENT_EXPORT uint32_t OpaUninitialize()
{
    CLogger::GetInstance().SetWriteLogFunc(nullptr);
    return OPA_SUCCESS;
}

/*
 * Registers the callback function registered by the app
 */
AGENT_EXPORT uint32_t OpaRegFunc(const OpaCallbacks& allFuncs)
{
    if (allFuncs.writeLog != nullptr) {
        CLogger::GetInstance().SetWriteLogFunc(allFuncs.writeLog);
        return OPA_SUCCESS;
    }

    static const std::string FRAMEWORK_LOG_NAME = "rdagentsdk.log";
    static const std::string FRAMEWORK_XML_CONF = "agent_cfg.xml";
    CLogger::GetInstance().Init(FRAMEWORK_LOG_NAME.c_str(), CPath::GetInstance().GetLogPath());

    int32_t iRet = CConfigXmlParser::GetInstance().Init(CPath::GetInstance().GetConfFilePath(FRAMEWORK_XML_CONF));
    if (iRet != MP_SUCCESS) {
        ERRLOG("Init conf file %s failed.", FRAMEWORK_XML_CONF.c_str());
        return OPA_FAILED;
    }

    return OPA_SUCCESS;
}

static bool GetPassword(std::string& pw)
{
    auto ret = CConfigXmlParser::GetInstance().GetValueString(
        CFG_MONITOR_SECTION, CFG_NGINX_SECTION, CFG_SSL_KEY_PASSWORD, pw);
    if (ret != MP_SUCCESS) {
        return false;
    }
    return true;
}

static bool GetAlgorithmSuite(std::string& suite)
{
    auto ret = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION,
        CFG_ALGORITHM_SUITE, suite);
    if (ret != MP_SUCCESS) {
        return false;
    }
    return true;
}

static bool GetCertificateFileNameFromXml(const std::string& config, std::string& value)
{
    auto ret = CConfigXmlParser::GetInstance().GetValueString(CFG_SECURITY_SECTION,
        config, value);
    if (ret != MP_SUCCESS || value.empty()) {
        return false;
    }
    return true;
}

static std::string GetCertificateFileName(SecurityItem type)
{
    if (type != SecurityItem::TRUSTE_CRETIFICATE_FILE_NAME &&
        type != SecurityItem::USE_CRETIFICATE_FILE_NAME &&
        type != SecurityItem::KEY_FILE_NAME) {
        return "";
    }
    std::string ret;
    if (GetCertificateFileNameFromXml(g_securityFileName[type].first, ret)) {
        return ret;
    }
    WARNLOG("Found file name from config file failed, return default file name");
    return g_securityFileName[type].second;
}

static std::string GetCertificateRootPath()
{
    return CPath::GetInstance().GetNginxConfFilePath("");
}

static bool GetHostHttpName(std::string& host)
{
    std::string useCertificate = GetCertificateRootPath() + "/" +
        GetCertificateFileName(SecurityItem::USE_CRETIFICATE_FILE_NAME);
    SecureCom::GetHostFromCert(useCertificate, host);
    if (host.empty()) {
        host = "DataBackup-AGENT";
    }
    return true;
}

EXTER_ATTACK static int32_t GetSecurityItemValueImpl(SecurityItem item, std::string& value)
{
    switch (item) {
        case CRETIFICATE_ROOT_PATH:
            value = GetCertificateRootPath();
            break;
        
        case PASSWORD:
            GetPassword(value);
            break;
        
        case ALGORITEHM_SUITE:
            GetAlgorithmSuite(value);
            break;
        
        case HOST_NAME:
            GetHostHttpName(value);
            break;
        
        case KEY_FILE_NAME:
        case TRUSTE_CRETIFICATE_FILE_NAME:
        case USE_CRETIFICATE_FILE_NAME:
            value = GetCertificateFileName(item);
            break;

        default:
            break;
    }

    if (!value.empty()) {
        return MP_SUCCESS;
    }
    return MP_FAILED;
}

static int32_t DecryptImpl(std::string input, std::string& output)
{
    DecryptStr(input, output);
    if (output.empty()) {
        WARNLOG("DecryptStr password output is empty");
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

AGENT_EXPORT int32_t GetSecurityItemValue(int32_t item, std::string& value)
{
    return GetSecurityItemValueImpl(static_cast<SecurityItem>(item), value);
}

AGENT_EXPORT int32_t Decrypt(std::string input, std::string& output)
{
    if (!g_initKmc) {
        ERRLOG("The kmc is not initialized. The decryption function cannot be used.");
        return MP_FAILED;
    }
    return DecryptImpl(input, output);
}
