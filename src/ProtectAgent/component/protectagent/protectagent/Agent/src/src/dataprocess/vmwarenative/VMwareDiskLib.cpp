#include "dataprocess/vmwarenative/VMwareDiskLib.h"

#include <fstream>
#include <regex>
#include <iostream>
#include <dlfcn.h>

#include "common/Log.h"
#include "common/Ip.h"
#include "common/Path.h"
#include "dataprocess/vmwarenative/VMwareDiskApi.h"
#include "dataprocess/vmwarenative/VddkDeadlockCheck.h"

namespace {
    const mp_double VERSION_CAN_USE_QUERYALLOCATEBLOCKS = 6.0;
}

std::mutex VMwareDiskLib::m_mutex;
mp_string VMwareDiskLib::VDDK_CONFI_FILE_NAME = "vddk.cfg";
mp_string VMwareDiskLib::VIX_DISK_LIBRARY_NAME = "libvixDiskLib.so";
mp_string VMwareDiskLib::APPLICATION_NAME = "AgentVMwareBackupPlugin";

// extract function address from VDDK lib handle
template<typename FunType>
bool ExtractFuction(void *handle, const std::string &funName, FunType &funPtr)
{
    if (handle == nullptr) {
        COMMLOG(OS_LOG_ERROR, "Invalid VDDK library handle!");
        return false;
    }

    mp_char *dlErrStr = dlerror();
    // Obtain function names according to VDDK lib handle and method names
    mp_void *p = dlsym(handle, funName.c_str());
    funPtr = reinterpret_cast<FunType>(p);
    if (funPtr == nullptr || dlErrStr != nullptr) {
        COMMLOG(OS_LOG_ERROR, "Get function ['%s'] failed from library. Error msg: '%s'!", funName.c_str(),
            dlErrStr);
        return false;
    }

    return true;
}
VMwareDiskLib::VMwareDiskLib() : m_isInitialized(false), m_vddkLibHandle(nullptr) {}
bool VMwareDiskLib::Init()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_isInitialized) {
        COMMLOG(OS_LOG_DEBUG, "The Vddk lib has alreadly been inited!");
        return true;
    }

    if (!StartVddkThread()) {
        return false;
    } else {
        COMMLOG(OS_LOG_INFO, "Start VDDK thread successfully!");
    }

    if (!LoadVddkLibFile()) {
        COMMLOG(OS_LOG_ERROR, "Unable to load vddk library!");
        return false;
    } else {
        COMMLOG(OS_LOG_INFO, "Load vddk library successfully!");
    }
    bool ret = InitExtractFuction();
    if (!ret) {
        COMMLOG(OS_LOG_ERROR, "Unable to extract funtion from vddk library!");
        return false;
    } else {
        COMMLOG(OS_LOG_INFO, "Extract funtion from vddk library successfully!");
    }

    if (!InitVddkLibEnv()) {
        COMMLOG(OS_LOG_ERROR, "Unable to init VDDK lib env!");
        return false;
    } else {
        COMMLOG(OS_LOG_INFO, "init VDDK lib env successfully!");
    }

    m_isInitialized = true;
    return true;
}

bool VMwareDiskLib::InitExtractFuction()
{
    bool ret = ExtractFuction(m_vddkLibHandle, "VixDiskLib_Init", m_vddkOperations.vixDiskLibInit) &&
        ExtractFuction(m_vddkLibHandle, "VixDiskLib_InitEx", m_vddkOperations.vixDiskLibInitEx) &&
        ExtractFuction(m_vddkLibHandle, "VixDiskLib_Exit", m_vddkOperations.vixDiskLibExit) &&
        ExtractFuction(m_vddkLibHandle, "VixDiskLib_Connect", m_vddkOperations.vixDiskLibConnect) &&
        ExtractFuction(m_vddkLibHandle, "VixDiskLib_ConnectEx", m_vddkOperations.vixDiskLibConnectEx) &&
        ExtractFuction(m_vddkLibHandle, "VixDiskLib_Disconnect", m_vddkOperations.vixDiskLibDisconnect) &&
        ExtractFuction(m_vddkLibHandle, "VixDiskLib_PrepareForAccess", m_vddkOperations.vixDiskLibPrepareForAccess) &&
        ExtractFuction(m_vddkLibHandle, "VixDiskLib_EndAccess", m_vddkOperations.vixDiskLibEndAccess) &&
        ExtractFuction(m_vddkLibHandle, "VixDiskLib_Open", m_vddkOperations.vixDiskLibOpen) &&
        ExtractFuction(m_vddkLibHandle, "VixDiskLib_Close", m_vddkOperations.vixDiskLibClose) &&
        ExtractFuction(m_vddkLibHandle, "VixDiskLib_GetInfo", m_vddkOperations.vixDiskLibGetInfo) &&
        ExtractFuction(m_vddkLibHandle, "VixDiskLib_FreeInfo", m_vddkOperations.vixDiskLibFreeInfo) &&
        ExtractFuction(m_vddkLibHandle, "VixDiskLib_Read", m_vddkOperations.vixDiskLibRead) &&
        ExtractFuction(m_vddkLibHandle, "VixDiskLib_Write", m_vddkOperations.vixDiskLibWrite) &&
        ExtractFuction(m_vddkLibHandle, "VixDiskLib_GetErrorText", m_vddkOperations.vixDiskLibGetErrorText) &&
        ExtractFuction(m_vddkLibHandle, "VixDiskLib_FreeErrorText", m_vddkOperations.vixDiskLibFreeErrorText) &&
        ExtractFuction(m_vddkLibHandle, "VixDiskLib_ListTransportModes",
                       m_vddkOperations.vixDiskLibListTransportModes) &&
        ExtractFuction(m_vddkLibHandle, "VixDiskLib_Cleanup", m_vddkOperations.vixDiskLibCleanup) &&
        ExtractFuction(m_vddkLibHandle, "VixDiskLib_GetTransportMode", m_vddkOperations.vixDiskLibGetTransportMode);
    if (m_version > VERSION_CAN_USE_QUERYALLOCATEBLOCKS) {
        ret = ret && ExtractFuction(m_vddkLibHandle, "VixDiskLib_QueryAllocatedBlocks",
            m_vddkOperations.vixDiskLibQueryAllocateBlocks) &&
        ExtractFuction(m_vddkLibHandle, "VixDiskLib_FreeBlockList", m_vddkOperations.vixDiskLib_FreeBlockList);
        INFOLOG("current version[%.1f] can use queryAllocateBlocks function", m_version);
    }

    return ret;
} // release connection with remote product env
VMWARE_DISK_RET_CODE VMwareDiskLib::Uninit(const VddkConnectParams &connectParams, std::string &errDesc)
{
    VddkDeadlockCheck::DeadlockCheck check("Uninit");
    std::unique_lock<std::mutex> lock(m_mutex);
    VMWARE_DISK_RET_CODE rc = VIX_OK;
    auto iter = m_connMap.find(connectParams.vmMoRef);
    if (iter != m_connMap.end()) {
        rc = DisConnect(iter->second);
        if (rc != VIX_OK) {
            errDesc = GetErrString(rc);
            COMMLOG(OS_LOG_ERROR, "Disconnect with remote vCenter or ESXI failed: '%s'.", errDesc.c_str());
            return rc;
        }
        m_connMap.erase(iter);
    }

    return rc;
}
bool VMwareDiskLib::StartVddkThread()
{
    if (!m_vddkThread.IsRunning()) {
        if (!m_vddkThread.Start()) {
            COMMLOG(OS_LOG_ERROR, "Unable to start deadlock thread!");
            return false;
        }
    }
    return true;
}

bool VMwareDiskLib::LoadVddkLibFile()
{
    if (m_vddkLibHandle != nullptr) {
        return true;
    }

    m_vddkLibHandle = dlopen(VIX_DISK_LIBRARY_NAME.c_str(), RTLD_NOW);
    if (m_vddkLibHandle == nullptr) {
        if (dlerror() != nullptr) {
            COMMLOG(OS_LOG_ERROR, "Unable to open dll '%s', error msg: '%s'!", VIX_DISK_LIBRARY_NAME.c_str(),
                    dlerror());
        } else {
            COMMLOG(OS_LOG_ERROR, "Unable to open dll!");
        }
        return false;
    }
    return true;
}

bool VMwareDiskLib::InitVddkLibEnv()
{
    VddkDeadlockCheck::DeadlockCheck DeadlockCheck("InitVddkLibEnv");
    std::string configPath;
    std::string libPath;
    GetVddkLibPath(libPath, configPath);

    COMMLOG(OS_LOG_INFO,
        "The VDDK path is '%s', and the VDDK config file path is '%s'",
        libPath.c_str(),
        configPath.c_str());

    const char *pvddkDir = libPath.empty() ? nullptr : libPath.c_str();
    const char *pvddkCfg = configPath.empty() ? "/dev/null" : configPath.c_str();

    VixError vixError = m_vddkOperations.vixDiskLibInitEx(
        m_majorVersion, m_minorVersion, &LogFunc, &WarnFunc, &PanicFunc, pvddkDir, pvddkCfg);

    COMMLOG(OS_LOG_DEBUG, "m_vddkOperations.vixDiskLibInitEx invoke successfully!");

    if (vixError != VIX_OK) {
        char *ErrorText = m_vddkOperations.vixDiskLibGetErrorText(vixError, nullptr);
        COMMLOG(OS_LOG_ERROR, "VDDK init error, error code '%d', error desc '%s'.", vixError, ErrorText);

        if (ErrorText != nullptr) {
            m_vddkOperations.vixDiskLibFreeErrorText(ErrorText);
            ErrorText = nullptr;
        }
        return false;
    }

    return true;
}

void VMwareDiskLib::GetVddkLibPath(std::string &libPath, std::string &configPath)
{
    libPath.clear();
    configPath.clear();

    // if the vddk lib path has alreadly been passed by parameter, no need to parse it
    if (!m_strLibPath.empty()) {
        libPath = m_strLibPath;
    } else {
        std::ifstream fin("/proc/self/maps");
        std::string line;
        while (std::getline(fin, line)) {
            std::string::size_type keyWordPos = line.find("/lib64/libvixDisk");
            if (std::string::npos == keyWordPos) {
                continue;
            }
            std::string::size_type blankPos = line.find_last_of(" ");
            if (std::string::npos == blankPos) {
                break;
            }
            line = TrimChars(line.substr(blankPos));
            keyWordPos = line.find("/lib64/libvixDisk");
            if (std::string::npos == keyWordPos) {
                break;
            }
            libPath = line.substr(0, keyWordPos);
            COMMLOG(OS_LOG_DEBUG, "VMwareDiskLib::GetVddkLibPath, libPath is: '%s'", libPath.c_str());
            break;
        }

        std::string xenPath = "/proc/xen";
        if (CMpFile::DirExist(xenPath.c_str())) {
            COMMLOG(OS_LOG_DEBUG, "It's a xen vm, can not support advanced transport, lib path is '%s'", libPath);
            libPath = "";
        }
    }

    configPath = CPath::GetInstance().GetConfPath() + "/" + VDDK_CONFI_FILE_NAME.c_str();
    if (!CMpFile::FileExist(configPath.c_str())) {
        COMMLOG(OS_LOG_ERROR, "The config file '%s' not exist.", configPath.c_str());
        configPath.clear();
    } else {
        COMMLOG(OS_LOG_DEBUG, "The config file location: '%s'.", configPath.c_str());
    }
}

mp_string VMwareDiskLib::TrimChars(const std::string &str)
{
    std::string result = str;
    std::string::size_type count = 0;
    for (std::string::iterator it = result.begin(); it != result.end(); ++it) {
        if (*it == '\n' || *it == '\r' || *it == ' ') {
            count++;
        }
    }
    result = result.substr(count);

    for (std::string::reverse_iterator it = result.rbegin(); it != result.rend(); ++it) {
        if (*it == '\n' || *it == '\r' || *it == ' ') {
            *(it) = '\0';
        } else {
            break;
        }
    }
    return result;
}

void VMwareDiskLib::LogFunc(const char *fmt, va_list args)
{
    char str_buffer[1024] = {0x0};
    if (sizeof(str_buffer) <= strlen(fmt)) {
        return;
    }

    int err = vsnprintf_s(str_buffer, sizeof(str_buffer), sizeof(str_buffer), fmt, args);
    if (err < 0) {
        COMMLOG(OS_LOG_ERROR, "vsnprintf_s errpr");
        return;
    }

    std::string tmp = str_buffer;
    COMMLOG(OS_LOG_INFO, "VDDK initialization, detail: %s", tmp.c_str());
}

void VMwareDiskLib::WarnFunc(const char *fmt, va_list args)
{
    char str_buffer[1024] = {0x0};
    if (sizeof(str_buffer) <= strlen(fmt)) {
        return;
    }
    int err = vsnprintf_s(str_buffer, sizeof(str_buffer), sizeof(str_buffer), fmt, args);
    if (err < 0) {
        COMMLOG(OS_LOG_ERROR, "vsnprintf_s errpr");
        return;
    }

    std::string tmp = str_buffer;
    COMMLOG(OS_LOG_INFO, "VDDK initialization, warnning: %s", tmp.c_str());
}

void VMwareDiskLib::PanicFunc(const char *fmt, va_list args)
{
    char str_buffer[1024] = {0x0};
    if (sizeof(str_buffer) <= strlen(fmt)) {
        return;
    }

    errno_t err = EOK;
    err = vsnprintf_s(str_buffer, sizeof(str_buffer), sizeof(str_buffer), fmt, args);
    if (err != EOK) {
        COMMLOG(OS_LOG_ERROR, "vsnprintf_s errpr");
        return;
    }

    std::string tmp = str_buffer;
    COMMLOG(OS_LOG_INFO, "VDDK initialization, critical error: %s", tmp.c_str());
}

VMwareDiskLib::~VMwareDiskLib()
{
    (void)m_vddkThread.Stop();
    if (m_isInitialized) {
        m_vddkOperations.vixDiskLibExit();
    }
}

// invoke this function after VixDiskLib_Disconnect to remove extra state for each virtual machine
VMWARE_DISK_RET_CODE VMwareDiskLib::Cleanup(const VddkConnectParams &connectParams, std::string &errDesc)
{
    VddkDeadlockCheck::DeadlockCheck DeadlockCheck("Cleanup");
    errDesc = "Initialize vddk failed.";
    if (!Init()) {
        COMMLOG(OS_LOG_ERROR, "Initialize vddk failed.");
        return VIX_E_FAIL;
    }
    uint32_t numCleanedUp = 0;
    uint32_t numRemaining = 0;
    VixDiskLibConnectParams params;
    params.serverName = const_cast<char *>(connectParams.serverName.c_str());
    params.port = static_cast<uint32>(connectParams.port);
    params.creds.uid.userName = const_cast<char *>(connectParams.userName.c_str());
    params.creds.uid.password = const_cast<char *>(connectParams.password.c_str());
    params.thumbPrint = const_cast<char *>(connectParams.thumbPrint.c_str());
    params.vmxSpec = nullptr;
    params.credType = VIXDISKLIB_CRED_UID;

    VixError code = m_vddkOperations.vixDiskLibCleanup(&params, &numCleanedUp, &numRemaining);
    errDesc = "Initialize vddk failed.";
    if (code != VIX_OK) {
        COMMLOG(OS_LOG_ERROR, "Failed to do VixDisk Cleanup. Reason is: '%s'", GetErrString(code).c_str());
    }
    COMMLOG(OS_LOG_DEBUG, "numCleanedUp:'%d', numRemaining: '%d'.", numCleanedUp, numRemaining);
    errDesc = GetErrString(code);
    return code;
}

VMWARE_DISK_RET_CODE VMwareDiskLib::PrepareForAccess(const VddkConnectParams &connectParams, std::string &errDesc)
{
    VddkDeadlockCheck::DeadlockCheck DeadlockCheck("PrepareForAccess");
    errDesc = "Initialize vddk failed.";
    if (!Init()) {
        COMMLOG(OS_LOG_ERROR, "Initialize vddk failed.");
        return VIX_E_FAIL;
    }
    VixDiskLibConnectParams params;
    params.serverName = const_cast<char *>(connectParams.serverName.c_str());
    params.port = static_cast<uint32>(connectParams.port);
    params.creds.uid.userName = const_cast<char *>(connectParams.userName.c_str());
    params.creds.uid.password = const_cast<char *>(connectParams.password.c_str());
    params.thumbPrint = const_cast<char *>(connectParams.thumbPrint.c_str());
    params.vmxSpec = const_cast<char *>(connectParams.vmSpec.c_str());
    params.credType = VIXDISKLIB_CRED_UID;

    VixError code = m_vddkOperations.vixDiskLibPrepareForAccess(&params, APPLICATION_NAME.c_str());
    errDesc = GetErrString(code);
    return code;
}

VMWARE_DISK_RET_CODE VMwareDiskLib::EndAccess(const VddkConnectParams &connectParams, std::string &errDesc)
{
    VddkDeadlockCheck::DeadlockCheck DeadlockCheck("EndAccess");
    errDesc = "Initialize vddk failed.";
    if (!Init()) {
        COMMLOG(OS_LOG_ERROR, "Initialize vddk failed.");
        return VIX_E_FAIL;
    }

    // notify end access to remote product env
    VixDiskLibConnectParams params = {0};
    params.serverName = const_cast<char *>(connectParams.serverName.c_str());
    params.port = static_cast<mp_uint64>(connectParams.port);
    params.creds.uid.userName = const_cast<char *>(connectParams.userName.c_str());
    params.creds.uid.password = const_cast<char *>(connectParams.password.c_str());
    params.thumbPrint = const_cast<char *>(connectParams.thumbPrint.c_str());
    params.vmxSpec = const_cast<char *>(connectParams.vmSpec.c_str());
    params.credType = VIXDISKLIB_CRED_UID;

    VixError code = m_vddkOperations.vixDiskLibEndAccess(&params, APPLICATION_NAME.c_str());
    errDesc = GetErrString(code);
    return code;
}

std::string VMwareDiskLib::ListTransportModes()
{
    VddkDeadlockCheck::DeadlockCheck DeadlockCheck("ListTransportModes");
    std::string str;
    if (Init()) {
        const char *transportModes = m_vddkOperations.vixDiskLibListTransportModes();
        if (transportModes != nullptr) {
            str = transportModes;
        }
    }
    return str;
}

std::string VMwareDiskLib::GetErrString(const VMWARE_DISK_RET_CODE code)
{
    std::string errString = "Unknown error.";
    if (!Init()) {
        COMMLOG(OS_LOG_ERROR, "Initialize vddk failed.");
        return errString;
    }

    char *errorText = m_vddkOperations.vixDiskLibGetErrorText(code, nullptr);

    if (errorText != nullptr) {
        errString = errorText;
        m_vddkOperations.vixDiskLibFreeErrorText(errorText);
    }
    return errString;
}
VMwareDiskLib *VMwareDiskLib::GetInstance()
{
    static VMwareDiskLib *vmwareDiskLib = nullptr;
    if (vmwareDiskLib != nullptr) {
        return vmwareDiskLib;
    }
    std::unique_lock<std::mutex> lock(m_mutex);
    if (vmwareDiskLib == nullptr) {
        vmwareDiskLib = new (std::nothrow) VMwareDiskLib;
        if (vmwareDiskLib == nullptr) {
            COMMLOG(OS_LOG_ERROR, "new  VMwareDiskLib object fail.");
        }
    }
    return vmwareDiskLib;
}

void VMwareDiskLib::DestroyInstance()
{
    VMwareDiskLib *vmwareDiskLib = GetInstance();
    if (vmwareDiskLib != nullptr) {
        delete vmwareDiskLib;
    }
}

std::shared_ptr<VMwareDiskApi> VMwareDiskLib::GetVMwareDiskApiInstance(const VddkConnectParams &params)
{
    if (!Init()) {
        COMMLOG(OS_LOG_ERROR, "Initialize vddk failed.");
        return nullptr;
    }

    // find the connection with remote product env that alreadly exists
    auto iter = m_connMap.find(params.vmMoRef);
    if (iter != m_connMap.end()) {
        return std::unique_ptr<VMwareDiskApi>(std::make_unique<VMwareDiskApi>(
            m_vddkOperations, m_connMap[params.vmMoRef], m_vddkThread.GetMessageloop()));
    }

    // create connection with remote product env
    VixDiskLibConnection connection;
    VMWARE_DISK_RET_CODE code = Connect(params, connection);
    if (code != VIX_OK) {
        COMMLOG(OS_LOG_ERROR, "Connect failed: '%s'.", GetErrString(code).c_str());
        return nullptr;
    }
    m_connMap.insert(std::pair<std::string, VixDiskLibConnection>(params.vmMoRef, connection));
    COMMLOG(OS_LOG_INFO, "Connect to vCenter/ESXI server '%s' completed.", params.serverName.c_str());

    return std::unique_ptr<VMwareDiskApi>(
        std::make_unique<VMwareDiskApi>(m_vddkOperations, connection, m_vddkThread.GetMessageloop()));
}

VMWARE_DISK_RET_CODE VMwareDiskLib::BuildConnectParams(
    const std::string &vmRef, const vmware_pm_info &pmInfo, VddkConnectParams &connectParams)
{
    if (vmRef.empty() || pmInfo.strIP.empty()) {
        return MP_FAILED;
    }

    std::string tempIp = CIP::FormatFullUrl(pmInfo.strIP);
    connectParams.vmSpec = "moref=" + vmRef;
    connectParams.serverName = std::move(tempIp);
    connectParams.port = pmInfo.ulPort;
    connectParams.userName = pmInfo.strUserName;
    connectParams.password = pmInfo.strPassword;
    connectParams.thumbPrint = pmInfo.strThumbPrint;
    connectParams.vmMoRef = vmRef;
    return MP_SUCCESS;
}

std::string VMwareDiskLib::GetVddkLibPathInner()
{
    return m_strLibPath;
}
void VMwareDiskLib::SetVddkLibPathAndVersion(const mp_string &path, uint32_t majorVersion, uint32_t minorVersion,
    const mp_string &version)
{
    m_strLibPath = path;
    std::stringstream ss(version);
    ss >> m_version;
    m_majorVersion = majorVersion;
    m_minorVersion = minorVersion;
}

// Get the vCenter or ESXI connection params
void VMwareDiskLib::GetVixConnectParams(const VddkConnectParams &connectParams, VixDiskLibConnectParams &params)
{
    params.serverName = const_cast<char *>(connectParams.serverName.c_str());
    params.port = static_cast<uint32>(connectParams.port);
    params.creds.uid.userName = const_cast<char *>(connectParams.userName.c_str());
    params.creds.uid.password = const_cast<char *>(connectParams.password.c_str());
    params.thumbPrint = const_cast<char *>(connectParams.thumbPrint.c_str());
    params.vmxSpec = const_cast<char *>(connectParams.vmSpec.c_str());
    params.credType = VIXDISKLIB_CRED_UID;
}

// Connect to vCenter/ESXI
VMWARE_DISK_RET_CODE VMwareDiskLib::Connect(const VddkConnectParams &connectParams, VixDiskLibConnection &connection)
{
    VixDiskLibConnectParams params = {0};
    GetVixConnectParams(connectParams, params);
    // specific nbdssl mode to do disk recovery once SAN not supported
    mp_string strTransMode = "";
    if (!connectParams.transportMode.empty()) {
        strTransMode = connectParams.transportMode;
        COMMLOG(OS_LOG_INFO, "Connect with transport mode idx: %s", strTransMode.c_str());
    } else if (connectParams.hostagentSystemVirt == VIRTUAL_MACHINE) {
        strTransMode = "hotadd:nbdssl";
        COMMLOG(OS_LOG_INFO, "Curr host is virtual machine, connect with %s", strTransMode.c_str());
    } else if ((connectParams.protectType == VMWARE_VM_RECOVERY) && !connectParams.bSupportSAN) {
        COMMLOG(OS_LOG_INFO,
            "None thick provision eager zeroed type disk's recovery is not supported by SAN transport mode, \
            will use hotadd/nbdssl mode.");
        if (connectParams.hostagentSystemVirt == PHYSICAL_MACHINE) {
            strTransMode = "nbdssl";
            COMMLOG(OS_LOG_INFO, "Curr host is physical machine, connect with nbdssl.");
        } else {
            COMMLOG(OS_LOG_ERROR, "Invalid hostagent system virt value, please check!");
            return VIX_E_FAIL;
        }
    }

    VMWARE_DISK_RET_CODE rc;
    if (!strTransMode.empty()) {
        rc = m_vddkOperations.vixDiskLibConnectEx(
            &params, connectParams.openMode, connectParams.vmSnapshotRef.c_str(), strTransMode.c_str(), &connection);
    } else {
        rc = m_vddkOperations.vixDiskLibConnectEx(
            &params, connectParams.openMode, connectParams.vmSnapshotRef.c_str(), nullptr, &connection);
    }
    
    return rc;
}

VMWARE_DISK_RET_CODE VMwareDiskLib::DisConnect(const VixDiskLibConnection& connection)
{
    if (connection == nullptr) {
        COMMLOG(OS_LOG_ERROR, "The connection has been disconnected!");
        return VIX_OK;
    }
    VixError vixError = m_vddkOperations.vixDiskLibDisconnect(connection);
    COMMLOG(OS_LOG_INFO, "Disconnect with remote vCenter or ESXI completed!");
    return vixError;
}
