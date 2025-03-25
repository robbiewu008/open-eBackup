#include "device/BackupMedium.h"

#include <vector>
#include "common/ErrorCode.h"
#include "common/JsonUtils.h"
#include "common/Log.h"
#include "securecom/RootCaller.h"
#include "common/CSystemExec.h"
#include "message/tcp/CDppMessage.h"
#include "taskmanager/TaskContext.h"
#include "common/Utils.h"
#include "securecom/SecureUtils.h"

using std::vector;
namespace {
const mp_string SCRIPT_PREPARE_FSBK_MEDIA = "preparebkfsmedia.bat";
const mp_string WIN_ORACLE_NATIVE_PREPAREASM = "oracleprepareasmmedia.bat";
}

BackupMedium::BackupMedium()
{
    createMediaFlag = "0";
}

BackupMedium::~BackupMedium()
{
    ClearString(asmPwd);
}

mp_void BackupMedium::InitFs(
    const mp_string& vgName, const mp_string& lvName, const mp_string& mountPath, const mp_string& fsType)
{
    this->vgName = vgName;
    this->lvName = lvName;
    this->mountPath = mountPath;
    this->fsType = fsType;
}

mp_void BackupMedium::InitFsType(const mp_string& fsType)
{
    this->fsType = fsType;
}

mp_void BackupMedium::InitExtrasParams(mp_int32 hostRole)
{
    this->hostRole = hostRole;
}

mp_void BackupMedium::InitAsm(const mp_string& dgName, const mp_string& asmInstance, const mp_string& asmUser,
    const mp_string& asmPwd, mp_int32 initMetaFs)
{
    this->dgName = dgName;
    this->asmInstance = asmInstance;
    this->asmUser = asmUser;
    this->asmPwd = asmPwd;
    this->initMetaDataFlag = initMetaFs;
}

mp_void BackupMedium::InitMetaPath(const mp_string& metaPath)
{
    this->metaPath = metaPath;
}

mp_void BackupMedium::SetCreateMediumWhenNoExist()
{
    // only backup data and log, need to create medium when medium is not exists.
    createMediaFlag = "1";
}

mp_int32 BackupMedium::CreateFsMedium(const mp_string& diskList)
{
    LOGGUARD("");
    mp_bool bFlag = mountPath.empty() || fsType.empty();
    if (bFlag) {
        COMMLOG(OS_LOG_ERROR,
            "BackupMedium need to initial parameter, mountPath:%s, fsType:%s.",
            mountPath.c_str(),
            fsType.c_str());
        return MP_FAILED;
    }

#ifndef WIN32
    bFlag = vgName.empty() || lvName.empty();
    if (bFlag) {
        COMMLOG(OS_LOG_ERROR,
            "BackupMedium need to initial parameter, vgName:%s, lvName:%s.",
            vgName.c_str(),
            lvName.c_str());
        return MP_FAILED;
    }
#endif

    if (fsType.compare("ASM") == 0) {
        COMMLOG(OS_LOG_ERROR, "create backup medium faile, filetype ASM.");
        return MP_FAILED;
    }

    mp_string scriptParams = BuildCreateFSParam(diskList);
#ifdef WIN32
    mp_int32 iRet = SecureCom::SysExecScript(SCRIPT_PREPARE_FSBK_MEDIA, scriptParams, NULL);
    ClearString(scriptParams);
    if (iRet != MP_SUCCESS) {
        mp_int32 iNewRet = ErrorCode::GetInstance().GetErrorCode(iRet);
        COMMLOG(OS_LOG_ERROR,
            "create backup fs media failed, initial return code is %d, tranformed return code is %d",
            iRet,
            iNewRet);
        return iNewRet;
    }
#else
    CRootCaller rootCaller;
    mp_int32 iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_BACKUP_PREPAREFS, scriptParams, NULL);
    ClearString(scriptParams);
    if (iRet != MP_SUCCESS) {
        iRet = ErrorCode::GetInstance().GetErrorCode(iRet);
        TRANSFORM_RETURN_CODE(iRet, ERROR_AGENT_PREPARE_FSMEDIA_FAILED);
        COMMLOG(OS_LOG_ERROR, "create backup fs media failed, iRet %d.", iRet);
        return iRet;
    }
#endif

    COMMLOG(OS_LOG_INFO, "file system backupPath is %s.", mountPath.c_str());
    return MP_SUCCESS;
}

mp_int32 BackupMedium::CreateAsmMedium(const mp_string& diskList)
{
    if (dgName.empty()) {
        COMMLOG(OS_LOG_ERROR, "dgName is empty.");
        return MP_FAILED;
    }

    mp_string scriptParams = BuildCreateASMParam(diskList);
    if (scriptParams.empty()) {
        COMMLOG(OS_LOG_ERROR, "CreateAsmMedium create asm script parameter failed.");
        return MP_FAILED;
    }
    ClearString(asmPwd);

#ifdef WIN32
    vector<mp_string> vecRst;
    mp_int32 iRet = SecureCom::SysExecScript(WIN_ORACLE_NATIVE_PREPAREASM, scriptParams, &vecRst);
    ClearString(scriptParams);
    if (iRet != MP_SUCCESS) {
        mp_int32 iNewRet = ErrorCode::GetInstance().GetErrorCode(iRet);
        COMMLOG(OS_LOG_ERROR, "stop db failed, initial return code is %d, tranformed return code is %d", iRet, iNewRet);
        return iNewRet;
    }
#else
    COMMLOG(OS_LOG_ERROR, "There is no sence to create backup ASM media.");
    return MP_FAILED;
#endif

    return MP_SUCCESS;
}

mp_string BackupMedium::GetFsMountPath()
{
    return mountPath;
}

mp_string BackupMedium::GetAsmMountPath()
{
    return ("+" + dgName);
}

mp_string BackupMedium::GetMetaPath()
{
    return metaPath;
}

mp_string BackupMedium::BuildCreateFSParam(const mp_string& diskList)
{
    static const mp_string keyVgName = "VGNAME=";
    static const mp_string keyLvName = "LVNAME=";
    static const mp_string keyMountPath = "MOUNTPATH=";
    static const mp_string keyDiskList = "DISKLIST=";
    static const mp_string keyFSType = "FSTYPE=";
    static const mp_string keyMediaType = "CREATEMEDIA=";
    static const mp_string keyHostRole = "HOSTROLE=";

    std::ostringstream oss;
    oss << keyVgName << vgName << NODE_COLON << keyLvName << lvName << NODE_COLON << keyMountPath << mountPath
        << NODE_COLON << keyDiskList << diskList << NODE_COLON << keyFSType << fsType << NODE_COLON << keyMediaType
        << createMediaFlag << NODE_COLON << keyHostRole << hostRole;
    return oss.str();
}

mp_string BackupMedium::BuildCreateASMParam(const mp_string& diskList)
{
    static const mp_string keyDgName = "DGNAME=";
    static const mp_string keyAsmInstance = "ASMInstanceName=";
    static const mp_string keyAsmUser = "ASMUserName=";
    static const mp_string keyAsmPwd = "ASMPassword=";
    static const mp_string keyDiskList = "DISKLIST=";
    static const mp_string keyMediaType = "CREATEMEDIA=";
    static const mp_string keyMetaDataPath = "METADATAPATH=";
    static const mp_string keyMetaDataFsType = "METADATAFSTYPE=";
    static const mp_string keyHostRole = "HOSTROLE=";

    std::ostringstream oss;
    oss << keyDgName << dgName << NODE_COLON << keyAsmInstance << asmInstance << NODE_COLON << keyAsmUser << asmUser
        << NODE_COLON << keyAsmPwd << asmPwd << NODE_COLON << keyDiskList << diskList << NODE_COLON << keyMediaType
        << createMediaFlag << NODE_COLON << keyHostRole << hostRole << NODE_COLON;

    if (initMetaDataFlag == 1) {
        mp_bool bFlag = metaPath.empty() || fsType.empty();
        if (bFlag) {
            COMMLOG(OS_LOG_ERROR, "BuildCreateASMParam need to initial metadata file system parameter.");
            return "";
        }
        oss << keyMetaDataPath << metaPath << NODE_COLON << keyMetaDataFsType << fsType;
    }

    ClearString(asmPwd);
    return oss.str();
}
