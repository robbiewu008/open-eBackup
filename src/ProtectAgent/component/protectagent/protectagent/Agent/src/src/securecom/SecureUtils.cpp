/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file Utils.cpp
 * @brief  Contains function declarations secure utils functions
 * @version 1.0.0
 * @date 2021-05-15
 * @author wangguitao 00510599
 */
#include <vector>
#include <iostream>
#include "common/ErrorCode.h"
#include "common/Path.h"
#include "common/Utils.h"
#include "common/ConfigXmlParse.h"
#include "common/CSystemExec.h"
#include "securecom/UniqueId.h"
#include "securecom/CryptAlg.h"
#include "securecom/SDPFunc.h"
#include "securecom/RootCaller.h"
#include "securecom/SecureUtils.h"

using std::vector;
namespace SecureCom {
#ifdef WIN32
const mp_string SCRIPT_LIST = "agent_func.bat;agent_func.ps1;agent_start.bat;agent_stop.bat;exchange.bat;"
    "initiator.bat;install-provider.cmd;online.bat;operexchangedb.ps1;oraasmaction.bat;oraclecheckarchive.bat;"
    "oracleconsistent.bat;oraclefunc.bat;oracleinfo.bat;oracleluninfo.bat;oracletest.bat;oraclecheckcdb.bat;"
    "oradbaction.bat;process_start.bat;process_stop.bat;procmonitor.bat;queryexchangeinfo.ps1;register_app.vbs;"
    "sqlserverinfo.bat;sqlserverluninfo.bat;sqlserverrecover.bat;sqlserversample.bat;uninstall-provider.cmd;"
    "packlog.bat;rotatenginxlog.bat;v2c_convert.bat;device_media_type.bat;device_media_type.ps1;cpu_gpu_info.bat;"
    "cpu_gpu_info.ps1;";
#else
const mp_string SCRIPT_LIST = "agent_sbin_func.sh;agent_start.sh;agent_stop.sh;db2clusterinfo.sh;"
    "db2info.sh;db2luninfo.sh;db2recover.sh;db2resourcegroup.sh;db2sample.sh;initiator.sh;oraclefunc.sh;oraasmaction."
    "sh;oraclecheckarchive.sh;oracleclusterinfo.sh;oracleinfo.sh;oracleluninfo.sh;"
    "oraclepdbinfo.sh;oraclecheckcdb.sh;oraclepdbstart.sh;oracleresourcegroup.sh;oracletest.sh;"
    "procmonitor.sh;lvmfunc.sh;packlog.sh;rotatenginxlog.sh;"
    "cachefunc.sh;cacheclusterinfo.sh;cacheinfo.sh;cacheluninfo.sh;cachesample.sh;scandisk.sh;"
    "hanainfo.sh;hanaluninfo.sh;hanaconsistent.sh;hanarecover.sh;hanatest.sh;hanafunc.sh;sybaseconsistent.sh;"
    "sybaserecover.sh;sybasetest.sh;sybasefunc.sh;agent_thirdpartyfunc.sh;"
    "MySQL_SingleNode_Freeze.sh;MySQL_SingleNode_Thaw.sh;MySQL_SingleNode_QueryFreeze.sh;MySQL_SingleNode_SimpleVolume_"
    "Start.sh;MySQL_SingleNode_LVM_Start.sh;MySQL_SingleNode_SimpleVolume_Stop.sh;"
    "MySQL_SingleNode_LVM_Stop.sh;checkcfg.sh;agent_install.sh;"
    "v2c_env.sh;v2c_modify_after_chroot.sh;v2c_mount_init.sh";
#endif
const mp_string CRT_USER_NAME = "/CN=";
const mp_string LOG_PACK_NAME_KEY = "logPackName=";
const mp_string LOG_OPERATION_KEY = "operation=";
const mp_string LOG_OPERATION_COLLECT = "collect";
const mp_string LOG_OPERATION_CLEAN = "clean";

/* ---------------------------------------------------------------------------
Function Name: GetOSType
Description  : 获取OS类型，需要使用root权限执行
Input        : OS_TYPE_E       -- 操作系统类型，获取后通过当前字段返回
------------------------------------------------------------- */
mp_void GetOSType(mp_int32 &iOSType)
{
    LOGGUARD("");
#ifdef WIN32
    iOSType = HOST_OS_WINDOWS;
    COMMLOG(OS_LOG_DEBUG, "Host is windows.");
#elif defined(AIX)
    iOSType = HOST_OS_AIX;
    COMMLOG(OS_LOG_DEBUG, "Host is AIX.");
#elif defined(HP_UX)
    iOSType = HOST_OS_HP_UX;
    COMMLOG(OS_LOG_DEBUG, "Host is HP-UX.");
#elif defined(SOLARIS)
    iOSType = HOST_OS_SOLARIS;
    COMMLOG(OS_LOG_DEBUG, "Host is Solaris.");
#elif defined(LINUX)
    // 条件判断顺序不能变化，因为oracle linux下也存在redhat-release文件
    if (MP_TRUE == CMpFile::FileExist("/etc/SuSE-release")) {
        iOSType = HOST_OS_SUSE;
        COMMLOG(OS_LOG_DEBUG, "Host is SUSE.");
    } else if (MP_TRUE == CMpFile::FileExist("/etc/oracle-release")) {
        iOSType = HOST_OS_ORACLE_LINUX;
        COMMLOG(OS_LOG_DEBUG, "Host is oracle linux.");
    } else if (MP_TRUE == CMpFile::FileExist("/etc/redhat-release")) {
        iOSType = HOST_OS_REDHAT;
        COMMLOG(OS_LOG_DEBUG, "Host is redhat.");
    } else {
        iOSType = HOST_OS_OTHER_LINUX;
        COMMLOG(OS_LOG_DEBUG, "Host is other linux.");
    }
#else
    iOSType = HOST_OS_UNKNOWN;
    COMMLOG(OS_LOG_WARN, "Canot not get OS type.");
#endif
}

// 获取OS版本信息
mp_int32 GetOSVersion(mp_int32 iOSType, mp_string &strOSVersion)
{
    strOSVersion = "";
#ifdef LINUX
    // 使用cat命令，简单处理
    mp_string strExecCmd;
    vector<mp_string> vecResult;
    mp_int32 iRet;

    if (iOSType == HOST_OS_SUSE) {
        strExecCmd = "cat /etc/SuSE-release | grep VERSION | awk  -F '=' '{print $2}' | sed 's/^[ \t]*//g'";
    } else if (iOSType == HOST_OS_ORACLE_LINUX || iOSType == HOST_OS_REDHAT) {
        // redhat和oracle linux都使用redhat-release获取版本号
        strExecCmd = "cat /etc/redhat-release | awk -F '.' '{print $1}' | awk '{print $NF}'";
    } else {
        COMMLOG(OS_LOG_WARN, "OS type is %d, not support to get os version.", iOSType);
        return MP_SUCCESS;
    }

    COMMLOG(OS_LOG_DEBUG, "cmd '%s' will be excute.", strExecCmd.c_str());
    iRet = CSystemExec::ExecSystemWithEcho(strExecCmd, vecResult);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Excute getting os version failed, iRet %d.", iRet);
        return iRet;
    }
    // 执行cat命令执行结果获取版本号
    if (vecResult.size() > 0) {
        strOSVersion = vecResult.front();
    } else {
        COMMLOG(OS_LOG_ERROR, "The result of getting os version is empty.");
        return MP_FAILED;
    }
    return MP_SUCCESS;
#else
    COMMLOG(OS_LOG_WARN, "OS type is %d, not support to get os version.", iOSType);
    return MP_SUCCESS;
#endif
}

/* ------------------------------------------------------------
Description  : generate script signature according to strFileName
Input        : strFileName -- scripr name
Output       : strFileSign -- script signature
Return       : MP_SUCCESS -- success
               not MP_SUCCESS -- failed, and return errorcode
Create By    : lishuai 00349472
------------------------------------------------------------- */
mp_int32 GenScriptSign(const mp_string& strFileName, mp_string &strFileSign)
{
    strFileSign.clear();
    mp_string strFilePath = CPath::GetInstance().GetSBinFilePath(strFileName);
    if (CMpFile::FileExist(strFilePath) == MP_FALSE) {
        strFilePath = CPath::GetInstance().GetThirdPartyFilePath(strFileName, AGENT_SAMPLE_SCRIPT);
    }
    mp_int32 iRet = ComputeHMAC(strFilePath, strFileSign);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "ComputeHMAC failed, iRet = %d.", iRet);
        return iRet;
    }
    return MP_SUCCESS;
}

// get admin user info from config file
mp_int32 GetAdminUserInfo(mp_string &userName, mp_string &userPwd)
{
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_BACKUP_SECTION, CFG_ADMINNODE_USER, userName);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get AdminNode user name failed.");
        return iRet;
    }

    mp_string encryptUserPwd;
    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_BACKUP_SECTION, CFG_ADMINNODE_PWD, encryptUserPwd);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get AdminNode user password failed.");
        return iRet;
    }

    // decrypt user password
    iRet = DecryptStrKmc(encryptUserPwd, userPwd);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "decrypt user password failed.");
        return iRet;
    }

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : generate all scripts' signature
Return       : MP_SUCCESS -- execute success
               not MP_SUCCESS -- failed, and return errorcode
Create By    : lishuai 00349472
------------------------------------------------------------- */
mp_int32 GenSignFile()
{
    vector<mp_string> vecScriptFiles;
    CMpString::StrSplit(vecScriptFiles, SCRIPT_LIST, ';');
    mp_int32 iRet = MP_SUCCESS;
    mp_string strScriptSignFile = CPath::GetInstance().GetConfFilePath(AGENT_SCRIPT_SIGN);
    if (CMpFile::FileExist(strScriptSignFile)) {
        iRet = CMpFile::DelFile(strScriptSignFile);
        if (iRet != MP_SUCCESS) {
            ERRLOG("Script sign file already exist, failed to delete, path is \"%s\", iRet = %d.",
                BaseFileName(strScriptSignFile).c_str(), iRet);
            return iRet;
        }
    }
    vector<mp_string> vecScriptSigns;
    mp_string strFileName;
    mp_string strFileSign;
    mp_string strTmp;
    for (mp_uint32 i = 0; i < vecScriptFiles.size(); i++) {
        strFileName = vecScriptFiles[i];
        iRet = GenScriptSign(strFileName, strFileSign);
        if (iRet == MP_SUCCESS) {
            strTmp = strFileName + " " + SIGN_FORMAT_STR + " " + strFileSign;
            vecScriptSigns.push_back(strTmp);
        }
    }
    iRet = CIPCFile::WriteFile(strScriptSignFile, vecScriptSigns);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Write script sign file failed, iRet = %d, size of vecResult is %d.", iRet, vecScriptSigns.size());
        return iRet;
    }
    COMMLOG(OS_LOG_INFO, "Generate script sign file succeeded.");
    return MP_SUCCESS;
}

/* ---------------------------------------------------------------------------
Function Name: PackageLog
Description  : 打包日志
------------------------------------------------------------- */
mp_int32 PackageLog(const mp_string& strLogName, bool isCollect)
{
    // 调用脚本打包日志
    // 部分日志是root权限，需要提升权限才可以导出，日志名称后缀有脚本添加
    COMMLOG(OS_LOG_INFO, "Package Log Name is %s.", strLogName.c_str());
    // 日志收集脚本输入日志打包名称不带后缀
    mp_string::size_type pos = strLogName.find(ZIP_SUFFIX);
    if (pos == mp_string::npos) {
        COMMLOG(OS_LOG_ERROR, "strLogName is invalid.", strLogName.c_str());
    }
    mp_string strLogNameWithoutSuffix = strLogName.substr(0, pos);
    mp_string operType = isCollect ? LOG_OPERATION_COLLECT : LOG_OPERATION_CLEAN;
    std::ostringstream scriptParam;
    scriptParam << LOG_OPERATION_KEY << operType << NODE_COLON
        << LOG_PACK_NAME_KEY << strLogNameWithoutSuffix;
    COMMLOG(OS_LOG_DEBUG, "Collect log params: %s", scriptParam.str().c_str());

#ifdef WIN32
    return SecureCom::SysExecScript(SCRIPT_PACKLOG_WIN, scriptParam.str(), nullptr);
#else
    CRootCaller rootCaller;
    CHECK_FAIL_EX(rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_PACKAGELOG, scriptParam.str(), nullptr));
    return MP_SUCCESS;
#endif
}

/* ------------------------------------------------------------
Function Name: CheckExecParam
Others       :-------------------------------------------------------- */
mp_int32 CheckExecParam(const mp_string &strScriptFileName, const mp_string &strParam, mp_bool &bNeedCheckSign,
    mp_string &strScriptFilePath, mp_string &strUniqueID)
{
    LOGGUARD("");

    // 判断脚本是否存在
    if (!CMpFile::FileExist(strScriptFilePath)) {
        COMMLOG(OS_LOG_ERROR, "Script is not exist, path is %s.", strScriptFilePath.c_str());
        // 统一在外部进行错误码转换
        return INTER_ERROR_SRCIPT_FILE_NOT_EXIST;
    }

    strScriptFilePath = CMpString::BlankComma(strScriptFilePath);

    if (!strParam.empty()) {
        // 将参数写入ipc文件
        mp_int32 iRet = CIPCFile::WriteInput(strUniqueID, strParam);
        if (iRet != MP_SUCCESS) {
            // 打印日志
            COMMLOG(OS_LOG_ERROR, "WriteInput failed, ret %d.", iRet);
            return iRet;
        }
    }
    return MP_SUCCESS;
}

mp_int32 SysExecScriptCmd(const mp_string& strScriptFileName, const mp_string& strSfile,
    const mp_string& strAPth, const mp_string& strUID, vector<mp_string> pvecResult[])
{
    // 组装命令
    mp_char acCmd[MAX_MAIN_CMD_LENGTH] = {0};
    // 为保证和R3C10兼容，保留以前的参数顺序UID,PATH
    mp_bool bIsThirdParty = MP_FALSE;
    if (strScriptFileName.find(AGENT_THIRDPARTY_DIR) != mp_string::npos) {
        bIsThirdParty = MP_TRUE;
        if (strSfile.find(CPath::GetInstance().GetThirdPartyPath()) != 0) {
            COMMLOG(OS_LOG_ERROR, "ThirdPartyScript path is invalid, %s.", strSfile.c_str());
            return MP_FAILED;
        }
        CHECK_FAIL(snprintf_s(acCmd, sizeof(acCmd), sizeof(acCmd) - 1, "%s %s %s", strSfile.c_str(), strUID.c_str(),
            strAPth.c_str()));
    } else { // 其他脚本的参数顺序是PATH,UID
        CHECK_FAIL(snprintf_s(acCmd, sizeof(acCmd), sizeof(acCmd) - 1, "%s %s %s", strSfile.c_str(), strAPth.c_str(),
            strUID.c_str()));
    }
    mp_string strCmd = acCmd;
    CHECK_FAIL_EX(CheckCmdDelimiter(strCmd));
    COMMLOG(OS_LOG_INFO, "Command \"%s\" will be executed, strCmd \"%s\".", strScriptFileName.c_str(), strCmd.c_str());
    // 执行脚本均不获取回显
    mp_int32 iRet = CSystemExec::ExecSystemWithoutEcho(strCmd);
    // 执行第三方脚本无论成功失败都需要读取结果文件，此处不能直接返回。
    if (!bIsThirdParty && iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "ExecSystemWithoutEcho failed.");
        return iRet;
    }

    // 读取结果文件
    mp_int32 iRet1 = MP_SUCCESS;
    if (pvecResult != nullptr) {
        bIsThirdParty ? (iRet1 = CIPCFile::ReadOldResult(strUID, *pvecResult)) :
                        (iRet1 = CIPCFile::ReadResult(RESULT_TMP_FILE + strUID, *pvecResult));
        if (iRet1 != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Read result file failed.");
        }
    }

    return (iRet == MP_SUCCESS) ? iRet1 : iRet;
}

/* ------------------------------------------------------------
Function Name: SysExecScript
Description  : 在当前agent用户下执行脚本，执行结果通过函数返回值返回
Input        : strScriptFileName 脚本文件名称
               strParam 脚本输入参数
Return       : 直接返回脚本执行返machenglin回值，如需转换，请在外层转换
Others       :-------------------------------------------------------- */
mp_int32 SysExecScript(const mp_string& strScriptFileName, const mp_string &strParam, vector<mp_string> pvecResult[],
    mp_bool bNeedCheckSign, pFunc cb)
{
    LOGGUARD("");
    // 获取agent路径
    mp_string strAPth = CPath::GetInstance().GetRootPath();
    // 获取脚本全路径
#ifdef WIN32
    mp_string strSfile = CPath::GetInstance().GetBinFilePath(strScriptFileName);
#else
    mp_string strSfile = CPath::GetInstance().GetSBinFilePath(strScriptFileName);
#endif // WIN32
    CMpString::FormattingPath(strSfile);
    // 获取唯一ID，用于生成临时文件
    mp_string strUID = CUniqueID::GetInstance().GetString();

    strAPth = CMpString::BlankComma(strAPth);
    mp_int32 iRet = CheckExecParam(strScriptFileName, strParam, bNeedCheckSign, strSfile, strUID);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Check exec param failed, ret %d.", iRet);
        return iRet;
    }

    // callback update pid
    if (cb != nullptr) {
        iRet = cb(strUID);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Update inner PID failed, ret %d.", iRet);
            return iRet;
        }
    }
    iRet = SysExecScriptCmd(strScriptFileName, strSfile, strAPth, strUID, pvecResult);
    return iRet;
}

/* ------------------------------------------------------------
Function Name: SysExecUserScript
Description  : 在当前agent用户下执行用户指定路径的脚本，执行结果通过函数返回值返回
Input        : strScriptFileName 脚本文件名称
               strParam 脚本输入参数
Return       : 直接返回脚本执行返回值，如需转换，请在外层转换
Others       :-------------------------------------------------------- */
mp_int32 SysExecUserScript(const mp_string& strScriptFileName, const mp_string &strParam,
    vector<mp_string> pvecResult[], mp_bool bNeedCheckSign, pFunc cb)
{
    LOGGUARD("Enter SysExecUserScript. script: %s.", strScriptFileName.c_str());
    // 获取agent路径
    mp_string strAPth = CPath::GetInstance().GetRootPath();
    strAPth = CMpString::BlankComma(strAPth);
    // 获取脚本全路径
    mp_string strSfile = strScriptFileName;
    CMpString::FormattingPath(strSfile);
    // 获取唯一ID，用于生成临时文件
    mp_string strUID = CUniqueID::GetInstance().GetString();
    mp_int32 iRet = CheckExecParam(strScriptFileName, strParam, bNeedCheckSign, strSfile, strUID);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Check exec param failed, ret %d.", iRet);
        return iRet;
    }

    // callback update pid
    if (cb != nullptr) {
        iRet = cb(strUID);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Update inner PID failed, ret %d.", iRet);
            return iRet;
        }
    }
    iRet = SysExecScriptCmd(strScriptFileName, strSfile, strAPth, strUID, pvecResult);
    return iRet;
}

static thread_lock_t *lockCs;
static mp_long *lockCount;

mp_ulong ThreadID(mp_void)
{
    return static_cast<mp_ulong>(CMpThread::GetThreadId());
}

mp_void LockCallback(mp_int32 mode, mp_int32 type, const mp_char *file, mp_int32 line)
{
    if (mode & CRYPTO_LOCK) {
        (mp_void)CMpThread::Lock(&(lockCs[type]));
        lockCount[type]++;
    } else {
        CMpThread::Unlock(&(lockCs[type]));
    }
}

mp_bool CryptoThreadSetup()
{
    lockCs = (thread_lock_t *)OPENSSL_malloc(CRYPTO_num_locks() * sizeof(thread_lock_t));
    lockCount = (mp_long *)OPENSSL_malloc(CRYPTO_num_locks() * sizeof(mp_long));
    if (lockCs == nullptr || lockCount == nullptr) {
        if (lockCs != nullptr) {
            OPENSSL_free(lockCs);
        }
        if (lockCount != nullptr) {
            OPENSSL_free(lockCount);
        }
        return false;
    }

    for (mp_int32 i = 0; i < CRYPTO_num_locks(); i++) {
        lockCount[i] = 0;
        CMpThread::InitLock(&(lockCs[i]));
    }

    CRYPTO_set_id_callback(static_cast<mp_ulong (*)()>(ThreadID));
    CRYPTO_set_locking_callback(static_cast<void (*)(int, int, const char *, int)>(LockCallback));
    return true;
}

mp_void CryptoThreadCleanup(mp_void)
{
    CRYPTO_set_locking_callback(nullptr);
    for (int i = 0; i < CRYPTO_num_locks(); i++) {
        CMpThread::DestroyLock(&(lockCs[i]));
    }
    OPENSSL_free(lockCs);
    OPENSSL_free(lockCount);
    INFOLOG("Cleanup multi-threads openssl running environment successfully!");
}

mp_void InnerFreeGetHostMem(BIO *key, X509 *pCert, char *pSubject)
{
    if (key != nullptr) {
        BIO_free_all(key);
    }
    if (pCert != nullptr) {
        X509_free(pCert);
    }
    if (pSubject != nullptr) {
        OPENSSL_free(pSubject);
    }
}

mp_void GetHostFromCert(const mp_string &certPath, mp_string &hostName)
{
    BIO *key = nullptr;
    X509 *pCert = nullptr;
    mp_string errorStr = "";
    char *pSubject = nullptr;

    do {
        key = BIO_new_file(certPath.c_str(), "r");
        if (key == nullptr) {
            errorStr = "BIO_new_file key is nullptr";
            break;
        }

        pCert = PEM_read_bio_X509(key, nullptr, nullptr, nullptr);
        if (pCert == nullptr) {
            errorStr = "PEM_read_bio_X509 failed.";
            break;
        }

        X509_NAME *name = X509_get_subject_name(pCert);
        if (name == nullptr) {
            errorStr = "get x509 subject name failed.";
            break;
        }
        pSubject = X509_NAME_oneline(name, 0, 0);
        if (pSubject == nullptr) {
            errorStr = "Invalid Cert.";
            break;
        }

        COMMLOG(OS_LOG_INFO, "subjectName:%s", pSubject);

        mp_string tmpHost(pSubject);
        int index = tmpHost.find(CRT_USER_NAME);
        if (index == mp_string::npos) {
            errorStr = "Cannot find hostName flag.";
            break;
        }

        index += CRT_USER_NAME.length();
        hostName = tmpHost.substr(index, tmpHost.length() - index);
        if (hostName.empty()) {
            errorStr = "host name is empty.";
            break;
        }

        index = hostName.find("/");
        if (index == mp_string::npos) {
            break;
        }
        hostName = hostName.substr(0, index);
    } while (0);

    InnerFreeGetHostMem(key, pCert, pSubject);
    if (!errorStr.empty()) {
        COMMLOG(OS_LOG_ERROR, errorStr);
    }
}
} // namespace SecureCom
